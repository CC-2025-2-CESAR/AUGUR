/* ============================================================================
 * profecia.c - IMPLEMENTAÇÃO DO GERADOR DE PROFECIAS
 * ============================================================================
 *
 * CONCEITOS IMPORTANTES USADOS AQUI:
 *   - MATRIZES de strings (requisito obrigatório de PIF): tabelas fixas com
 *     os nomes de cada enum. Consulta direta por índice.
 *   - Gerador pseudoaleatório determinístico via srand(seed) + rand().
 *     Mesma seed = mesma profecia toda vez.
 *   - "static const" nas tabelas: "static" = visível só nesse arquivo;
 *     "const" = somente leitura. Protege contra modificação acidental.
 *
 * SIMPLIFICAÇÃO (v3): pool encolheu pra 6 elementos × 4 condições × 6 efeitos
 * (~3 milhões de profecias) e o texto de revelação agora mostra magnitude
 * explícita ("Explosao de Fogo (40 dano, raio 150)") pra cada modificador.
 * O motor só dispara as 4 condições novas — todos os ramos antigos saíram.
 *
 * ATENÇÃO: as strings das tabelas abaixo VÃO PARA O DrawText() em
 * profecia_desenhar. Como a fonte default do Raylib é ASCII puro, as
 * strings ficam SEM acentos ("Relampago", "Explosao") — se acentuar aqui,
 * vira quadradinho na tela. Os comentários, sim, podem ter acento.
 * ========================================================================== */

#include "profecia.h"
#include "profecia_efeitos.h"
#include "inimigos.h"
#include "raylib.h"
#include <stdlib.h>   /* srand, rand */
#include <stdio.h>    /* snprintf */
#include <math.h>     /* sqrtf */


/* ============================================================================
 * TABELAS DE NOMES
 * --------------------------------------------------------------------------
 * MATRIZES (arrays) de strings. Cada índice corresponde ao valor do enum.
 * Ordem tem que bater com a ordem dos enums em tipos.h.
 *
 * Strings sem acento de propósito (vão pra DrawText, fonte ASCII).
 * ========================================================================== */

static const char *nomes_elementos[ELEMENTO_TOTAL] = {
    "Fogo",
    "Gelo",
    "Relampago",
    "Veneno",
    "Arcano",
    "Sombra"
};

static const char *nomes_condicoes[COND_TOTAL] = {
    "Ao matar",
    "Ao tomar dano",
    "Ao acertar",
    "A cada 10s"
};

static const char *nomes_efeitos[EF_TOTAL] = {
    "Explosao",
    "Cura",
    "Escudo",
    "Ignite",
    "Congelar",
    "Duplica projetil"
};


/* ============================================================================
 * GERAÇÃO DA PROFECIA
 * --------------------------------------------------------------------------
 * srand(seed) reinicia o gerador aleatório com um valor fixo. A partir daí,
 * cada chamada de rand() produz a mesma sequência de números. É por isso
 * que "mesma seed = mesma profecia".
 *
 * rand() % N retorna um valor entre 0 e N-1, que cai exatamente no range
 * de um enum. Pequena imperfeição: "modulo bias" — números baixos têm
 * probabilidade ligeiramente maior. Pra um projeto acadêmico, irrelevante.
 * ========================================================================== */
void profecia_gerar(Profecia *p, unsigned int seed) {
    p->seed = seed;
    srand(seed);

    /* Sorteia os 3 modificadores */
    for (int i = 0; i < 3; i++) {
        p->mods[i].elemento = (Elemento)(rand() % ELEMENTO_TOTAL);
        p->mods[i].condicao = (Condicao)(rand() % COND_TOTAL);
        p->mods[i].efeito   = (Efeito)  (rand() % EF_TOTAL);
    }
}


/* ============================================================================
 * LOOKUPS
 * --------------------------------------------------------------------------
 * Recebem um valor de enum e retornam a string correspondente.
 * Evitam switch gigante em todo lugar.
 * ========================================================================== */
const char *elemento_nome(Elemento e) {
    if (e < 0 || e >= ELEMENTO_TOTAL) return "???";
    return nomes_elementos[e];
}

const char *condicao_nome(Condicao c) {
    if (c < 0 || c >= COND_TOTAL) return "???";
    return nomes_condicoes[c];
}

const char *efeito_nome(Efeito e) {
    if (e < 0 || e >= EF_TOTAL) return "???";
    return nomes_efeitos[e];
}


/* ============================================================================
 * TEXTO DE MAGNITUDE
 * --------------------------------------------------------------------------
 * Cada efeito tem um número específico (dano, raio, duração) que vive em
 * profecia_efeitos.c. Esta função monta uma string curta que descreve QUANTO
 * o efeito faz, pra colocar embaixo do texto da condição na tela de revelação.
 *
 * Resultado é puro ASCII (DrawText): "(40 dano, raio 150)".
 * ========================================================================== */
static void texto_magnitude(Efeito ef, char *buf, int tam) {
    const MagnitudesEfeito *M = &MAGNITUDES_EFEITO;
    switch (ef) {
        case EF_EXPLOSAO:
            snprintf(buf, tam, "(%d dano, raio %d)",
                     (int)M->explosao_dano, (int)M->explosao_raio);
            break;
        case EF_CURA:
            snprintf(buf, tam, "(+%d HP)", M->cura_hp);
            break;
        case EF_ESCUDO:
            snprintf(buf, tam, "(anula o proximo hit recebido)");
            break;
        case EF_IGNITE:
            snprintf(buf, tam, "(%d dano/s por %ds, raio %d)",
                     (int)M->ignite_dps, (int)M->ignite_tempo,
                     (int)M->ignite_raio);
            break;
        case EF_CONGELAR:
            snprintf(buf, tam, "(stun %ds, raio %d)",
                     (int)M->congelar_tempo, (int)M->congelar_raio);
            break;
        case EF_DUPLICA_PROJETIL:
            snprintf(buf, tam, "(%d proximos disparos duplicam)",
                     M->duplica_qtd);
            break;
        default:
            buf[0] = '\0';
            break;
    }
}


/* ============================================================================
 * DESENHO
 * --------------------------------------------------------------------------
 * Mostra a profecia formatada na tela. Cada modificador em 2 linhas: a
 * primeira em DOURADO descreve "QUANDO + EFEITO de ELEMENTO", a segunda em
 * cinza mostra a magnitude explícita. Resultado é um puzzle legível, com o
 * jogador sabendo exatamente o que cada mod faz antes da run começar.
 *
 * snprintf é uma versão "segura" do sprintf: nunca escreve além do tamanho
 * do buffer, evitando buffer overflow.
 * ========================================================================== */
void profecia_desenhar(const Profecia *p) {
    DrawText("PROFECIA", LARGURA_TELA / 2 - 130, 60, 56, GOLD);

    /* Mostra seed pequena abaixo do título, pra identificar a run */
    char buffer_seed[64];
    snprintf(buffer_seed, sizeof(buffer_seed), "Seed: %u", p->seed);
    DrawText(buffer_seed, LARGURA_TELA / 2 - 60, 140, 18, GRAY);

    /* Desenha os 3 modificadores. Cada um numerado com I, II, III. */
    const char *numerais[3] = { "I", "II", "III" };

    for (int i = 0; i < 3; i++) {
        const Modificador *m = &p->mods[i];
        int y_principal  = 200 + i * 100;
        int y_magnitude  = y_principal + 32;

        /* Linha principal: "I. AO MATAR um inimigo: Explosao de Fogo" */
        char principal[192];
        snprintf(principal, sizeof(principal),
                 "%s.  %s:  %s de %s",
                 numerais[i],
                 condicao_nome(m->condicao),
                 efeito_nome(m->efeito),
                 elemento_nome(m->elemento));
        DrawText(principal, 130, y_principal, 24, WHITE);

        /* Linha de magnitude: "(40 dano, raio 150)" */
        char magnitude[96];
        texto_magnitude(m->efeito, magnitude, sizeof(magnitude));
        if (magnitude[0] != '\0') {
            DrawText(magnitude, 170, y_magnitude, 18, LIGHTGRAY);
        }
    }
}


/* ============================================================================
 * MOTOR DE PROFECIA
 * --------------------------------------------------------------------------
 * Avalia as Condições dos 3 mods e aplica os Efeitos. Toda magnitude vem de
 * MAGNITUDES_EFEITO / LIMIARES_CONDICAO (profecia_efeitos.c — Luísa tuna).
 *
 * Simplificação (v3): só 4 condições ativas. Removidos os ramos de
 * VIDA_ABAIXO_X, EM_COMBO, AO_CURAR, PRIMEIRA_HIT, AO_ROLAR_DADO,
 * INICIO_RUN — junto com os estados temporários associados.
 * ========================================================================== */

/* Distância ao quadrado entre dois pontos (evita sqrt em checagem de raio). */
static float dist2(Vector2 a, Vector2 b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

/* Dano em área (EF_EXPLOSAO): a morte passa pelo caminho único, então o
 * kill conta biomassa/combo igual a qualquer outro. */
static void dano_em_area(EstadoJogo *ej, Vector2 centro, float raio, float dano) {
    float r2 = raio * raio;
    for (InimigoNo *ino = ej->inimigos_cabeca; ino; ino = ino->proximo) {
        Inimigo *d = &ino->dados;
        if (!d->vivo || d->aliado) continue;
        if (dist2(centro, d->posicao) > r2) continue;
        d->vida -= (int)dano;
        if (d->vida <= 0) inimigos_registrar_morte(ej, d);
    }
}

/* Aplica um status em todos os inimigos dentro de um raio (congela/ignite). */
static void status_em_area(EstadoJogo *ej, Vector2 centro, float raio,
                           bool congela, float congela_t,
                           bool ignite, float ignite_t, float ignite_dps) {
    float r2 = raio * raio;
    for (InimigoNo *ino = ej->inimigos_cabeca; ino; ino = ino->proximo) {
        Inimigo *d = &ino->dados;
        if (!d->vivo || d->aliado) continue;
        if (dist2(centro, d->posicao) > r2) continue;
        if (congela && congela_t > d->congelado_tempo) {
            d->congelado_tempo = congela_t;
        }
        if (ignite) {
            if (ignite_t > d->veneno_tempo) d->veneno_tempo = ignite_t;
            if (ignite_dps > d->veneno_dps) d->veneno_dps = ignite_dps;
            if (d->veneno_stacks < 1) d->veneno_stacks = 1;
        }
    }
}


void profecia_aplicar_efeito(EstadoJogo *ej, Efeito ef, Vector2 ctx) {
    MotorProfecia *mp = &ej->motor_profecia;
    if (mp->em_aplicar_efeito) return;   /* corta recursão (efeito que mata) */
    mp->em_aplicar_efeito = true;

    const MagnitudesEfeito *M = &MAGNITUDES_EFEITO;

    switch (ef) {
        case EF_EXPLOSAO:
            dano_em_area(ej, ctx, M->explosao_raio, M->explosao_dano);
            break;
        case EF_CURA:
            profecia_curar_jogador(ej, M->cura_hp);
            break;
        case EF_ESCUDO:
            mp->escudo_ativo = 1.0f;   /* anula o próximo hit */
            break;
        case EF_IGNITE:
            status_em_area(ej, ctx, M->ignite_raio,
                           false, 0.0f, true, M->ignite_tempo, M->ignite_dps);
            break;
        case EF_CONGELAR:
            status_em_area(ej, ctx, M->congelar_raio,
                           true, M->congelar_tempo, false, 0.0f, 0.0f);
            break;
        case EF_DUPLICA_PROJETIL:
            mp->duplica_proximos += M->duplica_qtd;
            break;
        case EF_TOTAL:
        default:
            break;
    }

    mp->em_aplicar_efeito = false;
}


/* Dispara, pra cada um dos 3 mods cuja Condição == c, o Efeito do mod. */
static void disparar_condicao(EstadoJogo *ej, Condicao c, Vector2 ctx) {
    for (int m = 0; m < 3; m++) {
        if (ej->profecia.mods[m].condicao == c) {
            profecia_aplicar_efeito(ej, ej->profecia.mods[m].efeito, ctx);
        }
    }
}


void profecia_curar_jogador(EstadoJogo *ej, int qtd) {
    if (qtd <= 0) return;
    ej->jogador.vida += qtd;
    if (ej->jogador.vida > ej->jogador.vida_maxima) {
        ej->jogador.vida = ej->jogador.vida_maxima;
    }
    profecia_evento_ao_curar(ej);
}


/* Mantida pra compatibilidade com call-sites do main.c — agora é no-op porque
 * COND_INICIO_RUN saiu do pool. Não removo a função pra manter a interface
 * estável (o main chama explicitamente ao iniciar a run). */
void profecia_evento_inicio_run(EstadoJogo *ej) {
    (void)ej;
}


void profecia_evento_ao_matar(EstadoJogo *ej, Inimigo *morto) {
    Vector2 ctx = (morto != NULL) ? morto->posicao : ej->jogador.posicao;
    disparar_condicao(ej, COND_AO_MATAR, ctx);
}


void profecia_evento_ao_receber_dano(EstadoJogo *ej) {
    disparar_condicao(ej, COND_AO_RECEBER_DANO, ej->jogador.posicao);
}


void profecia_evento_ao_acertar(EstadoJogo *ej, Vector2 ctx) {
    disparar_condicao(ej, COND_AO_ACERTAR, ctx);
}


/* Mantida pra compatibilidade — COND_AO_CURAR saiu do pool. */
void profecia_evento_ao_curar(EstadoJogo *ej) {
    (void)ej;
}


/* Gancho pro sistema de dados (Sofia): COND_AO_ROLAR_DADO saiu do pool, então
 * essa função virou no-op. Mantida pra não quebrar call-sites futuros. */
void profecia_evento_ao_rolar_dado(EstadoJogo *ej) {
    (void)ej;
}


void profecia_motor_atualizar(EstadoJogo *ej) {
    float dt = ej->delta_tempo;
    MotorProfecia *mp = &ej->motor_profecia;

    /* Só sobrou COND_A_CADA_N_SEG no motor — todas as outras 3 condições do
     * pool são edge-triggered por eventos externos (ao_matar, ao_receber_dano,
     * ao_acertar), não precisam tick por frame. */
    for (int m = 0; m < 3; m++) {
        Condicao c = ej->profecia.mods[m].condicao;
        if (c != COND_A_CADA_N_SEG) continue;

        mp->timer_cond[m] += dt;
        if (mp->timer_cond[m] >= LIMIARES_CONDICAO.a_cada_n_seg) {
            mp->timer_cond[m] -= LIMIARES_CONDICAO.a_cada_n_seg;
            profecia_aplicar_efeito(ej, ej->profecia.mods[m].efeito,
                                    ej->jogador.posicao);
        }
    }
}
