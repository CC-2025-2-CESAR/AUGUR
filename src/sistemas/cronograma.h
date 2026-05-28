/* ============================================================================
 * cronograma.h - INTERFACE DA TIMELINE (5 MIN ATÉ O CHEFÃO)
 * ============================================================================
 *
 * RESPONSABILIDADE: Arthur (Dev 1) — engine.
 *
 * Substitui o antigo sistema de "ondas finitas". O cronograma é uma timeline
 * contínua de 5 minutos: a engine consulta a tabela EVENTOS_CRONOGRAMA[]
 * (em src/sistemas/cronograma_eventos.c) pra decidir quem spawnar e quando.
 *
 * COMO É USADO NO MAIN.C:
 *   - cronograma_inicializar() é chamada quando a run começa.
 *   - cronograma_atualizar() é chamada todo frame durante o combate.
 *   - cronograma_deve_abrir_cartas() avisa quando bater o próximo minuto cheio.
 *     Quando o main consome, chama cronograma_consumir_carta_pendente() pra
 *     avançar o gatilho pro próximo minuto.
 *   - O campo .vitoria do Cronograma sinaliza que o chefão morreu.
 *
 * O QUE A LUÍSA EDITA: NADA aqui. Ela só mexe em cronograma_eventos.c.
 * ============================================================================ */

#ifndef CRONOGRAMA_H
#define CRONOGRAMA_H

#include "tipos.h"

/* Duração total da run, em segundos. Aos 5:00 spawna o chefão. */
#define CRONOGRAMA_DURACAO_SEG          (5.0f * 60.0f)

/* Intervalo entre triggers de carta (em segundos). 60 = a cada minuto cheio. */
#define CRONOGRAMA_INTERVALO_CARTAS_SEG  60.0f


/* Reseta o cronograma e copia EVENTOS_CRONOGRAMA[] pro buffer interno.
 * Chamada uma vez no início de cada run. */
void cronograma_inicializar(Cronograma *c);

/* Avança o tempo, ativa/desativa eventos, dispara spawns e detecta vitória.
 * Chamada todo frame enquanto estado == ESTADO_COMBATE. */
void cronograma_atualizar(Cronograma *c, EstadoJogo *ej);

/* True quando a engine acaba de cruzar um minuto cheio e o main precisa abrir
 * a tela de cartas. O main consome com cronograma_consumir_carta_pendente(). */
bool cronograma_deve_abrir_cartas(const Cronograma *c);

/* Avança o gatilho de cartas pro próximo minuto. Chamada pelo main quando a
 * tela de cartas é fechada. */
void cronograma_consumir_carta_pendente(Cronograma *c);

#endif /* CRONOGRAMA_H */
