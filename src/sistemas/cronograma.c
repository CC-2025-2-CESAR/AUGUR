/* ============================================================================
 * cronograma.c - ENGINE DA TIMELINE
 * ============================================================================
 *
 * IMPLEMENTAÇÃO PURA — sem dados de balanceamento. A tabela declarativa de
 * eventos vive em cronograma_eventos.c (responsabilidade da Luísa).
 *
 * FLUXO POR FRAME:
 *   1. Avança tempo_decorrido.
 *   2. Pra cada evento da tabela: se está dentro da janela [inicio, fim) e
 *      não foi marcado como inativo, decrementa timer_interno; quando zera,
 *      pede pra inimigos.c spawnar 1 inimigo do tipo do evento numa posição
 *      aleatória ao redor da câmera.
 *   3. Quando tempo >= 5:00, spawna 1 chefão e desativa todos os outros
 *      eventos (a luta vira só contra o chefão).
 *   4. Quando o chefão morre (lista de inimigos vazia + esperando_chefao_morrer),
 *      seta vitoria = true. O main vê e troca o estado.
 *   5. Quando tempo cruza um múltiplo de CRONOGRAMA_INTERVALO_CARTAS_SEG,
 *      seta cartas_pendentes = true (o main consome quando trata o estado).
 * ============================================================================ */

#include "cronograma.h"
#include "cronograma_eventos.h"
#include "inimigos.h"
#include <math.h>
#include <stdlib.h>


/* Calcula uma posição aleatória logo fora da tela (off-screen) ao redor do
 * jogador. Usa atan2 + projeção pra chegar na borda visível com uma margem.
 * Mesmo padrão usado na sandbox quando spawnava inimigos.
 *
 * MARGEM extra: 60px além da meia-tela; assim o inimigo aparece "vindo de
 * fora" em vez de surgir de surpresa em cima do jogador. */
static Vector2 ponto_de_spawn_borda(const EstadoJogo *ej) {
    const float MARGEM = 60.0f;
    float meia_largura = (LARGURA_TELA / 2.0f) / ej->camera.zoom + MARGEM;
    float meia_altura  = (ALTURA_TELA  / 2.0f) / ej->camera.zoom + MARGEM;

    float angulo = ((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159265f;
    float dx = cosf(angulo);
    float dy = sinf(angulo);

    /* Projeta o vetor (dx,dy) até bater na "caixa" da câmera. Usa o lado
     * dominante: se |dx*meia_altura| > |dy*meia_largura|, bate na lateral. */
    float t_lateral = (fabsf(dx) > 0.0001f) ? meia_largura / fabsf(dx) : 1e9f;
    float t_topo    = (fabsf(dy) > 0.0001f) ? meia_altura  / fabsf(dy) : 1e9f;
    float t = fminf(t_lateral, t_topo);
    const float DIST_MIN = 200.0f;
    if (t < DIST_MIN) t = DIST_MIN;

    Vector2 alvo;
    alvo.x = ej->jogador.posicao.x + dx * t;
    alvo.y = ej->jogador.posicao.y + dy * t;
    return alvo;
}


void cronograma_inicializar(Cronograma *c) {
    c->tempo_decorrido          = 0.0f;
    c->tempo_proxima_carta      = CRONOGRAMA_INTERVALO_CARTAS_SEG;
    c->cartas_pendentes         = false;
    c->chefao_spawnado          = false;
    c->esperando_chefao_morrer  = false;
    c->vitoria                  = false;

    /* Copia a tabela declarativa pro buffer mutável.
     * Os ativos começam todos ligados; tempo_inicio decide se já agem. */
    int n = QTD_EVENTOS_CRONOGRAMA;
    if (n > MAX_EVENTOS_CRONOGRAMA) n = MAX_EVENTOS_CRONOGRAMA;

    for (int i = 0; i < n; i++) {
        c->eventos[i] = EVENTOS_CRONOGRAMA[i];
        c->eventos[i].timer_interno = c->eventos[i].intervalo_spawn;
        c->eventos[i].ativo = true;
    }
    c->qtd_eventos = n;
}


/* Conta quantos inimigos estão vivos na lista. Usado pra detectar vitória. */
static int contar_inimigos_vivos(const EstadoJogo *ej) {
    int n = 0;
    for (const InimigoNo *ino = ej->inimigos_cabeca;
         ino != NULL;
         ino = ino->proximo) {
        if (ino->dados.vivo) n++;
    }
    return n;
}


void cronograma_atualizar(Cronograma *c, EstadoJogo *ej) {
    /* Trava o tempo durante a tela de cartas: o main não chama essa função
     * fora do estado COMBATE, mas como guarda extra também não avança se a
     * carta ainda está pendente (caso o main demore um frame pra trocar). */
    if (c->cartas_pendentes) return;

    c->tempo_decorrido += ej->delta_tempo;

    /* ---- 1. Spawn do chefão aos 5:00 ---- */
    if (!c->chefao_spawnado &&
        c->tempo_decorrido >= CRONOGRAMA_DURACAO_SEG) {

        /* Limpa a arena ANTES de spawnar o chefão. Dois motivos:
         *  - Atmosfera: a luta vira só "você vs ele", coerente com o aviso
         *    visual de boss fight.
         *  - Garantia: aos 5min a lista pode estar perto de MAX_INIMIGOS
         *    (eventos sobrepostos somam vários spawns/s). Sem limpar, o
         *    inimigos_spawnar_em do chefe pode falhar silenciosamente, e o
         *    jogo cairia em ESTADO_VITORIA assim que o jogador matasse os
         *    inimigos restantes — sem chefão na tela. */
        inimigos_liberar_tudo(ej);

        Vector2 pos = ponto_de_spawn_borda(ej);
        inimigos_spawnar_em(ej, pos, INIMIGO_CHEFE);
        c->chefao_spawnado         = true;
        c->esperando_chefao_morrer = true;

        /* Quando o chefão entra, todos os outros eventos param. A briga
         * fica focada nele. */
        for (int i = 0; i < c->qtd_eventos; i++) {
            if (c->eventos[i].tipo != INIMIGO_CHEFE) {
                c->eventos[i].ativo = false;
            }
        }
    }

    /* ---- 2. Detecção de vitória ----
     * Quando o chefão morre, a lista de inimigos fica vazia (todos os
     * outros já foram derrotados antes). */
    if (c->esperando_chefao_morrer && contar_inimigos_vivos(ej) == 0) {
        c->vitoria = true;
        return;   /* não spawna mais nada após vitória */
    }

    /* ---- 3. Iteração dos eventos da timeline ---- */
    for (int i = 0; i < c->qtd_eventos; i++) {
        EventoCronograma *e = &c->eventos[i];
        if (!e->ativo) continue;

        /* Janela do evento: ativo quando tempo está dentro de [inicio, fim). */
        if (c->tempo_decorrido < e->tempo_inicio_seg) continue;
        if (c->tempo_decorrido >= e->tempo_fim_seg) {
            e->ativo = false;
            continue;
        }

        e->timer_interno -= ej->delta_tempo;
        while (e->timer_interno <= 0.0f) {
            Vector2 pos = ponto_de_spawn_borda(ej);
            inimigos_spawnar_em(ej, pos, e->tipo);
            e->timer_interno += e->intervalo_spawn;

            /* Se intervalo_spawn for muito pequeno, evita loop infinito. */
            if (e->intervalo_spawn <= 0.001f) {
                e->ativo = false;
                break;
            }
        }
    }

    /* ---- 4. Trigger de cartas a cada minuto cheio ----
     * Importante: NÃO disparar depois que a vitória está decidida
     * (não queremos abrir cartas no instante da morte do chefão). */
    if (!c->vitoria &&
        !c->esperando_chefao_morrer &&
        c->tempo_decorrido >= c->tempo_proxima_carta) {
        c->cartas_pendentes = true;
    }
}


bool cronograma_deve_abrir_cartas(const Cronograma *c) {
    return c->cartas_pendentes;
}


void cronograma_consumir_carta_pendente(Cronograma *c) {
    c->cartas_pendentes = false;
    c->tempo_proxima_carta += CRONOGRAMA_INTERVALO_CARTAS_SEG;
}
