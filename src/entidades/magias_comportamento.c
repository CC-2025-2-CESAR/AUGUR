/* ============================================================================
 * magias_comportamento.c - TABELA DE RIDERS POR ELEMENTO (LUÍSA EDITA AQUI)
 * ============================================================================
 *
 * Os números são chutes razoáveis pra começar — ajuste no playtest. A ordem
 * das linhas casa com o enum Elemento:
 *   [0] FOGO  [1] GELO  [2] RELAMPAGO  [3] VENENO  [4] ARCANO  [5] SOMBRA
 *
 * Combos (Choque Térmico, Arcano+Veneno) usam estes mesmos campos como base;
 * os multiplicadores do combo ficam em profecia_efeitos.c.
 * ============================================================================ */

#include "magias_comportamento.h"

const ComportamentoMagia COMPORTAMENTO_MAGIA[] = {
    /* [0] FOGO — dano alto, projétil puro. Sem rider (o "punch" vem do dano). */
    { .rider = RIDER_NENHUM },

    /* [1] GELO — leve e lento, congela quem o projétil toca. */
    {
        .rider         = RIDER_CONGELA,
        .congela_tempo = 1.5f,
    },

    /* [2] RELAMPAGO — rápido, salta pra até 3 inimigos próximos. */
    {
        .rider           = RIDER_CHAIN,
        .chain_saltos    = 3,
        .chain_raio      = 180.0f,
        .chain_dano_frac = 0.6f,
    },

    /* [3] VENENO — dano baixo, DoT acumulativo (até 5 stacks). */
    {
        .rider            = RIDER_VENENO,
        .veneno_tempo     = 3.0f,
        .veneno_dps       = 6.0f,
        .veneno_max_stacks = 5,
    },

    /* [4] ARCANO — equilibrado, magia "default". Sem rider. */
    { .rider = RIDER_NENHUM },

    /* [5] SOMBRA — alto dano, baixa cadência. Sem rider. */
    { .rider = RIDER_NENHUM },
};

const int QTD_COMPORTAMENTO_MAGIA =
    sizeof(COMPORTAMENTO_MAGIA) / sizeof(COMPORTAMENTO_MAGIA[0]);
