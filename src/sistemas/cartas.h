/* ============================================================================
 * cartas.h - INTERFACE DO SISTEMA DE CARTAS DE UPGRADE
 * ============================================================================
 *
 * RESPONSABILIDADE: Dev 2 (Sofia)
 *
 * Entre cada onda, o jogador vê 3 cartas e escolhe uma. Opcionalmente pode
 * gastar um dado pra "rolar" a carta e tentar um resultado melhor (ou pior).
 *
 * Fluxo esperado:
 *   1. Onda termina -> main chama cartas_gerar_escolhas()
 *   2. Estado muda pra CARTAS_UPGRADE
 *   3. cartas_desenhar_ui() mostra as 3 opções
 *   4. Jogador aperta 1, 2 ou 3 -> main chama cartas_aplicar()
 *   5. Estado volta pra COMBATE
 * ========================================================================== */

#ifndef CARTAS_H
#define CARTAS_H

#include "tipos.h"

/* Preenche ej->escolhas_upgrade com 3 cartas aleatórias. */
void cartas_gerar_escolhas(EstadoJogo *ej);

/* Aplica o efeito da carta escolhida (índice 0, 1 ou 2) no jogador. */
void cartas_aplicar(EstadoJogo *ej, int indice_escolhido);

/* Desenha as 3 cartas na tela durante CARTAS_UPGRADE. */
void cartas_desenhar_ui(const EstadoJogo *ej);

#endif /* CARTAS_H */
