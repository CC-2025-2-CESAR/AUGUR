/* ============================================================================
 * projeteis_inimigo.c - ENGINE DO TIRO DE INIMIGO
 * ============================================================================
 *
 * Mesma estrutura da engine de magias: insere na cabeça (O(1)), move/expira,
 * remove os mortos com ponteiro duplo no fim do frame. Stats vêm da tabela
 * tunável da Luísa (projeteis_inimigo_tipos.c). Quem dispara é inimigos.c;
 * quem resolve o hit no jogador é colisao.c.
 * ============================================================================ */

#include "projeteis_inimigo.h"
#include "projeteis_inimigo_tipos.h"
#include "../core/assets.h"
#include <stdlib.h>
#include <math.h>


static int contar_nos(const ProjetilInimigoNo *cabeca) {
    int n = 0;
    for (const ProjetilInimigoNo *p = cabeca; p != NULL; p = p->proximo) n++;
    return n;
}


void projeteis_inimigo_spawnar(EstadoJogo *ej,
                               Vector2     posicao,
                               Vector2     direcao_normalizada,
                               TipoInimigo tipo_origem) {
    if ((int)tipo_origem < 0 ||
        (int)tipo_origem >= QTD_PARAMETROS_PROJETIL_INIMIGO) return;

    const ParametrosProjetilInimigo *p =
        &PARAMETROS_PROJETIL_INIMIGO[tipo_origem];
    if (!p->pode_atirar) return;
    if (contar_nos(ej->projeteis_inimigo_cabeca) >= MAX_PROJETEIS_INIMIGO) return;

    ProjetilInimigoNo *novo =
        (ProjetilInimigoNo *)malloc(sizeof(ProjetilInimigoNo));
    if (novo == NULL) return;

    novo->dados.posicao       = posicao;
    novo->dados.velocidade.x  = direcao_normalizada.x * p->velocidade;
    novo->dados.velocidade.y  = direcao_normalizada.y * p->velocidade;
    novo->dados.dano          = p->dano;
    novo->dados.tempo_de_vida = p->tempo_vida;
    novo->dados.raio          = p->raio;
    novo->dados.cor           = p->cor;
    novo->dados.vivo          = true;
    novo->dados.tipo_origem   = (int)tipo_origem;

    novo->proximo                 = ej->projeteis_inimigo_cabeca;
    ej->projeteis_inimigo_cabeca  = novo;
}


void projeteis_inimigo_atualizar(EstadoJogo *ej) {
    float dt = ej->delta_tempo;

    for (ProjetilInimigoNo *pno = ej->projeteis_inimigo_cabeca;
         pno != NULL; pno = pno->proximo) {
        if (!pno->dados.vivo) continue;
        pno->dados.posicao.x += pno->dados.velocidade.x * dt;
        pno->dados.posicao.y += pno->dados.velocidade.y * dt;
        pno->dados.tempo_de_vida -= dt;
        if (pno->dados.tempo_de_vida <= 0.0f) {
            pno->dados.vivo = false;
        }
    }

    /* Remoção dos mortos (ponteiro duplo). */
    ProjetilInimigoNo **atual = &ej->projeteis_inimigo_cabeca;
    while (*atual != NULL) {
        if (!(*atual)->dados.vivo) {
            ProjetilInimigoNo *morto = *atual;
            *atual = morto->proximo;
            free(morto);
        } else {
            atual = &(*atual)->proximo;
        }
    }
}


void projeteis_inimigo_desenhar(const EstadoJogo *ej) {
    for (const ProjetilInimigoNo *pno = ej->projeteis_inimigo_cabeca;
         pno != NULL; pno = pno->proximo) {
        if (!pno->dados.vivo) continue;

        /* Tenta usar sprite do projetil baseado no tipo do inimigo origem.
         * Se a textura nao carregou (id==0), cai pro fallback de circulo. */
        Texture2D tex = (Texture2D){0};
        int t = pno->dados.tipo_origem;
        if (t >= 0 && t < ASSETS_NUM_INIMIGOS) {
            tex = g_assets.projeteis_inimigo[t];
        }

        if (tex.id != 0) {
            /* Sprite apontando +X no PNG: rotaciona pela direcao da velocidade. */
            float ang_deg = atan2f(pno->dados.velocidade.y,
                                   pno->dados.velocidade.x) * (180.0f / 3.14159265f);
            /* Tamanho em tela = diametro * SPRITE_VISUAL_SCALE, igual ao
             * sistema das magias do player. */
            float lado = pno->dados.raio * 2.0f * SPRITE_VISUAL_SCALE;
            Rectangle src = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
            Rectangle dst = { pno->dados.posicao.x, pno->dados.posicao.y,
                              lado, lado };
            Vector2 origem = { lado * 0.5f, lado * 0.5f };  /* gira no centro */
            DrawTexturePro(tex, src, dst, origem, ang_deg, WHITE);
        } else {
            /* Fallback: circulo colorido (mantem comportamento antigo). */
            DrawCircleV(pno->dados.posicao, pno->dados.raio, pno->dados.cor);
            DrawCircleV(pno->dados.posicao, pno->dados.raio * 0.4f, WHITE);
        }
    }
}


void projeteis_inimigo_liberar_tudo(EstadoJogo *ej) {
    ProjetilInimigoNo *atual = ej->projeteis_inimigo_cabeca;
    while (atual != NULL) {
        ProjetilInimigoNo *prox = atual->proximo;
        free(atual);
        atual = prox;
    }
    ej->projeteis_inimigo_cabeca = NULL;
}
