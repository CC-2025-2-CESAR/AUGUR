/* ============================================================================
 * magias_tipos.c - TABELA DE MAGIAS + AUTO-FIRE (LUÍSA EDITA AQUI)
 * ============================================================================
 *
 * Os números abaixo são chutes razoáveis pra começar — ajuste no playtest.
 * Pra mudar a frequência ou força de uma magia, mexa só na tabela; pra
 * mudar o jeito que o jogador atira (ex.: spread em leque), mexa em
 * magias_tipos_processar_auto_fire.
 * ============================================================================ */

#include "magias_tipos.h"
#include "magias.h"     /* magias_spawnar — fornecida pela engine */
#include <math.h>


/* ----------------------------------------------------------------------------
 * 1. TABELA DE STATS POR ELEMENTO
 * ----------------------------------------------------------------------------
 * A ordem das linhas casa com a ordem do enum Elemento:
 *   [0] ELEMENTO_FOGO
 *   [1] ELEMENTO_GELO
 *   [2] ELEMENTO_RELAMPAGO
 *   [3] ELEMENTO_VENENO
 *   [4] ELEMENTO_ARCANO
 *   [5] ELEMENTO_SOMBRA
 * --------------------------------------------------------------------------*/
const ParametrosMagia PARAMETROS_MAGIA[] = {
    /* 🔴 fogo — dano alto, projétil médio. */ 
    {
        .dano_base           = 18.0f,
        .velocidade_projetil = 480.0f,
        .tempo_de_vida       = 1.5f,
        .raio_projetil       = 7.0f,
        .intervalo_disparo   = 0.45f,
        .cor                 = (Color){ 255, 110,  40, 255 },
    },

    /* 🔵 gelo — leve, mais lento, fica vivo um pouco mais. */
    {
        .dano_base           = 12.0f,
        .velocidade_projetil = 380.0f,
        .tempo_de_vida       = 1.8f,
        .raio_projetil       = 8.0f,
        .intervalo_disparo   = 0.55f,
        .cor                 = (Color){ 130, 200, 255, 255 },
    },

    /* 🟡 relâmpago — rápido, dano médio, cooldown curto. */
    {
        .dano_base           = 14.0f,
        .velocidade_projetil = 600.0f,
        .tempo_de_vida       = 1.2f,
        .raio_projetil       = 6.0f,
        .intervalo_disparo   = 0.30f,
        .cor                 = (Color){ 245, 215,  90, 255 },
    },

    /* 🟢 veneno — dano baixo, mas continua vivo bastante (ideia: DoT futuro). */
    {
        .dano_base           = 10.0f,
        .velocidade_projetil = 350.0f,
        .tempo_de_vida       = 2.5f,
        .raio_projetil       = 9.0f,
        .intervalo_disparo   = 0.50f,
        .cor                 = (Color){ 110, 200,  90, 255 },
    },

    /* 🔘 arcano — equilibrado, magia "default". */
    {
        .dano_base           = 15.0f,
        .velocidade_projetil = 460.0f,
        .tempo_de_vida       = 1.5f,
        .raio_projetil       = 7.0f,
        .intervalo_disparo   = 0.40f,
        .cor                 = (Color){ 85, 100, 145, 255 },
    },

    /* 🟣 sombra — alto dano, baixa cadência. */
    {
        .dano_base           = 22.0f,
        .velocidade_projetil = 420.0f,
        .tempo_de_vida       = 1.4f,
        .raio_projetil       = 7.0f,
        .intervalo_disparo   = 0.65f,
        .cor                 = (Color){ 140,  80, 200, 255 },
    },
};

const int QTD_PARAMETROS_MAGIA =
    sizeof(PARAMETROS_MAGIA) / sizeof(PARAMETROS_MAGIA[0]);


/* ----------------------------------------------------------------------------
 * 2. AUTO-FIRE
 * ----------------------------------------------------------------------------
 * A cada frame processa 4 slots de disparo: os 3 elementos dos mods da
 * profecia + a magia inicial escolhida pelo jogador. Round-robin entre os
 * slots; cada slot dispara seu elemento no inimigo mais próximo, com
 * cadência própria.
 *
 * Por que 4 slots? A "Escolha da Magia Inicial" do GDD permite ao jogador
 * adicionar um 4º elemento ao leque sem alterar a profecia. Se ele escolhe
 * um elemento já presente em algum mod, dois slots disparam o mesmo
 * elemento — maior cadência daquele elemento e mais chance de ativar os
 * efeitos da profecia que combinam com ele (sinergia).
 *
 * Sem alvo, magias_disparar_elemento retorna false e o ciclo testa o próximo
 * slot — atira assim que um inimigo aparece.
 * --------------------------------------------------------------------------*/

#define AUTO_FIRE_SLOTS 4

/* Devolve o elemento associado ao slot do auto-fire:
 *   slots 0..2 = elementos dos mods da profecia
 *   slot 3     = magia inicial escolhida (ou Arcano default se ainda nao foi escolhida)
 */
static Elemento elemento_do_slot(const EstadoJogo *ej, int slot) {
    if (slot < 3) return ej->profecia.mods[slot].elemento;
    if (ej->magia_inicial_definida) return ej->magia_inicial_escolhida;
    return ELEMENTO_ARCANO;
}


void magias_tipos_processar_auto_fire(EstadoJogo *ej) {
    if (!ej->tiros_ativos) return;

    MotorProfecia *mp = &ej->motor_profecia;
    mp->cooldown_global_disparo -= ej->delta_tempo;

    if (mp->cooldown_global_disparo > 0.0f) return;

    /* Tenta cada slot uma vez, em ordem, até achar um que dispare. */
    for (int tentativa = 0; tentativa < AUTO_FIRE_SLOTS; tentativa++) {
        int slot   = mp->prox_slot_disparo;
        Elemento e = elemento_do_slot(ej, slot);

        if ((int)e < 0 || (int)e >= QTD_PARAMETROS_MAGIA) e = ELEMENTO_ARCANO;

        if (magias_disparar_elemento(ej, e)) {
            mp->cooldown_global_disparo = PARAMETROS_MAGIA[e].intervalo_disparo;
            mp->prox_slot_disparo = (slot + 1) % AUTO_FIRE_SLOTS;
            return;
        }

        mp->prox_slot_disparo = (slot + 1) % AUTO_FIRE_SLOTS;
    }

    mp->cooldown_global_disparo = 0.0f;
}
