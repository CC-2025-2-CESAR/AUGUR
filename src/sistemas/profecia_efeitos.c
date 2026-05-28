/* ============================================================================
 * profecia_efeitos.c - TABELA DE BALANCEAMENTO DA PROFECIA (LUÍSA EDITA AQUI)
 * ============================================================================
 *
 * Todos os números do sistema de profecias e combos vivem aqui. Chutes
 * iniciais — ajuste no playtest conforme os testers pedirem. A engine
 * (profecia.c) lê estes structs e nunca embute número.
 * ============================================================================ */

#include "profecia_efeitos.h"

const LimiaresCondicao LIMIARES_CONDICAO = {
    .a_cada_10s_seg   = 10.0f,
    .vida_abaixo_frac = 0.30f,   /* dispara quando HP < 30% */
    .combo_limiar     = 5,       /* 5 kills na janela */
    .combo_janela_seg = 3.0f,    /* cada kill renova a janela por 3s */
};

const MagnitudesEfeito MAGNITUDES_EFEITO = {
    .explosao_raio        = 160.0f,
    .explosao_dano        = 40.0f,
    .cura_hp              = 20,
    .duplica_qtd          = 6,      /* duplica os próximos 6 disparos */
    .congela_raio         = 150.0f,
    .congela_tempo        = 2.0f,
    .dado_drop_qtd        = 1,      /* gancho Sofia */
    .bonus_roll_qtd       = 2,      /* gancho Sofia */
    .dano_triplo_mult     = 3.0f,
    .aliado_vida_frac     = 0.6f,
    .aliado_duracao       = 12.0f,
    .reduz_cooldown_tempo = 6.0f,
    .ignite_raio          = 140.0f,
    .ignite_tempo         = 3.0f,
    .ignite_dps           = 8.0f,
    .roubo_vida_tempo     = 6.0f,
    .roubo_vida_frac      = 0.25f,

    /* combos */
    .choque_janela_seg          = 3.0f,
    .choque_stun_seg            = 2.0f,
    .choque_mult_proxima        = 3.0f,
    .arcano_vs_envenenado_mult  = 2.0f,
};
