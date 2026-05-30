/* ============================================================================
 * cartas.h - SISTEMA DE CARTAS DE UPGRADE (Sofia / Dev 2)
 * ----------------------------------------------------------------------------
 * A cada minuto cheio o jogo abre a tela CARTAS_UPGRADE com 3 cartas sorteadas;
 * o jogador escolhe uma (1/2/3) e pode gastar um dado pra rolar o valor antes.
 * ============================================================================ */

#ifndef CARTAS_H
#define CARTAS_H

#include "tipos.h"

/* Sorteia 3 cartas em ej->escolhas_upgrade. */
void cartas_gerar_escolhas(EstadoJogo *ej);

/* Aplica o efeito da carta escolhida (índice 0, 1 ou 2) no jogador. */
void cartas_aplicar(EstadoJogo *ej, int indice_escolhido);

/* Gasta o primeiro dado carregado pra rolar e aplicar o resultado na carta i.
 * Retorna false se não há dado disponível ou a carta já foi rolada. */
bool cartas_usar_dado(EstadoJogo *ej, int indice_carta);

/* Desenha as 3 cartas (+ os dados) durante CARTAS_UPGRADE. */
void cartas_desenhar_ui(const EstadoJogo *ej);

#endif /* CARTAS_H */
