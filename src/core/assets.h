/* ============================================================================
 * assets.h - CARGA E DESENHO DE SPRITE SHEETS
 * ============================================================================
 *
 * Centraliza o que vem dos PNGs/JSONs entregues pela Luísa em assets/sprites/.
 *
 *   - Assets g_assets: handle de todas as texturas carregadas (jogador, 4
 *     inimigos, 6 magias, tileset do chão). Acesso direto via campos.
 *
 *   - MetaSheet: metadata de uma sprite sheet (frame_w/h, qual row começa
 *     cada animação, quantos frames e a que fps). META_JOGADOR e
 *     META_INIMIGO[QTD_PARAMETROS_INIMIGO] expõem isso pras funções de
 *     desenho de cada entidade.
 *
 *   - desenhar_sheet(): helper único que mapeia (animação, direção, tempo)
 *     pra (row, frame) na sheet e chama DrawTexturePro centralizando em pos.
 *     Faz no-op silencioso se a textura não carregou (tex.id == 0) — o
 *     chamador testa antes pra cair em fallback de primitiva.
 *
 * Por que MetaSheet em vez de ler os JSONs em runtime: zero dependência de
 * parser JSON e o layout das sheets é fixo (mesma convenção pra todas:
 * idle/walk/cast em 4 rows direcionais + hurt + death omnidirecionais).
 * Mudar o layout mudaria todos juntos de qualquer forma.
 * ============================================================================ */

#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"
#include "tipos.h"

/* Quantos sprite sheets de inimigo existem (= valores no enum TipoInimigo:
 * CORPO_A_CORPO, A_DISTANCIA, ELITE, CHEFE). Usado pra dimensionar arrays em
 * escopo global — precisa ser #define (constante de compilação), e não a
 * QTD_PARAMETROS_INIMIGO da tabela da Luísa que é `extern const int`. */
#define ASSETS_NUM_INIMIGOS 4

/* Fator visual aplicado em cima da escala "natural" (= 2*raio / frame_w).
 * Desacopla tamanho do sprite na tela do raio de hitbox: aumentar este
 * número faz o sprite aparecer maior SEM mudar colisão/balance.
 *
 * 1.0 = sprite ocupa exatamente o diâmetro da hitbox (visual = hitbox).
 * 2.0 = sprite com o dobro do diâmetro da hitbox (mais "presente" na tela).
 *
 * Usado por jogador_desenhar, inimigos_desenhar e magias_desenhar. */
#define SPRITE_VISUAL_SCALE 2.0f

/* Texturas globais carregadas uma vez em jogo_inicializar e liberadas em
 * jogo_finalizar. Acesso direto: `g_assets.jogador`, `g_assets.inimigos[tipo]`.
 *
 * Se um PNG faltar no disco, LoadTexture retorna uma textura com id=0; as
 * funções de render testam isso e caem pro fallback (DrawCircle etc.). */
typedef struct {
    Texture2D jogador;
    Texture2D inimigos[ASSETS_NUM_INIMIGOS];          /* indexado por TipoInimigo */
    Texture2D projeteis_inimigo[ASSETS_NUM_INIMIGOS]; /* 1 sprite por TipoInimigo (id=0 se nao atira) */
    Texture2D magias[ELEMENTO_TOTAL];                 /* indexado por Elemento */
    Texture2D tileset;                                /* tileset do chao (10 tiles 64x64 - grama) */
} Assets;

/* Layout de uma sprite sheet. Convenção (igual pros 5 personagens animados):
 *   row idle_row_base + 0..3  → idle_{down,up,left,right}, cada uma com idle_n frames
 *   row walk_row_base + 0..3  → walk_*, walk_n frames
 *   row cast_row_base + 0..3  → cast_*, cast_n frames
 *   row hurt_row              → hurt (omnidirecional), hurt_n frames
 *   row death_row             → death (omnidirecional), death_n frames
 *
 * frame_w/frame_h vêm dos JSONs da Luísa (32 pro jogador e inimigos comuns,
 * 40 pro elite arauto, 64 pro chefe oraculo). */
typedef struct {
    int frame_w, frame_h;
    int idle_row_base;  int idle_n;   float idle_fps;
    int walk_row_base;  int walk_n;   float walk_fps;
    int cast_row_base;  int cast_n;   float cast_fps;
    int hurt_row;       int hurt_n;   float hurt_fps;
    int death_row;      int death_n;  float death_fps;
} MetaSheet;

extern Assets    g_assets;
extern MetaSheet META_JOGADOR;
extern MetaSheet META_INIMIGO[ASSETS_NUM_INIMIGOS];

/* Carrega todas as texturas. Chamar UMA vez depois de InitWindow (a OpenGL
 * precisa estar pronta). Texturas que falharem ficam com id=0 — os render
 * functions caem em fallback. */
void assets_carregar(void);

/* Libera todas as texturas. Chamar antes de CloseWindow. */
void assets_liberar(void);

/* Desenha um frame da sheet centralizado em `pos`.
 *
 * Argumentos:
 *   sheet     — Texture2D carregada (no-op se id==0)
 *   meta      — metadata da sheet (rows, fps por animação)
 *   pos       — centro de desenho em coord do contexto atual
 *               (mundo se dentro de BeginMode2D, tela caso contrário)
 *   direcao   — DIR_DOWN/UP/LEFT/RIGHT — ignorado pra HURT/DEATH (omni)
 *   animacao  — ANIM_IDLE/WALK/CAST/HURT/DEATH
 *   tempo     — segundos acumulados na animação (frame = (tempo*fps) % n)
 *   escala    — multiplica frame_w/frame_h (1.0 = tamanho original)
 *   tint      — multiplica a cor da textura (WHITE = sem alteração) */
void desenhar_sheet(Texture2D sheet, const MetaSheet *meta,
                    Vector2 pos, int direcao, int animacao,
                    float tempo, float escala, Color tint);

#endif /* ASSETS_H */
