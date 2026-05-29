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
#include "assets.h"   /* g_assets.magias[], desenhar do projétil rotacionado */
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


/* Resolve a equação de interceptação balística:
 *   |D + V·t| = S·t
 * onde D = E − P (vetor jogador→inimigo), V = velocidade do inimigo,
 * S = velocidade escalar do projétil. Devolve o menor t > 0 que satisfaz.
 *
 * Expandindo: (V·V − S²)·t² + 2·(D·V)·t + (D·D) = 0 — quadrática at²+bt+c=0.
 *
 * Casos cobertos:
 *   1. |a| ~ 0 (inimigo na velocidade do projétil): degenera em linear b·t+c=0.
 *   2. discriminante < 0: sem solução real (inimigo foge mais rápido que dá
 *      pra interceptar). Retorna false; chamador faz fallback.
 *   3. t > T_MAX: predição irreal (inimigo quase escapando) → fallback.
 *
 * Forma numericamente estável das raízes pra evitar perda de precisão quando
 * b e √Δ têm magnitudes próximas: q = −½·(b + sign(b)·√Δ); t1 = q/a; t2 = c/q. */
static bool calcular_tempo_intercepcao(Vector2 D, Vector2 V,
                                       float S, float *out_t) {
    const float EPS   = 1e-4f;
    const float T_MAX = 2.0f;   /* s; acima disso, mira atual fica melhor */

    float a = V.x * V.x + V.y * V.y - S * S;
    float b = 2.0f * (D.x * V.x + D.y * V.y);
    float c = D.x * D.x + D.y * D.y;

    float t = -1.0f;

    if (fabsf(a) < EPS) {
        /* V·V ≈ S²: linear b·t + c = 0. c ≥ 0 sempre, então só serve b < 0. */
        if (b < -EPS) t = -c / b;
    } else {
        float disc = b * b - 4.0f * a * c;
        if (disc < 0.0f) return false;
        float sq = sqrtf(disc);
        float q  = -0.5f * (b + (b >= 0.0f ? sq : -sq));
        float t1 = (fabsf(q) > EPS) ? (q / a) : -1.0f;
        float t2 = (fabsf(q) > EPS) ? (c / q) : -1.0f;
        if      (t1 > EPS && t2 > EPS) t = (t1 < t2) ? t1 : t2;
        else if (t1 > EPS)             t = t1;
        else if (t2 > EPS)             t = t2;
    }

    if (t <= EPS || t > T_MAX) return false;
    *out_t = t;
    return true;
}


/* Vetor unitário do jogador até o inimigo VIVO (não-aliado) mais próximo,
 * com MIRA PREDITIVA: aponta pra onde o inimigo VAI estar quando o projétil
 * chegar, em vez de onde ele está agora. Resolve uma quadrática de
 * interceptação balística usando a velocidade que a IA escreveu no frame
 * anterior (defasagem desprezível com dt~16ms).
 *
 * Fallback pra mira na posição atual quando:
 *   - inimigo está parado (V²~0; inclui congelado)
 *   - não há solução real (inimigo foge mais rápido que dá pra interceptar)
 *   - tempo de interceptação > T_MAX (predição irreal)
 *
 * Engine própria de mira: centraliza o disparo aqui (a Luísa só agenda
 * QUANDO disparar, em magias_tipos.c). Retorna false se não há alvo. */
static bool mirar_mais_proximo(const EstadoJogo *ej,
                               float velocidade_projetil,
                               Vector2 *out_dir) {
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

    Vector2 D = { perto->dados.posicao.x - ej->jogador.posicao.x,
                  perto->dados.posicao.y - ej->jogador.posicao.y };
    Vector2 V = perto->dados.velocidade;

    /* Alvo parado (V·V ~ 0) ou sem interceptação válida: mira direta no alvo. */
    Vector2 alvo;
    float vv = V.x * V.x + V.y * V.y;
    float t;
    if (vv < 1e-4f ||
        !calcular_tempo_intercepcao(D, V, velocidade_projetil, &t)) {
        alvo = perto->dados.posicao;
    } else {
        alvo.x = perto->dados.posicao.x + V.x * t;
        alvo.y = perto->dados.posicao.y + V.y * t;
    }

    float dx = alvo.x - ej->jogador.posicao.x;
    float dy = alvo.y - ej->jogador.posicao.y;
    float c  = sqrtf(dx * dx + dy * dy);
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
    float vproj = PARAMETROS_MAGIA[elemento].velocidade_projetil;
    if (!mirar_mais_proximo(ej, vproj, &dir)) return false;

    magias_spawnar(ej, ej->jogador.posicao, dir, elemento);

    if (ej->motor_profecia.duplica_proximos > 0) {
        ej->motor_profecia.duplica_proximos--;
        /* segundo projétil girado ~15° pra não sobrepor o primeiro */
        Vector2 d2 = { dir.x * 0.9659f - dir.y * 0.2588f,
                       dir.x * 0.2588f + dir.y * 0.9659f };
        magias_spawnar(ej, ej->jogador.posicao, d2, elemento);
    }

    /* Animação de CAST no jogador: vira pra mira e roda anim por janela curta.
     * Não sobrepõe HURT (HURT tem prioridade no atualizar_jogador via timer). */
    ej->jogador.cast_tempo_restante = 0.4f;
    ej->jogador.animacao_atual      = ANIM_CAST;
    ej->jogador.animacao_tempo      = 0.0f;
    if (fabsf(dir.x) > fabsf(dir.y))
        ej->jogador.direcao_atual = (dir.x < 0.0f) ? DIR_LEFT : DIR_RIGHT;
    else
        ej->jogador.direcao_atual = (dir.y < 0.0f) ? DIR_UP   : DIR_DOWN;

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
        const Magia *mg = &mno->dados;
        if (!mg->viva) continue;
        if ((int)mg->elemento < 0 || (int)mg->elemento >= QTD_PARAMETROS_MAGIA) continue;

        Texture2D tex = g_assets.magias[mg->elemento];
        if (tex.id == 0) {
            /* Fallback original: bolinha colorida + núcleo branco. */
            Color cor = PARAMETROS_MAGIA[mg->elemento].cor;
            DrawCircleV(mg->posicao, mg->raio, cor);
            DrawCircleV(mg->posicao, mg->raio * 0.4f, WHITE);
            continue;
        }

        /* Sprite estático (frame único nos PNGs de magia) rotacionado pra
         * apontar na direção do voo. atan2 dá o ângulo da velocidade em rad;
         * RAD2DEG converte porque DrawTexturePro usa graus. */
        float angulo_deg = atan2f(mg->velocidade.y, mg->velocidade.x) * RAD2DEG;
        float lado = mg->raio * 2.0f;
        Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
        Rectangle dst = { mg->posicao.x, mg->posicao.y, lado, lado };
        /* origem = (lado/2, lado/2) → rotaciona em volta do centro do projétil. */
        Vector2 origem = { lado * 0.5f, lado * 0.5f };
        DrawTexturePro(tex, src, dst, origem, angulo_deg, WHITE);
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
