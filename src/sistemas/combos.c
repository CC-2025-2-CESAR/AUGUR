/* ============================================================================
 * combos.c - IMPLEMENTAÇÃO DOS COMBOS EMERGENTES
 * ============================================================================
 *
 * Engine pura: lê só os números tunáveis de profecia_efeitos.c. A janela da
 * marca térmica decai no PASS 1 de inimigos.c (marca_termica_tempo).
 * ============================================================================ */

#include "combos.h"
#include "profecia_efeitos.h"


float combos_multiplicador_dano(const EstadoJogo *ej,
                                Elemento elemento,
                                const Inimigo *alvo) {
    (void)ej;
    /* Corrente Arcana + Veneno: Arcano arrebenta quem está envenenado. */
    if (elemento == ELEMENTO_ARCANO &&
        alvo->veneno_tempo > 0.0f &&
        MAGNITUDES_EFEITO.arcano_vs_envenenado_mult > 0.0f) {
        return MAGNITUDES_EFEITO.arcano_vs_envenenado_mult;
    }
    return 1.0f;
}


void combos_aplicar_no_acerto(EstadoJogo *ej,
                              Elemento elemento,
                              Inimigo *alvo) {
    (void)ej;
    const MagnitudesEfeito *M = &MAGNITUDES_EFEITO;

    if (elemento == ELEMENTO_FOGO) {
        /* Fogo deixa a marca térmica (renova a janela a cada hit de Fogo). */
        alvo->marca_termica_tempo = M->choque_janela_seg;
    } else if (elemento == ELEMENTO_GELO && alvo->marca_termica_tempo > 0.0f) {
        /* Choque Térmico: consome a marca, stuna e amplifica a próxima hit. */
        alvo->marca_termica_tempo = 0.0f;
        if (M->choque_stun_seg > alvo->congelado_tempo) {
            alvo->congelado_tempo = M->choque_stun_seg;
        }
        alvo->proxima_hit_multiplicador = M->choque_mult_proxima;
    }
}
