/* ============================================================================
 * cartas.c - SISTEMA DE CARTAS DE UPGRADE (Sofia / Dev 2)
 * ----------------------------------------------------------------------------
 * A cada minuto cheio abre a tela de upgrade com 3 cartas sorteadas. O jogador
 * escolhe uma (1/2/3) ou gasta um dado pra rolar o valor antes (R + 1/2/3).
 * Os textos das cartas vivem na MATRIZ TABELA_CARTAS[tipo][raridade].
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cartas.h"
#include "dados.h"

#define INICIO_CARTA_X  160
#define INICIO_CARTA_Y  200
#define LARGURA_CARTA   240
#define ALTURA_CARTA    320
#define GAP_CARTAS      360

/* Marca se a carta i já foi rolada com dado nesta tela (uma rolagem por carta). */
static bool carta_ja_rolada[CARTAS_POR_ESCOLHA] = { false, false, false };

typedef struct {
    char *nome;
    char *descricao;
    int   valor;
} DadosCarta;

/* MATRIZ [tipo][raridade] — 6 raridades (comum..lendária) por tipo de carta.
 * A ordem das linhas casa com o enum TipoCarta. */
DadosCarta TABELA_CARTAS[CARTA_TOTAL][6] = {
    {   /* CARTA_DANO_UP */
        { "Dano nvl comum",     "Aumenta o dano das suas magias.\n+%d de dano.", 10 },
        { "Dano nvl incomum",   "Aumenta o dano das suas magias.\n+%d de dano.", 15 },
        { "Dano nvl raro",      "Aumenta o dano das suas magias.\n+%d de dano.", 20 },
        { "Dano nvl epico",     "Aumenta o dano das suas magias.\n+%d de dano.", 25 },
        { "Dano nvl mitico",    "Aumenta o dano das suas magias.\n+%d de dano.", 30 },
        { "Dano nvl lendario",  "Aumenta o dano das suas magias.\n+%d de dano.", 35 },
    },
    {   /* CARTA_VIDA_UP */
        { "Vida nvl comum",     "Aumenta seu HP maximo.\n+%d de HP.", 15 },
        { "Vida nvl incomum",   "Aumenta seu HP maximo.\n+%d de HP.", 20 },
        { "Vida nvl raro",      "Aumenta seu HP maximo.\n+%d de HP.", 30 },
        { "Vida nvl epico",     "Aumenta seu HP maximo.\n+%d de HP.", 35 },
        { "Vida nvl mitico",    "Aumenta seu HP maximo.\n+%d de HP.", 40 },
        { "Vida nvl lendario",  "Aumenta seu HP maximo.\n+%d de HP.", 50 },
    },
    {   /* CARTA_VELOCIDADE_UP */
        { "Velocidade nvl comum",    "Aumenta sua velocidade de movimento.\n+%d de velocidade.",  5 },
        { "Velocidade nvl incomum",  "Aumenta sua velocidade de movimento.\n+%d de velocidade.", 10 },
        { "Velocidade raro",         "Aumenta sua velocidade de movimento.\n+%d de velocidade.", 15 },
        { "Velocidade epico",        "Aumenta sua velocidade de movimento.\n+%d de velocidade.", 20 },
        { "Velocidade mitico",       "Aumenta sua velocidade de movimento.\n+%d de velocidade.", 25 },
        { "Velocidade lendario",     "Aumenta sua velocidade de movimento.\n+%d de velocidade.", 18 },
    },
    {   /* CARTA_RECARGA_DADO */
        { "Recarregar Dado",    "Devolve um dado para voce.\n+%d dado.",     1 },
        { "Recarregar 2 Dados", "Devolve dois dados para voce.\n+%d dados.", 2 },
        { "Recarregar 3 Dados", "Devolve tres dados para voce.\n+%d dados.", 3 },
        { "Recarregar 4 Dados", "Devolve tres dados para voce.\n+%d dados.", 4 },
        { "Recarregar 5 Dados", "Devolve tres dados para voce.\n+%d dados.", 5 },
        { "Recarregar 6 Dados", "Devolve tres dados para voce.\n+%d dados.", 6 },
    },
};

/* Sorteia as 3 cartas da tela: tipo aleatório + raridade por curva de chance. */
void cartas_gerar_escolhas(EstadoJogo *ej) {
    for (int i = 0; i < CARTAS_POR_ESCOLHA; i++)
        carta_ja_rolada[i] = false;

    for (int i = 0; i < CARTAS_POR_ESCOLHA; i++) {
        int tipo    = rand() % CARTA_TOTAL;
        int sorteio = rand() % 100;
        int raridade;
        if      (sorteio < 55) raridade = 0;   /* comum */
        else if (sorteio < 75) raridade = 1;   /* incomum */
        else if (sorteio < 85) raridade = 2;   /* raro */
        else if (sorteio < 93) raridade = 3;   /* épico */
        else if (sorteio < 98) raridade = 4;   /* mítico */
        else                   raridade = 5;   /* lendário */

        ej->escolhas_upgrade[i].tipo     = tipo;
        ej->escolhas_upgrade[i].raridade = raridade;
        ej->escolhas_upgrade[i].valor    = TABELA_CARTAS[tipo][raridade].valor;
        strncpy(ej->escolhas_upgrade[i].nome, TABELA_CARTAS[tipo][raridade].nome, 64);
        snprintf(ej->escolhas_upgrade[i].descricao, 256,
                 TABELA_CARTAS[tipo][raridade].descricao,
                 TABELA_CARTAS[tipo][raridade].valor);
    }
}

/* Aplica o efeito da carta escolhida no jogador. */
void cartas_aplicar(EstadoJogo *ej, int indice_escolhido) {
    if (indice_escolhido < 0 || indice_escolhido >= CARTAS_POR_ESCOLHA)
        return;

    Carta carta = ej->escolhas_upgrade[indice_escolhido];

    if (carta.tipo == CARTA_DANO_UP) {
        ej->jogador.bonus_dano += carta.valor;

    } else if (carta.tipo == CARTA_VIDA_UP) {
        ej->jogador.vida_maxima += carta.valor;
        ej->jogador.vida        += carta.valor;

    } else if (carta.tipo == CARTA_VELOCIDADE_UP) {
        ej->jogador.velocidade_movimento += (float)carta.valor;

    } else if (carta.tipo == CARTA_RECARGA_DADO) {
        /* Recarrega até `valor` dados gastos (deixa-os carregados de novo). */
        int qtd = carta.valor;
        if (qtd > MAX_DADOS_JOGADOR) qtd = MAX_DADOS_JOGADOR;
        int recarregados = 0;
        for (int k = 0; k < MAX_DADOS_JOGADOR && recarregados < qtd; k++) {
            if (!dado_esta_carregado(&ej->dados_ativos[k])) {
                ej->dados_ativos[k].ultimo_resultado = 0;
                recarregados++;
            }
        }
    }

    for (int i = 0; i < CARTAS_POR_ESCOLHA; i++)
        carta_ja_rolada[i] = false;
}

/* Gasta o primeiro dado carregado pra rolar o valor da carta i (uma vez só). */
bool cartas_usar_dado(EstadoJogo *ej, int indice_carta) {
    if (indice_carta < 0 || indice_carta >= CARTAS_POR_ESCOLHA) return false;
    if (carta_ja_rolada[indice_carta]) return false;

    int slot_dado = -1;
    for (int k = 0; k < MAX_DADOS_JOGADOR; k++) {
        if (dado_esta_carregado(&ej->dados_ativos[k])) { slot_dado = k; break; }
    }
    if (slot_dado < 0) return false;

    int resultado = dado_rolar(&ej->dados_ativos[slot_dado]);
    int faces     = ej->dados_ativos[slot_dado].faces;
    dado_aplicar_na_carta(resultado, faces, &ej->escolhas_upgrade[indice_carta]);
    carta_ja_rolada[indice_carta] = true;
    return true;
}

/* Desenha as 3 cartas (borda por raridade) + os dados embaixo. Strings sem
 * acento (fonte default do Raylib é ASCII). */
void cartas_desenhar_ui(const EstadoJogo *ej) {
    const Color CORES_RARIDADE[] = { GRAY, GREEN, BLUE, PURPLE, RED, GOLD };

    for (int i = 0; i < CARTAS_POR_ESCOLHA; i++) {
        const Carta *c = &ej->escolhas_upgrade[i];
        int x = INICIO_CARTA_X + i * GAP_CARTAS;
        int y = INICIO_CARTA_Y;
        Color cor = CORES_RARIDADE[c->raridade];

        if (carta_ja_rolada[i])
            DrawRectangle(x - 4, y - 4, LARGURA_CARTA + 8, ALTURA_CARTA + 8, GOLD);
        DrawRectangle(x - 2, y - 2, LARGURA_CARTA + 4, ALTURA_CARTA + 4, cor);
        DrawRectangle(x, y, LARGURA_CARTA, ALTURA_CARTA, BLACK);

        DrawText(c->nome, x + 10, y + 15, 16, cor);
        DrawLine(x + 10, y + 40, x + LARGURA_CARTA - 10, y + 40, DARKGRAY);
        DrawText(c->descricao, x + 10, y + 55, 14, LIGHTGRAY);

        if (carta_ja_rolada[i])
            DrawText("~ DADO ~", x + LARGURA_CARTA / 2 - 32, y + ALTURA_CARTA - 55, 14, GOLD);

        char tecla[8];
        snprintf(tecla, sizeof(tecla), "[%d]", i + 1);
        DrawText(tecla, x + LARGURA_CARTA / 2 - 10, y + ALTURA_CARTA - 30, 18, WHITE);
    }

    int dados_carregados = 0;
    for (int k = 0; k < MAX_DADOS_JOGADOR; k++)
        if (dado_esta_carregado(&ej->dados_ativos[k])) dados_carregados++;

    int dado_base_x = LARGURA_TELA / 2 - (MAX_DADOS_JOGADOR * 55) / 2;
    int dado_base_y = ALTURA_TELA - 80;
    for (int k = 0; k < MAX_DADOS_JOGADOR; k++)
        dado_desenhar(&ej->dados_ativos[k], dado_base_x + k * 55, dado_base_y);

    if (dados_carregados > 0)
        DrawText("R + [1/2/3] para rolar dado na carta",
                 LARGURA_TELA / 2 - 210, ALTURA_TELA - 110, 18, YELLOW);
    else
        DrawText("Sem dados disponiveis",
                 LARGURA_TELA / 2 - 110, ALTURA_TELA - 110, 18, DARKGRAY);
}
