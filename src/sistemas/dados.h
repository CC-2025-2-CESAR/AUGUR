/* ============================================================================
 * dados.h - INTERFACE DO SISTEMA DE DADOS
 * ============================================================================
 *
 * RESPONSABILIDADE: Dev 2 (Sofia)
 *
 * O jogador escolhe 2 dados antes de cada run. Entre ondas, pode gastar um
 * dado pra "rolar" uma carta e potencialmente melhorar (ou piorar) o efeito.
 *
 * Ex.: carta base é "+10 dano". O jogador rola um d6:
 *   1-2 → +5 dano  (pior)
 *   3-4 → +10 dano (igual)
 *   5   → +15 dano + área maior
 *   6   → +20 dano + ignite
 *
 * A struct Dado (definida em tipos.h) tem: faces (tipo de dado) e
 * ultimo_resultado (pra mostrar na animação de rolar).
 * ========================================================================== */

#ifndef DADOS_H
#define DADOS_H

#include "tipos.h"

/* Rola o dado d e retorna o resultado (1 a d->faces).
 * Também guarda em d->ultimo_resultado pra UI mostrar. */
int dado_rolar(Dado *d);

/* Desenha o dado na tela (número de faces + último resultado). */
void dado_desenhar(const Dado *d, int posicao_x, int posicao_y);

#endif /* DADOS_H */
