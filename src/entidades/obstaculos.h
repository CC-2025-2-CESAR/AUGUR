/* ============================================================================
 * obstaculos.h - INTERFACE DO MÓDULO DE OBSTÁCULOS
 * ============================================================================
 *
 * RESPONSABILIDADE: Dev 3 (Luísa)
 *
 * Cuida dos objetos fixos do mapa — árvores, pedras, e o que mais a gente
 * decidir adicionar (toco, arbusto, runa). Eles BLOQUEIAM o jogador e os
 * inimigos (push-out físico) mas não causam dano.
 *
 * IDEIA:
 *   - O layout do mapa é gerado UMA ÚNICA VEZ no início da run, a partir
 *     da seed da profecia. Mesma seed = mesmo mapa, sempre. Isso ajuda no
 *     debug e permite jogadores compartilharem mapas legais.
 *   - Como são FIXOS durante o combate (não spawnam nem somem), não precisa
 *     de lista encadeada: usamos o array obstaculos[] em EstadoJogo.
 *
 * CONTRATO COM A COLISÃO:
 *   - colisao.c (Dev 1) já resolve jogador-vs-inimigos e magia-vs-inimigos.
 *   - As funções obstaculos_resolver_* são chamadas APÓS colisao_verificar_tudo,
 *     pra garantir que o jogador/inimigo termine o frame fora de qualquer
 *     obstáculo, mesmo que outra colisão tenha tentado empurrar pra dentro.
 *
 * REGRA: Se você mudar a geometria (raios, distribuição, área do mapa),
 * teste com seeds variadas pra confirmar que o jogador nunca nasce dentro
 * de um obstáculo.
 * ========================================================================== */

#ifndef OBSTACULOS_H
#define OBSTACULOS_H

#include "tipos.h"

/* Gera o layout aleatório de obstáculos a partir da seed.
 * Substitui qualquer layout anterior. Chamada quando uma run começa. */
void obstaculos_gerar(EstadoJogo *ej, unsigned int seed);

/* Desenha todos os obstáculos. DEVE ser chamado DENTRO de BeginMode2D
 * (eles vivem em coordenadas de mundo, não de tela). */
void obstaculos_desenhar(const EstadoJogo *ej);

/* Empurra o jogador pra fora de qualquer obstáculo que ele esteja sobrepondo.
 * Chamada após colisao_verificar_tudo, no fim do update do combate. */
void obstaculos_resolver_jogador(EstadoJogo *ej);

/* Mesmo padrão, mas pra cada inimigo da lista encadeada. */
void obstaculos_resolver_inimigos(EstadoJogo *ej);

#endif /* OBSTACULOS_H */
