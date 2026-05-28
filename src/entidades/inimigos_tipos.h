/* ============================================================================
 * inimigos_tipos.h - PARÂMETROS E IA DOS TIPOS DE INIMIGO (LUÍSA)
 * ============================================================================
 *
 * RESPONSABILIDADE: Luísa (Dev 3).
 *
 * Aqui ficam:
 *   1. PARAMETROS_INIMIGO[] — tabela de stats (vida, dano, cor, ...) indexada
 *      por TipoInimigo.
 *   2. inimigos_tipos_executar_ia() — dispatcher que escreve em i->velocidade
 *      o vetor de movimento desejado pra esse frame.
 *
 * A ENGINE (inimigos.c, Arthur) cuida de:
 *   - Alocação/liberação dos nós da lista.
 *   - Aplicar a velocidade na posição (posicao += velocidade * dt).
 *   - Push-out entre inimigos (impedindo empilhamento).
 *   - Render (DrawCircle usando cor da tabela).
 *
 * ----------------------------------------------------------------------------
 * COMO ADICIONAR UM TIPO NOVO DE INIMIGO (passo a passo):
 *   1. Adicione um valor novo no enum TipoInimigo (em src/core/tipos.h),
 *      por exemplo INIMIGO_ELITE_DISTANCIA.
 *   2. Adicione uma linha nova em PARAMETROS_INIMIGO[] (em inimigos_tipos.c)
 *      na MESMA ORDEM do enum. Coloque vida, dano, cor, e qual IA usar.
 *   3. Se quiser uma IA inédita (nem chase nem kiter):
 *      a) Adicione um valor novo no enum ComportamentoIA (tipos.h).
 *      b) Crie uma função estática `static void ia_minha_ia(Inimigo *i,
 *         EstadoJogo *ej)` em inimigos_tipos.c.
 *      c) Adicione um case no switch dentro de inimigos_tipos_executar_ia.
 *
 *   Não precisa mexer em mais NENHUM arquivo. A engine do Arthur já chama
 *   inimigos_tipos_executar_ia uma vez por inimigo por frame.
 * ============================================================================ */

#ifndef INIMIGOS_TIPOS_H
#define INIMIGOS_TIPOS_H

#include "tipos.h"

/* Tabela de parâmetros indexada por TipoInimigo. */
extern const ParametrosInimigo PARAMETROS_INIMIGO[];

/* Quantas entradas a tabela tem (validado em runtime se for usar). */
extern const int QTD_PARAMETROS_INIMIGO;

/* Escreve i->velocidade com o movimento desejado deste frame. A engine
 * aplica `posicao += velocidade * dt` em seguida. */
void inimigos_tipos_executar_ia(Inimigo *i, EstadoJogo *ej);

#endif /* INIMIGOS_TIPOS_H */
