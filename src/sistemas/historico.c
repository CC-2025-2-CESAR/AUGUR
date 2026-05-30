/* ============================================================================
 * historico.c - LISTA DE SEEDS RECENTES + RENDER
 * ============================================================================
 *
 * Inserção é shift O(n) com n=10 — trivial pra esse tamanho. Mantém o array
 * sempre ordenado: índice 0 é a entrada mais recente.
 *
 * Render usa o mesmo padrão visual do leaderboard: título + cursor "> " na
 * linha selecionada, cor verde pra VITORIA e vermelho pra DERROTA.
 * ============================================================================ */

#include "historico.h"
#include "raylib.h"
#include <stdio.h>


void historico_registrar(DadosSalvos *ds,
                         unsigned int seed, bool venceu,
                         float tempo_seg, int pontuacao) {
    if (!ds) return;

    /* Shift do array pra abrir espaço no índice 0. A entrada mais antiga
     * (índice HISTORICO_SEEDS_TAM-1) cai do final automaticamente. */
    for (int i = HISTORICO_SEEDS_TAM - 1; i > 0; i--) {
        ds->historico[i] = ds->historico[i - 1];
    }

    ds->historico[0].seed           = seed;
    ds->historico[0].venceu         = venceu;
    ds->historico[0].tempo_segundos = tempo_seg;
    ds->historico[0].pontuacao      = pontuacao;
    ds->historico[0].ocupado        = true;

    if (ds->historico_qtd < HISTORICO_SEEDS_TAM) {
        ds->historico_qtd++;
    }
}


bool historico_tem_entradas(const DadosSalvos *ds) {
    return ds != NULL && ds->historico_qtd > 0;
}


/* Formata o tempo em "M:SS" no buffer dado. */
static void formatar_tempo(float seg, char *buf, int tam) {
    int total = (int)seg;
    int m = total / 60;
    int s = total % 60;
    snprintf(buf, tam, "%d:%02d", m, s);
}


void historico_desenhar(const DadosSalvos *ds, int cursor) {
    DrawText("HISTORICO", LARGURA_TELA / 2 - 130, 60, 50, GOLD);
    DrawText("Ultimas seeds jogadas (mais recente em cima)",
             LARGURA_TELA / 2 - 280, 130, 20, GRAY);

    if (!ds || ds->historico_qtd == 0) {
        DrawText("Nenhuma run registrada ainda",
                 LARGURA_TELA / 2 - 180, 280, 22, LIGHTGRAY);
        DrawText("[ESC] voltar",
                 LARGURA_TELA / 2 - 60, ALTURA_TELA - 50, 18, DARKGRAY);
        return;
    }

    /* Cabeçalho da tabela. */
    int x_seed   = 100;
    int x_resul  = 360;
    int x_tempo  = 600;
    int x_biom   = 800;
    int y_topo   = 180;
    int linha_h  = 32;

    DrawText("#",        x_seed - 50, y_topo, 18, GRAY);
    DrawText("Seed",     x_seed,      y_topo, 18, GRAY);
    DrawText("Resultado",x_resul,     y_topo, 18, GRAY);
    DrawText("Tempo",    x_tempo,     y_topo, 18, GRAY);
    DrawText("Biomassa", x_biom,      y_topo, 18, GRAY);

    /* Linhas: uma entrada por linha. */
    for (int i = 0; i < ds->historico_qtd && i < HISTORICO_SEEDS_TAM; i++) {
        const EntradaHistorico *e = &ds->historico[i];
        int y = y_topo + 30 + i * linha_h;

        Color cor_linha = (i == cursor) ? GOLD : LIGHTGRAY;
        const char *prefixo = (i == cursor) ? "> " : "  ";

        /* Coluna #: numerada de 1 a N. */
        char idx[24];
        snprintf(idx, sizeof(idx), "%s%d.", prefixo, i + 1);
        DrawText(idx, x_seed - 50, y, 22, cor_linha);

        /* Coluna seed: número grande. */
        char seed_buf[32];
        snprintf(seed_buf, sizeof(seed_buf), "%u", e->seed);
        DrawText(seed_buf, x_seed, y, 22, cor_linha);

        /* Coluna resultado: V verde ou D vermelho. */
        Color cor_resul = e->venceu
            ? (Color){ 100, 220, 120, 255 }
            : (Color){ 220, 100, 100, 255 };
        DrawText(e->venceu ? "VITORIA" : "DERROTA",
                 x_resul, y, 22, cor_resul);

        /* Coluna tempo. */
        char tempo_buf[16];
        formatar_tempo(e->tempo_segundos, tempo_buf, sizeof(tempo_buf));
        DrawText(tempo_buf, x_tempo, y, 22, cor_linha);

        /* Coluna biomassa. */
        char biom_buf[24];
        snprintf(biom_buf, sizeof(biom_buf), "%d", e->pontuacao);
        DrawText(biom_buf, x_biom, y, 22, cor_linha);
    }

    DrawText("[UP/DOWN] navegar   [ENTER] carregar seed   [ESC] voltar",
             LARGURA_TELA / 2 - 330, ALTURA_TELA - 50, 18, DARKGRAY);
}
