/* ============================================================================
 * magias.h - INTERFACE DA ENGINE DE MAGIAS/PROJÉTEIS
 * ============================================================================
 *
 * RESPONSABILIDADE: Arthur (Dev 1) — engine.
 *
 * A engine cuida de:
 *   - Lista encadeada de projéteis (alocação, remoção dos mortos, free total).
 *   - Movimento e expiração (tempo_de_vida).
 *   - Render circular usando a cor do elemento.
 *   - Função pública magias_spawnar() pra criar um projétil — usada pela
 *     lógica de auto-fire em src/entidades/magias_tipos.c.
 *
 * O CONTEÚDO (Luísa) vive em magias_tipos.c: tabela de stats por elemento
 * e a regra de quando/onde disparar.
 *
 * O módulo colisao.c marca .viva = false quando o projétil acerta — o
 * remover é responsabilidade desta engine.
 * ============================================================================ */

#ifndef MAGIAS_H
#define MAGIAS_H

#include "tipos.h"

/* Auto-fire (chama magias_tipos) + movimento + expiração + remoção dos mortos. */
void magias_atualizar(EstadoJogo *ej);

/* DrawCircleV pra cada projétil vivo. */
void magias_desenhar(const EstadoJogo *ej);

/* Libera todos os nós da lista. Chamada pelo main na finalização e quando
 * uma run termina. */
void magias_liberar_tudo(EstadoJogo *ej);

/* Spawna um projétil de `elemento` em `posicao` se movendo na direção
 * `direcao_normalizada`. Usa stats de PARAMETROS_MAGIA[elemento]. */
void magias_spawnar(EstadoJogo *ej,
                    Vector2     posicao,
                    Vector2     direcao_normalizada,
                    Elemento    elemento);

/* Mira no inimigo mais próximo e dispara `elemento` (engine de mira própria).
 * Usada pelo auto-fire (magias_tipos.c) — itera os 3 mods da profecia.
 * Retorna false se não havia alvo (nada foi disparado). */
bool magias_disparar_elemento(EstadoJogo *ej, Elemento elemento);

#endif /* MAGIAS_H */
