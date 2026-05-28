/* ============================================================================
 * cronograma_eventos.c - TIMELINE DE 5 MINUTOS (LUÍSA EDITA AQUI)
 * ============================================================================
 *
 * Esta é a "linha do tempo" da run. Cada linha da tabela diz:
 *   "Do tempo X ao tempo Y, spawnar inimigos do tipo T a cada Z segundos".
 *
 * A engine (cronograma.c) lê essa tabela e faz o resto: copia pra um buffer
 * mutável, mantém timers, pede pros inimigos.c spawnarem na hora certa.
 *
 * >>> LUÍSA: a run agora dura 5:00 (300s) — o chefão entra aos 5:00 e a
 *     engine desliga todos os eventos. Os tempos abaixo já foram comprimidos
 *     proporcionalmente (era 15min, virou 5min: tudo /3). Ajuste fino do
 *     balanceamento (intervalos, quando cada tipo entra, densidade) é seu:
 *     mexa SÓ nesta tabela. Mantenha o último tempo_fim <= 300s e lembre
 *     que múltiplos eventos podem rodar juntos (somam pressão). <<<
 *
 * ----------------------------------------------------------------------------
 * COMO ADICIONAR UMA NOVA FASE NA TIMELINE:
 *   1. Acrescentar uma linha na tabela abaixo, no formato:
 *        { tempo_inicio, tempo_fim, TIPO, intervalo_spawn, 0.0f, false }
 *   2. Os campos finais (timer_interno, ativo) ficam zerados — a engine
 *      preenche em runtime quando copia a tabela.
 *   3. Tempos em SEGUNDOS. Pra "minuto 5" use 5 * 60 = 300.
 *   4. Pra um spawn ÚNICO (chefão), use intervalo_spawn = 1e9 ou só confie
 *      no fato de que o evento desativa quando passa do tempo_fim.
 *
 * REGRAS DE OURO:
 *   - O CHEFÃO (INIMIGO_CHEFE) é spawnado pela engine automaticamente aos
 *     15:00. NÃO precisa ter linha pra ele aqui — mas se quiser sub-bosses
 *     em outros tempos, a tabela aceita.
 *   - Múltiplos eventos podem rodar ao mesmo tempo. Querer melee + ranged
 *     juntos a partir dos 2:00? Duas linhas, ambas começando em 120.
 *
 * INFINITY (math.h) marca "evento sem fim" — usado quando você quer que o
 * spawn dure até o chefão aparecer e parar tudo.
 * ============================================================================ */

#include "cronograma_eventos.h"
#include <math.h>      /* INFINITY */

const EventoCronograma EVENTOS_CRONOGRAMA[] = {
    /* tempo_inicio, tempo_fim,   tipo,                  intervalo, timer, ativo */

    /* 0:00 — corpo a corpo desde o início, baixa pressão. */
    { 0.0f,          5.0f * 60.0f, INIMIGO_CORPO_A_CORPO, 1.5f,      0.0f,  false },

    /* 0:40 — entram os ranged. Player precisa começar a se mover mais. */
    { 40.0f,         5.0f * 60.0f, INIMIGO_A_DISTANCIA,   3.0f,      0.0f,  false },

    /* 1:40 — elites começam a aparecer (raros). */
    { 100.0f,        5.0f * 60.0f, INIMIGO_ELITE,         8.0f,      0.0f,  false },

    /* 2:40 — melee acelera (sobreposto à linha de cima — fica mais denso). */
    { 160.0f,        5.0f * 60.0f, INIMIGO_CORPO_A_CORPO, 0.8f,      0.0f,  false },

    /* 3:20 — elites ficam mais frequentes. Caos cresce na reta final. */
    { 200.0f,        5.0f * 60.0f, INIMIGO_ELITE,         5.0f,      0.0f,  false },

    /* 4:00 — ranged também acelera, último minuto antes do chefão. */
    { 240.0f,        5.0f * 60.0f, INIMIGO_A_DISTANCIA,   1.5f,      0.0f,  false },
};

const int QTD_EVENTOS_CRONOGRAMA =
    sizeof(EVENTOS_CRONOGRAMA) / sizeof(EVENTOS_CRONOGRAMA[0]);
