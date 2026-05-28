/* ============================================================================
 * leaderboard.c - implementação dos top-10
 * ============================================================================
 *
 * Insertion-sort O(n²) é trivial com 10 slots. Cada inserção:
 *   1. Acha a posição certa percorrendo a lista do início até achar slot
 *      onde a nova entrada é melhor (ou um slot vazio).
 *   2. Desloca o resto pra direita.
 *   3. Escreve a nova entrada na posição achada. Quem cair fora do array é
 *      descartado (top-10 estrito).
 *
 * Critério de ordenação:
 *   - top_tempo:    venceu=true E (tempo crescente). Quem não venceu é ignorado.
 *   - top_biomassa: pontuação decrescente. Aceita venceu=true e false.
 * ============================================================================ */

#include "leaderboard.h"
#include "raylib.h"
#include <stdio.h>      /* snprintf */
#include <string.h>     /* memmove */


/* --- helpers de inserção em array ordenado ----------------------------- */

/* Insere `nova` em `tabela` na posição `idx`, deslocando o resto. Quem
 * estiver no último slot é descartado (queda da tabela). */
static void inserir_em(EntradaLeaderboard *tabela, int idx, EntradaLeaderboard nova) {
    /* Desloca tabela[idx..TAM-2] uma posição pra direita. memmove lida com
     * overlap, diferente de memcpy. */
    if (idx < LEADERBOARD_TAM - 1) {
        memmove(&tabela[idx + 1], &tabela[idx],
                sizeof(EntradaLeaderboard) * (size_t)(LEADERBOARD_TAM - 1 - idx));
    }
    tabela[idx] = nova;
}

/* Atualiza top_tempo: tempo crescente, só vitórias. */
static void registrar_no_top_tempo(EntradaLeaderboard *t, EntradaLeaderboard nova) {
    if (!nova.venceu) return;   /* só vitórias entram aqui */

    for (int i = 0; i < LEADERBOARD_TAM; i++) {
        /* Slot vazio: usa direto. */
        if (!t[i].ocupado) {
            t[i] = nova;
            return;
        }
        /* Slot ocupado com tempo PIOR (maior) que a nova entrada: insere
         * aqui e empurra o resto pra trás. */
        if (nova.tempo_segundos < t[i].tempo_segundos) {
            inserir_em(t, i, nova);
            return;
        }
    }
    /* Caiu fora do top-10. Descarta. */
}

/* Atualiza top_biomassa: pontuação decrescente, aceita qualquer run. */
static void registrar_no_top_biomassa(EntradaLeaderboard *t, EntradaLeaderboard nova) {
    for (int i = 0; i < LEADERBOARD_TAM; i++) {
        if (!t[i].ocupado) {
            t[i] = nova;
            return;
        }
        if (nova.pontuacao > t[i].pontuacao) {
            inserir_em(t, i, nova);
            return;
        }
    }
}


void leaderboard_registrar(DadosSalvos *ds,
                           int pontuacao, float tempo_seg,
                           unsigned int seed, bool venceu) {
    if (!ds) return;

    EntradaLeaderboard nova = {
        .pontuacao      = pontuacao,
        .tempo_segundos = tempo_seg,
        .seed           = seed,
        .venceu         = venceu,
        .ocupado        = true,
    };

    registrar_no_top_tempo(ds->top_tempo, nova);
    registrar_no_top_biomassa(ds->top_biomassa, nova);
}


/* --- desenho da tela de leaderboard ------------------------------------ */

static void desenhar_linha(int idx, const EntradaLeaderboard *e,
                           int x, int y, bool mostrar_resultado) {
    char buf[160];
    if (!e->ocupado) {
        snprintf(buf, sizeof(buf), "%2d.  --", idx + 1);
        DrawText(buf, x, y, 20, DARKGRAY);
        return;
    }

    int min = (int)(e->tempo_segundos / 60.0f);
    int seg = (int)e->tempo_segundos % 60;

    if (mostrar_resultado) {
        /* Tabela de biomassa: linha = "pos pontos tempo seed [VITORIA|DERROTA]" */
        snprintf(buf, sizeof(buf),
                 "%2d.  %6d pts   %02d:%02d   seed %-10u   %s",
                 idx + 1, e->pontuacao, min, seg, e->seed,
                 e->venceu ? "VITORIA" : "DERROTA");
    } else {
        /* Tabela de tempo: linha = "pos tempo pontos seed" (já é só vitória) */
        snprintf(buf, sizeof(buf),
                 "%2d.  %02d:%02d   %6d pts   seed %-10u",
                 idx + 1, min, seg, e->pontuacao, e->seed);
    }

    Color cor = (idx < 3) ? GOLD : LIGHTGRAY;
    DrawText(buf, x, y, 20, cor);
}


void leaderboard_desenhar(const DadosSalvos *ds, bool aba_biomassa) {
    if (!ds) return;

    int x = LARGURA_TELA / 2 - 360;
    int y = 100;

    DrawText("LEADERBOARD", LARGURA_TELA / 2 - 140, 40, 40, GOLD);

    /* Abas: a ativa em GOLD, a outra em DARKGRAY. */
    const char *txt_tempo    = "[TAB] TEMPO (vitorias)";
    const char *txt_biomassa = "[TAB] BIOMASSA (todas)";
    DrawText(txt_tempo,    x,       y, 22, aba_biomassa ? DARKGRAY : GOLD);
    DrawText(txt_biomassa, x + 380, y, 22, aba_biomassa ? GOLD     : DARKGRAY);

    y += 50;

    const EntradaLeaderboard *tabela = aba_biomassa ? ds->top_biomassa : ds->top_tempo;

    int linhas_mostradas = 0;
    for (int i = 0; i < LEADERBOARD_TAM; i++) {
        desenhar_linha(i, &tabela[i], x, y, aba_biomassa);
        y += 30;
        linhas_mostradas++;
    }

    if (linhas_mostradas == 0) {
        DrawText("(vazio)", x, y, 20, DARKGRAY);
    }

    DrawText("ESC para voltar ao menu",
             LARGURA_TELA / 2 - 160, ALTURA_TELA - 50, 20, GRAY);
}
