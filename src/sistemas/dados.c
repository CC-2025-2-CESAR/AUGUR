#include <stdio.h>
#include <stdlib.h>
#include "dados.h"
#include "raylib.h"

#define DADO_TAMANHO  40
#define DADO_FONTE    18
#define DADO_FONTE_SM 12

int dado_rolar(Dado *d) {
    if (d == NULL || d->faces <= 0)
        return 0;

    int resultado = (rand() % d->faces) + 1;
    d->ultimo_resultado = resultado;
    return resultado;
}

void dado_desenhar(const Dado *d, int posicao_x, int posicao_y) {
    if (d == NULL) return;

    bool gasto = (d->ultimo_resultado > 0);

    Color cor_borda;
    Color cor_fundo;

    if (gasto) {
        cor_borda = DARKGRAY;
        cor_fundo = (Color){ 40, 40, 40, 220 };
    } 
    else {
        cor_borda = WHITE;
        cor_fundo = (Color){ 20, 20, 60, 220 };
    }

    DrawRectangle(posicao_x, posicao_y, DADO_TAMANHO, DADO_TAMANHO, cor_fundo);
    DrawRectangleLines(posicao_x, posicao_y, DADO_TAMANHO, DADO_TAMANHO, cor_borda);

    char buf[16];

    if (!gasto) {
        snprintf(buf, sizeof(buf), "d%d", d->faces);
        int larg = MeasureText(buf, DADO_FONTE);
        DrawText(buf,
                posicao_x + (DADO_TAMANHO - larg) / 2,
                posicao_y + (DADO_TAMANHO - DADO_FONTE) / 2,
                DADO_FONTE, WHITE);
    } else {
        snprintf(buf, sizeof(buf), "%d", d->ultimo_resultado);
        int larg = MeasureText(buf, DADO_FONTE);
        DrawText(buf,
                posicao_x + (DADO_TAMANHO - larg) / 2,
                posicao_y + 4,
                DADO_FONTE, LIGHTGRAY);

        const char *leg = "usado";
        int larg_leg = MeasureText(leg, DADO_FONTE_SM);
        DrawText(leg,
                posicao_x + (DADO_TAMANHO - larg_leg) / 2,
                posicao_y + DADO_TAMANHO - DADO_FONTE_SM - 4,
                DADO_FONTE_SM, DARKGRAY);
    }
}


bool dado_esta_carregado(const Dado *d) {
    return (d != NULL && d->ultimo_resultado == 0);
}


void dado_aplicar_na_carta(int resultado, int faces, Carta *carta) {
    if (carta == NULL || faces <= 0 || resultado <= 0)
        return;

    if (carta->tipo == CARTA_RECARGA_DADO)
        return;

    float multiplicador;
    char *sufixo = "";

    if (resultado <= faces / 3) {
        multiplicador = 0.5f;
        sufixo = " (ENFRAQUECIDO)";
    } 
    else if (resultado <= (2 * faces) / 3) {
        multiplicador = 1.0f;
        sufixo = "";
    } 
    else if (resultado < faces) {
        multiplicador = 1.5f;
        sufixo = " (MELHORADO)";
    } 
    else {
        multiplicador = 2.0f;
        sufixo = " (CRITICO!)";
    }

    int novo_valor = (int)(carta->valor * multiplicador);
    if (novo_valor < 1) novo_valor = 1;
    carta->valor = novo_valor;

    if (carta->tipo == CARTA_DANO_UP) {
        snprintf(carta->descricao, 256, "Aumenta o dano das suas magias.\n+%d de dano.%s", novo_valor, sufixo);
    } 
    else if (carta->tipo == CARTA_VIDA_UP) {
        snprintf(carta->descricao, 256, "Aumenta seu HP maximo.\n+%d de HP.%s", novo_valor, sufixo);
    } 
    else if (carta->tipo == CARTA_VELOCIDADE_UP) {
        snprintf(carta->descricao, 256, "Aumenta sua velocidade de movimento.\n+%d de velocidade.%s", novo_valor, sufixo);
    }
}