/* ============================================================================
 * profecia_efeitos.h - MAGNITUDES/LIMIARES DA PROFECIA (LUÍSA EDITA AQUI)
 * ============================================================================
 *
 * O MOTOR de profecia (profecia.c, engine do Arthur) só decide QUANDO uma
 * Condição dispara e QUAL Efeito aplicar. O QUANTO (raio da explosão, HP da
 * cura, duração do congelamento, limiar do combo, etc.) vem 100% daqui.
 *
 * Pra nerfar/buffar qualquer efeito ou condição da profecia, mexa SÓ neste
 * par de structs — a engine não tem número de balanceamento embutido.
 *
 * Efeitos que dependem do sistema de dados (Sofia) — Dropa dado, +2 no
 * próximo roll — hoje só acumulam um "pending" no motor; quando o sistema
 * de dados existir, ele consome esses campos. Não fazem nada visível ainda.
 * ============================================================================ */

#ifndef PROFECIA_EFEITOS_H
#define PROFECIA_EFEITOS_H

typedef struct {
    float a_cada_10s_seg;     /* período de COND_A_CADA_10S */
    float vida_abaixo_frac;   /* COND_VIDA_ABAIXO_X: fração de HP (0.30 = 30%) */
    int   combo_limiar;       /* kills na janela pra COND_EM_COMBO disparar */
    float combo_janela_seg;   /* janela do combo (s); kill renova */
} LimiaresCondicao;

typedef struct {
    float explosao_raio;       /* EF_EXPLOSAO: raio do dano em área (px) */
    float explosao_dano;       /* EF_EXPLOSAO: dano em cada inimigo no raio */
    int   cura_hp;             /* EF_CURA: HP recuperado */
    int   duplica_qtd;         /* EF_DUPLICA_PROJETIL: quantos disparos duplicam */
    float congela_raio;        /* EF_CONGELA: raio de efeito (px) */
    float congela_tempo;       /* EF_CONGELA: duração do congelamento (s) */
    int   dado_drop_qtd;       /* EF_DROPA_DADO: gancho Sofia (no-op por ora) */
    int   bonus_roll_qtd;      /* EF_MAIS2_PROX_ROLL: gancho Sofia (no-op) */
    float dano_triplo_mult;    /* EF_DANO_TRIPLO: multiplicador da próxima hit */
    float aliado_vida_frac;    /* EF_SPAWNA_ALIADO: vida = frac * vida do melee */
    float aliado_duracao;      /* EF_SPAWNA_ALIADO: s de vida do aliado */
    float reduz_cooldown_tempo;/* EF_REDUZ_COOLDOWN: duração do buff (s) */
    float ignite_raio;         /* EF_IGNITE_PASSIVO: raio de efeito (px) */
    float ignite_tempo;        /* EF_IGNITE_PASSIVO: duração do DoT (s) */
    float ignite_dps;          /* EF_IGNITE_PASSIVO: dano/s do DoT */
    float roubo_vida_tempo;    /* EF_ROUBO_DE_VIDA: duração do lifesteal (s) */
    float roubo_vida_frac;     /* EF_ROUBO_DE_VIDA: fração do dano que cura */

    /* --- Combos (Fase 7) --- */
    float choque_janela_seg;       /* janela da marca de Fogo p/ Choque Térmico */
    float choque_stun_seg;         /* stun (congelamento) do Choque Térmico */
    float choque_mult_proxima;     /* multiplicador da próxima hit no Choque */
    float arcano_vs_envenenado_mult; /* Arcano x inimigo envenenado: x dano */
} MagnitudesEfeito;

extern const LimiaresCondicao LIMIARES_CONDICAO;
extern const MagnitudesEfeito MAGNITUDES_EFEITO;

#endif /* PROFECIA_EFEITOS_H */
