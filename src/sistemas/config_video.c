/* ============================================================================
 * config_video.c - aplica resolução e fullscreen sobre a janela do Raylib
 * ============================================================================
 *
 * O Raylib tem APIs simples pra mexer na janela:
 *   - SetWindowSize(w, h) muda o tamanho do client area.
 *   - ToggleFullscreen() troca janela <-> fullscreen (não há SetFullscreen).
 *
 * Como não dá pra perguntar "estou em fullscreen?" de forma totalmente
 * confiável em todas as plataformas, mantemos uma variável estática
 * `s_fullscreen_atual` que rastreia o estado pra decidir se chamamos toggle
 * ou não. Idempotente: chamar duas vezes com o mesmo save é no-op.
 * ============================================================================ */

#include "config_video.h"
#include "raylib.h"

/* Resoluções pré-definidas que aparecem na tela de Opções.
 * Ordem importa — é a ordem em que o cursor vai navegar. */
const Vector2 RESOLUCOES_DISPONIVEIS[] = {
    { 1280.0f,  720.0f },   /* 720p */
    { 1600.0f,  900.0f },   /* 900p */
    { 1920.0f, 1080.0f },   /* 1080p */
};
const int RESOLUCOES_TOTAL =
    (int)(sizeof(RESOLUCOES_DISPONIVEIS) / sizeof(RESOLUCOES_DISPONIVEIS[0]));

/* Cache interno do último estado aplicado. Inicia em "janela" porque é o
 * default de InitWindow no main.c. */
static bool s_fullscreen_atual = false;
static int  s_largura_atual    = LARGURA_TELA;
static int  s_altura_atual     = ALTURA_TELA;


void config_video_normalizar(DadosSalvos *ds) {
    if (!ds) return;

    /* Save novo (ou save antigo zerado por mismatch de versão): valores caem
     * em 0/false. Preenche com defaults. */
    if (ds->largura_tela <= 0 || ds->altura_tela <= 0) {
        ds->largura_tela = LARGURA_TELA;
        ds->altura_tela  = ALTURA_TELA;
        ds->fullscreen   = false;
    }
}


void config_video_aplicar(const DadosSalvos *ds) {
    if (!ds) return;

    /* Resolução: só chama SetWindowSize se mudou de fato. Evita flicker. */
    if (ds->largura_tela != s_largura_atual ||
        ds->altura_tela  != s_altura_atual) {
        /* Se estamos em fullscreen, sair primeiro pra que SetWindowSize tenha
         * efeito visível ao voltar. */
        if (s_fullscreen_atual) {
            ToggleFullscreen();
            s_fullscreen_atual = false;
        }
        SetWindowSize(ds->largura_tela, ds->altura_tela);
        s_largura_atual = ds->largura_tela;
        s_altura_atual  = ds->altura_tela;
    }

    /* Fullscreen: só toggla se o estado-alvo diferir do atual. */
    if (ds->fullscreen != s_fullscreen_atual) {
        ToggleFullscreen();
        s_fullscreen_atual = ds->fullscreen;
    }
}


int config_video_indice_resolucao_atual(const DadosSalvos *ds) {
    if (!ds) return 0;
    for (int i = 0; i < RESOLUCOES_TOTAL; i++) {
        if ((int)RESOLUCOES_DISPONIVEIS[i].x == ds->largura_tela &&
            (int)RESOLUCOES_DISPONIVEIS[i].y == ds->altura_tela) {
            return i;
        }
    }
    return 0;
}
