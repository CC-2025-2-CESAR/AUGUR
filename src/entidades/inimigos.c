/* ============================================================================
 * inimigos.c - ENGINE DE INIMIGOS
 * ----------------------------------------------------------------------------
 * Lista encadeada de inimigos. Não conhece tipos específicos — orquestra o
 * frame em passes: (1) status + IA + movimento, (1.5) disparo de projétil,
 * (2) push-out O(n²) entre inimigos, (3) animação de morte + free. Os stats e
 * a IA vêm das tabelas da Luísa (inimigos_tipos.c, projeteis_inimigo_tipos.c).
 * ============================================================================ */

#include "inimigos.h"
#include "inimigos_tipos.h"
#include "projeteis_inimigo.h"
#include "projeteis_inimigo_tipos.h"
#include "profecia.h"
#include "assets.h"   /* g_assets, META_INIMIGO, desenhar_sheet */
#include <math.h>
#include <stdlib.h>


/* Conta nós da lista — usado pra respeitar MAX_INIMIGOS no spawn. */
static int contar_nos(const InimigoNo *cabeca) {
    int n = 0;
    for (const InimigoNo *p = cabeca; p != NULL; p = p->proximo) n++;
    return n;
}


/* Cria um inimigo do tipo dado na posição dada (insere na cabeça, O(1)). */
void inimigos_spawnar_em(EstadoJogo *ej, Vector2 posicao, TipoInimigo tipo) {
    if ((int)tipo < 0 || (int)tipo >= QTD_PARAMETROS_INIMIGO) return;
    if (contar_nos(ej->inimigos_cabeca) >= MAX_INIMIGOS) return;

    InimigoNo *novo = (InimigoNo *)malloc(sizeof(InimigoNo));
    if (novo == NULL) return;

    const ParametrosInimigo *p = &PARAMETROS_INIMIGO[tipo];

    novo->dados.posicao              = posicao;
    novo->dados.velocidade           = (Vector2){ 0.0f, 0.0f };
    novo->dados.raio                 = p->raio;
    novo->dados.vida                 = p->vida_base;
    novo->dados.vida_maxima          = p->vida_base;
    novo->dados.dano                 = p->dano;
    novo->dados.velocidade_movimento = p->velocidade_movimento;
    novo->dados.tipo                 = tipo;
    novo->dados.recompensa_biomassa  = p->recompensa_biomassa;
    novo->dados.vivo                 = true;

    /* Spawn atribui campo a campo (não usa {0}), então todo campo precisa ser
     * zerado explicitamente aqui. */
    novo->dados.congelado_tempo           = 0.0f;
    novo->dados.veneno_tempo              = 0.0f;
    novo->dados.veneno_dps                = 0.0f;
    novo->dados.veneno_stacks             = 0;
    novo->dados.veneno_acumulado          = 0.0f;
    novo->dados.marca_termica_tempo       = 0.0f;
    novo->dados.proxima_hit_multiplicador = 1.0f;
    novo->dados.timer_disparo             = 0.0f;

    novo->dados.direcao_atual    = DIR_DOWN;
    novo->dados.animacao_atual   = ANIM_IDLE;
    novo->dados.animacao_tempo   = 0.0f;
    novo->dados.morrendo_tempo   = 0.0f;

    novo->proximo            = ej->inimigos_cabeca;
    ej->inimigos_cabeca      = novo;
}


/* Caminho único de morte. Idempotente (não credita biomassa duas vezes se DoT
 * e projétil matam no mesmo frame). Inicia a animação DEATH; o free real fica
 * no PASS 3 de inimigos_atualizar. Alimenta o motor de profecia (Ao matar). */
void inimigos_registrar_morte(EstadoJogo *ej, Inimigo *i) {
    if (i == NULL || !i->vivo) return;
    i->vivo = false;
    i->morrendo_tempo  = 0.6f;   /* anim DEATH rola por este tempo antes do free */
    i->animacao_atual  = ANIM_DEATH;
    i->animacao_tempo  = 0.0f;
    i->velocidade      = (Vector2){ 0.0f, 0.0f };

    ej->jogador.biomassa += i->recompensa_biomassa;
    profecia_evento_ao_matar(ej, i);
}


void inimigos_atualizar(EstadoJogo *ej) {
    float dt = ej->delta_tempo;

    /* ----- PASS 1: status + IA + movimento ----- */
    for (InimigoNo *ino = ej->inimigos_cabeca; ino != NULL; ino = ino->proximo) {
        if (!ino->dados.vivo) continue;
        Inimigo *d = &ino->dados;

        /* Timers de status expiram. */
        if (d->congelado_tempo > 0.0f) {
            d->congelado_tempo -= dt;
            if (d->congelado_tempo < 0.0f) d->congelado_tempo = 0.0f;
        }
        if (d->marca_termica_tempo > 0.0f) {
            d->marca_termica_tempo -= dt;
            if (d->marca_termica_tempo < 0.0f) d->marca_termica_tempo = 0.0f;
        }

        /* DoT do veneno: acumulador float evita truncar pra 0 com o int vida. */
        if (d->veneno_tempo > 0.0f) {
            d->veneno_tempo -= dt;
            d->veneno_acumulado += d->veneno_dps * dt;
            int aplicar = (int)d->veneno_acumulado;
            if (aplicar > 0) {
                d->vida -= aplicar;
                d->veneno_acumulado -= (float)aplicar;
            }
            if (d->veneno_tempo <= 0.0f) {
                d->veneno_tempo = 0.0f;
                d->veneno_dps = 0.0f;
                d->veneno_stacks = 0;
                d->veneno_acumulado = 0.0f;
            }
            if (d->vida <= 0) {
                inimigos_registrar_morte(ej, d);   /* DoT credita biomassa */
                continue;
            }
        }

        inimigos_tipos_executar_ia(d, ej);

        /* Congelado: a IA já escreveu velocidade; zeramos antes de integrar. */
        if (d->congelado_tempo > 0.0f) {
            d->velocidade.x = 0.0f;
            d->velocidade.y = 0.0f;
        }

        d->posicao.x += d->velocidade.x * dt;
        d->posicao.y += d->velocidade.y * dt;

        /* Direção do sprite olha pro jogador (eixo dominante). */
        float dxp = ej->jogador.posicao.x - d->posicao.x;
        float dyp = ej->jogador.posicao.y - d->posicao.y;
        if (fabsf(dxp) > fabsf(dyp))
            d->direcao_atual = (dxp < 0.0f) ? DIR_LEFT : DIR_RIGHT;
        else
            d->direcao_atual = (dyp < 0.0f) ? DIR_UP   : DIR_DOWN;

        /* Animação: walk se movendo, idle se parado. Reseta o tempo na transição
         * pra cada animação começar do frame 0. */
        float vel2 = d->velocidade.x * d->velocidade.x +
                     d->velocidade.y * d->velocidade.y;
        int nova_anim = (vel2 > 1.0f) ? ANIM_WALK : ANIM_IDLE;
        if (nova_anim != d->animacao_atual) {
            d->animacao_atual = nova_anim;
            d->animacao_tempo = 0.0f;
        } else {
            d->animacao_tempo += dt;
        }
    }

    /* ----- PASS 1.5: disparo de projétil -----
     * Mecânica 100% engine; o QUE/QUANTO vem da tabela da Luísa. Inimigo
     * congelado não atira; só dispara com o jogador dentro do alcance. */
    for (InimigoNo *ino = ej->inimigos_cabeca; ino != NULL; ino = ino->proximo) {
        Inimigo *d = &ino->dados;
        if (!d->vivo || d->congelado_tempo > 0.0f) continue;
        if ((int)d->tipo < 0 ||
            (int)d->tipo >= QTD_PARAMETROS_PROJETIL_INIMIGO) continue;

        const ParametrosProjetilInimigo *pp =
            &PARAMETROS_PROJETIL_INIMIGO[d->tipo];
        if (!pp->pode_atirar) continue;

        float dx = ej->jogador.posicao.x - d->posicao.x;
        float dy = ej->jogador.posicao.y - d->posicao.y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist > pp->alcance_disparo || dist < 0.0001f) continue;

        d->timer_disparo -= dt;
        if (d->timer_disparo <= 0.0f) {
            Vector2 dir = { dx / dist, dy / dist };
            projeteis_inimigo_spawnar(ej, d->posicao, dir, d->tipo);
            d->timer_disparo = pp->cooldown_disparo;
        }
    }

    /* ----- PASS 2: push-out inimigo↔inimigo (O(n²)) -----
     * Pra cada par sobreposto, empurra cada um por metade do overlap. Caso
     * degenerado (centros coincidentes): separa "a" minimamente. */
    for (InimigoNo *a = ej->inimigos_cabeca; a != NULL; a = a->proximo) {
        if (!a->dados.vivo) continue;
        for (InimigoNo *b = a->proximo; b != NULL; b = b->proximo) {
            if (!b->dados.vivo) continue;

            float dx = b->dados.posicao.x - a->dados.posicao.x;
            float dy = b->dados.posicao.y - a->dados.posicao.y;
            float dist2 = dx * dx + dy * dy;
            float soma = a->dados.raio + b->dados.raio;

            if (dist2 < soma * soma && dist2 > 0.0001f) {
                float dist = sqrtf(dist2);
                float overlap = soma - dist;
                float empurrar_x = (dx / dist) * (overlap * 0.5f);
                float empurrar_y = (dy / dist) * (overlap * 0.5f);
                a->dados.posicao.x -= empurrar_x;
                a->dados.posicao.y -= empurrar_y;
                b->dados.posicao.x += empurrar_x;
                b->dados.posicao.y += empurrar_y;
            } else if (dist2 <= 0.0001f) {
                a->dados.posicao.x -= 1.0f;
            }
        }
    }

    /* ----- PASS 3: animação de morte + remoção (ponteiro duplo) -----
     * `**atual` aponta pro próximo ponteiro a modificar (cabeça ou o `proximo`
     * do nó anterior), removendo sem caso especial pra cabeça. Quem está com
     * morrendo_tempo > 0 fica na lista até a animação DEATH terminar. */
    InimigoNo **atual = &ej->inimigos_cabeca;
    while (*atual != NULL) {
        Inimigo *d = &(*atual)->dados;
        if (!d->vivo) {
            if (d->morrendo_tempo > 0.0f) {
                d->morrendo_tempo -= dt;
                d->animacao_tempo += dt;
                atual = &(*atual)->proximo;
            } else {
                InimigoNo *morto = *atual;
                *atual = morto->proximo;
                free(morto);
            }
        } else {
            atual = &(*atual)->proximo;
        }
    }
}


void inimigos_desenhar(const EstadoJogo *ej) {
    for (const InimigoNo *ino = ej->inimigos_cabeca;
         ino != NULL;
         ino = ino->proximo) {
        const Inimigo *i = &ino->dados;
        /* Continua desenhando quem está morrendo (anim DEATH). */
        if (!i->vivo && i->morrendo_tempo <= 0.0f) continue;
        if ((int)i->tipo < 0 || (int)i->tipo >= QTD_PARAMETROS_INIMIGO) continue;

        Texture2D tex = g_assets.inimigos[i->tipo];
        if (tex.id == 0) {
            /* Fallback (sprite ausente): círculo com alpha proporcional ao HP. */
            const ParametrosInimigo *p = &PARAMETROS_INIMIGO[i->tipo];
            Color cor = p->cor;
            float vida_ratio = (i->vida_maxima > 0)
                ? (float)i->vida / (float)i->vida_maxima : 1.0f;
            if (vida_ratio < 0.0f) vida_ratio = 0.0f;
            cor.a = (unsigned char)(80 + 175 * vida_ratio);
            DrawCircleV(i->posicao, p->raio_visual, cor);
            DrawCircleLines((int)i->posicao.x, (int)i->posicao.y,
                            p->raio_visual, (Color){ 0, 0, 0, 150 });
            continue;
        }

        /* Sprite: escala pra o frame ocupar ~raio*2*SPRITE_VISUAL_SCALE de
         * diâmetro em mundo (desacopla visual de hitbox, ver assets.h). */
        float escala = (i->raio * 2.0f * SPRITE_VISUAL_SCALE) /
                       (float)META_INIMIGO[i->tipo].frame_w;
        desenhar_sheet(tex, &META_INIMIGO[i->tipo], i->posicao,
                       i->direcao_atual, i->animacao_atual,
                       i->animacao_tempo, escala, WHITE);
    }
}


void inimigos_liberar_tudo(EstadoJogo *ej) {
    InimigoNo *atual = ej->inimigos_cabeca;
    while (atual != NULL) {
        InimigoNo *prox = atual->proximo;
        free(atual);
        atual = prox;
    }
    ej->inimigos_cabeca = NULL;
}
