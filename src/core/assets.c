/* ============================================================================
 * assets.c - CARGA E DESENHO DAS SPRITE SHEETS
 * ----------------------------------------------------------------------------
 * Carrega as sheets/tileset (assets/sprites/...) na init, libera no shutdown e
 * provê desenhar_sheet() (mapeia animação+direção+tempo -> row/frame). Resiliente:
 * se um PNG faltar, LoadTexture deixa id=0 e o render cai em primitiva.
 * ============================================================================ */

#include "assets.h"

Assets g_assets = {0};

/* --- Metadata por entidade --- */

/* JOGADOR (augur_sheet.png) — 64×64 (pack v5 HD), augur.json:
 *   idle: 6 frames @ 6 fps; walk: 8 frames @ 10 fps; cast: 6 frames @ 12 fps
 *   hurt: 4 frames @ 12 fps; death: 8 frames @ 8 fps */
MetaSheet META_JOGADOR = {
    .frame_w = 64, .frame_h = 64,
    .idle_row_base = 0,  .idle_n = 6,  .idle_fps = 6.0f,
    .walk_row_base = 4,  .walk_n = 8,  .walk_fps = 10.0f,
    .cast_row_base = 8,  .cast_n = 6,  .cast_fps = 12.0f,
    .hurt_row      = 12, .hurt_n = 4,  .hurt_fps = 12.0f,
    .death_row     = 13, .death_n = 8, .death_fps = 8.0f,
};

/* INIMIGOS — uma entrada por TipoInimigo (0..3).
 * Pack v5 HD: corpo-a-corpo e a-distância 64×64, elite (arauto) 80×80,
 * chefe (oraculo) 128×128.
 * FPS variam: cast mais rápido em carniçal (agressividade), mais lento no
 * oráculo (gravitas). */
MetaSheet META_INIMIGO[ASSETS_NUM_INIMIGOS] = {
    /* [INIMIGO_CORPO_A_CORPO] — carnical_sheet.png (64×64) */
    {
        .frame_w = 64, .frame_h = 64,
        .idle_row_base = 0,  .idle_n = 6,  .idle_fps = 6.0f,
        .walk_row_base = 4,  .walk_n = 8,  .walk_fps = 10.0f,
        .cast_row_base = 8,  .cast_n = 6,  .cast_fps = 14.0f,
        .hurt_row      = 12, .hurt_n = 4,  .hurt_fps = 12.0f,
        .death_row     = 13, .death_n = 8, .death_fps = 8.0f,
    },
    /* [INIMIGO_A_DISTANCIA] — vidente_sheet.png (64×64) */
    {
        .frame_w = 64, .frame_h = 64,
        .idle_row_base = 0,  .idle_n = 6,  .idle_fps = 6.0f,
        .walk_row_base = 4,  .walk_n = 8,  .walk_fps = 10.0f,
        .cast_row_base = 8,  .cast_n = 6,  .cast_fps = 12.0f,
        .hurt_row      = 12, .hurt_n = 4,  .hurt_fps = 12.0f,
        .death_row     = 13, .death_n = 8, .death_fps = 8.0f,
    },
    /* [INIMIGO_ELITE] — arauto_sheet.png (80×80) */
    {
        .frame_w = 80, .frame_h = 80,
        .idle_row_base = 0,  .idle_n = 6,  .idle_fps = 6.0f,
        .walk_row_base = 4,  .walk_n = 8,  .walk_fps = 9.0f,
        .cast_row_base = 8,  .cast_n = 6,  .cast_fps = 10.0f,
        .hurt_row      = 12, .hurt_n = 4,  .hurt_fps = 12.0f,
        .death_row     = 13, .death_n = 8, .death_fps = 7.0f,
    },
    /* [INIMIGO_CHEFE] — oraculo_sheet.png (128×128) */
    {
        .frame_w = 128, .frame_h = 128,
        .idle_row_base = 0,  .idle_n = 6,  .idle_fps = 5.0f,
        .walk_row_base = 4,  .walk_n = 8,  .walk_fps = 8.0f,
        .cast_row_base = 8,  .cast_n = 6,  .cast_fps = 9.0f,
        .hurt_row      = 12, .hurt_n = 4,  .hurt_fps = 12.0f,
        .death_row     = 13, .death_n = 8, .death_fps = 6.0f,
    },
};


/* --- Carga / liberação --- */

void assets_carregar(void) {
    g_assets.jogador = LoadTexture("assets/sprites/personagem/augur_sheet.png");

    g_assets.inimigos[INIMIGO_CORPO_A_CORPO] = LoadTexture("assets/sprites/inimigos/carnical_sheet.png");
    g_assets.inimigos[INIMIGO_A_DISTANCIA]   = LoadTexture("assets/sprites/inimigos/vidente_sheet.png");
    g_assets.inimigos[INIMIGO_ELITE]         = LoadTexture("assets/sprites/inimigos/arauto_sheet.png");
    g_assets.inimigos[INIMIGO_CHEFE]         = LoadTexture("assets/sprites/inimigos/oraculo_sheet.png");

    /* Projeteis dos inimigos (1 PNG 32x32 por TipoInimigo). Corpo-a-corpo
     * e Elite nao atiram, mas carregamos pra manter o array alinhado;
     * suas texturas ficam vazias/transparentes e o render so usa as que
     * tiverem id != 0 (id == 0 cai no fallback de circulo colorido). */
    g_assets.projeteis_inimigo[INIMIGO_CORPO_A_CORPO] = LoadTexture("assets/sprites/inimigos/projetil_corpo_a_corpo.png");
    g_assets.projeteis_inimigo[INIMIGO_A_DISTANCIA]   = LoadTexture("assets/sprites/inimigos/projetil_vidente.png");
    g_assets.projeteis_inimigo[INIMIGO_ELITE]         = LoadTexture("assets/sprites/inimigos/projetil_elite.png");
    g_assets.projeteis_inimigo[INIMIGO_CHEFE]         = LoadTexture("assets/sprites/inimigos/projetil_oraculo.png");

    g_assets.magias[ELEMENTO_FOGO]      = LoadTexture("assets/sprites/magias/magia_fogo.png");
    g_assets.magias[ELEMENTO_GELO]      = LoadTexture("assets/sprites/magias/magia_gelo.png");
    g_assets.magias[ELEMENTO_RELAMPAGO] = LoadTexture("assets/sprites/magias/magia_relampago.png");
    g_assets.magias[ELEMENTO_VENENO]    = LoadTexture("assets/sprites/magias/magia_veneno.png");
    g_assets.magias[ELEMENTO_ARCANO]    = LoadTexture("assets/sprites/magias/magia_arcano.png");
    g_assets.magias[ELEMENTO_SOMBRA]    = LoadTexture("assets/sprites/magias/magia_sombra.png");

    g_assets.tileset = LoadTexture("assets/sprites/background/tileset.png");
}


void assets_liberar(void) {
    UnloadTexture(g_assets.jogador);
    for (int i = 0; i < ASSETS_NUM_INIMIGOS; i++) UnloadTexture(g_assets.inimigos[i]);
    for (int i = 0; i < ASSETS_NUM_INIMIGOS; i++) UnloadTexture(g_assets.projeteis_inimigo[i]);
    for (int i = 0; i < ELEMENTO_TOTAL;          i++) UnloadTexture(g_assets.magias[i]);
    UnloadTexture(g_assets.tileset);
}


/* --- Desenho --- */

void desenhar_sheet(Texture2D sheet, const MetaSheet *meta, Vector2 pos,
                    int direcao, int animacao, float tempo,
                    float escala, Color tint) {
    if (sheet.id == 0 || meta == NULL) return;

    /* Sanitiza direção pra ficar em [0, 3] (DOWN se inválido). */
    if (direcao < 0 || direcao > 3) direcao = DIR_DOWN;

    int   row;
    int   n;
    float fps;
    switch (animacao) {
        case ANIM_WALK:
            row = meta->walk_row_base + direcao;
            n   = meta->walk_n;
            fps = meta->walk_fps;
            break;
        case ANIM_CAST:
            row = meta->cast_row_base + direcao;
            n   = meta->cast_n;
            fps = meta->cast_fps;
            break;
        case ANIM_HURT:
            row = meta->hurt_row;
            n   = meta->hurt_n;
            fps = meta->hurt_fps;
            break;
        case ANIM_DEATH:
            row = meta->death_row;
            n   = meta->death_n;
            fps = meta->death_fps;
            break;
        case ANIM_IDLE:
        default:
            row = meta->idle_row_base + direcao;
            n   = meta->idle_n;
            fps = meta->idle_fps;
            break;
    }

    /* Frame atual = (tempo * fps) % n, com guards pra n inválido. */
    int frame = 0;
    if (n > 0 && fps > 0.0f) {
        frame = ((int)(tempo * fps)) % n;
        if (frame < 0) frame = 0;
    }

    Rectangle src = {
        (float)(frame * meta->frame_w),
        (float)(row   * meta->frame_h),
        (float)meta->frame_w,
        (float)meta->frame_h
    };
    float w = (float)meta->frame_w * escala;
    float h = (float)meta->frame_h * escala;
    Rectangle dst = { pos.x - w * 0.5f, pos.y - h * 0.5f, w, h };

    DrawTexturePro(sheet, src, dst, (Vector2){0, 0}, 0.0f, tint);
}
