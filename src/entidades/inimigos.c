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
#include "jogador.h"   /* jogador_sofrer_dano (golpe corpo a corpo) */
#include "assets.h"   /* g_assets, META_INIMIGO, desenhar_sheet */
#include <math.h>
#include <stdlib.h>

/* Lunge do golpe corpo a corpo: ao bater, o inimigo avança um passo curto na
 * direção do jogador (deixa o ataque mais "grudento"). */
#define LUNGE_DURACAO 0.12f   /* s de avanço */
#define LUNGE_MULT    3.0f    /* velocidade do lunge = velocidade_movimento * isto */


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
    novo->dados.dano_flash_tempo = 0.0f;
    novo->dados.ataque_telegrafo_restante = 0.0f;
    novo->dados.ataque_cooldown_restante  = 0.0f;
    novo->dados.ataque_lunge_restante     = 0.0f;
    novo->dados.ataque_lunge_vel          = (Vector2){ 0.0f, 0.0f };

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


/* Ataque corpo a corpo telegrafado. Inimigos com alcance_ataque>0 param ao
 * chegar perto, "carregam" o golpe por telegrafo_ataque segundos (janela pra
 * esquivar) e batem UMA vez se o jogador ainda estiver no alcance; depois
 * entram em cooldown. Retorna true enquanto carrega (o caller para o inimigo e
 * trava a animação em CAST). */
static bool inimigo_processar_ataque(EstadoJogo *ej, Inimigo *d, float dt) {
    if ((int)d->tipo < 0 || (int)d->tipo >= QTD_PARAMETROS_INIMIGO) return false;
    const ParametrosInimigo *p = &PARAMETROS_INIMIGO[d->tipo];
    if (p->alcance_ataque <= 0.0f) return false;   /* não é atacante corpo a corpo */

    if (d->ataque_cooldown_restante > 0.0f) {
        d->ataque_cooldown_restante -= dt;
        if (d->ataque_cooldown_restante < 0.0f) d->ataque_cooldown_restante = 0.0f;
    }

    float dx = ej->jogador.posicao.x - d->posicao.x;
    float dy = ej->jogador.posicao.y - d->posicao.y;
    float dist = sqrtf(dx * dx + dy * dy);

    /* Carregando: conta o windup; ao zerar, dispara o golpe + o lunge. */
    if (d->ataque_telegrafo_restante > 0.0f) {
        d->ataque_telegrafo_restante -= dt;
        if (d->ataque_telegrafo_restante <= 0.0f) {
            d->ataque_telegrafo_restante = 0.0f;

            /* Direção travada AGORA — durante o lunge o inimigo não corrige a
             * mira, então dá pra esquivar saindo de lado. */
            float dirx = (dist > 0.0001f) ? dx / dist : 0.0f;
            float diry = (dist > 0.0001f) ? dy / dist : 0.0f;
            float lunge_dist = p->velocidade_movimento * LUNGE_MULT * LUNGE_DURACAO;

            d->ataque_lunge_restante = LUNGE_DURACAO;
            d->ataque_lunge_vel.x = dirx * p->velocidade_movimento * LUNGE_MULT;
            d->ataque_lunge_vel.y = diry * p->velocidade_movimento * LUNGE_MULT;

            /* Conecta se o avanço terminaria encostando no jogador (posição
             * projetada do inimigo após o lunge perto o bastante de encostar). */
            float proj_x = d->posicao.x + dirx * lunge_dist;
            float proj_y = d->posicao.y + diry * lunge_dist;
            float pdx = ej->jogador.posicao.x - proj_x;
            float pdy = ej->jogador.posicao.y - proj_y;
            float proj_dist = sqrtf(pdx * pdx + pdy * pdy);
            if (proj_dist <= d->raio + ej->jogador.raio + 8.0f) {
                if (ej->motor_profecia.escudo_ativo > 0.0f) {
                    ej->motor_profecia.escudo_ativo = 0.0f;   /* escudo anula o golpe */
                } else {
                    jogador_sofrer_dano(&ej->jogador, (int)d->dano);
                    profecia_evento_ao_receber_dano(ej);
                }
            }
            d->ataque_cooldown_restante = p->cooldown_ataque;
            return false;   /* golpe saiu: o lunge assume o movimento */
        }
        return true;   /* ainda carregando */
    }

    /* Pronto (fora de cooldown) e jogador no alcance: começa a carregar. */
    if (d->ataque_cooldown_restante <= 0.0f && dist <= p->alcance_ataque) {
        d->ataque_telegrafo_restante = p->telegrafo_ataque;
        return true;
    }

    return false;
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
        if (d->dano_flash_tempo > 0.0f) {
            d->dano_flash_tempo -= dt;
            if (d->dano_flash_tempo < 0.0f) d->dano_flash_tempo = 0.0f;
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

        /* Ataque corpo a corpo telegrafado: para e carrega; ao bater, AVANÇA
         * (lunge) na direção travada — golpe "grudento". Congelado não ataca. */
        bool carregando_ataque = false;
        if (d->congelado_tempo <= 0.0f) {
            carregando_ataque = inimigo_processar_ataque(ej, d, dt);
            if (carregando_ataque) {
                d->velocidade.x = 0.0f;   /* windup: para no lugar */
                d->velocidade.y = 0.0f;
            } else if (d->ataque_lunge_restante > 0.0f) {
                d->ataque_lunge_restante -= dt;
                if (d->ataque_lunge_restante < 0.0f) d->ataque_lunge_restante = 0.0f;
                d->velocidade = d->ataque_lunge_vel;   /* avança no golpe */
            }
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

        /* Animação: CAST enquanto carrega o golpe; senão walk/idle pela
         * velocidade. Reseta o tempo na transição pra começar do frame 0. */
        int nova_anim;
        if (carregando_ataque) {
            nova_anim = ANIM_CAST;
        } else {
            float vel2 = d->velocidade.x * d->velocidade.x +
                         d->velocidade.y * d->velocidade.y;
            nova_anim = (vel2 > 1.0f) ? ANIM_WALK : ANIM_IDLE;
        }
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

        const ParametrosInimigo *p = &PARAMETROS_INIMIGO[i->tipo];

        /* Feedback de hit: logo após levar dano o inimigo pisca vermelho. flash
         * vai de 1 (acabou de tomar) a 0; usado pra "avermelhar" sprite/círculo. */
        float flash = 0.0f;
        if (i->vivo && i->dano_flash_tempo > 0.0f) {
            flash = i->dano_flash_tempo / DANO_FLASH_DURACAO;
            if (flash > 1.0f) flash = 1.0f;
        }

        /* Telegrafo do golpe corpo a corpo: "esquenta" pro amarelo durante o
         * windup (0 → 1 conforme o golpe se aproxima). */
        float windup = 0.0f;
        if (i->vivo && i->ataque_telegrafo_restante > 0.0f && p->telegrafo_ataque > 0.0f) {
            windup = 1.0f - (i->ataque_telegrafo_restante / p->telegrafo_ataque);
            if (windup < 0.0f) windup = 0.0f;
            if (windup > 1.0f) windup = 1.0f;
        }

        Texture2D tex = g_assets.inimigos[i->tipo];
        if (tex.id == 0) {
            /* Fallback (sprite ausente): círculo com alpha proporcional ao HP. */
            Color cor = p->cor;
            float vida_ratio = (i->vida_maxima > 0)
                ? (float)i->vida / (float)i->vida_maxima : 1.0f;
            if (vida_ratio < 0.0f) vida_ratio = 0.0f;
            cor.a = (unsigned char)(80 + 175 * vida_ratio);
            if (flash > 0.0f) {   /* puxa o círculo pro vermelho */
                cor.r = (unsigned char)(cor.r + (255 - cor.r) * flash);
                cor.g = (unsigned char)(cor.g * (1.0f - 0.75f * flash));
                cor.b = (unsigned char)(cor.b * (1.0f - 0.75f * flash));
            } else if (windup > 0.0f) {   /* carregando golpe: puxa pro amarelo */
                cor.r = (unsigned char)(cor.r + (255 - cor.r) * windup);
                cor.g = (unsigned char)(cor.g + (255 - cor.g) * windup);
            }
            DrawCircleV(i->posicao, p->raio_visual, cor);
            DrawCircleLines((int)i->posicao.x, (int)i->posicao.y,
                            p->raio_visual, (Color){ 0, 0, 0, 150 });
            continue;
        }

        /* Sprite: escala pra o frame ocupar ~raio*2*SPRITE_VISUAL_SCALE de
         * diâmetro em mundo (desacopla visual de hitbox, ver assets.h). */
        float escala = (i->raio * 2.0f * SPRITE_VISUAL_SCALE) /
                       (float)META_INIMIGO[i->tipo].frame_w;
        Color tint = WHITE;
        if (flash > 0.0f) {   /* tint multiplicativo: some G e B → vermelho */
            unsigned char gb = (unsigned char)(255.0f * (1.0f - 0.75f * flash));
            tint = (Color){ 255, gb, gb, 255 };
        } else if (windup > 0.0f) {   /* carregando: some o azul → amarelo */
            unsigned char b = (unsigned char)(255.0f * (1.0f - 0.7f * windup));
            tint = (Color){ 255, 255, b, 255 };
        }
        desenhar_sheet(tex, &META_INIMIGO[i->tipo], i->posicao,
                       i->direcao_atual, i->animacao_atual,
                       i->animacao_tempo, escala, tint);
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
