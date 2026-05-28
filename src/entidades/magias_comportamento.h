/* ============================================================================
 * magias_comportamento.h - RIDER DE STATUS POR ELEMENTO (LUÍSA EDITA AQUI)
 * ============================================================================
 *
 * Cada elemento, além dos stats numéricos de magias_tipos.c, carrega um
 * "rider": o efeito de status que a magia aplica ao acertar um inimigo.
 *
 *   RIDER_NENHUM  — projétil puro (Fogo/Arcano/Sombra).
 *   RIDER_CONGELA — congela o alvo por congela_tempo s (Gelo).
 *   RIDER_CHAIN   — salta pra inimigos próximos com dano reduzido (Relâmpago).
 *   RIDER_VENENO  — DoT acumulativo (stacks) por veneno_tempo s (Veneno).
 *
 * A ENGINE (colisao.c) lê esta tabela e aplica o efeito. Pra nerfar/buffar,
 * mexa SÓ aqui — não precisa tocar em engine. A estrutura já tem campos pra
 * todos os riders; deixe em 0 os que não se aplicam ao elemento.
 * ============================================================================ */

#ifndef MAGIAS_COMPORTAMENTO_H
#define MAGIAS_COMPORTAMENTO_H

#include "tipos.h"

typedef enum {
    RIDER_NENHUM,
    RIDER_CONGELA,
    RIDER_CHAIN,
    RIDER_VENENO
} TipoRider;

typedef struct {
    TipoRider rider;
    float congela_tempo;       /* RIDER_CONGELA: segundos congelado */
    int   chain_saltos;        /* RIDER_CHAIN: nº de saltos extras */
    float chain_raio;          /* RIDER_CHAIN: alcance de cada salto (px) */
    float chain_dano_frac;     /* RIDER_CHAIN: fração do dano por salto (0..1) */
    float veneno_tempo;        /* RIDER_VENENO: duração do DoT (s) */
    float veneno_dps;          /* RIDER_VENENO: dano/s POR stack */
    int   veneno_max_stacks;   /* RIDER_VENENO: teto de stacks */
} ComportamentoMagia;

/* Indexada por Elemento (mesma ordem do enum em tipos.h). */
extern const ComportamentoMagia COMPORTAMENTO_MAGIA[];
extern const int QTD_COMPORTAMENTO_MAGIA;

#endif /* MAGIAS_COMPORTAMENTO_H */
