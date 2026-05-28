/* ============================================================================
 * magias_tipos.h - PARÂMETROS E AUTO-FIRE DAS MAGIAS (LUÍSA)
 * ============================================================================
 *
 * RESPONSABILIDADE: Luísa (Dev 3).
 *
 * Aqui ficam:
 *   1. PARAMETROS_MAGIA[] — tabela de stats por elemento (dano, velocidade,
 *      raio, intervalo entre disparos, cor).
 *   2. magias_tipos_processar_auto_fire() — lógica de "quando disparar e
 *      contra quem mirar". Chamada pela engine todo frame durante o combate.
 *
 * A ENGINE (magias.c, Arthur) cuida de:
 *   - Alocação/liberação dos nós da lista de projéteis.
 *   - Mover e expirar os projéteis.
 *   - Renderizar cada projétil.
 *   - Spawnar uma magia individual quando você chamar magias_spawnar(),
 *     usando os stats da tabela acima.
 *
 * COMO ADICIONAR UM ELEMENTO NOVO:
 *   1. Acrescente um valor antes de ELEMENTO_TOTAL no enum em tipos.h.
 *   2. Adicione uma linha em PARAMETROS_MAGIA[] na MESMA ORDEM.
 *   3. Se quiser comportamento de auto-fire diferente (e.g., disparar 3
 *      projéteis em leque), edite magias_tipos_processar_auto_fire.
 * ============================================================================ */

#ifndef MAGIAS_TIPOS_H
#define MAGIAS_TIPOS_H

#include "tipos.h"

extern const ParametrosMagia PARAMETROS_MAGIA[];
extern const int             QTD_PARAMETROS_MAGIA;

/* Decide quando disparar e cria projéteis chamando magias_spawnar().
 * Chamada pela engine uma vez por frame. */
void magias_tipos_processar_auto_fire(EstadoJogo *ej);

#endif /* MAGIAS_TIPOS_H */
