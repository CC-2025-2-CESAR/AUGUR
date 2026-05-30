/* ============================================================================
 * historico.h - HISTÓRICO DE SEEDS JOGADAS
 * ============================================================================
 *
 * Persiste as últimas HISTORICO_SEEDS_TAM seeds que o jogador rodou (em ordem
 * cronológica reversa: índice 0 = mais recente). Cada entrada tem seed + vitória/
 * derrota + tempo + biomassa. Permite re-jogar uma seed antiga sem precisar
 * anotar — o jogador escolhe da lista na tela ESTADO_HISTORICO.
 *
 * Persistência: salva junto com DadosSalvos no `fwrite` único da Sofia. Zero
 * custo adicional de I/O.
 * ============================================================================ */

#ifndef HISTORICO_H
#define HISTORICO_H

#include "tipos.h"

/* Insere uma run no histórico. Shift do array (mais recente sempre em [0]).
 * Quando cheio, a entrada mais antiga (índice HISTORICO_SEEDS_TAM-1) é
 * descartada automaticamente. */
void historico_registrar(DadosSalvos *ds,
                         unsigned int seed, bool venceu,
                         float tempo_seg, int pontuacao);

/* true se há pelo menos uma entrada salva. */
bool historico_tem_entradas(const DadosSalvos *ds);

/* Desenha a lista do histórico na tela. `cursor` é o índice da linha
 * selecionada (0..historico_qtd-1). */
void historico_desenhar(const DadosSalvos *ds, int cursor);

#endif /* HISTORICO_H */
