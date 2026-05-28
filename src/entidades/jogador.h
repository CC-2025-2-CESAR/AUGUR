/* ============================================================================
 * jogador.h - INTERFACE DO MÓDULO JOGADOR
 * ============================================================================
 *
 * RESPONSABILIDADE: Dev 1 (Arthur)
 *
 * Cuida de tudo relacionado ao personagem do jogador:
 *   - Movimento (WASD ou setas, com normalização da diagonal).
 *   - HP atual e máximo.
 *   - Dano recebido (chamado pela colisão quando inimigo encosta).
 *   - Render do círculo do jogador.
 *
 * O jogador vive em coordenadas de mundo (mundo infinito). A câmera, definida
 * em main.c, cuida de mantê-lo sempre centralizado na tela.
 *
 * (Convenções gerais de C deste projeto — header×implementação, include-guard,
 *  EstadoJogo *ej — estão documentadas no README.md, seção "Convenções de
 *  código C".)
 * ========================================================================== */

#ifndef JOGADOR_H
#define JOGADOR_H

#include "tipos.h"

/* --- Protótipos públicos --- */

/* Prepara o jogador com valores padrão (posição, HP, velocidade). */
void jogador_inicializar(Jogador *j);

/* Lê WASD (ou setas) e move o jogador. delta_tempo garante movimento suave. */
void jogador_atualizar(Jogador *j, float delta_tempo);

/* Desenha o jogador na tela. "const" = função não modifica o jogador. */
void jogador_desenhar(const Jogador *j);

/* Aplica dano ao jogador. Main chama isso quando colisão detecta hit. */
void jogador_sofrer_dano(Jogador *j, int quantidade);

#endif /* JOGADOR_H */
