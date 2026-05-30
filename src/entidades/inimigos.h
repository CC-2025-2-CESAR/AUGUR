/* ============================================================================
 * inimigos.h - INTERFACE DA ENGINE DE INIMIGOS (Arthur / Dev 1)
 * ----------------------------------------------------------------------------
 * A engine cuida da lista encadeada, push-out, render e dispatch de IA. Os
 * DADOS de cada tipo (vida, dano, IA) vivem em inimigos_tipos.c (Luísa). main.c
 * chama atualizar/desenhar/liberar por frame; cronograma.c chama spawnar_em.
 * ============================================================================ */

#ifndef INIMIGOS_H
#define INIMIGOS_H

#include "tipos.h"

/* Uma chamada por frame: IA + movimento + push-out + animação de morte + free. */
void inimigos_atualizar(EstadoJogo *ej);

/* Desenha cada inimigo (sprite, ou círculo de fallback se a sheet faltar). */
void inimigos_desenhar(const EstadoJogo *ej);

/* Libera todos os nós da lista (fim de run / shutdown). */
void inimigos_liberar_tudo(EstadoJogo *ej);

/* Cria um inimigo do tipo na posição (lê PARAMETROS_INIMIGO). No-op se a lista
 * estourar MAX_INIMIGOS. */
void inimigos_spawnar_em(EstadoJogo *ej, Vector2 posicao, TipoInimigo tipo);

/* Caminho ÚNICO de morte: idempotente, credita biomassa e inicia a anim DEATH.
 * Toda fonte de dano (colisão, DoT, combo, explosão) deve matar por aqui — o
 * free do nó continua no PASS 3 de inimigos_atualizar. */
void inimigos_registrar_morte(EstadoJogo *ej, Inimigo *i);

#endif /* INIMIGOS_H */
