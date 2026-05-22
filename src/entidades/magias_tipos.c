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
 * Um unico cooldown global entre disparos + round-robin entre os 3 mods da
 * profecia: cada chamada dispara no maximo UMA magia. Apos cada disparo, o
 * cooldown global = intervalo_disparo da magia que acabou de sair (sofre o
 * fator de Reduz Cooldown se estiver ativo). Assim:
 *   - 3 mods iguais  -> cadencia constante daquela magia.
 *   - 2 iguais + 1   -> cicla; 2/3 dos disparos usam o intervalo da repetida.
 *   - 3 diferentes   -> cooldown varia conforme cicla pelos 3 elementos.
 *
 * O estado vive em ej->motor_profecia.cooldown_global_disparo (timer) e
 * proximo_slot_disparo (indice 0..2). Ambos resetam junto com a run via
 * jogo_resetar_run (memset implicito em MotorProfecia = {0}).
 *
 * Se o slot atual nao tem alvo (mirar retorna false), a gente avanca o indice
 * e tenta o proximo no mesmo frame — assim um slot "azarado" nao trava o
 * disparo. Se nenhum dos 3 acha alvo, cooldown fica em 0 e o proximo frame
 * tenta de novo (atira assim que um inimigo aparece).
 * --------------------------------------------------------------------------*/

/* Quanto o cooldown encolhe enquanto o efeito Reduz Cooldown da profecia
 * está ativo. Tunável: 1.0 = sem efeito, 0.5 = dobro de cadência. */
static const float FATOR_REDUZ_COOLDOWN = 0.5f;

void magias_tipos_processar_auto_fire(EstadoJogo *ej) {
    /* Q toggle global: se o jogador desligou os tiros, sai sem mexer no
     * cooldown (retoma de onde parou quando religar). */
    if (!ej->tiros_ativos) return;

    MotorProfecia *mp = &ej->motor_profecia;
    mp->cooldown_global_disparo -= ej->delta_tempo;
    if (mp->cooldown_global_disparo > 0.0f) return;

    /* Tenta ate os 3 slots no mesmo frame; primeiro com alvo dispara. */
    for (int tentativa = 0; tentativa < 3; tentativa++) {
        int slot = mp->proximo_slot_disparo;
        Elemento e = ej->profecia.mods[slot].elemento;
        if ((int)e < 0 || (int)e >= QTD_PARAMETROS_MAGIA) {
            e = ELEMENTO_ARCANO;   /* fallback seguro */
        }

        if (magias_disparar_elemento(ej, e)) {
            float intervalo = PARAMETROS_MAGIA[e].intervalo_disparo;
            if (mp->reduz_cooldown_tempo > 0.0f) {
                intervalo *= FATOR_REDUZ_COOLDOWN;
            }
            mp->cooldown_global_disparo = intervalo;
            mp->proximo_slot_disparo = (slot + 1) % 3;
            return;
        }

        /* Slot sem alvo: avanca o indice e tenta o proximo neste mesmo frame. */
        mp->proximo_slot_disparo = (slot + 1) % 3;
    }

    /* Nenhum slot achou alvo: deixa o cooldown em 0 (dispara assim que aparecer). */
    mp->cooldown_global_disparo = 0.0f;
}
