/* ============================================================================
 * hud.h - INTERFACE DO HUD (Sofia / Dev 2)
 * ----------------------------------------------------------------------------
 * Camada de UI fixa sobre o jogo durante o combate: barra de vida, tempo da
 * run e moeda de biomassa. Módulo separado pra não poluir o main.c com UI.
 * ============================================================================ */

#ifndef HUD_H
#define HUD_H

#include "tipos.h"

/* Desenha todo o HUD. Chamada no fim de jogo_desenhar(), por cima do mundo. */
void desenhar_hud(const EstadoJogo *ej);

#endif /* HUD_H */
