/* ============================================================================
 * magia_inicial.h - SORTEIO E RENDER DAS 3 MAGIAS INICIAIS
 * ============================================================================
 *
 * Após REVELACAO_PROFECIA, o jogador escolhe 1 de 3 magias sorteadas. A
 * magia escolhida vira um 4º elemento no auto-fire (round-robin se soma aos
 * 3 elementos dos mods da profecia) — `magias_tipos_processar_auto_fire`
 * trata o slot extra.
 *
 * REGRA DE SINERGIA: pelo menos 1 das 3 opções tem elemento que aparece em
 * algum mod da profecia. Se o sorteio puro não produz nenhuma sinergética, a
 * primeira opção é trocada por uma com elemento de mod aleatório (mantendo
 * a raridade já sorteada).
 *
 * Implementação dos helpers de raridade/naming fica aqui pra não vazar pra
 * cartas.c — eles têm responsabilidades distintas (cartas = upgrades em meio
 * de run, magias iniciais = escolha inicial).
 * ============================================================================ */

#ifndef MAGIA_INICIAL_H
#define MAGIA_INICIAL_H

#include "tipos.h"

/* Curva idêntica à de cartas.c: 55% comum (0), 25% incomum (1), 12% rara (2),
 * 6% épica (3), 5% mítica (4), 2% lendária (5). Mantida espelhada em vez de
 * compartilhada porque as duas curvas podem divergir no balanceamento futuro. */
int magia_inicial_sortear_raridade(void);

/* Preenche o campo `nome` da opção baseado em (elemento, raridade). Tabela
 * de nomes vive no .c. */
void magia_inicial_nomear(OpcaoMagiaInicial *m);

/* Popula ej->opcoes_magia[3] com 3 opções sorteadas + garante sinergia (>=1
 * tem elemento que aparece em algum mod da profecia). Reseta o cursor
 * (opcao_magia_selecionada = 0). */
void magia_inicial_sortear_opcoes(EstadoJogo *ej);

/* Desenha as 3 cards lado a lado. `selecionado` é o índice (0..2) atual. */
void magia_inicial_desenhar(const EstadoJogo *ej, int selecionado);

#endif /* MAGIA_INICIAL_H */
