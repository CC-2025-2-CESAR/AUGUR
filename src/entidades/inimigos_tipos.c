/* ============================================================================
 * inimigos_tipos.c - TABELA DE STATS + IA POR TIPO (LUÍSA EDITA AQUI)
 * ============================================================================
 *
 * Os números abaixo são chutes razoáveis pra começar — ajuste conforme o
 * playtest pedir. Não precisa mexer em mais nenhum arquivo da engine quando
 * for balancear: tudo que a engine faz vem das tabelas e do dispatch de IA.
 * ============================================================================ */

#include "inimigos_tipos.h"
#include "inimigos.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>


/* ----------------------------------------------------------------------------
 * 1. TABELA DE STATS POR TIPO
 * ----------------------------------------------------------------------------
 * A ordem das linhas TEM que casar com a ordem do enum TipoInimigo:
 *   [0] INIMIGO_CORPO_A_CORPO
 *   [1] INIMIGO_A_DISTANCIA
 *   [2] INIMIGO_ELITE
 *   [3] INIMIGO_CHEFE
 *
 * Pra adicionar um tipo novo: adicione no enum (tipos.h) E na tabela aqui.
 * --------------------------------------------------------------------------*/
const ParametrosInimigo PARAMETROS_INIMIGO[] = {
    /* corpo a corpo — base do jogo, anda direto no jogador. */
    {
        .vida_base            = 50,
        .dano                 = 10,
        .velocidade_movimento = 120.0f,
        .raio                 = 14.0f,
        .raio_visual          = 14.0f,
        .cor                  = (Color){ 220,  60,  60, 255 },  /* vermelho */
        .recompensa_biomassa  = 5,
        .comportamento        = IA_CHASE,
    },

    /* à distância — mantém afastado e atira. */
    {
        .vida_base            = 30,
        .dano                 = 8,
        .velocidade_movimento = 120.0f,
        .raio                 = 12.0f,
        .raio_visual          = 12.0f,
        .cor                  = (Color){ 240, 140,  40, 255 },  /* laranja */
        .recompensa_biomassa  = 7,
        .comportamento        = IA_KITER,
    },

    /* elite — corpo a corpo mais forte e mais rápido. */
    {
        .vida_base            = 90,
        .dano                 = 18,
        .velocidade_movimento = 180.0f,
        .raio                 = 20.0f,
        .raio_visual          = 20.0f,
        .cor                  = (Color){ 160,  80, 220, 255 },  /* roxo */
        .recompensa_biomassa  = 25,
        .comportamento        = IA_CHASE,
    },

    /* chefão — encerra a run. Tank com fases por % de vida. */
    {
        .vida_base            = 1000,
        .dano                 = 30,
        .velocidade_movimento = 70.0f,
        .raio                 = 45.0f,
        .raio_visual          = 50.0f,
        .cor                  = (Color){ 240, 200,  60, 255 },  /* dourado */
        .recompensa_biomassa  = 500,
        .comportamento        = IA_BOSS_FASES,
    },

    /* === SUGESTÃO PRÓXIMA: elite à distância (deixa pra depois) ===
     * Pra criar, adicione INIMIGO_ELITE_DISTANCIA no enum em tipos.h e
     * acrescente uma linha aqui copiando o ranged base e bombando os
     * stats (vida ~70, dano ~16, velocidade ~80, cor ciano). Reuse
     * IA_KITER. */
};

const int QTD_PARAMETROS_INIMIGO =
    sizeof(PARAMETROS_INIMIGO) / sizeof(PARAMETROS_INIMIGO[0]);


/* ----------------------------------------------------------------------------
 * 2. FUNÇÕES DE IA
 * ----------------------------------------------------------------------------
 * Cada função recebe o inimigo e o estado do jogo, e ESCREVE em i->velocidade
 * o vetor de movimento desejado neste frame. A engine aplica a posição em
 * seguida (posicao += velocidade * dt).
 *
 * Como são `static`, ficam internas a este arquivo — o exterior só vê o
 * dispatch inimigos_tipos_executar_ia().
 * --------------------------------------------------------------------------*/

/* Helper: vetor unitário do inimigo até o jogador. Retorna (0,0) se for o
 * caso degenerado de coincidência exata. */
static Vector2 direcao_ate_jogador(const Inimigo *i, const EstadoJogo *ej) {
    float dx = ej->jogador.posicao.x - i->posicao.x;
    float dy = ej->jogador.posicao.y - i->posicao.y;
    float comprimento = sqrtf(dx * dx + dy * dy);
    if (comprimento < 0.0001f) return (Vector2){ 0.0f, 0.0f };
    return (Vector2){ dx / comprimento, dy / comprimento };
}


/* Hash determinístico de um ponteiro pra um float em [-1, 1]. Usado pra dar
 * "personalidade" estável a um inimigo sem precisar de um campo id. Como o
 * endereço do nó não muda enquanto ele vive, o valor é constante por toda a
 * vida do inimigo. Multiplicação com a constante de Knuth dispersa bits. */
static float hash_pointer_para_unitario(const void *p) {
    uintptr_t x = (uintptr_t)p;
    uint32_t h = (uint32_t)(x ^ (x >> 16));
    h *= 2654435761u;
    /* Range [0, 1) → [-1, 1) */
    return ((float)h / 4294967296.0f) * 2.0f - 1.0f;
}


/* Chase: anda na direção do jogador. Duas perturbações pra dar vida ao
 * movimento sem virar fila indiana ou linha reta:
 *
 *   1. OFFSET ANGULAR FIXO por inimigo (~±23°), via hash do ponteiro.
 *      Cada melee aproxima por um flanco persistente — uns vêm pela
 *      direita, outros pela esquerda. Distribui os vetores de aproximação.
 *
 *   2. TANGENCIAL PROGRESSIVO: longe vem direto; perto (< 220px) começa
 *      a circular. A intensidade do componente lateral cresce conforme
 *      o melee se aproxima — `(1 - dist/RAIO_TANG) * PESO_TANG`. Lado
 *      (horário/anti-horário) vem do mesmo hash, então é estável e
 *      coerente com o offset fixo. Efeito: na hora do encontro o melee
 *      "dança" um pouco antes de bater, dando frame extra pro jogador
 *      esquivar e tornando a horda menos previsível. */
static void ia_chase(Inimigo *i, EstadoJogo *ej) {
    const float OFFSET_ANG = 0.8f;     /* ±0.4 rad (~±23°) */
    const float RAIO_TANG  = 300.0f;   /* px — começa a circular abaixo disso */
    const float PESO_TANG  = 0.9f;    /* intensidade máxima do tangencial */

    /* Distância e direção até o jogador. */
    float dx = ej->jogador.posicao.x - i->posicao.x;
    float dy = ej->jogador.posicao.y - i->posicao.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 0.0001f) {
        i->velocidade.x = 0.0f;
        i->velocidade.y = 0.0f;
        return;
    }
    float dirx = dx / dist;
    float diry = dy / dist;

    /* Hash do ponteiro: usado pro offset angular E pro lado do tangencial.
     * Ambos usam a mesma personalidade — aproximação por flanco fixo. */
    float h = hash_pointer_para_unitario(i);

    /* 1. Rotação 2D do vetor direção pelo offset angular fixo. */
    float ang = h * OFFSET_ANG;
    float ca = cosf(ang);
    float sa = sinf(ang);
    float rx = dirx * ca - diry * sa;
    float ry = dirx * sa + diry * ca;

    /* 2. Tangencial progressivo: cresce conforme se aproxima. Zera fora do
     * raio de ativação. Usa cópias do (rx, ry) original pra que a soma do
     * y não enxergue o x já modificado — senão o vetor lateral fica torto. */
    if (dist < RAIO_TANG) {
        float intensidade = (1.0f - dist / RAIO_TANG) * PESO_TANG;
        float lado = (h >= 0.0f) ? 1.0f : -1.0f;
        float ox = rx;
        float oy = ry;
        rx = ox + (-oy) * intensidade * lado;
        ry = oy + ( ox) * intensidade * lado;
        /* Renormaliza pra manter velocidade constante. */
        float mag = sqrtf(rx * rx + ry * ry);
        if (mag > 0.0001f) {
            rx /= mag;
            ry /= mag;
        }
    }

    i->velocidade.x = rx * i->velocidade_movimento;
    i->velocidade.y = ry * i->velocidade_movimento;
}


/* Kiter: cada inimigo decide isoladamente — sem coordenação explícita
 * entre eles. A "formação" emerge naturalmente do conjunto:
 *
 *   1. Componente RADIAL (aproxima/recua):
 *        - longe demais (> ideal + zona)  → aproxima
 *        - perto demais (< ideal - zona)  → recua
 *        - dentro da zona ideal           → estaciona radialmente
 *   2. Componente TANGENCIAL (orbita):
 *        - sempre presente, intensidade fixa (peso 0.6)
 *        - lado (horário ou anti-horário) é PERSISTENTE por kiter,
 *          determinado por hash do ponteiro do nó. Metade dos kiters
 *          orbita num sentido, metade no outro — uns ficam no flanco
 *          direito, outros no esquerdo, formando um cerco natural.
 *   3. Vetor final = normalize(radial + tangencial) * velocidade.
 *
 * Por que abandonei a versão "círculo coordenado": parecia carrossel
 * mecânico — todos giravam no mesmo sentido, raio crescia ao spawnar
 * novo, slots saltavam quando algum morria. A versão isolada é mais
 * orgânica, sem custo de O(N²), e o push-out de inimigos.c ainda evita
 * sobreposição. O disparo do kiter é feito pela engine (inimigos.c, PASS 1.5). */
static void ia_kiter(Inimigo *i, EstadoJogo *ej) {
    const float DISTANCIA_IDEAL = 280.0f;
    const float ZONA            =  60.0f;
    const float PESO_TANG       =   1.0f;

    float dx = ej->jogador.posicao.x - i->posicao.x;
    float dy = ej->jogador.posicao.y - i->posicao.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 0.0001f) {
        i->velocidade.x = 0.0f;
        i->velocidade.y = 0.0f;
        return;
    }

    /* Direção radial unitária (aponta do kiter pro jogador). */
    float rx = dx / dist;
    float ry = dy / dist;

    /* Componente radial: 1 quando longe, -1 quando perto, 0 na zona ideal. */
    float radial = 0.0f;
    if      (dist > DISTANCIA_IDEAL + ZONA) radial =  1.0f;
    else if (dist < DISTANCIA_IDEAL - ZONA) radial = -1.0f;

    /* Componente tangencial: perpendicular à radial, com sinal persistente
     * por kiter. Hash >= 0 → orbita no sentido anti-horário; < 0 → horário.
     * Como o hash vem do endereço do nó (estável durante a vida), o sentido
     * não muda entre frames — sem jitter. */
    float lado = (hash_pointer_para_unitario(i) >= 0.0f) ? 1.0f : -1.0f;
    float tx = -ry * lado;
    float ty =  rx * lado;

    /* Soma vetorial e normalização final. */
    float vx = rx * radial + tx * PESO_TANG;
    float vy = ry * radial + ty * PESO_TANG;
    float mag = sqrtf(vx * vx + vy * vy);

    if (mag < 0.0001f) {
        i->velocidade.x = 0.0f;
        i->velocidade.y = 0.0f;
        return;
    }

    i->velocidade.x = (vx / mag) * i->velocidade_movimento;
    i->velocidade.y = (vy / mag) * i->velocidade_movimento;
}


/* Boss: chase implacável com fases por % de vida.
 *   - Acima de 50%:  velocidade base.
 *   - Abaixo de 50%: 1.25× velocidade.
 *   - Abaixo de 25%: 1.50× velocidade + spawna 1 corpo a corpo a cada 5s.
 *
 * O timer dos minions vive como `static float` interno à função: como o
 * cronograma garante que existe no máximo 1 chefão na run inteira, não há
 * conflito entre múltiplas instâncias compartilhando o timer. Reset em
 * jogo_resetar_run() não toca isso, mas o timer começa em 0 no primeiro
 * uso e só conta enquanto pct < 0.25, então qualquer "lixo" residual é
 * sobrescrito pelo `>= INTERVALO_MINIONS` na primeira passagem. */
static void ia_boss_fases(Inimigo *i, EstadoJogo *ej) {
    static float timer_minions = 0.0f;
    const float INTERVALO_MINIONS = 5.0f;
    const float OFFSET_MINION     = 80.0f;   /* spawn perto do boss */

    Vector2 dir = direcao_ate_jogador(i, ej);

    float modificador_velocidade = 1.0f;
    float pct = 1.0f;
    if (i->vida_maxima > 0) {
        pct = (float)i->vida / (float)i->vida_maxima;
        if (pct < 0.50f) modificador_velocidade = 1.25f;
        if (pct < 0.25f) modificador_velocidade = 1.50f;
    }

    i->velocidade.x = dir.x * i->velocidade_movimento * modificador_velocidade;
    i->velocidade.y = dir.y * i->velocidade_movimento * modificador_velocidade;

    /* Fase 3 (< 25% vida): chama reforços. */
    if (pct < 0.25f) {
        timer_minions += ej->delta_tempo;
        if (timer_minions >= INTERVALO_MINIONS) {
            timer_minions = 0.0f;

            float ang = ((float)rand() / (float)RAND_MAX) * 6.28318530718f;
            Vector2 pos = {
                i->posicao.x + cosf(ang) * OFFSET_MINION,
                i->posicao.y + sinf(ang) * OFFSET_MINION,
            };
            inimigos_spawnar_em(ej, pos, INIMIGO_CORPO_A_CORPO);
        }
    } else {
        /* Fora da fase final, mantém o timer pronto pra disparar no
         * primeiro tick assim que a vida cair abaixo de 25%. */
        timer_minions = INTERVALO_MINIONS;
    }
}


/* Dispatcher chamado pela engine. Olha o comportamento do tipo (lido da
 * tabela) e escolhe a função certa. Pra adicionar IA inédita: novo valor
 * em ComportamentoIA, novo case aqui. */
void inimigos_tipos_executar_ia(Inimigo *i, EstadoJogo *ej) {
    /* A tabela é indexada pelo tipo. Se o tipo for inválido (não deveria
     * acontecer), trata como chase silenciosamente. */
    if ((int)i->tipo < 0 || (int)i->tipo >= QTD_PARAMETROS_INIMIGO) {
        ia_chase(i, ej);
        return;
    }

    switch (PARAMETROS_INIMIGO[i->tipo].comportamento) {
        case IA_CHASE:      ia_chase(i, ej);      break;
        case IA_KITER:      ia_kiter(i, ej);      break;
        case IA_BOSS_FASES: ia_boss_fases(i, ej); break;
        default:            ia_chase(i, ej);      break;
    }
}
