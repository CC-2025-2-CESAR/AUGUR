/* ============================================================================
 * inimigos.c - ENGINE DE INIMIGOS
 * ============================================================================
 *
 * Lista encadeada de inimigos. Esta camada NÃO conhece tipos específicos —
 * ela orquestra o frame em três passes:
 *
 *   PASS 1 (IA + movimento):
 *     Pra cada inimigo vivo, chama inimigos_tipos_executar_ia(), que escreve
 *     em i->velocidade. Em seguida aplica `posicao += velocidade * dt`.
 *
 *   PASS 2 (push-out inimigo↔inimigo):
 *     Loop O(n²): pra cada par sobrepondo, empurra metade do overlap em cada
 *     direção. Resultado: inimigos não se empilham.
 *
 *   PASS 3 (remoção):
 *     Caminha a lista com ponteiro duplo e dá free() em quem foi marcado
 *     como morto pela colisão.
 *
 * Render usa a cor de PARAMETROS_INIMIGO[tipo].cor — é só a engine olhando
 * a tabela do conteúdo.
 * ============================================================================ */

#include "inimigos.h"
#include "inimigos_tipos.h"
#include "projeteis_inimigo.h"
#include "projeteis_inimigo_tipos.h"
#include "profecia.h"
#include <math.h>
#include <stdlib.h>


/* Conta nós da lista — usado pra respeitar MAX_INIMIGOS no spawn. */
static int contar_nos(const InimigoNo *cabeca) {
    int n = 0;
    for (const InimigoNo *p = cabeca; p != NULL; p = p->proximo) n++;
    return n;
}


void inimigos_spawnar_em(EstadoJogo *ej, Vector2 posicao, TipoInimigo tipo) {
    /* Validações de segurança: tipo dentro do enum e limite de lista. */
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

    /* Campos de status: inimigos_spawnar_em atribui campo a campo (não usa
     * {0}), então TODO campo novo precisa ser zerado explicitamente aqui. */
    novo->dados.congelado_tempo           = 0.0f;
    novo->dados.veneno_tempo              = 0.0f;
    novo->dados.veneno_dps                = 0.0f;
    novo->dados.veneno_stacks             = 0;
    novo->dados.veneno_acumulado          = 0.0f;
    novo->dados.marca_termica_tempo       = 0.0f;
    novo->dados.proxima_hit_multiplicador = 1.0f;
    novo->dados.aliado                    = false;
    novo->dados.vida_aliado_restante      = 0.0f;
    novo->dados.timer_disparo             = 0.0f;

    novo->proximo            = ej->inimigos_cabeca;
    ej->inimigos_cabeca      = novo;
}


/* Caminho único de morte. Idempotente: se já está morto, não credita de
 * novo (evita biomassa dobrada quando DoT e projétil matam no mesmo frame).
 * O free real fica no PASS 3 de inimigos_atualizar. Aliados (spawnados por
 * profecia) não dão recompensa — são removidos direto, sem passar por aqui. */
void inimigos_registrar_morte(EstadoJogo *ej, Inimigo *i) {
    if (i == NULL || !i->vivo) return;
    i->vivo = false;
    ej->jogador.biomassa += i->recompensa_biomassa;
    /* Caminho único de morte alimenta o motor de profecia (combo, Ao matar).
     * O guard de reentrância em profecia_aplicar_efeito impede recursão se
     * o efeito disparado também matar (explosão em cadeia). */
    profecia_evento_ao_matar(ej, i);
}


void inimigos_spawnar_aliado(EstadoJogo *ej, Vector2 pos,
                             float vida_frac, float duracao) {
    InimigoNo *antes = ej->inimigos_cabeca;
    inimigos_spawnar_em(ej, pos, INIMIGO_CORPO_A_CORPO);
    if (ej->inimigos_cabeca == antes) return;   /* lista cheia: spawn falhou */

    Inimigo *d = &ej->inimigos_cabeca->dados;   /* spawnar_em insere na cabeça */
    d->aliado               = true;
    d->vida_aliado_restante = (duracao > 0.0f) ? duracao : 0.01f;

    int nova = (int)((float)d->vida_maxima * vida_frac);
    if (nova < 1) nova = 1;
    d->vida        = nova;
    d->vida_maxima = nova;
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

        /* DoT do veneno: acumulador float evita truncar pra 0 com int vida. */
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

        /* Aliado temporário (EF_SPAWNA_ALIADO) expira sem recompensa. */
        if (d->vida_aliado_restante > 0.0f) {
            d->vida_aliado_restante -= dt;
            if (d->vida_aliado_restante <= 0.0f) {
                d->vivo = false;   /* direto: aliado não vale pontuação */
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
    }

    /* ----- PASS 1.5: disparo de projétil -----
     * Mecânica 100% engine; o QUE/QUANTO vem da tabela tunável da Luísa
     * (projeteis_inimigo_tipos.c). A IA dela (ia_kiter) fica intocada — só
     * mantém distância; o tiro acontece aqui. Inimigo congelado ou aliado
     * não atira; só dispara com o jogador dentro do alcance. */
    for (InimigoNo *ino = ej->inimigos_cabeca; ino != NULL; ino = ino->proximo) {
        Inimigo *d = &ino->dados;
        if (!d->vivo || d->aliado || d->congelado_tempo > 0.0f) continue;
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
     * Pra cada par (a, b) com a vindo antes de b na lista: se os círculos
     * se sobrepõem, empurra cada um por metade do overlap. Caso degenerado
     * (centros coincidentes): empurra "a" pra direita, deixa "b" parado. */
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
                /* Coincidência exata — separa minimamente. */
                a->dados.posicao.x -= 1.0f;
            }
        }
    }

    /* ----- PASS 3: remoção dos mortos (ponteiro duplo) -----
     * `**atual` aponta pro próximo ponteiro a ser modificado: ou a cabeça
     * da lista, ou o campo `proximo` do nó anterior. Permite remover nós
     * sem if especial pra cabeça. */
    InimigoNo **atual = &ej->inimigos_cabeca;
    while (*atual != NULL) {
        if (!(*atual)->dados.vivo) {
            InimigoNo *morto = *atual;
            *atual = morto->proximo;
            free(morto);
        } else {
            atual = &(*atual)->proximo;
        }
    }
}


void inimigos_desenhar(const EstadoJogo *ej) {
    for (const InimigoNo *ino = ej->inimigos_cabeca;
        ino != NULL;
        ino = ino->proximo) {
        if (!ino->dados.vivo) continue;
        if ((int)ino->dados.tipo < 0 ||
            (int)ino->dados.tipo >= QTD_PARAMETROS_INIMIGO) continue;

        const ParametrosInimigo *p = &PARAMETROS_INIMIGO[ino->dados.tipo];
        Color cor = p->cor; //copia daa cor
        float vida_ratio = (float)ino->dados.vida / (float)ino->dados.vida_maxima; //porcentagem de vida
        cor.a = (unsigned char)(80 + 175 * vida_ratio); //transparencia
        DrawCircleV(ino->dados.posicao, p->raio_visual, cor);
        /* Outline mais escuro pra dar volume ao sprite circular. */
        DrawCircleLines((int)ino->dados.posicao.x,
                        (int)ino->dados.posicao.y,
                        p->raio_visual,
                        (Color){ 0, 0, 0, 150 });
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
