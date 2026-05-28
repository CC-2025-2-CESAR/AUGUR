/* ============================================================================
 * profecia.c - IMPLEMENTAÇÃO DO GERADOR DE PROFECIAS
 * ============================================================================
 *
 * CONCEITOS IMPORTANTES USADOS AQUI:
 *   - MATRIZES de strings (requisito obrigatório de PIF): tabelas fixas com
 *     os nomes de cada enum. Consulta direta por índice.
 *   - Gerador pseudoaleatório determinístico via srand(seed) + rand().
 *     Mesma seed = mesma profecia toda vez.
 *   - "static const" nas tabelas: "static" = visível só nesse arquivo;
 *     "const" = somente leitura. Protege contra modificação acidental.
 *
 * ATENÇÃO: as strings das tabelas abaixo VÃO PARA O DrawText() em
 * profecia_desenhar. Como a fonte default do Raylib é ASCII puro, as
 * strings ficam SEM acentos ("Relampago", "Explosao") — se acentuar aqui,
 * vira quadradinho na tela. Os comentários, sim, podem ter acento.
 * ========================================================================== */

#include "profecia.h"
#include "profecia_efeitos.h"
#include "inimigos.h"
#include "raylib.h"
#include <stdlib.h>   /* srand, rand */
#include <stdio.h>    /* snprintf */
#include <math.h>     /* sqrtf */


/* ============================================================================
 * TABELAS DE NOMES
 * --------------------------------------------------------------------------
 * MATRIZES (arrays) de strings. Cada índice corresponde ao valor do enum.
 * Ordem tem que bater com a ordem dos enums em tipos.h.
 *
 * Exemplo: nomes_elementos[ELEMENTO_FOGO] = "Fogo"
 *
 * Strings sem acento de propósito (vão pra DrawText, fonte ASCII).
 * ========================================================================== */

static const char *nomes_elementos[ELEMENTO_TOTAL] = {
    "Fogo",
    "Gelo",
    "Relampago",
    "Veneno",
    "Arcano",
    "Sombra"
};

static const char *nomes_condicoes[COND_TOTAL] = {
    "Ao matar",
    "Ao tomar dano",
    "A cada 10s",
    "Em combo",
    "Ao rolar dado",
    "Ao curar",
    "Na primeira hit",
    "Vida abaixo do limite",
    "Ao acertar",
    "Inicio da run"
};

static const char *nomes_efeitos[EF_TOTAL] = {
    "Explosao em area",
    "Cura o jogador",
    "Duplica projetil",
    "Congela alvo",
    "Dropa dado",
    "+2 no proximo roll",
    "Dano triplo",
    "Spawna aliado fraco",
    "Reduz cooldown",
    "Ignite passivo",
    "Escudo",
    "Roubo de vida"
};


/* ============================================================================
 * GERAÇÃO DA PROFECIA
 * --------------------------------------------------------------------------
 * srand(seed) reinicia o gerador aleatório com um valor fixo. A partir daí,
 * cada chamada de rand() produz a mesma sequência de números. É por isso
 * que "mesma seed = mesma profecia".
 *
 * rand() % N retorna um valor entre 0 e N-1, que cai exatamente no range
 * de um enum. Pequena imperfeição: "modulo bias" — números baixos têm
 * probabilidade ligeiramente maior. Pra um projeto acadêmico, irrelevante.
 * ========================================================================== */
void profecia_gerar(Profecia *p, unsigned int seed) {
    p->seed = seed;
    srand(seed);

    /* Sorteia os 3 modificadores */
    for (int i = 0; i < 3; i++) {
        p->mods[i].elemento = (Elemento)(rand() % ELEMENTO_TOTAL);
        p->mods[i].condicao = (Condicao)(rand() % COND_TOTAL);
        p->mods[i].efeito   = (Efeito)  (rand() % EF_TOTAL);
    }
}


/* ============================================================================
 * LOOKUPS
 * --------------------------------------------------------------------------
 * Recebem um valor de enum e retornam a string correspondente.
 * Evitam switch gigante em todo lugar.
 * ========================================================================== */
const char *elemento_nome(Elemento e) {
    if (e < 0 || e >= ELEMENTO_TOTAL) return "???";
    return nomes_elementos[e];
}

const char *condicao_nome(Condicao c) {
    if (c < 0 || c >= COND_TOTAL) return "???";
    return nomes_condicoes[c];
}

const char *efeito_nome(Efeito e) {
    if (e < 0 || e >= EF_TOTAL) return "???";
    return nomes_efeitos[e];
}


/* ============================================================================
 * DESENHO
 * --------------------------------------------------------------------------
 * Mostra a profecia formatada na tela. Cada modificador em uma linha,
 * dourado pra dar destaque.
 *
 * snprintf é uma versão "segura" do sprintf: nunca escreve além do tamanho
 * do buffer, evitando buffer overflow.
 * ========================================================================== */
void profecia_desenhar(const Profecia *p) {
    DrawText("PROFECIA", LARGURA_TELA / 2 - 130, 80, 56, GOLD);

    /* Mostra seed pequena abaixo do título, pra identificar a run */
    char buffer_seed[64];
    snprintf(buffer_seed, sizeof(buffer_seed), "Seed: %u", p->seed);
    DrawText(buffer_seed, LARGURA_TELA / 2 - 60, 160, 18, GRAY);

    /* Desenha os 3 modificadores. Cada um numerado com I, II, III
     * pra dar um toque "misterioso". */
    const char *numerais[3] = { "I", "II", "III" };

    for (int i = 0; i < 3; i++) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s.  %s  |  %s  ->  %s",
                 numerais[i],
                 elemento_nome(p->mods[i].elemento),
                 condicao_nome(p->mods[i].condicao),
                 efeito_nome(p->mods[i].efeito));

        /* Espaçamento vertical: cada linha 60 pixels abaixo da anterior. */
        int y = 240 + i * 70;
        DrawText(buffer, 150, y, 26, WHITE);
    }
}


/* ============================================================================
 * MOTOR DE PROFECIA
 * --------------------------------------------------------------------------
 * Avalia as Condições dos 3 mods e aplica os Efeitos. Toda magnitude vem de
 * MAGNITUDES_EFEITO / LIMIARES_CONDICAO (profecia_efeitos.c — Luísa tuna).
 * ========================================================================== */

/* Distância ao quadrado entre dois pontos (evita sqrt em checagem de raio). */
static float dist2(Vector2 a, Vector2 b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

/* Dano em área (EF_EXPLOSAO): a morte passa pelo caminho único, então o
 * kill conta biomassa/combo igual a qualquer outro. */
static void dano_em_area(EstadoJogo *ej, Vector2 centro, float raio, float dano) {
    float r2 = raio * raio;
    for (InimigoNo *ino = ej->inimigos_cabeca; ino; ino = ino->proximo) {
        Inimigo *d = &ino->dados;
        if (!d->vivo || d->aliado) continue;
        if (dist2(centro, d->posicao) > r2) continue;
        d->vida -= (int)dano;
        if (d->vida <= 0) inimigos_registrar_morte(ej, d);
    }
}

/* Aplica um status em todos os inimigos dentro de um raio (congela/ignite). */
static void status_em_area(EstadoJogo *ej, Vector2 centro, float raio,
                           bool congela, float congela_t,
                           bool ignite, float ignite_t, float ignite_dps) {
    float r2 = raio * raio;
    for (InimigoNo *ino = ej->inimigos_cabeca; ino; ino = ino->proximo) {
        Inimigo *d = &ino->dados;
        if (!d->vivo || d->aliado) continue;
        if (dist2(centro, d->posicao) > r2) continue;
        if (congela && congela_t > d->congelado_tempo) {
            d->congelado_tempo = congela_t;
        }
        if (ignite) {
            if (ignite_t > d->veneno_tempo) d->veneno_tempo = ignite_t;
            if (ignite_dps > d->veneno_dps) d->veneno_dps = ignite_dps;
            if (d->veneno_stacks < 1) d->veneno_stacks = 1;
        }
    }
}


void profecia_aplicar_efeito(EstadoJogo *ej, Efeito ef, Vector2 ctx) {
    MotorProfecia *mp = &ej->motor_profecia;
    if (mp->em_aplicar_efeito) return;   /* corta recursão (efeito que mata) */
    mp->em_aplicar_efeito = true;

    const MagnitudesEfeito *M = &MAGNITUDES_EFEITO;

    switch (ef) {
        case EF_EXPLOSAO:
            dano_em_area(ej, ctx, M->explosao_raio, M->explosao_dano);
            break;
        case EF_CURA:
            profecia_curar_jogador(ej, M->cura_hp);
            break;
        case EF_DUPLICA_PROJETIL:
            mp->duplica_proximos += M->duplica_qtd;
            break;
        case EF_CONGELA:
            status_em_area(ej, ctx, M->congela_raio,
                           true, M->congela_tempo, false, 0.0f, 0.0f);
            break;
        case EF_DROPA_DADO:
            mp->pending_dado_drop += M->dado_drop_qtd;   /* gancho Sofia */
            break;
        case EF_MAIS2_PROX_ROLL:
            mp->pending_bonus_roll += M->bonus_roll_qtd;  /* gancho Sofia */
            break;
        case EF_DANO_TRIPLO:
            mp->dano_triplo_proxima = M->dano_triplo_mult;
            break;
        case EF_SPAWNA_ALIADO: {
            Vector2 p = { ej->jogador.posicao.x + 40.0f,
                          ej->jogador.posicao.y };
            inimigos_spawnar_aliado(ej, p, M->aliado_vida_frac,
                                    M->aliado_duracao);
            break;
        }
        case EF_REDUZ_COOLDOWN:
            mp->reduz_cooldown_tempo = M->reduz_cooldown_tempo;
            break;
        case EF_IGNITE_PASSIVO:
            status_em_area(ej, ctx, M->ignite_raio,
                           false, 0.0f, true, M->ignite_tempo, M->ignite_dps);
            break;
        case EF_ESCUDO:
            mp->escudo_ativo = 1.0f;   /* anula o próximo hit */
            break;
        case EF_ROUBO_DE_VIDA:
            mp->roubo_vida_tempo = M->roubo_vida_tempo;
            break;
        case EF_TOTAL:
        default:
            break;
    }

    mp->em_aplicar_efeito = false;
}


/* Dispara, pra cada um dos 3 mods cuja Condição == c, o Efeito do mod. */
static void disparar_condicao(EstadoJogo *ej, Condicao c, Vector2 ctx) {
    for (int m = 0; m < 3; m++) {
        if (ej->profecia.mods[m].condicao == c) {
            profecia_aplicar_efeito(ej, ej->profecia.mods[m].efeito, ctx);
        }
    }
}


void profecia_curar_jogador(EstadoJogo *ej, int qtd) {
    if (qtd <= 0) return;
    ej->jogador.vida += qtd;
    if (ej->jogador.vida > ej->jogador.vida_maxima) {
        ej->jogador.vida = ej->jogador.vida_maxima;
    }
    profecia_evento_ao_curar(ej);
}


void profecia_evento_inicio_run(EstadoJogo *ej) {
    if (ej->motor_profecia.inicio_run_disparado) return;
    ej->motor_profecia.inicio_run_disparado = true;
    disparar_condicao(ej, COND_INICIO_RUN, ej->jogador.posicao);
}


void profecia_evento_ao_matar(EstadoJogo *ej, Inimigo *morto) {
    Vector2 ctx = (morto != NULL) ? morto->posicao : ej->jogador.posicao;
    MotorProfecia *mp = &ej->motor_profecia;

    mp->combo_contador++;
    mp->combo_janela_restante = LIMIARES_CONDICAO.combo_janela_seg;

    disparar_condicao(ej, COND_AO_MATAR, ctx);

    if (mp->combo_contador >= LIMIARES_CONDICAO.combo_limiar) {
        mp->combo_contador = 0;
        disparar_condicao(ej, COND_EM_COMBO, ctx);
    }
}


void profecia_evento_ao_receber_dano(EstadoJogo *ej) {
    disparar_condicao(ej, COND_AO_RECEBER_DANO, ej->jogador.posicao);
}


void profecia_evento_ao_acertar(EstadoJogo *ej, Vector2 ctx) {
    if (!ej->motor_profecia.primeira_hit_consumida) {
        ej->motor_profecia.primeira_hit_consumida = true;
        disparar_condicao(ej, COND_PRIMEIRA_HIT, ctx);
    }
    disparar_condicao(ej, COND_AO_ACERTAR, ctx);
}


void profecia_evento_ao_curar(EstadoJogo *ej) {
    disparar_condicao(ej, COND_AO_CURAR, ej->jogador.posicao);
}


/* Gancho pro sistema de dados (Sofia): hoje sem call-site. Quando o roll
 * existir, chamar isto após cada rolagem. */
void profecia_evento_ao_rolar_dado(EstadoJogo *ej) {
    disparar_condicao(ej, COND_AO_ROLAR_DADO, ej->jogador.posicao);
}


void profecia_motor_atualizar(EstadoJogo *ej) {
    float dt = ej->delta_tempo;
    MotorProfecia *mp = &ej->motor_profecia;

    /* Decaimento dos buffs temporários. */
    if (mp->roubo_vida_tempo > 0.0f) {
        mp->roubo_vida_tempo -= dt;
        if (mp->roubo_vida_tempo < 0.0f) mp->roubo_vida_tempo = 0.0f;
    }
    if (mp->reduz_cooldown_tempo > 0.0f) {
        mp->reduz_cooldown_tempo -= dt;
        if (mp->reduz_cooldown_tempo < 0.0f) mp->reduz_cooldown_tempo = 0.0f;
    }

    /* Janela de combo: sem kill por combo_janela_seg, zera o contador. */
    if (mp->combo_janela_restante > 0.0f) {
        mp->combo_janela_restante -= dt;
        if (mp->combo_janela_restante <= 0.0f) {
            mp->combo_janela_restante = 0.0f;
            mp->combo_contador = 0;
        }
    }

    float vida_frac = (ej->jogador.vida_maxima > 0)
        ? (float)ej->jogador.vida / (float)ej->jogador.vida_maxima
        : 1.0f;

    for (int m = 0; m < 3; m++) {
        Condicao c = ej->profecia.mods[m].condicao;

        if (c == COND_A_CADA_10S) {
            mp->timer_cond[m] += dt;
            if (mp->timer_cond[m] >= LIMIARES_CONDICAO.a_cada_10s_seg) {
                mp->timer_cond[m] -= LIMIARES_CONDICAO.a_cada_10s_seg;
                profecia_aplicar_efeito(ej, ej->profecia.mods[m].efeito,
                                        ej->jogador.posicao);
            }
        } else if (c == COND_VIDA_ABAIXO_X) {
            /* Edge-triggered: dispara só na borda de entrada, re-arma ao
             * recuperar acima do limiar (não spamma todo frame). */
            if (vida_frac < LIMIARES_CONDICAO.vida_abaixo_frac) {
                if (!mp->cond_vida_armada[m]) {
                    mp->cond_vida_armada[m] = true;
                    profecia_aplicar_efeito(ej, ej->profecia.mods[m].efeito,
                                            ej->jogador.posicao);
                }
            } else {
                mp->cond_vida_armada[m] = false;
            }
        }
    }
}
