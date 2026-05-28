/* ============================================================================
 * projeteis_inimigo_tipos.h - STATS DO TIRO DE INIMIGO (LUÍSA EDITA AQUI)
 * ============================================================================
 *
 * Tiro PADRÃO (não-elemental) que cada tipo de inimigo dispara no jogador.
 * A engine (inimigos.c decide QUANDO; projeteis_inimigo.c cuida da lista)
 * só lê esta tabela. Pra nerfar/buffar, ou pra ligar/desligar o tiro de um
 * tipo, mexa SÓ aqui.
 *
 * `pode_atirar = false` => aquele tipo nunca atira (só dano de toque).
 * Futuro: dá pra tornar elemental adicionando um campo de Elemento aqui e
 * lendo na engine — sem refatorar a lista.
 * ============================================================================ */

#ifndef PROJETEIS_INIMIGO_TIPOS_H
#define PROJETEIS_INIMIGO_TIPOS_H

#include "tipos.h"

typedef struct {
    bool  pode_atirar;
    float dano;
    float velocidade;        /* px/s */
    float raio;
    float tempo_vida;        /* s até sumir */
    Color cor;
    float cooldown_disparo;  /* s entre tiros */
    float alcance_disparo;   /* só atira se o jogador estiver a <= isso (px) */
} ParametrosProjetilInimigo;

/* Indexada por TipoInimigo (mesma ordem do enum em tipos.h). */
extern const ParametrosProjetilInimigo PARAMETROS_PROJETIL_INIMIGO[];
extern const int QTD_PARAMETROS_PROJETIL_INIMIGO;

#endif /* PROJETEIS_INIMIGO_TIPOS_H */
