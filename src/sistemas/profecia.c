/* ============================================================================
 * profecia.c - GERADOR + MOTOR DE PROFECIAS
 * ----------------------------------------------------------------------------
 * Gera 3 modificadores [Elemento]+[Condição]+[Efeito] a partir de uma seed
 * (determinístico: mesma seed = mesma profecia) e, durante o combate, avalia
 * as Condições e aplica os Efeitos. As magnitudes vivem em profecia_efeitos.c
 * (Luísa tuna). As strings das tabelas vão pro DrawText, então ficam SEM acento
 * (a fonte default do Raylib é ASCII).
 * ============================================================================ */

#include "profecia.h"
#include "profecia_efeitos.h"
#include "inimigos.h"
#include "raylib.h"
#include <stdlib.h>   /* srand, rand */
#include <stdio.h>    /* snprintf */
#include <math.h>


/* Tabelas de nomes (MATRIZ de strings — requisito do PIF). A ordem casa com a
 * ordem dos enums em tipos.h. Sem acento de propósito (vão pro DrawText). */
static const char *nomes_elementos[ELEMENTO_TOTAL] = {
    "Fogo", "Gelo", "Relampago", "Veneno", "Arcano", "Sombra"
};

static const char *nomes_condicoes[COND_TOTAL] = {
    "Ao matar", "Ao tomar dano", "Ao acertar", "A cada 10s"
};

static const char *nomes_efeitos[EF_TOTAL] = {
    "Explosao", "Cura", "Escudo", "Ignite", "Congelar", "Duplica projetil"
};


/* Gera a profecia. srand(seed) fixa a sequência de rand(), então a mesma seed
 * sempre produz os mesmos 3 modificadores. */
void profecia_gerar(Profecia *p, unsigned int seed) {
    p->seed = seed;
    srand(seed);
    for (int i = 0; i < 3; i++) {
        p->mods[i].elemento = (Elemento)(rand() % ELEMENTO_TOTAL);
        p->mods[i].condicao = (Condicao)(rand() % COND_TOTAL);
        p->mods[i].efeito   = (Efeito)  (rand() % EF_TOTAL);
    }
}


/* Lookups enum -> string (com guarda de range). */
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


/* Monta a string de magnitude de um efeito ("(40 dano, raio 150)") pra mostrar
 * embaixo do texto da condição na tela de revelação. Os números vêm de
 * profecia_efeitos.c. */
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


/* Desenha a profecia (estado REVELACAO_PROFECIA): cada mod em 2 linhas — a
 * condição/efeito/elemento em branco e a magnitude em cinza. */
void profecia_desenhar(const Profecia *p) {
    DrawText("PROFECIA", LARGURA_TELA / 2 - 130, 60, 56, GOLD);

    char buffer_seed[64];
    snprintf(buffer_seed, sizeof(buffer_seed), "Seed: %u", p->seed);
    DrawText(buffer_seed, LARGURA_TELA / 2 - 60, 140, 18, GRAY);

    const char *numerais[3] = { "I", "II", "III" };
    for (int i = 0; i < 3; i++) {
        const Modificador *m = &p->mods[i];
        int y_principal = 200 + i * 100;

        char principal[192];
        snprintf(principal, sizeof(principal),
                 "%s.  %s:  %s de %s",
                 numerais[i],
                 condicao_nome(m->condicao),
                 efeito_nome(m->efeito),
                 elemento_nome(m->elemento));
        DrawText(principal, 130, y_principal, 24, WHITE);

        char magnitude[96];
        texto_magnitude(m->efeito, magnitude, sizeof(magnitude));
        if (magnitude[0] != '\0') {
            DrawText(magnitude, 170, y_principal + 32, 18, LIGHTGRAY);
        }
    }
}


/* ============================================================================
 * MOTOR — avalia Condições e aplica Efeitos. Magnitudes em profecia_efeitos.c.
 * ========================================================================== */

/* Distância ao quadrado (evita sqrt na checagem de raio). */
static float dist2(Vector2 a, Vector2 b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

/* Dano em área (EF_EXPLOSAO): mata pelo caminho único pra o kill contar. */
static void dano_em_area(EstadoJogo *ej, Vector2 centro, float raio, float dano) {
    float r2 = raio * raio;
    for (InimigoNo *ino = ej->inimigos_cabeca; ino; ino = ino->proximo) {
        Inimigo *d = &ino->dados;
        if (!d->vivo) continue;
        if (dist2(centro, d->posicao) > r2) continue;
        d->vida -= (int)dano;
        if (d->vida <= 0) inimigos_registrar_morte(ej, d);
    }
}

/* Aplica congela/ignite em todos os inimigos dentro de um raio. */
static void status_em_area(EstadoJogo *ej, Vector2 centro, float raio,
                           bool congela, float congela_t,
                           bool ignite, float ignite_t, float ignite_dps) {
    float r2 = raio * raio;
    for (InimigoNo *ino = ej->inimigos_cabeca; ino; ino = ino->proximo) {
        Inimigo *d = &ino->dados;
        if (!d->vivo) continue;
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


/* Aplica UM efeito no contexto dado. Tem guard de reentrância: um efeito que
 * mata (explosão) não recursiona em "Ao matar" infinitamente. */
void profecia_aplicar_efeito(EstadoJogo *ej, Efeito ef, Vector2 ctx) {
    MotorProfecia *mp = &ej->motor_profecia;
    if (mp->em_aplicar_efeito) return;
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
            mp->escudo_ativo = 1.0f;
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


/* Dispara, pra cada mod cuja Condição == c, o Efeito daquele mod. */
static void disparar_condicao(EstadoJogo *ej, Condicao c, Vector2 ctx) {
    for (int m = 0; m < 3; m++) {
        if (ej->profecia.mods[m].condicao == c) {
            profecia_aplicar_efeito(ej, ej->profecia.mods[m].efeito, ctx);
        }
    }
}


/* Cura o jogador com clamp em vida_maxima (usado por EF_CURA). */
void profecia_curar_jogador(EstadoJogo *ej, int qtd) {
    if (qtd <= 0) return;
    ej->jogador.vida += qtd;
    if (ej->jogador.vida > ej->jogador.vida_maxima) {
        ej->jogador.vida = ej->jogador.vida_maxima;
    }
}


/* Gatilhos pontuais, chamados pela engine quando o evento acontece. */
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


/* Tick por frame: só COND_A_CADA_N_SEG precisa de timer (as outras 3 são
 * disparadas por eventos externos). */
void profecia_motor_atualizar(EstadoJogo *ej) {
    float dt = ej->delta_tempo;
    MotorProfecia *mp = &ej->motor_profecia;

    for (int m = 0; m < 3; m++) {
        if (ej->profecia.mods[m].condicao != COND_A_CADA_N_SEG) continue;

        mp->timer_cond[m] += dt;
        if (mp->timer_cond[m] >= LIMIARES_CONDICAO.a_cada_n_seg) {
            mp->timer_cond[m] -= LIMIARES_CONDICAO.a_cada_n_seg;
            profecia_aplicar_efeito(ej, ej->profecia.mods[m].efeito,
                                    ej->jogador.posicao);
        }
    }
}
