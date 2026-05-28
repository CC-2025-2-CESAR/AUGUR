/* ============================================================================
 * leaderboard.h - TOP-10 DE RUNS (tempo e biomassa)
 * ============================================================================
 *
 * Duas tabelas em DadosSalvos:
 *   - top_tempo:    só vitórias, ordenado por tempo CRESCENTE (mais rápido 1º)
 *   - top_biomassa: vitórias e derrotas, ordenado por pontuação DECRESCENTE
 *
 * Insertion-sort O(LEADERBOARD_TAM²) é o bastante — só 10 slots por tabela.
 *
 * COMO INTEGRAR:
 *   - main.c::atualizar_combate, quando jogador.vida<=0:
 *       leaderboard_registrar(&ej->salvamento, biomassa, tempo, seed, false);
 *       salvamento_salvar(&ej->salvamento);
 *   - main.c, na transição pra ESTADO_VITORIA:
 *       leaderboard_registrar(&ej->salvamento, biomassa, tempo, seed, true);
 *       salvamento_salvar(&ej->salvamento);
 *
 * Pra desenhar a tela: leaderboard_desenhar(&ej->salvamento, aba_biomassa).
 * ============================================================================ */

#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include "tipos.h"

/* Insere uma run nas tabelas. Quem mantém ordenado e poda em LEADERBOARD_TAM
 * é esta função. Vitórias entram nas duas tabelas; derrotas só em top_biomassa.
 *
 * pontuacao    = biomassa coletada na run (Jogador.biomassa).
 * tempo_seg    = cronograma.tempo_decorrido na hora do fim.
 * seed         = profecia.seed (pra replay do recorde).
 * venceu       = true se chegou ao chefão e derrotou. */
void leaderboard_registrar(DadosSalvos *ds,
                           int pontuacao, float tempo_seg,
                           unsigned int seed, bool venceu);

/* Desenha a tela de leaderboard usando primitivas do Raylib. Coord de tela
 * (chamar FORA de BeginMode2D). aba_biomassa=true mostra top_biomassa;
 * false mostra top_tempo. */
void leaderboard_desenhar(const DadosSalvos *ds, bool aba_biomassa);

#endif /* LEADERBOARD_H */
