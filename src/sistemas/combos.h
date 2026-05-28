/* ============================================================================
 * combos.h - COMBOS EMERGENTES (engine — Arthur)
 * ============================================================================
 *
 * Combos não aparecem em nenhum tutorial: o jogador descobre por experimento.
 * A MECÂNICA fica aqui (engine); os NÚMEROS ficam em profecia_efeitos.c
 * (Luísa tuna: janela, stun, multiplicadores, ligar/desligar via valores).
 *
 *   - Choque Térmico (Fogo -> Gelo): inimigo atingido por Fogo recebe uma
 *     "marca térmica" por alguns segundos. Se for atingido por Gelo dentro
 *     da janela: stun + a PRÓXIMA hit nele é amplificada.
 *
 *   - Corrente Arcana + Nuvem Venenosa: adaptado ao modelo "projétil +
 *     status" (não há campo de chão). Regra: Arcano causa dano multiplicado
 *     em inimigo que esteja envenenado. Custo zero, decidido no próprio hit
 *     (a alternativa "cooldown pela metade" exigiria varrer a lista por
 *     frame no auto-fire).
 * ============================================================================ */

#ifndef COMBOS_H
#define COMBOS_H

#include "tipos.h"

/* Multiplicador de dano do combo pra ESTE hit (Arcano x envenenado).
 * Retorna 1.0 quando nenhum combo de dano se aplica. */
float combos_multiplicador_dano(const EstadoJogo *ej,
                                Elemento elemento,
                                const Inimigo *alvo);

/* Pós-hit: marca/dispara o Choque Térmico conforme o elemento. */
void combos_aplicar_no_acerto(EstadoJogo *ej,
                              Elemento elemento,
                              Inimigo *alvo);

#endif /* COMBOS_H */
