/* ============================================================================
 * magias.c - ENGINE DE MAGIAS/PROJÉTEIS
 * ============================================================================
 *
 * Lista encadeada simples: novos projéteis vão pra cabeça (O(1) no insert),
 * mortos são removidos com ponteiro duplo no fim do frame.
 *
 * O auto-fire em si vive em magias_tipos.c — esta engine apenas chama
 * magias_tipos_processar_auto_fire(ej) no início do update.
 * ============================================================================ */

#include "magias.h"
#include "magias_tipos.h"
#include "magias_comportamento.h"
#include <stdlib.h>
#include <math.h>


/* Conta nós da lista pra respeitar MAX_PROJETEIS. */
static int contar_nos(const MagiaNo *cabeca) {
    int n = 0;
    for (const MagiaNo *p = cabeca; p != NULL; p = p->proxima) n++;
    return n;
}


void magias_spawnar(EstadoJogo *ej,
                    Vector2     posicao,
                    Vector2     direcao_normalizada,
                    Elemento    elemento) {
    if ((int)elemento < 0 || (int)elemento >= QTD_PARAMETROS_MAGIA) return;
    if (contar_nos(ej->magias_cabeca) >= MAX_PROJETEIS) return;

    MagiaNo *novo = (MagiaNo *)malloc(sizeof(MagiaNo));
    if (novo == NULL) return;

    const ParametrosMagia *p = &PARAMETROS_MAGIA[elemento];

    /* saltos de chain (Relâmpago) vêm da tabela de comportamento da Luísa. */
    int saltos = 0;
    if (elemento >= 0 && (int)elemento < QTD_COMPORTAMENTO_MAGIA &&
        COMPORTAMENTO_MAGIA[elemento].rider == RIDER_CHAIN) {
        saltos = COMPORTAMENTO_MAGIA[elemento].chain_saltos;
    }

    novo->dados.posicao          = posicao;
    novo->dados.velocidade.x     = direcao_normalizada.x * p->velocidade_projetil;
    novo->dados.velocidade.y     = direcao_normalizada.y * p->velocidade_projetil;
    /* bonus_dano do jogador entra AQUI, no nascimento do projétil (nunca
     * iterando a lista em voo — projéteis morrem e o bônus se perderia). */
    novo->dados.dano             = p->dano_base + (float)ej->jogador.bonus_dano;
    novo->dados.tempo_de_vida    = p->tempo_de_vida;
    novo->dados.raio             = p->raio_projetil;
    novo->dados.elemento         = elemento;
    novo->dados.viva             = true;
    novo->dados.saltos_restantes = saltos;
    novo->dados.ja_acertou       = false;

    novo->proxima        = ej->magias_cabeca;
    ej->magias_cabeca    = novo;
}


/* Vetor unitário do jogador até o inimigo VIVO (não-aliado) mais próximo.
 * Engine própria de mira: centraliza o disparo aqui (a Luísa só agenda
 * QUANDO disparar, em magias_tipos.c). Retorna false se não há alvo. */
static bool mirar_mais_proximo(const EstadoJogo *ej, Vector2 *out_dir) {
    const InimigoNo *perto = NULL;
    float menor = 1e30f;
    for (const InimigoNo *ino = ej->inimigos_cabeca; ino; ino = ino->proximo) {
        if (!ino->dados.vivo || ino->dados.aliado) continue;
        float dx = ino->dados.posicao.x - ej->jogador.posicao.x;
        float dy = ino->dados.posicao.y - ej->jogador.posicao.y;
        float d2 = dx * dx + dy * dy;
        if (d2 < menor) { menor = d2; perto = ino; }
    }
    if (perto == NULL) return false;

    float dx = perto->dados.posicao.x - ej->jogador.posicao.x;
    float dy = perto->dados.posicao.y - ej->jogador.posicao.y;
    float c = sqrtf(dx * dx + dy * dy);
    if (c < 0.0001f) return false;
    out_dir->x = dx / c;
    out_dir->y = dy / c;
    return true;
}


/* Dispara um elemento mirando no inimigo mais próximo. Consome um "duplica
 * próximos" do motor de profecia (EF_DUPLICA_PROJETIL) quando ativo.
 * Retorna false (e não dispara) se não houver alvo — o chamador usa isso
 * pra não acumular cooldown enquanto não há inimigos. */
bool magias_disparar_elemento(EstadoJogo *ej, Elemento elemento) {
    Vector2 dir;
    if (!mirar_mais_proximo(ej, &dir)) return false;

    magias_spawnar(ej, ej->jogador.posicao, dir, elemento);

    if (ej->motor_profecia.duplica_proximos > 0) {
        ej->motor_profecia.duplica_proximos--;
        /* segundo projétil girado ~15° pra não sobrepor o primeiro */
        Vector2 d2 = { dir.x * 0.9659f - dir.y * 0.2588f,
                       dir.x * 0.2588f + dir.y * 0.9659f };
        magias_spawnar(ej, ej->jogador.posicao, d2, elemento);
    }
    return true;
}


void magias_atualizar(EstadoJogo *ej) {
    /* 1. Auto-fire — pode criar novos projéteis (entram na cabeça da lista). */
    magias_tipos_processar_auto_fire(ej);

    float dt = ej->delta_tempo;

    /* 2. Movimento + expiração. */
    for (MagiaNo *mno = ej->magias_cabeca; mno != NULL; mno = mno->proxima) {
        if (!mno->dados.viva) continue;
        mno->dados.posicao.x += mno->dados.velocidade.x * dt;
        mno->dados.posicao.y += mno->dados.velocidade.y * dt;
        mno->dados.tempo_de_vida -= dt;
        if (mno->dados.tempo_de_vida <= 0.0f) {
            mno->dados.viva = false;
        }
    }

    /* 3. Remoção dos mortos (ponteiro duplo). */
    MagiaNo **atual = &ej->magias_cabeca;
    while (*atual != NULL) {
        if (!(*atual)->dados.viva) {
            MagiaNo *morta = *atual;
            *atual = morta->proxima;
            free(morta);
        } else {
            atual = &(*atual)->proxima;
        }
    }
}


void magias_desenhar(const EstadoJogo *ej) {
    for (const MagiaNo *mno = ej->magias_cabeca;
         mno != NULL;
         mno = mno->proxima) {
        if (!mno->dados.viva) continue;
        if ((int)mno->dados.elemento < 0 ||
            (int)mno->dados.elemento >= QTD_PARAMETROS_MAGIA) continue;

        Color cor = PARAMETROS_MAGIA[mno->dados.elemento].cor;
        DrawCircleV(mno->dados.posicao, mno->dados.raio, cor);
        /* Núcleo branco pra dar punch visual. */
        DrawCircleV(mno->dados.posicao, mno->dados.raio * 0.4f, WHITE);
    }
}


void magias_liberar_tudo(EstadoJogo *ej) {
    MagiaNo *atual = ej->magias_cabeca;
    while (atual != NULL) {
        MagiaNo *prox = atual->proxima;
        free(atual);
        atual = prox;
    }
    ej->magias_cabeca = NULL;
}
