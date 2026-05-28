/* ============================================================================
 * colisao.h - INTERFACE DO SISTEMA DE COLISÃO
 * ============================================================================
 *
 * Dois níveis de API:
 *
 * 1. Primitivas GEOMÉTRICAS (colisao_* com parâmetros de forma):
 *    Funções pequenas e reutilizáveis que qualquer módulo pode chamar.
 *    Ex.: "essas duas bolinhas estão se tocando?".
 *
 * 2. Verificação ALTO NÍVEL (colisao_verificar_tudo):
 *    Percorre listas de magias e inimigos do estado do jogo, aplica dano,
 *    marca entidades como mortas e empurra o jogador pra fora dos inimigos
 *    que ele estiver sobrepondo. Chamada uma vez por frame pelo main.
 * ========================================================================== */

#ifndef COLISAO_H
#define COLISAO_H

#include "tipos.h"

/* --- Primitivas geométricas --- */

/* Dois círculos se tocam? (centros a,b e raios ra,rb) */
bool colisao_circulo_circulo(Vector2 a, float ra, Vector2 b, float rb);

/* Duas caixas se tocam? (formato Rectangle do Raylib: x, y, width, height) */
bool colisao_caixa_caixa(Rectangle a, Rectangle b);

/* Um círculo e uma caixa se tocam? Útil pra HUD clicável, botões, etc. */
bool colisao_circulo_caixa(Vector2 c, float r, Rectangle caixa);

/* --- Alto nível --- */

/* Percorre TODAS as listas encadeadas do jogo e resolve colisões:
 *   - Jogador × inimigos (aplica dano no jogador + push-out físico)
 *   - Magias × inimigos (aplica dano no inimigo, marca magia como morta)
 * Deve ser chamada UMA vez por frame após atualizar as posições. */
void colisao_verificar_tudo(EstadoJogo *ej);

#endif /* COLISAO_H */
