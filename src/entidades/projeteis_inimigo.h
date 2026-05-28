/* ============================================================================
 * projeteis_inimigo.h - INTERFACE DA ENGINE DE TIRO DE INIMIGO
 * ============================================================================
 *
 * RESPONSABILIDADE: Arthur (Dev 1) — engine.
 *
 * Espelha a engine de magias: lista encadeada (malloc/free), movimento,
 * expiração por tempo_de_vida e render circular. A DECISÃO de disparar é
 * tomada pela engine de inimigos (inimigos.c) com base na tabela tunável
 * projeteis_inimigo_tipos.c (Luísa). A colisão com o jogador é resolvida
 * em colisao.c.
 * ============================================================================ */

#ifndef PROJETEIS_INIMIGO_H
#define PROJETEIS_INIMIGO_H

#include "tipos.h"

/* Cria um projétil em `posicao` indo em `direcao_normalizada`, usando os
 * stats de PARAMETROS_PROJETIL_INIMIGO[tipo_origem]. No-op se o tipo não
 * pode atirar ou se a lista estourou MAX_PROJETEIS_INIMIGO. */
void projeteis_inimigo_spawnar(EstadoJogo *ej,
                               Vector2     posicao,
                               Vector2     direcao_normalizada,
                               TipoInimigo tipo_origem);

/* Movimento + expiração + remoção dos mortos. Uma vez por frame no combate. */
void projeteis_inimigo_atualizar(EstadoJogo *ej);

/* DrawCircleV pra cada projétil vivo (cor da tabela). */
void projeteis_inimigo_desenhar(const EstadoJogo *ej);

/* Libera todos os nós da lista (fim de run / finalização). */
void projeteis_inimigo_liberar_tudo(EstadoJogo *ej);

#endif /* PROJETEIS_INIMIGO_H */
