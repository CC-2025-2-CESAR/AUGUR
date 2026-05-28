/* ============================================================================
 * hud.c - IMPLEMENTAÇÃO DO HUD
 * ============================================================================
 *
 * RESPONSABILIDADE: Sofia (Dev 2)
 *
 * Compõe a HUD chamando 3 helpers (barra de vida, contador de onda, moeda
 * de biomassa) e expõe uma única função pública desenhar_hud() que junta
 * tudo. Main chama essa função UMA vez por frame, depois do mundo desenhado.
 *
 * Strings de DrawText ficam em ASCII puro porque a fonte default do Raylib
 * não renderiza acento.
 * ============================================================================*/

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

#define DADO_TAMANHO       40
#define DADO_ESPACO        10


void hud_desenhar_barra_vida(const EstadoJogo *ej)
{
    int x = HUD_MARGEM;
    int y = HUD_MARGEM;
    DrawRectangle(x, y, HUD_BARRA_LARG, HUD_BARRA_ALT, DARKGRAY);

    int largura_vida = 0;
    if (ej->jogador.vida_maxima > 0) {
        largura_vida = (int)(HUD_BARRA_LARG * ej->jogador.vida / (float)ej->jogador.vida_maxima);
    }

    float porcentagem_vida;
    if (ej->jogador.vida_maxima > 0) {
        porcentagem_vida = (float)ej->jogador.vida / ej->jogador.vida_maxima;
    } else {
        porcentagem_vida = 0.0f;
    }

    Color cor_vida;
    if (porcentagem_vida >= 0.50f) cor_vida = GREEN;
    else if (porcentagem_vida > 0.30f) cor_vida = YELLOW;
    else cor_vida = RED;

    DrawRectangle(x, y, largura_vida, HUD_BARRA_ALT, cor_vida);
    DrawRectangleLines(x, y, HUD_BARRA_LARG, HUD_BARRA_ALT, DARKGRAY);

    char buf[32];
    snprintf(buf, sizeof(buf), "HP: %d/%d", ej->jogador.vida, ej->jogador.vida_maxima);
    DrawText(buf, x + 4, y + 2, HUD_FONTE_PEQUENA, WHITE);
}



void hud_desenhar_onda(const EstadoJogo *ej)
{
    float t        = ej->cronograma.tempo_decorrido;
    float duracao  = CRONOGRAMA_DURACAO_SEG;
    float restante = duracao - t;

    int min = (int)(t / 60.0f);
    int seg = (int)t % 60;
    int total_min = (int)(duracao / 60.0f);


    Color cor;
    if (t >= duracao - 60.0f) {
        // Último minuto do 4 p 5 vermelho 
        cor = RED;
    } else if (t >= duracao - 2.0f * 60.0f) {
        // Penúltimo minuto (do 3 p 4) amarelo 
        cor = YELLOW;
    } else cor = GOLD;
    

    //Timer principal
    char buf[48];
    snprintf(buf, sizeof(buf), "%02d:%02d / %02d:00", min, seg, total_min);
    int larg_timer = MeasureText(buf, HUD_FONTE_GRANDE);
    DrawText(buf, LARGURA_TELA / 2 - larg_timer / 2, HUD_MARGEM, HUD_FONTE_GRANDE, cor);

    // Contagem regressiva pro chefão (últimos 30s) 
    if (restante > 0.0f && restante <= 30.0f && !ej->cronograma.chefao_spawnado) {
        char buf2[48];
        snprintf(buf2, sizeof(buf2), "CHEFAO EM %d", (int)ceilf(restante));
        int larg2 = MeasureText(buf2, HUD_FONTE_MEDIA);
        DrawText(buf2, LARGURA_TELA / 2 - larg2 / 2, HUD_MARGEM + HUD_FONTE_GRANDE + 4, HUD_FONTE_MEDIA, RED);
    }

    // Barrinha de progresso até a próxima carta 

    float seg_no_minuto = t - (float)((int)(t / CRONOGRAMA_INTERVALO_CARTAS_SEG)) * CRONOGRAMA_INTERVALO_CARTAS_SEG;
    float progresso = seg_no_minuto / CRONOGRAMA_INTERVALO_CARTAS_SEG;
    if (progresso > 1.0f) progresso = 1.0f;

    int barra_larg = 120;
    int barra_alt  = 6;
    int barra_x    = LARGURA_TELA / 2 - barra_larg / 2;
    int barra_y    = HUD_MARGEM + HUD_FONTE_GRANDE + 28;

    DrawRectangle(barra_x, barra_y, barra_larg, barra_alt, DARKGRAY);
    DrawRectangle(barra_x, barra_y, (int)(barra_larg * progresso), barra_alt, SKYBLUE);
    DrawRectangleLines(barra_x, barra_y, barra_larg, barra_alt, GRAY);

    
    DrawText("[+]", barra_x + barra_larg + 4, barra_y - 2, HUD_FONTE_PEQUENA - 4, SKYBLUE);
}


void hud_desenhar_biomassa(const EstadoJogo *ej)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", ej->jogador.biomassa);

    int moeda_raio = HUD_MARGEM - 2;
    int moeda_x    = HUD_MARGEM + moeda_raio;
    int moeda_y    = HUD_MARGEM + HUD_BARRA_ALT + HUD_MARGEM + moeda_raio;

    DrawCircle(moeda_x, moeda_y, moeda_raio + 2, GOLD);
    DrawCircle(moeda_x, moeda_y, moeda_raio, YELLOW);
    int cifrao = MeasureText("B", HUD_FONTE_PEQUENA);
    DrawText("B", moeda_x - cifrao / 2, moeda_y - HUD_FONTE_PEQUENA / 2, HUD_FONTE_PEQUENA, DARKBROWN);

    int quant_moeda_x = moeda_x + moeda_raio + 7;
    int quant_moeda_y = moeda_y - HUD_FONTE_MEDIA / 2;

    DrawText(buf, quant_moeda_x, quant_moeda_y, HUD_FONTE_MEDIA, WHITE);
}


void desenhar_hud(const EstadoJogo *ej)
{
    hud_desenhar_barra_vida(ej);
    hud_desenhar_onda(ej);
    hud_desenhar_biomassa(ej);
}