/* ============================================================================
 * jogador.c - IMPLEMENTAÇÃO DO MÓDULO JOGADOR
 * ============================================================================
 *
 * Movimentação com WASD, HP e indicador visual de direção.
 *
 * CONCEITOS IMPORTANTES USADOS AQUI:
 *   - Ponteiro pra struct: recebemos "Jogador *j" em vez de "Jogador j".
 *     Assim a função modifica o objeto ORIGINAL, sem copiar tudo.
 *   - Delta_tempo: multiplicamos velocidade pelo tempo decorrido desde o
 *     último frame, garantindo velocidade constante mesmo com FPS variável.
 *   - Normalização de vetor: ao apertar W+D (diagonal), o jogador não pode
 *     andar mais rápido que na horizontal pura. Pra isso dividimos o vetor
 *     de direção pelo seu comprimento.
 * ========================================================================== */

#include "jogador.h"
#include "raylib.h"
#include <math.h>    /* sqrtf para normalizar vetor */


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
    j->velocidade_movimento = 200.0f;   /* 250 pixels por segundo */
    j->biomassa       = 0;
    j->bonus_dano     = 0;
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
}


/* ----- DESENHO -----
 * Bolinha azul com contorno branco. Luísa (Dev 3) pode trocar por sprite depois.
 *
 * Quando o jogador está se movendo, um pontinho branco é desenhado no raio,
 * na direção do movimento. Dá leitura visual de "pra onde estou indo" sem
 * complicar o asset. Se estiver parado, o pontinho some. */
void jogador_desenhar(const Jogador *j) {
    DrawCircleV(j->posicao, j->raio, SKYBLUE);
    DrawCircleLines((int)j->posicao.x, (int)j->posicao.y, j->raio, WHITE);

    /* Comprimento do vetor velocidade. Se muito próximo de zero, está parado. */
    float vel2 = j->velocidade.x * j->velocidade.x +
                 j->velocidade.y * j->velocidade.y;
    if (vel2 > 0.01f) {
        float comprimento = sqrtf(vel2);
        /* Normaliza a direção e coloca o pontinho bem na borda, no raio. */
        Vector2 indicador = {
            j->posicao.x + (j->velocidade.x / comprimento) * j->raio,
            j->posicao.y + (j->velocidade.y / comprimento) * j->raio
        };
        DrawCircleV(indicador, 3.0f, WHITE);
    }
}


/* ----- SOFRER DANO -----
 * Subtrai HP, clampa em zero (não deixa negativo pra UI não ficar estranha).
 * Main checa "vida <= 0" pra transicionar pra GAME_OVER. */
void jogador_sofrer_dano(Jogador *j, int quantidade) {
    j->vida -= quantidade;
    if (j->vida < 0) {
        j->vida = 0;
    }
}
