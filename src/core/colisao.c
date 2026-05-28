/* ============================================================================
 * colisao.c - IMPLEMENTAÇÃO DO SISTEMA DE COLISÃO
 * ============================================================================
 *
 * Colisão em jogos 2D se resume a: "essas duas formas estão se sobrepondo?".
 *
 * TRUQUE DE PERFORMANCE IMPORTANTE:
 *   Pra saber se dois círculos se tocam, o cálculo direto seria
 *     distancia = sqrt((ax-bx)^2 + (ay-by)^2);
 *     se distancia <= ra + rb, tocam.
 *   Mas sqrt é caro. Podemos comparar o QUADRADO da distância com o QUADRADO
 *   dos raios somados — dá o mesmo resultado sem o sqrt.
 *
 * COMPLEXIDADE:
 *   colisao_verificar_tudo é O(N×M) onde N=magias, M=inimigos. Pra ~100
 *   inimigos × 200 projéteis = 20000 checagens por frame. A 60fps isso é
 *   ~1.2M/s, ainda tranquilo pra CPU moderna. Se virar gargalo, no futuro
 *   dava pra usar grid espacial, mas por ora KISS.
 * ========================================================================== */

#include "colisao.h"
#include "jogador.h"
#include "inimigos.h"
#include "magias_comportamento.h"
#include "profecia.h"
#include "profecia_efeitos.h"
#include "combos.h"
#include <math.h>


/* ----- PRIMITIVA: CÍRCULO × CÍRCULO ----- */
bool colisao_circulo_circulo(Vector2 a, float ra, Vector2 b, float rb) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dist2 = dx * dx + dy * dy;     /* distância ao quadrado */
    float soma_raios = ra + rb;
    return dist2 <= soma_raios * soma_raios;
}


/* ----- PRIMITIVA: CAIXA × CAIXA (AABB - Axis-Aligned Bounding Box) ----- */
bool colisao_caixa_caixa(Rectangle a, Rectangle b) {
    /* Se uma caixa termina antes da outra começar em qualquer eixo,
     * não há colisão. Todos os 4 testes precisam passar. */
    return a.x < b.x + b.width  &&
           a.x + a.width  > b.x &&
           a.y < b.y + b.height &&
           a.y + a.height > b.y;
}


/* ----- PRIMITIVA: CÍRCULO × CAIXA -----
 * Acha o ponto da caixa mais próximo do círculo (com clamp) e checa
 * distância daquele ponto até o centro do círculo.
 * fminf/fmaxf são da math.h — versões em float de min/max. */
bool colisao_circulo_caixa(Vector2 c, float r, Rectangle caixa) {
    float mais_proximo_x = fmaxf(caixa.x,
                                  fminf(c.x, caixa.x + caixa.width));
    float mais_proximo_y = fmaxf(caixa.y,
                                  fminf(c.y, caixa.y + caixa.height));
    float dx = c.x - mais_proximo_x;
    float dy = c.y - mais_proximo_y;
    return (dx * dx + dy * dy) <= r * r;
}


/* ----- DANO A INIMIGO (caminho único) -----
 * Consome multiplicadores de "próxima hit" (combo Choque Térmico) e o
 * "dano triplo" da profecia, depois subtrai vida. Se zerou, mata pelo
 * caminho único (inimigos_registrar_morte) pra biomassa contar sempre.
 * Lifesteal (roubo_vida) é plugado aqui na Fase 6. */
static void aplicar_dano_inimigo(EstadoJogo *ej, Inimigo *i, float dano) {
    float mult = 1.0f;

    if (i->proxima_hit_multiplicador > 1.0f) {
        mult *= i->proxima_hit_multiplicador;
        i->proxima_hit_multiplicador = 1.0f;   /* consome */
    }
    if (ej->motor_profecia.dano_triplo_proxima > 1.0f) {
        mult *= ej->motor_profecia.dano_triplo_proxima;
        ej->motor_profecia.dano_triplo_proxima = 0.0f;   /* consome */
    }

    float dano_final = dano * mult;
    i->vida -= (int)dano_final;

    /* Lifesteal (EF_ROUBO_DE_VIDA): cura fração do dano enquanto ativo. */
    if (ej->motor_profecia.roubo_vida_tempo > 0.0f) {
        int cura = (int)(dano_final * MAGNITUDES_EFEITO.roubo_vida_frac);
        profecia_curar_jogador(ej, cura);
    }

    if (i->vida <= 0) {
        inimigos_registrar_morte(ej, i);
    }
}


/* ----- RIDER DA MAGIA -----
 * Aplica o efeito de status do elemento (tabela da Luísa) no inimigo
 * acertado. CHAIN salta iterativamente pro inimigo não-atingido mais
 * próximo, com dano reduzido. */
#define MAX_CHAIN_ALVOS 16

static void aplicar_rider_magia(EstadoJogo *ej, MagiaNo *mno, Inimigo *primario) {
    int e = (int)mno->dados.elemento;
    if (e < 0 || e >= QTD_COMPORTAMENTO_MAGIA) return;
    const ComportamentoMagia *cm = &COMPORTAMENTO_MAGIA[e];

    switch (cm->rider) {
        case RIDER_CONGELA:
            if (cm->congela_tempo > primario->congelado_tempo) {
                primario->congelado_tempo = cm->congela_tempo;
            }
            break;

        case RIDER_VENENO:
            if (primario->veneno_stacks < cm->veneno_max_stacks) {
                primario->veneno_stacks++;
            }
            primario->veneno_tempo = cm->veneno_tempo;   /* refresh duração */
            primario->veneno_dps =
                cm->veneno_dps * (float)primario->veneno_stacks;
            break;

        case RIDER_CHAIN: {
            int saltos = cm->chain_saltos;
            if (saltos > MAX_CHAIN_ALVOS - 1) saltos = MAX_CHAIN_ALVOS - 1;

            Inimigo *visitados[MAX_CHAIN_ALVOS];
            int nv = 0;
            visitados[nv++] = primario;
            Inimigo *origem = primario;

            for (int s = 0; s < saltos; s++) {
                Inimigo *melhor = NULL;
                float menor = cm->chain_raio * cm->chain_raio;

                for (InimigoNo *ino = ej->inimigos_cabeca;
                     ino != NULL; ino = ino->proximo) {
                    Inimigo *c = &ino->dados;
                    if (!c->vivo || c->aliado) continue;

                    bool ja = false;
                    for (int k = 0; k < nv; k++) {
                        if (visitados[k] == c) { ja = true; break; }
                    }
                    if (ja) continue;

                    float dx = c->posicao.x - origem->posicao.x;
                    float dy = c->posicao.y - origem->posicao.y;
                    float d2 = dx * dx + dy * dy;
                    if (d2 < menor) { menor = d2; melhor = c; }
                }

                if (melhor == NULL) break;   /* ninguém no alcance */
                aplicar_dano_inimigo(ej, melhor,
                                     mno->dados.dano * cm->chain_dano_frac);
                visitados[nv++] = melhor;
                origem = melhor;
            }
            break;
        }

        case RIDER_NENHUM:
        default:
            break;
    }
}


/* ============================================================================
 * VERIFICAÇÃO DE ALTO NÍVEL
 * --------------------------------------------------------------------------
 * Percorre as LISTAS ENCADEADAS do estado do jogo e resolve colisões.
 *
 * Caminhar em lista encadeada: começa em "cabeça" e segue "próximo" até NULL.
 *   for (InimigoNo *ino = ej->inimigos_cabeca; ino != NULL; ino = ino->proximo)
 *
 * Não removemos nós aqui — só marcamos "vivo = false". A remoção efetiva
 * (com free) fica com quem cuida das listas: Dev 3 nos módulos magias.c e
 * inimigos.c. Separar essas responsabilidades evita bugs de "uso após free".
 * ========================================================================== */
void colisao_verificar_tudo(EstadoJogo *ej) {
    /* ---- 1. Jogador vs Inimigos ---- */
    for (InimigoNo *ino = ej->inimigos_cabeca; ino != NULL; ino = ino->proximo) {
        if (!ino->dados.vivo) continue;   /* pula inimigos já mortos */
        if (ino->dados.aliado) continue;  /* aliado da profecia não fere o jogador */

        /* Vetor do inimigo para o jogador. Distância ao quadrado pra evitar
         * sqrt no caso de "não tocou". */
        float dx = ej->jogador.posicao.x - ino->dados.posicao.x;
        float dy = ej->jogador.posicao.y - ino->dados.posicao.y;
        float distancia2 = dx * dx + dy * dy;
        float soma_raios = ej->jogador.raio + ino->dados.raio;

        if (distancia2 <= soma_raios * soma_raios) {
            /* Escudo da profecia anula 1 hit; senão dano + gatilho da
             * condição "Ao tomar dano". */
            if (ej->motor_profecia.escudo_ativo > 0.0f) {
                ej->motor_profecia.escudo_ativo = 0.0f;
            } else {
                jogador_sofrer_dano(&ej->jogador, (int)ino->dados.dano);
                profecia_evento_ao_receber_dano(ej);
            }

            /* COLISÃO FÍSICA: empurra o jogador pra fora do inimigo pela
             * quantidade do "overlap" (quanto eles estão se sobrepondo).
             * Resultado: o inimigo bloqueia o jogador em vez de ser
             * atravessado.
             *
             * Caso degenerado (centros exatamente sobrepostos → distância=0):
             * empurra pra direita por uma quantidade fixa, só pra evitar a
             * divisão por zero. */
            if (distancia2 > 0.0001f) {
                float distancia = sqrtf(distancia2);
                float overlap   = soma_raios - distancia;
                ej->jogador.posicao.x += (dx / distancia) * overlap;
                ej->jogador.posicao.y += (dy / distancia) * overlap;
            } else {
                ej->jogador.posicao.x += soma_raios;
            }
        }
    }

    /* ---- 2. Magias (projéteis) vs Inimigos ---- */
    for (MagiaNo *mno = ej->magias_cabeca; mno != NULL; mno = mno->proxima) {
        if (!mno->dados.viva) continue;

        for (InimigoNo *ino = ej->inimigos_cabeca;
             ino != NULL;
             ino = ino->proximo) {

            if (!ino->dados.vivo) continue;
            if (ino->dados.aliado) continue;   /* não acerta aliado da profecia */

            /* Raio do projétil vem da struct Magia, preenchido pela engine
             * (magias.c) a partir de PARAMETROS_MAGIA[elemento].raio_projetil. */
            if (colisao_circulo_circulo(mno->dados.posicao, mno->dados.raio,
                                         ino->dados.posicao, ino->dados.raio)) {
                Vector2 ctx = ino->dados.posicao;
                float mc = combos_multiplicador_dano(ej, mno->dados.elemento,
                                                     &ino->dados);
                aplicar_dano_inimigo(ej, &ino->dados, mno->dados.dano * mc);
                aplicar_rider_magia(ej, mno, &ino->dados);
                combos_aplicar_no_acerto(ej, mno->dados.elemento, &ino->dados);
                profecia_evento_ao_acertar(ej, ctx);
                mno->dados.viva       = false;   /* projétil some no hit */
                mno->dados.ja_acertou = true;
                break;   /* 1 inimigo por projétil; chain cuida do resto */
            }
        }
    }

    /* ---- 3. Projéteis de inimigo vs Jogador ----
     * Escudo da profecia (motor.escudo_ativo) anula 1 hit. Senão, dano
     * normal. O gancho profecia_evento_ao_receber_dano entra aqui na Fase 6. */
    for (ProjetilInimigoNo *pno = ej->projeteis_inimigo_cabeca;
         pno != NULL; pno = pno->proximo) {
        if (!pno->dados.vivo) continue;

        if (colisao_circulo_circulo(pno->dados.posicao, pno->dados.raio,
                                     ej->jogador.posicao, ej->jogador.raio)) {
            if (ej->motor_profecia.escudo_ativo > 0.0f) {
                ej->motor_profecia.escudo_ativo = 0.0f;   /* consome o escudo */
            } else {
                jogador_sofrer_dano(&ej->jogador, (int)pno->dados.dano);
                profecia_evento_ao_receber_dano(ej);
            }
            pno->dados.vivo = false;
        }
    }
}
