/* ============================================================================
 * hud.h - INTERFACE DO HUD (HEADS-UP DISPLAY)
 * ============================================================================
 *
 * RESPONSABILIDADE: Dev 2 (Sofia)
 *
 * O HUD é a camada de interface que fica sobre o jogo durante o combate:
 *   - Barra de vida do jogador
 *   - Número da onda atual
 *   - Contador de biomassa
 *   - Ícones dos dados disponíveis
 *
 * Fica como módulo separado pra não poluir main.c com código de UI.
 * ========================================================================== */

#ifndef HUD_H
#define HUD_H

#include "tipos.h"

/* Desenha todos os elementos de HUD sobre o jogo. Chamada no final do
 * jogo_desenhar(), pra ficar por cima de tudo. */
void desenhar_hud(const EstadoJogo *ej);


#endif /* HUD_H */
