#include <stdio.h>
#include <string.h>
#include "dados.h"
#include <stdlib.h>
#include "raylib.h"

#define DADO_TAMANHO  40
#define DADO_FONTE    18
#define DADO_FONTE_SM 12

int dado_rolar(Dado *d) {
    if (d == NULL || d->faces <= 0) return 0;
    d->ultimo_resultado = (rand() % d->faces) + 1;
    return d->ultimo_resultado;
}

void dado_desenhar(const Dado *d, int posicao_x, int posicao_y) {
    if (d == NULL) return;

    Color cor_borda;
    switch (d->faces) {
        case 4:  cor_borda = ORANGE; break;
        case 6:  cor_borda = WHITE;  break;
        case 8:  cor_borda = GREEN;  break;
        case 10: cor_borda = BLUE;   break;
        case 12: cor_borda = PURPLE; break;
        case 20: cor_borda = GOLD;   break;
        default: cor_borda = GRAY;   break;
    }

    DrawRectangle(posicao_x, posicao_y, DADO_TAMANHO, DADO_TAMANHO, DARKGRAY);
    DrawRectangleLines(posicao_x, posicao_y, DADO_TAMANHO, DADO_TAMANHO, cor_borda);

    char label_tipo[8];
    snprintf(label_tipo, sizeof(label_tipo), "d%d", d->faces);
    DrawText(label_tipo, posicao_x + 3, posicao_y + 3, DADO_FONTE_SM, cor_borda);

    char label_resultado[8];
    if (d->ultimo_resultado <= 0){
        snprintf(label_resultado, sizeof(label_resultado), "?");
    }
    else{ 
        snprintf(label_resultado, sizeof(label_resultado), "%d", d->ultimo_resultado);
    }

    int larg_texto = MeasureText(label_resultado, DADO_FONTE);
    int centro_x   = posicao_x + (DADO_TAMANHO - larg_texto) / 2;
    int centro_y   = posicao_y + (DADO_TAMANHO - DADO_FONTE) / 2 + 4;

    DrawText(label_resultado, centro_x, centro_y, DADO_FONTE, WHITE);
}