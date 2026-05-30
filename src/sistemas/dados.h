/* ============================================================================
 * dados.h - SISTEMA DE DADOS (Sofia / Dev 2)
 * ----------------------------------------------------------------------------
 * O jogador leva 2 dados por run. Na tela de upgrade pode gastar um dado pra
 * "rolar" uma carta e melhorar (ou piorar) o valor dela — risco gerenciado.
 * A struct Dado (tipos.h) tem faces e ultimo_resultado (0 = ainda carregado).
 * ============================================================================ */

#ifndef DADOS_H
#define DADOS_H

#include "tipos.h"

/* Rola o dado (1..faces) e guarda em ultimo_resultado. */
int dado_rolar(Dado *d);

/* Aplica o resultado da rolagem na carta (ajusta valor + descrição). */
void dado_aplicar_na_carta(int resultado, int faces, Carta *carta);

/* true se o dado está carregado (ultimo_resultado == 0). */
bool dado_esta_carregado(const Dado *d);

/* Desenha o dado (faces + último resultado). */
void dado_desenhar(const Dado *d, int posicao_x, int posicao_y);

#endif /* DADOS_H */
