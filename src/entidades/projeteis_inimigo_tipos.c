/* ============================================================================
 * projeteis_inimigo_tipos.c - TABELA DO TIRO DE INIMIGO (LUÍSA EDITA AQUI)
 * ============================================================================
 *
 * Chutes iniciais — ajuste no playtest. A ordem casa com o enum TipoInimigo:
 *   [0] CORPO_A_CORPO  [1] A_DISTANCIA  [2] ELITE  [3] CHEFE
 * ============================================================================ */

#include "projeteis_inimigo_tipos.h"

const ParametrosProjetilInimigo PARAMETROS_PROJETIL_INIMIGO[] = {
    /* [0] CORPO A CORPO — não atira (só dano de toque). */
    { .pode_atirar = false },

    /* [1] A DISTANCIA (kiter) — o tiro padrão do jogo. */
    {
        .pode_atirar      = true,
        .dano             = 8.0f,
        .velocidade       = 320.0f,
        .raio             = 6.0f,
        .tempo_vida       = 2.5f,
        .cor              = (Color){ 255, 170, 60, 255 },  /* laranja */
        .cooldown_disparo = 1.6f,
        .alcance_disparo  = 520.0f,
    },

    /* [2] ELITE — corpo a corpo forte; não atira. */
    { .pode_atirar = false },

    /* [3] CHEFE — atira mais forte e de longe. */
    {
        .pode_atirar      = true,
        .dano             = 14.0f,
        .velocidade       = 300.0f,
        .raio             = 10.0f,
        .tempo_vida       = 3.0f,
        .cor              = (Color){ 255, 210, 90, 255 },  /* dourado */
        .cooldown_disparo = 1.0f,
        .alcance_disparo  = 900.0f,
    },
};

const int QTD_PARAMETROS_PROJETIL_INIMIGO =
    sizeof(PARAMETROS_PROJETIL_INIMIGO) / sizeof(PARAMETROS_PROJETIL_INIMIGO[0]);
