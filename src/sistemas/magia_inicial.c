/* ============================================================================
 * magia_inicial.c - SORTEIO + RENDER DAS 3 OPÇÕES DE MAGIA INICIAL
 * ============================================================================
 *
 * Sorteia 3 opções (elemento + raridade + nome) e garante sinergia (>=1
 * elemento bate com algum mod da profecia). O auto-fire em magias_tipos.c
 * adiciona o elemento escolhido como 4º slot do ciclo round-robin.
 *
 * Tabela de nomes: 6 elementos × 6 raridades = 36 entradas. Strings sem
 * acento porque vão pra DrawText.
 * ============================================================================ */

#include "magia_inicial.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* ----------------------------------------------------------------------------
 * TABELA DE NOMES
 * ----------------------------------------------------------------------------
 * Indexada por [elemento][raridade]. Sem acento (DrawText/Raylib é ASCII).
 * Os nomes ganham peso conforme a raridade sobe — comum é direto, lendária é
 * mais épico. Adapta sem dor se quiser temas diferentes depois.
 * ----------------------------------------------------------------------------*/
static const char *NOMES_MAGIA[ELEMENTO_TOTAL][6] = {
    /* FOGO */ {
        "Bola de Fogo",
        "Chama Ardente",
        "Lanca Flamejante",
        "Cometa de Fogo",
        "Inferno Voraz",
        "Sol Esquecido",
    },
    /* GELO */ {
        "Lanca de Gelo",
        "Estilhaco Gelido",
        "Flecha Polar",
        "Tempestade Gelida",
        "Lamina Glacial",
        "Coracao do Inverno",
    },
    /* RELAMPAGO */ {
        "Faisca",
        "Relampago Vivo",
        "Raio Trovejante",
        "Tempestade Eletrica",
        "Trovao Antigo",
        "Furia de Zeus",
    },
    /* VENENO */ {
        "Espinho Toxico",
        "Veneno Acido",
        "Maldicao Venenosa",
        "Toxina Letal",
        "Sopro Pestilento",
        "Praga da Cobra",
    },
    /* ARCANO */ {
        "Pulso Arcano",
        "Dardo Mistico",
        "Bola Arcana",
        "Lamina Etereal",
        "Selo do Mago",
        "Decreto Cosmico",
    },
    /* SOMBRA */ {
        "Lasca Sombria",
        "Garra Umbral",
        "Cega-Trevas",
        "Manto da Noite",
        "Eclipse",
        "Vazio Devorador",
    },
};


int magia_inicial_sortear_raridade(void) {
    int s = rand() % 100;
    if (s < 55) return 0;   /* comum */
    if (s < 80) return 1;   /* incomum */
    if (s < 92) return 2;   /* rara */
    if (s < 96) return 3;   /* epica */
    if (s < 98) return 4;   /* mitica */
    return 5;               /* lendaria */
}


void magia_inicial_nomear(OpcaoMagiaInicial *m) {
    if (!m) return;
    int e = (int)m->elemento;
    int r = m->raridade;
    if (e < 0 || e >= ELEMENTO_TOTAL) e = ELEMENTO_ARCANO;
    if (r < 0) r = 0;
    if (r > 5) r = 5;
    strncpy(m->nome, NOMES_MAGIA[e][r], sizeof(m->nome) - 1);
    m->nome[sizeof(m->nome) - 1] = '\0';
}


void magia_inicial_sortear_opcoes(EstadoJogo *ej) {
    if (!ej) return;

    /* Coleta os elementos da profecia pra checar sinergia depois. */
    Elemento elementos_prof[3] = {
        ej->profecia.mods[0].elemento,
        ej->profecia.mods[1].elemento,
        ej->profecia.mods[2].elemento,
    };

    /* Sorteio puro: elemento + raridade aleatórios. */
    for (int i = 0; i < 3; i++) {
        ej->opcoes_magia[i].elemento    = (Elemento)(rand() % ELEMENTO_TOTAL);
        ej->opcoes_magia[i].raridade    = magia_inicial_sortear_raridade();
        ej->opcoes_magia[i].sinergetica = false;
        magia_inicial_nomear(&ej->opcoes_magia[i]);
    }

    /* Marca as que casam com algum mod da profecia. */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (ej->opcoes_magia[i].elemento == elementos_prof[j]) {
                ej->opcoes_magia[i].sinergetica = true;
                break;
            }
        }
    }

    /* Garantia: se nenhuma das 3 ficou sinergética, troca a primeira por uma
     * que use o elemento de um mod aleatório da profecia, mantendo a raridade
     * sorteada. */
    bool tem = false;
    for (int i = 0; i < 3; i++) if (ej->opcoes_magia[i].sinergetica) { tem = true; break; }
    if (!tem) {
        int j = rand() % 3;
        ej->opcoes_magia[0].elemento    = elementos_prof[j];
        ej->opcoes_magia[0].sinergetica = true;
        magia_inicial_nomear(&ej->opcoes_magia[0]);
    }

    ej->opcao_magia_selecionada = 0;
}


/* ----------------------------------------------------------------------------
 * RENDER
 * ----------------------------------------------------------------------------
 * 3 cards lado a lado, no estilo de cartas.c. Cor de fundo do card vem do
 * elemento (puxa de PARAMETROS_MAGIA via tabela própria embaixo, evita
 * dependência circular com magias_tipos.c). Borda dourada extra na opção
 * sinergética e na selecionada (cursor).
 * ----------------------------------------------------------------------------*/

#define LARGURA_CARD   260
#define ALTURA_CARD    340
#define GAP_CARDS      320

/* Cor associada a cada elemento — espelha as cores de PARAMETROS_MAGIA em
 * magias_tipos.c, mas guardamos uma cópia aqui pra evitar dependência. */
static const Color CORES_ELEMENTO[ELEMENTO_TOTAL] = {
    { 255, 110,  40, 255 },   /* FOGO */
    { 130, 200, 255, 255 },   /* GELO */
    { 245, 215,  90, 255 },   /* RELAMPAGO */
    { 110, 200,  90, 255 },   /* VENENO */
    {  85, 100, 145, 255 },   /* ARCANO */
    { 140,  80, 200, 255 },   /* SOMBRA */
};

static const Color CORES_RARIDADE[6] = {
    GRAY, GREEN, BLUE, PURPLE, RED, GOLD
};

static const char *LABEL_RARIDADE[6] = {
    "Comum", "Incomum", "Raro", "Epico", "Mitico", "Lendario"
};


/* Nome legível do elemento (espelho de profecia.c, sem dependência). */
static const char *nome_elemento_curto(Elemento e) {
    switch (e) {
        case ELEMENTO_FOGO:      return "Fogo";
        case ELEMENTO_GELO:      return "Gelo";
        case ELEMENTO_RELAMPAGO: return "Relampago";
        case ELEMENTO_VENENO:    return "Veneno";
        case ELEMENTO_ARCANO:    return "Arcano";
        case ELEMENTO_SOMBRA:    return "Sombra";
        default: return "?";
    }
}


void magia_inicial_desenhar(const EstadoJogo *ej, int selecionado) {
    DrawText("ESCOLHA SUA MAGIA INICIAL",
             LARGURA_TELA / 2 - 320, 50, 40, GOLD);
    DrawText("A profecia te guia - uma das opcoes ressoa com ela",
             LARGURA_TELA / 2 - 320, 110, 20, GRAY);

    int x0 = LARGURA_TELA / 2 - (3 * LARGURA_CARD + 2 * (GAP_CARDS - LARGURA_CARD)) / 2;
    int y0 = 180;

    for (int i = 0; i < 3; i++) {
        const OpcaoMagiaInicial *m = &ej->opcoes_magia[i];
        int x = x0 + i * GAP_CARDS;

        Color cor_elem = CORES_ELEMENTO[m->elemento];
        Color cor_rar  = CORES_RARIDADE[m->raridade];

        /* Borda dourada extra se for sinergética (visual fixo, sempre destacada). */
        if (m->sinergetica) {
            DrawRectangle(x - 8, y0 - 8, LARGURA_CARD + 16, ALTURA_CARD + 16, GOLD);
        }
        /* Cursor de seleção: borda branca em cima. */
        if (i == selecionado) {
            DrawRectangle(x - 4, y0 - 4, LARGURA_CARD + 8, ALTURA_CARD + 8, WHITE);
        }
        /* Borda externa por raridade. */
        DrawRectangle(x - 2, y0 - 2, LARGURA_CARD + 4, ALTURA_CARD + 4, cor_rar);
        /* Fundo preto. */
        DrawRectangle(x, y0, LARGURA_CARD, ALTURA_CARD, BLACK);

        /* Faixa de cor do elemento no topo do card. */
        DrawRectangle(x + 10, y0 + 12, LARGURA_CARD - 20, 32, cor_elem);
        DrawText(LABEL_RARIDADE[m->raridade], x + 14, y0 + 18, 20, BLACK);

        /* Nome da magia. */
        DrawText(m->nome, x + 14, y0 + 64, 22, cor_elem);

        /* Linha separadora. */
        DrawLine(x + 14, y0 + 100, x + LARGURA_CARD - 14, y0 + 100, DARKGRAY);

        /* Detalhes: elemento + label de sinergia. */
        char buf[64];
        snprintf(buf, sizeof(buf), "Elemento: %s", nome_elemento_curto(m->elemento));
        DrawText(buf, x + 14, y0 + 116, 18, LIGHTGRAY);

        if (m->sinergetica) {
            DrawText("RESSONANTE com a profecia",
                     x + 14, y0 + 145, 16, GOLD);
        } else {
            DrawText("Sem sinergia direta",
                     x + 14, y0 + 145, 16, GRAY);
        }

        /* Descrição genérica do elemento (espelho dos chutes em magias_tipos.c). */
        const char *desc;
        switch (m->elemento) {
            case ELEMENTO_FOGO:      desc = "Dano alto, cadencia\nmedia."; break;
            case ELEMENTO_GELO:      desc = "Lento, projetil\nlonge."; break;
            case ELEMENTO_RELAMPAGO: desc = "Rapido, dano medio,\ncadencia alta."; break;
            case ELEMENTO_VENENO:    desc = "Dano baixo direto,\nDoT no acerto."; break;
            case ELEMENTO_ARCANO:    desc = "Equilibrado.\nDano fixo, sem CC."; break;
            case ELEMENTO_SOMBRA:    desc = "Dano alto, cadencia\nbaixa."; break;
            default: desc = ""; break;
        }
        DrawText(desc, x + 14, y0 + 180, 16, LIGHTGRAY);

        /* Indicador de tecla. */
        char tecla[8];
        snprintf(tecla, sizeof(tecla), "[%d]", i + 1);
        DrawText(tecla, x + LARGURA_CARD / 2 - 12, y0 + ALTURA_CARD - 36, 20, WHITE);
    }

    DrawText("[<-/->] navegar    [1/2/3] selecionar direto    [ENTER] confirmar",
             LARGURA_TELA / 2 - 380, ALTURA_TELA - 60, 18, DARKGRAY);
}
