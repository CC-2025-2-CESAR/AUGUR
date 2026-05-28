/* ============================================================================
 * obstaculos.c - STUB DO MÓDULO DE OBSTÁCULOS
 * ============================================================================
 *
 * TODO: Dev 3 (Luísa) implementar este arquivo.
 *
 * Por enquanto as 4 funções existem mas não fazem nada. main.c já chama
 * todas elas — assim que você escrever o conteúdo, o jogo passa a ter
 * obstáculos sem mexer em nenhum outro arquivo.
 *
 * Sugestão de implementação:
 *
 * void obstaculos_gerar(EstadoJogo *ej, unsigned int seed):
 *
 *   // Stream de rand próprio pra não conflitar com profecia.c (que já
 *   // consumiu a seed lá). Multiplica e soma um valor mágico pra tirar
 *   // uma sequência diferente, mas ainda determinística pra mesma seed.
 *   srand(seed * 2654435761u + 12345u);
 *
 *   ej->qtd_obstaculos = 0;
 *   int qtd_alvo = 16 + rand() % 17;   // entre 16 e 32 obstáculos
 *
 *   for (int i = 0; i < qtd_alvo && ej->qtd_obstaculos < MAX_OBSTACULOS; i++) {
 *       // Sorteia tipo, raio compatível, posição em [-2500, 2500]^2.
 *       // Tente algumas vezes pra evitar:
 *       //   - sobreposição com outro obstáculo já colocado
 *       //   - spawn dentro da "zona segura" do jogador (~180 px da origem)
 *       // Se não achou em 8 tentativas, pula esse obstáculo.
 *   }
 *
 * void obstaculos_desenhar(const EstadoJogo *ej):
 *
 *   // PRECISA ser chamada DENTRO de BeginMode2D — usa coord de mundo.
 *   // Para cada obstáculo no array:
 *   //   - ÁRVORE: tronco marrom (DrawRectangle) + copa verde (DrawCircleV) +
 *   //            highlight verde-claro deslocado pra dar volume
 *   //   - PEDRA:  círculo cinza-escuro + reflexo cinza-claro deslocado +
 *   //             contorno escuro com DrawCircleLines
 *
 * void obstaculos_resolver_jogador(EstadoJogo *ej):
 *
 *   // Para cada obstáculo, se houver overlap com o círculo do jogador:
 *   //   distancia = hypot(dx, dy);
 *   //   overlap = (raio_jogador + raio_obstaculo) - distancia;
 *   //   ej->jogador.posicao += unit(dx, dy) * overlap;
 *   // Caso degenerado (centros sobrepostos): empurra arbitrariamente.
 *
 * void obstaculos_resolver_inimigos(EstadoJogo *ej):
 *
 *   // Mesmo padrão, mas iterando sobre a lista encadeada de inimigos.
 *
 * "(void)ej;" silencia o warning de "parâmetro não usado". Remova quando
 * for implementar de verdade. Mesmo no parâmetro 'seed' do gerar.
 * ========================================================================== */

#include "obstaculos.h"

void obstaculos_gerar(EstadoJogo *ej, unsigned int seed) {
    (void)ej; (void)seed;   /* TODO: Luísa — gerar layout determinístico */
}

void obstaculos_desenhar(const EstadoJogo *ej) {
    (void)ej;   /* TODO: Luísa — desenhar árvores e pedras (dentro de BeginMode2D) */
}

void obstaculos_resolver_jogador(EstadoJogo *ej) {
    (void)ej;   /* TODO: Luísa — empurrar jogador pra fora de obstáculos */
}

void obstaculos_resolver_inimigos(EstadoJogo *ej) {
    (void)ej;   /* TODO: Luísa — empurrar cada inimigo pra fora de obstáculos */
}
