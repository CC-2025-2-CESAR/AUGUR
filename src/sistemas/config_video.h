/* ============================================================================
 * config_video.h - CONFIGURAÇÃO DE VÍDEO (resolução + fullscreen)
 * ============================================================================
 *
 * Aplica as preferências de vídeo salvas em DadosSalvos sobre a janela do
 * Raylib. Trata o caso "save vazio / save antigo" preenchendo defaults
 * sensatos (LARGURA_TELA × ALTURA_TELA, modo janela).
 *
 * Como integrar: em main.c::jogo_inicializar, depois de salvamento_carregar,
 * chamar config_video_normalizar e em seguida config_video_aplicar.
 * ============================================================================ */

#ifndef CONFIG_VIDEO_H
#define CONFIG_VIDEO_H

#include "tipos.h"

/* Lista de resoluções suportadas pela tela de Opções.
 * Definida em config_video.c. */
extern const Vector2 RESOLUCOES_DISPONIVEIS[];
extern const int     RESOLUCOES_TOTAL;

/* Se o save ainda não tem resolução definida (campos zerados, save novo ou
 * upgrade da struct), preenche com os defaults LARGURA_TELA × ALTURA_TELA
 * e fullscreen=false. Idempotente — chamar várias vezes não muda nada. */
void config_video_normalizar(DadosSalvos *ds);

/* Aplica resolução e fullscreen no estado atual do Raylib. Garante que o
 * estado interno (variável estática) bate com o save antes de chamar
 * ToggleFullscreen. Pode ser chamada várias vezes (idempotente). */
void config_video_aplicar(const DadosSalvos *ds);

/* Devolve o índice da resolução em RESOLUCOES_DISPONIVEIS que bate com o
 * save atual. Se nenhuma bater (resolução customizada), retorna 0. Útil pra
 * tela de Opções iniciar o cursor na linha certa. */
int config_video_indice_resolucao_atual(const DadosSalvos *ds);

#endif /* CONFIG_VIDEO_H */
