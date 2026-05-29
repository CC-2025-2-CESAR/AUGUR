/* ============================================================================
 * jogador.c - IMPLEMENTAÇÃO DO MÓDULO JOGADOR
 * ============================================================================
 *
 * Movimentação com WASD, HP e renderização via sprite sheet direcional
 * (entrega da Luísa). Caso a textura não tenha carregado (PNG faltando),
 * desenhar cai em fallback de bolinha azul — comportamento original
 * preservado pra robustez.
 *
 * CONCEITOS IMPORTANTES USADOS AQUI:
 *   - Ponteiro pra struct: recebemos "Jogador *j" em vez de "Jogador j".
 *     Assim a função modifica o objeto ORIGINAL, sem copiar tudo.
 *   - Delta_tempo: multiplicamos velocidade pelo tempo decorrido desde o
 *     último frame, garantindo velocidade constante mesmo com FPS variável.
 *   - Normalização de vetor: ao apertar W+D (diagonal), o jogador não pode
 *     andar mais rápido que na horizontal pura. Pra isso dividimos o vetor
 *     de direção pelo seu comprimento.
 *   - Estado de animação (Luísa): direcao_atual + animacao_atual + tempo
 *     acumulado. hurt_tempo_restante e cast_tempo_restante são timers que
 *     "travam" a animação em HURT/CAST por uma janela curta antes de
 *     liberar pro idle/walk natural.
 * ========================================================================== */

#include "jogador.h"
#include "assets.h"   /* g_assets, META_JOGADOR, desenhar_sheet */
#include "raylib.h"
#include <math.h>     /* sqrtf, fabsf */


/* ----- INICIALIZAÇÃO ----- */
void jogador_inicializar(Jogador *j) {
    /* Posição inicial: origem do mundo (0, 0). O mundo é infinito e a Camera2D
     * enquadra o jogador sempre no centro da tela, então não importa onde ele
     * começa — importa que sua posição seja interpretada em coordenadas de
     * mundo, não de tela. */
    j->posicao        = (Vector2){ 0.0f, 0.0f };
    j->velocidade     = (Vector2){ 0.0f, 0.0f };
    j->raio           = 16.0f;
    j->vida_maxima    = 100;
    j->vida           = j->vida_maxima;
    j->velocidade_movimento = 200.0f;
    j->biomassa       = 0;
    j->bonus_dano     = 0;

    /* Estado visual default: olhando pra baixo, idle. */
    j->direcao_atual         = DIR_DOWN;
    j->animacao_atual        = ANIM_IDLE;
    j->animacao_tempo        = 0.0f;
    j->hurt_tempo_restante   = 0.0f;
    j->cast_tempo_restante   = 0.0f;
}


/* Atualiza a direção do sprite baseado no vetor de movimento (eixo dominante).
 * Se o vetor é nulo (input zerado), MANTÉM a direção anterior — fica olhando
 * pro lado que estava antes de parar, em vez de "resetar" pra baixo. */
static void atualizar_direcao_jogador(Jogador *j, Vector2 dir_movimento) {
    if (fabsf(dir_movimento.x) < 0.001f && fabsf(dir_movimento.y) < 0.001f) {
        return;   /* parado: preserva última direção */
    }
    if (fabsf(dir_movimento.x) > fabsf(dir_movimento.y)) {
        j->direcao_atual = (dir_movimento.x < 0.0f) ? DIR_LEFT : DIR_RIGHT;
    } else {
        j->direcao_atual = (dir_movimento.y < 0.0f) ? DIR_UP   : DIR_DOWN;
    }
}


/* ----- ATUALIZAÇÃO (chamada a cada frame durante combate) ----- */
void jogador_atualizar(Jogador *j, float delta_tempo) {
    /* 1. Ler input das teclas WASD ou setinhas (qualquer um dos dois).
     * IsKeyDown retorna true ENQUANTO a tecla está pressionada (diferente
     * de IsKeyPressed, que só retorna true no frame em que foi apertada).
     * Apertar W e seta-pra-cima ao mesmo tempo é tratado como uma direção só
     * (a normalização adiante garante que não dobra a velocidade). */
    Vector2 direcao = { 0.0f, 0.0f };
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    direcao.y -= 1.0f;   /* Y invertido */
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  direcao.y += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  direcao.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direcao.x += 1.0f;

    /* 2. Normalizar o vetor de direção.
     * Se o jogador aperta W+D ao mesmo tempo, direcao = (1, -1), cujo
     * comprimento é sqrt(2) ~ 1.41. Dividindo pelo comprimento, o vetor fica
     * com comprimento 1, e a velocidade na diagonal fica igual à horizontal.
     *
     * Pulamos a divisão se comprimento = 0 (nenhuma tecla apertada) pra
     * evitar divisão por zero. */
    float comprimento = sqrtf(direcao.x * direcao.x + direcao.y * direcao.y);
    if (comprimento > 0.0f) {
        direcao.x /= comprimento;
        direcao.y /= comprimento;
    }

    /* 3. Aplicar velocidade */
    j->velocidade.x = direcao.x * j->velocidade_movimento;
    j->velocidade.y = direcao.y * j->velocidade_movimento;

    /* 4. Integrar (mover a posição). delta_tempo mantém o movimento suave. */
    j->posicao.x += j->velocidade.x * delta_tempo;
    j->posicao.y += j->velocidade.y * delta_tempo;

    /* Sem clamp: o mundo é infinito (estilo Vampire Survivors). A sensação de
     * arena vem dos inimigos spawnando em volta do jogador, não de paredes. */

    /* 5. Direção do sprite acompanha o movimento (mantém última se parado). */
    atualizar_direcao_jogador(j, direcao);

    /* 6. Decide animação atual.
     * Prioridade: HURT > CAST > (WALK/IDLE pelo módulo da velocidade).
     * Timers são decrementados antes do teste pra que a animação realmente
     * dure o tempo prometido. animacao_tempo é zerado SÓ na transição entre
     * animações — assim cada frame da sheet é mostrado. */
    j->animacao_tempo += delta_tempo;

    int nova_anim;
    if (j->hurt_tempo_restante > 0.0f) {
        j->hurt_tempo_restante -= delta_tempo;
        nova_anim = ANIM_HURT;
    } else if (j->cast_tempo_restante > 0.0f) {
        j->cast_tempo_restante -= delta_tempo;
        nova_anim = ANIM_CAST;
    } else {
        float vel2 = j->velocidade.x * j->velocidade.x +
                     j->velocidade.y * j->velocidade.y;
        nova_anim = (vel2 > 1.0f) ? ANIM_WALK : ANIM_IDLE;
    }
    if (nova_anim != j->animacao_atual) {
        j->animacao_atual = nova_anim;
        j->animacao_tempo = 0.0f;
    }
}


/* ----- DESENHO -----
 * Sprite via desenhar_sheet (rows direcionais), com fallback pra bolinha azul
 * quando a textura não foi carregada (PNG faltando). O fallback preserva o
 * indicador de direção do código original pra que ainda dê pra "ver" pra onde
 * o jogador está olhando mesmo sem a sheet.
 *
 * A escala é calculada pra que o sprite (frame_w × frame_h, geralmente 32×32)
 * ocupe um diâmetro igual a 2 × raio do jogador. Mantém leitura visual
 * coerente com a hitbox circular. */
void jogador_desenhar(const Jogador *j) {
    Texture2D tex = g_assets.jogador;
    if (tex.id == 0) {
        /* Fallback: comportamento original (bolinha azul + indicador). */
        DrawCircleV(j->posicao, j->raio, SKYBLUE);
        DrawCircleLines((int)j->posicao.x, (int)j->posicao.y, j->raio, WHITE);
        float vel2 = j->velocidade.x * j->velocidade.x +
                     j->velocidade.y * j->velocidade.y;
        if (vel2 > 0.01f) {
            float comprimento = sqrtf(vel2);
            Vector2 indicador = {
                j->posicao.x + (j->velocidade.x / comprimento) * j->raio,
                j->posicao.y + (j->velocidade.y / comprimento) * j->raio
            };
            DrawCircleV(indicador, 3.0f, WHITE);
        }
        return;
    }

    /* SPRITE_VISUAL_SCALE (assets.h) deixa o sprite maior na tela sem mexer
     * na hitbox (raio fica intocado). */
    float escala = (j->raio * 2.0f * SPRITE_VISUAL_SCALE) /
                   (float)META_JOGADOR.frame_w;
    desenhar_sheet(tex, &META_JOGADOR, j->posicao,
                   j->direcao_atual, j->animacao_atual,
                   j->animacao_tempo, escala, WHITE);
}


/* ----- SOFRER DANO -----
 * Subtrai HP, clampa em zero (não deixa negativo pra UI não ficar estranha).
 * Main checa "vida <= 0" pra transicionar pra GAME_OVER.
 *
 * Dispara também a animação de HURT por uma janela curta — feedback visual
 * pro jogador perceber que tomou hit mesmo sem olhar pro HUD. */
void jogador_sofrer_dano(Jogador *j, int quantidade) {
    j->vida -= quantidade;
    if (j->vida < 0) {
        j->vida = 0;
    }
    j->hurt_tempo_restante = 0.25f;
    j->animacao_atual      = ANIM_HURT;
    j->animacao_tempo      = 0.0f;
}
