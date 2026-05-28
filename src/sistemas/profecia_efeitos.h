/* ============================================================================
 * profecia_efeitos.h - MAGNITUDES/LIMIARES DA PROFECIA (LUÍSA EDITA AQUI)
 * ============================================================================
 *
 * O MOTOR de profecia (profecia.c) só decide QUANDO uma Condição dispara e
 * QUAL Efeito aplicar. O QUANTO (dano, raio, duração) vem 100% daqui.
 *
 * Pra nerfar/buffar qualquer efeito ou condição, mexa SÓ neste par de structs.
 * A engine não tem número de balanceamento embutido.
 *
 * SIMPLIFICAÇÃO (v3): pool de Condicao caiu pra 4 (AO_MATAR, AO_RECEBER_DANO,
 * AO_ACERTAR, A_CADA_N_SEG) e pool de Efeito caiu pra 6 (EXPLOSAO, CURA,
 * ESCUDO, IGNITE, CONGELAR, DUPLICA_PROJETIL). Os campos abaixo refletem só o
 * que ainda é usado. Os campos de combos (Choque Térmico, Corrente Arcana)
 * são bônus emergente em combos.c e continuam intactos.
 * ============================================================================ */

#ifndef PROFECIA_EFEITOS_H
#define PROFECIA_EFEITOS_H

typedef struct {
    float a_cada_n_seg;       /* período de COND_A_CADA_N_SEG (segundos) */
} LimiaresCondicao;

typedef struct {
    /* --- Efeitos da profecia (6 sobreviventes) --- */
    float explosao_raio;          /* EF_EXPLOSAO: raio do dano em área (px) */
    float explosao_dano;          /* EF_EXPLOSAO: dano em cada inimigo no raio */
    int   cura_hp;                /* EF_CURA: HP recuperado */
    int   duplica_qtd;            /* EF_DUPLICA_PROJETIL: quantos disparos duplicam */
    float congelar_raio;          /* EF_CONGELAR: raio de efeito (px) */
    float congelar_tempo;         /* EF_CONGELAR: duração do congelamento (s) */
    float ignite_raio;            /* EF_IGNITE: raio de efeito (px) */
    float ignite_tempo;           /* EF_IGNITE: duração do DoT (s) */
    float ignite_dps;             /* EF_IGNITE: dano/s do DoT */

    /* --- Combos elementais (bônus emergente — combos.c) --- */
    float choque_janela_seg;       /* janela da marca de Fogo p/ Choque Térmico */
    float choque_stun_seg;         /* stun (congelamento) do Choque Térmico */
    float choque_mult_proxima;     /* multiplicador da próxima hit no Choque */
    float arcano_vs_envenenado_mult; /* Arcano x inimigo envenenado: x dano */
} MagnitudesEfeito;

extern const LimiaresCondicao LIMIARES_CONDICAO;
extern const MagnitudesEfeito MAGNITUDES_EFEITO;

#endif /* PROFECIA_EFEITOS_H */
