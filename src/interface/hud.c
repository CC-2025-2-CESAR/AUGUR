/* ============================================================================
 * hud.c - HUD DE COMBATE (Sofia / Dev 2)
 * ----------------------------------------------------------------------------
 * Desenha a interface fixa sobre o jogo: barra de vida, tempo da run (com
 * contagem regressiva pro chefão) e moeda de biomassa. desenhar_hud() junta os
 * três e é chamada uma vez por frame, depois do mundo. Strings em ASCII (a
 * fonte default do Raylib não renderiza acento).
 * ============================================================================ */

#include "hud.h"
#include "cronograma.h"
#include <stdio.h>
#include <math.h>

#define HUD_MARGEM         10
#define HUD_BARRA_LARG    200
#define HUD_BARRA_ALT      20
#define HUD_FONTE_PEQUENA  18
#define HUD_FONTE_MEDIA    20
#define HUD_FONTE_GRANDE   26


/* Barra de vida no canto superior esquerdo (verde/amarelo/vermelho por %). */
static void hud_desenhar_barra_vida(const EstadoJogo *ej) {
    int x = HUD_MARGEM, y = HUD_MARGEM;
    DrawRectangle(x, y, HUD_BARRA_LARG, HUD_BARRA_ALT, DARKGRAY);

    float pct = (ej->jogador.vida_maxima > 0)
        ? (float)ej->jogador.vida / ej->jogador.vida_maxima : 0.0f;
    int largura_vida = (int)(HUD_BARRA_LARG * pct);

    Color cor_vida = (pct >= 0.50f) ? GREEN : (pct > 0.30f) ? YELLOW : RED;
    DrawRectangle(x, y, largura_vida, HUD_BARRA_ALT, cor_vida);
    DrawRectangleLines(x, y, HUD_BARRA_LARG, HUD_BARRA_ALT, DARKGRAY);

    char buf[32];
    snprintf(buf, sizeof(buf), "HP: %d/%d", ej->jogador.vida, ej->jogador.vida_maxima);
    DrawText(buf, x + 4, y + 2, HUD_FONTE_PEQUENA, WHITE);
}


/* Timer da run no topo central (MM:SS / 05:00), mudando de cor conforme se
 * aproxima do chefão, + contagem regressiva nos últimos 30s + barrinha de
 * progresso até a próxima carta. */
static void hud_desenhar_tempo(const EstadoJogo *ej) {
    float t        = ej->cronograma.tempo_decorrido;
    float duracao  = CRONOGRAMA_DURACAO_SEG;
    float restante = duracao - t;
    int   min = (int)(t / 60.0f);
    int   seg = (int)t % 60;
    int   total_min = (int)(duracao / 60.0f);

    Color cor;
    if      (t >= duracao - 60.0f)        cor = RED;     /* último minuto */
    else if (t >= duracao - 2.0f * 60.0f) cor = YELLOW;  /* penúltimo minuto */
    else                                  cor = GOLD;

    char buf[48];
    snprintf(buf, sizeof(buf), "%02d:%02d / %02d:00", min, seg, total_min);
    int larg_timer = MeasureText(buf, HUD_FONTE_GRANDE);
    DrawText(buf, LARGURA_TELA / 2 - larg_timer / 2, HUD_MARGEM, HUD_FONTE_GRANDE, cor);

    if (restante > 0.0f && restante <= 30.0f && !ej->cronograma.chefao_spawnado) {
        char buf2[48];
        snprintf(buf2, sizeof(buf2), "CHEFAO EM %d", (int)ceilf(restante));
        int larg2 = MeasureText(buf2, HUD_FONTE_MEDIA);
        DrawText(buf2, LARGURA_TELA / 2 - larg2 / 2,
                 HUD_MARGEM + HUD_FONTE_GRANDE + 4, HUD_FONTE_MEDIA, RED);
    }

    /* Progresso até a próxima carta (fração do minuto atual). */
    float seg_no_minuto = t - (float)((int)(t / CRONOGRAMA_INTERVALO_CARTAS_SEG))
                              * CRONOGRAMA_INTERVALO_CARTAS_SEG;
    float progresso = seg_no_minuto / CRONOGRAMA_INTERVALO_CARTAS_SEG;
    if (progresso > 1.0f) progresso = 1.0f;

    int barra_larg = 120, barra_alt = 6;
    int barra_x = LARGURA_TELA / 2 - barra_larg / 2;
    int barra_y = HUD_MARGEM + HUD_FONTE_GRANDE + 28;
    DrawRectangle(barra_x, barra_y, barra_larg, barra_alt, DARKGRAY);
    DrawRectangle(barra_x, barra_y, (int)(barra_larg * progresso), barra_alt, SKYBLUE);
    DrawRectangleLines(barra_x, barra_y, barra_larg, barra_alt, GRAY);
    DrawText("[+]", barra_x + barra_larg + 4, barra_y - 2, HUD_FONTE_PEQUENA - 4, SKYBLUE);
}


/* Moeda de biomassa (pontuação da run) abaixo da barra de vida. */
static void hud_desenhar_biomassa(const EstadoJogo *ej) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", ej->jogador.biomassa);

    int moeda_raio = HUD_MARGEM - 2;
    int moeda_x = HUD_MARGEM + moeda_raio;
    int moeda_y = HUD_MARGEM + HUD_BARRA_ALT + HUD_MARGEM + moeda_raio;

    DrawCircle(moeda_x, moeda_y, moeda_raio + 2, GOLD);
    DrawCircle(moeda_x, moeda_y, moeda_raio, YELLOW);
    int cifrao = MeasureText("B", HUD_FONTE_PEQUENA);
    DrawText("B", moeda_x - cifrao / 2, moeda_y - HUD_FONTE_PEQUENA / 2,
             HUD_FONTE_PEQUENA, DARKBROWN);
    DrawText(buf, moeda_x + moeda_raio + 7, moeda_y - HUD_FONTE_MEDIA / 2,
             HUD_FONTE_MEDIA, WHITE);
}


void desenhar_hud(const EstadoJogo *ej) {
    hud_desenhar_barra_vida(ej);
    hud_desenhar_tempo(ej);
    hud_desenhar_biomassa(ej);
}
