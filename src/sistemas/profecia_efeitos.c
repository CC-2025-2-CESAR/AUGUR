/* ============================================================================
 * profecia_efeitos.c - TABELA DE BALANCEAMENTO DA PROFECIA (LUÍSA EDITA AQUI)
 * ============================================================================
 *
 * Todos os números do sistema de profecias e combos vivem aqui. A engine
 * (profecia.c) lê estes structs e nunca embute número.
 *
 * SIMPLIFICAÇÃO (v3): só restam 6 efeitos e 1 limiar de condição (timer de
 * COND_A_CADA_N_SEG). Cada magnitude aparece no texto da profecia revelada,
 * pra o jogador entender o impacto antes mesmo de começar a run.
 * ============================================================================ */

#include "profecia_efeitos.h"

const LimiaresCondicao LIMIARES_CONDICAO = {
    .a_cada_n_seg = 10.0f,   /* a cada 10 segundos */
};

const MagnitudesEfeito MAGNITUDES_EFEITO = {
    /* Efeitos da profecia */
    .explosao_raio  = 150.0f,
    .explosao_dano  = 40.0f,
    .cura_hp        = 20,
    .duplica_qtd    = 6,         /* duplica os próximos 6 disparos */
    .congelar_raio  = 150.0f,
    .congelar_tempo = 2.0f,
    .ignite_raio    = 140.0f,
    .ignite_tempo   = 3.0f,
    .ignite_dps     = 8.0f,

    /* Combos elementais (intactos) */
    .choque_janela_seg          = 3.0f,
    .choque_stun_seg            = 2.0f,
    .choque_mult_proxima        = 3.0f,
    .arcano_vs_envenenado_mult  = 2.0f,
};
