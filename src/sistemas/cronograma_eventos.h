/* ============================================================================
 * cronograma_eventos.h - TABELA DECLARATIVA DA TIMELINE (LUÍSA)
 * ============================================================================
 *
 * RESPONSABILIDADE: Luísa (Dev 3).
 *
 * Este header só expõe a tabela. Toda a lógica de timeline, spawn e detecção
 * de vitória vive em cronograma.c (engine, Arthur). Pra mudar o ritmo da
 * run, edite APENAS a tabela em cronograma_eventos.c.
 * ============================================================================ */

#ifndef CRONOGRAMA_EVENTOS_H
#define CRONOGRAMA_EVENTOS_H

#include "tipos.h"

/* Tabela declarativa de eventos da timeline. Cada linha é uma "fase" de
 * spawn que vai rodar entre tempo_inicio_seg e tempo_fim_seg. */
extern const EventoCronograma EVENTOS_CRONOGRAMA[];
extern const int              QTD_EVENTOS_CRONOGRAMA;

#endif /* CRONOGRAMA_EVENTOS_H */
