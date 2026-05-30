/* ============================================================================
 * profecia.h - INTERFACE DO GERADOR + MOTOR DE PROFECIAS
 * ----------------------------------------------------------------------------
 * Uma profecia = 3 modificadores [Elemento]+[Condição]+[Efeito], gerados de
 * forma determinística a partir de uma seed (mesma seed = mesma profecia, pra
 * debug e pra compartilhar runs). O motor avalia as Condições durante o combate
 * e aplica os Efeitos; as magnitudes vivem em profecia_efeitos.c.
 * ============================================================================ */

#ifndef PROFECIA_H
#define PROFECIA_H

#include "tipos.h"

/* Gera a profecia (3 mods + seed) a partir da seed. */
void profecia_gerar(Profecia *p, unsigned int seed);

/* Desenha a profecia na tela (estado REVELACAO_PROFECIA). */
void profecia_desenhar(const Profecia *p);

/* Lookups enum -> texto legível (HUD, debug). */
const char *elemento_nome(Elemento e);
const char *condicao_nome(Condicao c);
const char *efeito_nome(Efeito e);

/* Tick por frame do motor (avalia COND_A_CADA_N_SEG). Chamado no combate. */
void profecia_motor_atualizar(EstadoJogo *ej);

/* Gatilhos pontuais. ctx é a posição onde aplicar o efeito (AoE etc.).
 *   - inimigos.c: ao_matar (caminho único de morte).
 *   - colisao.c:  ao_acertar / ao_receber_dano. */
void profecia_evento_ao_matar(EstadoJogo *ej, Inimigo *morto);
void profecia_evento_ao_receber_dano(EstadoJogo *ej);
void profecia_evento_ao_acertar(EstadoJogo *ej, Vector2 ctx);

/* Cura o jogador com clamp em vida_maxima (EF_CURA). */
void profecia_curar_jogador(EstadoJogo *ej, int qtd);

/* Aplica UM efeito no contexto dado. Tem guard de reentrância (um efeito que
 * mata não recursiona em "Ao matar" infinitamente). */
void profecia_aplicar_efeito(EstadoJogo *ej, Efeito ef, Vector2 ctx);

#endif /* PROFECIA_H */
