/* ============================================================================
 * main.c - PONTO DE ENTRADA E GAME LOOP
 * ============================================================================
 *
 * Responsabilidade: inicializar a janela do Raylib, rodar o loop principal
 * e chamar os módulos certos dependendo do estado atual do jogo.
 *
 * COMO FUNCIONA UM GAME LOOP:
 *   while (janela aberta) {
 *       1. Calcular delta_tempo (tempo desde o último frame)
 *       2. Atualizar estado (mover coisas, checar colisão, etc.)
 *       3. Desenhar tudo na tela
 *   }
 * A cada iteração = 1 frame. Com SetTargetFPS(60), o Raylib garante que o loop
 * roda ~60 vezes por segundo.
 *
 * IMPORTANTE — MÁQUINA DE ESTADOS:
 *   O jogo tem várias "telas" (menu, combate, upgrade, game over). Em vez de
 *   ter um loop gigante com 20 if/else, usamos um ENUM (EstadoAtual) e um
 *   SWITCH que chama a função certa. Fácil de adicionar novos estados depois.
 * ========================================================================== */

#include "raylib.h"
#include "tipos.h"

/* Módulos implementados pelo Dev 1 (Arthur) */
#include "jogador.h"
#include "profecia.h"
#include "colisao.h"

/* Módulos restantes. Magias/inimigos/cronograma já têm engine pronta — Luísa
 * preenche apenas as tabelas de tipos/eventos (inimigos_tipos.c, magias_tipos.c,
 * cronograma_eventos.c). Dev 2 (Sofia) ainda popula cartas/dados/salvamento/hud. */
#include "magias.h"
#include "inimigos.h"
#include "projeteis_inimigo.h"
#include "obstaculos.h"
#include "cronograma.h"
#include "cartas.h"
#include "dados.h"
#include "salvamento.h"
#include "hud.h"
#include "config_video.h"   /* aplica resolução e fullscreen do save */
#include "leaderboard.h"    /* top-10 de runs */
#include "historico.h"      /* últimas 10 seeds jogadas */

#include <stdlib.h>   /* srand, rand, strtoul */
#include <stdio.h>    /* snprintf */
#include <string.h>   /* memset, strlen */
#include <time.h>     /* time() pra seed inicial */
#include <math.h>     /* floorf, ceilf pra desenhar o grid infinito */


/* ============================================================================
 * PROTÓTIPOS (declarações) de funções LOCAIS deste arquivo.
 * Ficam como "static" porque só são usadas dentro de main.c.
 * ========================================================================== */
static void jogo_inicializar(EstadoJogo *ej);
static void jogo_atualizar(EstadoJogo *ej);
static void jogo_desenhar(const EstadoJogo *ej);
static void jogo_finalizar(EstadoJogo *ej);

static void desenhar_grid_mundo(const Camera2D *camera);

static void atualizar_menu(EstadoJogo *ej);
static void atualizar_opcoes(EstadoJogo *ej);
static void atualizar_leaderboard(EstadoJogo *ej);
static void atualizar_historico(EstadoJogo *ej);
static void atualizar_inserir_seed(EstadoJogo *ej);
static void atualizar_revelacao_profecia(EstadoJogo *ej);
static void atualizar_combate(EstadoJogo *ej);
static void atualizar_pausa(EstadoJogo *ej);
static void atualizar_cartas_upgrade(EstadoJogo *ej);
static void atualizar_game_over(EstadoJogo *ej);
static void atualizar_vitoria(EstadoJogo *ej);

static void desenhar_menu(const EstadoJogo *ej);
static void desenhar_opcoes(const EstadoJogo *ej);
static void desenhar_inserir_seed(const EstadoJogo *ej);

/* Inicia uma nova run com a seed dada. Centraliza o que rola ao confirmar
 * "Novo Jogo", "Carregar Jogo" e "Inserir Seed": gera profecia, gera mapa,
 * salva a seed em ultima_seed pra próximo "Carregar Jogo", persiste, e
 * transita pra revelação da profecia. */
static void iniciar_run_com_seed(EstadoJogo *ej, unsigned int seed);

/* Reset completo do estado da run. Chamada toda vez que uma nova run começa
 * (depois da revelação da profecia). Garante que não sobra nada da run
 * anterior — listas vazias, jogador na origem com vida cheia. Não toca em
 * salvamento nem em tempo_total (esses são meta-progressão / debug). */
static void jogo_resetar_run(EstadoJogo *ej);


/* ============================================================================
 * ESTADO DAS TELAS DE MENU
 * --------------------------------------------------------------------------
 * Cursores das telas (item selecionado, aba ativa, buffer do input de seed).
 * Vivem como "static" fora das funções pra persistir entre frames sem precisar
 * carregar variável de transição na struct EstadoJogo.
 * ========================================================================== */
static int  s_menu_item                 = 0;     /* cursor do menu principal */
static int  s_opcoes_item               = 0;     /* cursor da tela de opções */
static bool s_leaderboard_aba_biomassa  = false; /* false=tempo, true=biomassa */
static int  s_historico_cursor          = 0;     /* cursor da tela de histórico */
static char s_seed_buffer[SEED_MAX_DIGITOS + 1] = {0}; /* buffer de input de seed */
static int  s_seed_buffer_len           = 0;


/* ============================================================================
 * MAIN — PONTO DE ENTRADA
 * ========================================================================== */
int main(void) {
    /* 1. Inicialização do Raylib (abre a janela, prepara contexto gráfico) */
    InitWindow(LARGURA_TELA, ALTURA_TELA, "AUGUR - Projeto PIF CESAR");
    SetTargetFPS(FPS_ALVO);

    /* Por padrão o Raylib fecha a janela ao apertar ESC. Como queremos usar
     * ESC pra pausar/retomar o jogo, removemos esse atalho. O usuário ainda
     * pode fechar pelo X da janela (WindowShouldClose continua respondendo). */
    SetExitKey(KEY_NULL);

    /* Inicializa o gerador aleatório do sistema com o horário atual.
     * Sem isso, toda vez que o jogo abre, as seeds seriam iguais. */
    srand((unsigned int)time(NULL));

    /* 2. Prepara o estado do jogo.
     * A sintaxe " = {0}" zera TODOS os campos da struct (ponteiros = NULL,
     * números = 0, bools = false). É um idioma clássico em C. */
    EstadoJogo ej = {0};
    jogo_inicializar(&ej);

    /* 3. LOOP PRINCIPAL.
     * WindowShouldClose() = true quando o usuário clica no X ou aperta ESC.
     * ESTADO_SAIR permite que a gente encerre pelo código também. */
    while (!WindowShouldClose() && ej.estado_atual != ESTADO_SAIR) {
        jogo_atualizar(&ej);

        /* Bloco de desenho — tudo entre BeginDrawing e EndDrawing vai pra tela. */
        BeginDrawing();
            ClearBackground(BLACK);   /* limpa o frame anterior */
            jogo_desenhar(&ej);
        EndDrawing();
    }

    /* 4. Limpeza — libera memória alocada dinamicamente e fecha a janela. */
    jogo_finalizar(&ej);
    CloseWindow();
    return 0;
}


/* ============================================================================
 * INICIALIZAÇÃO
 * --------------------------------------------------------------------------
 * Chamada UMA VEZ no início. Prepara valores default das structs.
 * ========================================================================== */
static void jogo_inicializar(EstadoJogo *ej) {
    ej->estado_atual   = ESTADO_MENU;
    ej->proximo_estado = ESTADO_MENU;

    jogador_inicializar(&ej->jogador);
    salvamento_carregar(&ej->salvamento);
    config_video_normalizar(&ej->salvamento);   /* preenche defaults se save vazio */
    config_video_aplicar(&ej->salvamento);      /* aplica resolução e fullscreen agora */

    /* Listas começam vazias (cabeça = NULL) */
    ej->magias_cabeca   = NULL;
    ej->inimigos_cabeca = NULL;

    /* Câmera 2D: offset no centro da tela + target na posição do jogador.
     * Resultado: o jogador aparece sempre centralizado e o mundo "rola"
     * conforme ele anda. Zoom 1.0 = sem zoom. */
    ej->camera.offset   = (Vector2){ LARGURA_TELA / 2.0f, ALTURA_TELA / 2.0f };
    ej->camera.target   = ej->jogador.posicao;
    ej->camera.rotation = 0.0f;
    ej->camera.zoom     = 1.0f;

    ej->modo_debug   = false;
    ej->tiros_ativos = true;        /* Q desliga, Q liga */
}


/* ============================================================================
 * ATUALIZAÇÃO (LÓGICA DO FRAME)
 * --------------------------------------------------------------------------
 * Chamada UMA VEZ POR FRAME. Decide o que acontece baseado no estado atual.
 * ========================================================================== */
static void jogo_atualizar(EstadoJogo *ej) {
    /* GetFrameTime() retorna quantos segundos passaram desde o último frame.
     * Usamos pra movimento suave: "posicao += velocidade * delta_tempo"
     * garante velocidade constante mesmo se o FPS variar. */
    ej->delta_tempo = GetFrameTime();
    ej->tempo_total += ej->delta_tempo;
    ej->contador_frames++;

    /* F1 alterna modo debug (mostra FPS, etc.) */
    if (IsKeyPressed(KEY_F1)) {
        ej->modo_debug = !ej->modo_debug;
    }

    /* DISPATCHER DA MÁQUINA DE ESTADOS.
     * Cada estado tem sua própria função de atualização. */
    switch (ej->estado_atual) {
        case ESTADO_MENU:               atualizar_menu(ej);               break;
        case ESTADO_OPCOES:             atualizar_opcoes(ej);             break;
        case ESTADO_LEADERBOARD:        atualizar_leaderboard(ej);        break;
        case ESTADO_HISTORICO:          atualizar_historico(ej);          break;
        case ESTADO_INSERIR_SEED:       atualizar_inserir_seed(ej);       break;
        case ESTADO_REVELACAO_PROFECIA: atualizar_revelacao_profecia(ej); break;
        case ESTADO_COMBATE:            atualizar_combate(ej);            break;
        case ESTADO_PAUSA:              atualizar_pausa(ej);              break;
        case ESTADO_CARTAS_UPGRADE:     atualizar_cartas_upgrade(ej);     break;
        case ESTADO_GAME_OVER:          atualizar_game_over(ej);          break;
        case ESTADO_VITORIA:            atualizar_vitoria(ej);            break;
        default: break;
    }

    /* Transição limpa entre estados.
     * Se algum módulo mudou "proximo_estado", a troca efetiva acontece aqui,
     * ao final do frame. Isso evita bugs tipo "troquei de estado no meio do
     * update e a função X ainda rodou com o estado errado". */
    if (ej->proximo_estado != ej->estado_atual) {
        ej->estado_atual = ej->proximo_estado;
    }
}


/* --- Items do menu principal ---------------------------------------------
 * Mantemos a lista como dois arrays paralelos pra ficar simples adicionar/
 * remover itens. O item CARREGAR aparece SOMENTE se há um save com seed
 * registrada (salvamento.tem_ultima_seed). */
typedef enum {
    MENU_NOVO_JOGO,
    MENU_CARREGAR,
    MENU_INSERIR_SEED,
    MENU_HISTORICO,
    MENU_LEADERBOARD,
    MENU_OPCOES,
    MENU_SAIR,
    MENU_TOTAL_ITENS
} ItemMenu;

static const char *MENU_LABEL[MENU_TOTAL_ITENS] = {
    "Novo Jogo",
    "Carregar Jogo",
    "Inserir Seed",
    "Historico",
    "Leaderboard",
    "Opcoes",
    "Sair",
};

/* Constrói a lista de itens visíveis no menu atual. Filtra:
 *   - "Carregar Jogo" se não há save de seed (1 clique pra última seed)
 *   - "Histórico"     se o histórico está vazio
 * Retorna a quantidade de itens visíveis e preenche `mapa[]` com os índices
 * originais (ItemMenu) na ordem em que aparecem. */
static int montar_menu_visivel(const EstadoJogo *ej, ItemMenu mapa[MENU_TOTAL_ITENS]) {
    int n = 0;
    for (int i = 0; i < MENU_TOTAL_ITENS; i++) {
        if (i == MENU_CARREGAR  && !ej->salvamento.tem_ultima_seed) continue;
        if (i == MENU_HISTORICO && !historico_tem_entradas(&ej->salvamento)) continue;
        mapa[n++] = (ItemMenu)i;
    }
    return n;
}


/* --- Estado: MENU ---------------------------------------------------------
 * Tela inicial. UP/DOWN move o cursor entre os itens; ENTER seleciona. ESC
 * é no-op aqui (não tem onde voltar). */
static void atualizar_menu(EstadoJogo *ej) {
    ItemMenu mapa[MENU_TOTAL_ITENS];
    int n = montar_menu_visivel(ej, mapa);
    if (n == 0) return;   /* defensivo */

    /* Mantém s_menu_item dentro da janela visível (em runs antigas pode ter
     * ficado fora do range). */
    if (s_menu_item >= n) s_menu_item = n - 1;
    if (s_menu_item < 0)  s_menu_item = 0;

    if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) {
        s_menu_item = (s_menu_item - 1 + n) % n;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        s_menu_item = (s_menu_item + 1) % n;
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        switch (mapa[s_menu_item]) {
            case MENU_NOVO_JOGO: {
                /* Sorteia uma seed nova e começa a run. */
                unsigned int seed = (unsigned int)rand();
                iniciar_run_com_seed(ej, seed);
                break;
            }
            case MENU_CARREGAR:
                /* Replay da última seed jogada (só aparece se há save). */
                iniciar_run_com_seed(ej, ej->salvamento.ultima_seed);
                break;
            case MENU_INSERIR_SEED:
                /* Limpa o buffer e abre a tela de input. */
                s_seed_buffer_len = 0;
                s_seed_buffer[0]  = '\0';
                ej->proximo_estado = ESTADO_INSERIR_SEED;
                break;
            case MENU_HISTORICO:
                s_historico_cursor = 0;
                ej->proximo_estado = ESTADO_HISTORICO;
                break;
            case MENU_LEADERBOARD:
                ej->proximo_estado = ESTADO_LEADERBOARD;
                break;
            case MENU_OPCOES:
                s_opcoes_item = 0;
                ej->proximo_estado = ESTADO_OPCOES;
                break;
            case MENU_SAIR:
                ej->proximo_estado = ESTADO_SAIR;
                break;
            default: break;
        }
    }
}


/* --- Estado: OPCOES -------------------------------------------------------
 * Configuração de vídeo: navega entre resoluções e o toggle de fullscreen.
 * As mudanças são aplicadas e salvas ao confirmar com ENTER no item escolhido.
 * ESC volta sem aplicar. */
static void atualizar_opcoes(EstadoJogo *ej) {
    /* Itens visíveis: as N resoluções + 1 linha "Fullscreen" + 1 "Voltar". */
    int total_itens = RESOLUCOES_TOTAL + 2;

    if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) {
        s_opcoes_item = (s_opcoes_item - 1 + total_itens) % total_itens;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        s_opcoes_item = (s_opcoes_item + 1) % total_itens;
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (s_opcoes_item < RESOLUCOES_TOTAL) {
            /* Linha de resolução: aplica a escolhida. */
            ej->salvamento.largura_tela = (int)RESOLUCOES_DISPONIVEIS[s_opcoes_item].x;
            ej->salvamento.altura_tela  = (int)RESOLUCOES_DISPONIVEIS[s_opcoes_item].y;
            config_video_aplicar(&ej->salvamento);
            salvamento_salvar(&ej->salvamento);
        } else if (s_opcoes_item == RESOLUCOES_TOTAL) {
            /* Linha "Fullscreen": toggle. */
            ej->salvamento.fullscreen = !ej->salvamento.fullscreen;
            config_video_aplicar(&ej->salvamento);
            salvamento_salvar(&ej->salvamento);
        } else {
            /* "Voltar". */
            ej->proximo_estado = ESTADO_MENU;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        ej->proximo_estado = ESTADO_MENU;
    }
}


/* --- Estado: LEADERBOARD --------------------------------------------------
 * Duas tabelas: tempo (só vitórias) e biomassa (vit. e derrotas). TAB alterna.
 * ESC volta. */
static void atualizar_leaderboard(EstadoJogo *ej) {
    if (IsKeyPressed(KEY_TAB)) {
        s_leaderboard_aba_biomassa = !s_leaderboard_aba_biomassa;
    }
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
        ej->proximo_estado = ESTADO_MENU;
    }
}


/* --- Estado: HISTORICO ----------------------------------------------------
 * Lista cronológica das últimas seeds (mais recente em [0]). UP/DOWN move o
 * cursor; ENTER recarrega a seed selecionada (vai pra revelação da profecia);
 * ESC volta pro menu. */
static void atualizar_historico(EstadoJogo *ej) {
    int n = ej->salvamento.historico_qtd;
    if (n > HISTORICO_SEEDS_TAM) n = HISTORICO_SEEDS_TAM;

    if (n > 0) {
        /* Mantém cursor dentro do range mesmo após mudança de tamanho. */
        if (s_historico_cursor >= n) s_historico_cursor = n - 1;
        if (s_historico_cursor < 0)  s_historico_cursor = 0;

        if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) {
            s_historico_cursor = (s_historico_cursor - 1 + n) % n;
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            s_historico_cursor = (s_historico_cursor + 1) % n;
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            unsigned int seed = ej->salvamento.historico[s_historico_cursor].seed;
            iniciar_run_com_seed(ej, seed);
            return;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        ej->proximo_estado = ESTADO_MENU;
    }
}


/* --- Estado: INSERIR_SEED -------------------------------------------------
 * Input numérico simples: GetCharPressed() em loop pra capturar dígitos.
 * BACKSPACE apaga, ENTER confirma, ESC cancela. */
static void atualizar_inserir_seed(EstadoJogo *ej) {
    /* Captura todos os caracteres pressionados neste frame. GetCharPressed
     * devolve 0 quando a fila esvazia. */
    int c;
    while ((c = GetCharPressed()) != 0) {
        if (c >= '0' && c <= '9' &&
            s_seed_buffer_len < SEED_MAX_DIGITOS) {
            s_seed_buffer[s_seed_buffer_len++] = (char)c;
            s_seed_buffer[s_seed_buffer_len]   = '\0';
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE) && s_seed_buffer_len > 0) {
        s_seed_buffer_len--;
        s_seed_buffer[s_seed_buffer_len] = '\0';
    }

    if (IsKeyPressed(KEY_ENTER) && s_seed_buffer_len > 0) {
        /* strtoul devolve unsigned long; cabe em unsigned int (32-bit) já que
         * limitamos a 10 dígitos no buffer. Overflow vira UINT_MAX, aceitável. */
        unsigned long valor = strtoul(s_seed_buffer, NULL, 10);
        iniciar_run_com_seed(ej, (unsigned int)valor);
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        ej->proximo_estado = ESTADO_MENU;
    }
}


/* Encapsula o que rola quando o jogador confirma uma seed (Novo Jogo,
 * Carregar Jogo ou Inserir Seed). Persiste a seed pra próximo "Carregar". */
static void iniciar_run_com_seed(EstadoJogo *ej, unsigned int seed) {
    profecia_gerar(&ej->profecia, seed);
    obstaculos_gerar(ej, seed);

    ej->salvamento.ultima_seed     = seed;
    ej->salvamento.tem_ultima_seed = true;
    salvamento_salvar(&ej->salvamento);

    ej->proximo_estado = ESTADO_REVELACAO_PROFECIA;
}

/* --- Estado: REVELACAO_PROFECIA -------------------------------------------
 * Mostra os 3 modificadores sorteados. Jogador lê e aperta ESPAÇO pra
 * começar o combate. */
static void atualizar_revelacao_profecia(EstadoJogo *ej) {
    if (IsKeyPressed(KEY_SPACE)) {
        /* Reseta o estado da run ANTES de iniciar a timeline. Em retries
         * (game over → menu → nova run), sem isso o jogador continuaria com
         * vida 0 e a lista de inimigos da run anterior — disparando game
         * over no primeiro frame de combate. */
        jogo_resetar_run(ej);
        cronograma_inicializar(&ej->cronograma);
        profecia_evento_inicio_run(ej);   /* dispara COND_INICIO_RUN */
        ej->proximo_estado = ESTADO_COMBATE;
    }
}

/* --- Estado: COMBATE ------------------------------------------------------
 * O coração do jogo. Atualiza jogador, magias, inimigos, cronograma e
 * colisões. Transições possíveis:
 *   - vida <= 0          → GAME_OVER
 *   - cronograma.vitoria → VITORIA (matou o chefão)
 *   - cartas pendentes   → CARTAS_UPGRADE (a cada minuto cheio)
 * A ordem dos checks favorece a derrota (não dá pra "vencer" no mesmo
 * frame em que o jogador morre). */
static void atualizar_combate(EstadoJogo *ej) {
    /* ESC pausa o combate. O early return evita que o resto do frame rode
     * (jogador, magias, inimigos, cronograma) — o mundo congela na hora. */
    if (IsKeyPressed(KEY_ESCAPE)) {
        ej->proximo_estado = ESTADO_PAUSA;
        return;
    }

    /* Q liga/desliga o auto-fire. Tem efeito imediato no próximo tick de
     * magias_atualizar, que checa ej->tiros_ativos antes de disparar. */
    if (IsKeyPressed(KEY_Q)) {
        ej->tiros_ativos = !ej->tiros_ativos;
    }

    /* F3 pula pro próximo evento (próxima tela de cartas). Útil pra testar
     * builds rápido sem esperar 60s. Sem gate de debug — feature pro jogador. */
    if (IsKeyPressed(KEY_F3)) {
        cronograma_pular_proxima_carta(&ej->cronograma);
    }

    jogador_atualizar(&ej->jogador, ej->delta_tempo);
    profecia_motor_atualizar(ej);   /* condições de tempo/vida/combo */

    /* A câmera segue o jogador em tempo real. Como o offset é o centro da
     * tela, isso mantém o player sempre centralizado e o mundo rola em volta. */
    ej->camera.target = ej->jogador.posicao;

    magias_atualizar(ej);                              /* engine Arthur */
    inimigos_atualizar(ej);                            /* engine Arthur */
    projeteis_inimigo_atualizar(ej);                   /* engine Arthur */
    cronograma_atualizar(&ej->cronograma, ej);         /* engine Arthur */

    colisao_verificar_tudo(ej);                        /* engine Arthur */

    /* Obstáculos do mapa bloqueiam tanto o jogador quanto os inimigos.
     * Resolvidos APÓS colisao_verificar_tudo pra ter a palavra final — assim
     * inimigo não consegue empurrar o jogador pra dentro de uma árvore.
     * Stubs por enquanto (porte do sandbox em outra sessão). */
    obstaculos_resolver_jogador(ej);
    obstaculos_resolver_inimigos(ej);

    if (ej->jogador.vida <= 0) {
        /* Registra a derrota no leaderboard de biomassa antes de transitar.
         * top_tempo ignora (filtra por venceu=true internamente). Também
         * empurra a entrada pro histórico (índice 0 = mais recente). */
        leaderboard_registrar(&ej->salvamento,
                              ej->jogador.biomassa,
                              ej->cronograma.tempo_decorrido,
                              ej->profecia.seed,
                              false);
        historico_registrar(&ej->salvamento,
                            ej->profecia.seed,
                            false,
                            ej->cronograma.tempo_decorrido,
                            ej->jogador.biomassa);
        salvamento_salvar(&ej->salvamento);
        ej->proximo_estado = ESTADO_GAME_OVER;
    } else if (ej->cronograma.vitoria) {
        /* Registra a vitória nas duas tabelas + histórico. */
        leaderboard_registrar(&ej->salvamento,
                              ej->jogador.biomassa,
                              ej->cronograma.tempo_decorrido,
                              ej->profecia.seed,
                              true);
        historico_registrar(&ej->salvamento,
                            ej->profecia.seed,
                            true,
                            ej->cronograma.tempo_decorrido,
                            ej->jogador.biomassa);
        salvamento_salvar(&ej->salvamento);
        ej->proximo_estado = ESTADO_VITORIA;
    } else if (cronograma_deve_abrir_cartas(&ej->cronograma)) {
        cartas_gerar_escolhas(ej);
        ej->proximo_estado = ESTADO_CARTAS_UPGRADE;
    }
}

/* --- Estado: PAUSA --------------------------------------------------------
 * Mundo completamente congelado. ESC retoma o combate, ENTER volta pro menu
 * (descarta a run). Cronograma, inimigos e magias nem rodam aqui — o switch
 * em jogo_atualizar simplesmente não chama as funções deles. */
static void atualizar_pausa(EstadoJogo *ej) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        ej->proximo_estado = ESTADO_COMBATE;
    } else if (IsKeyPressed(KEY_ENTER)) {
        /* Limpa as listas antes de voltar ao menu pra não vazar memória entre
         * runs. A engine usa malloc/free, então isso é obrigatório. */
        magias_liberar_tudo(ej);
        inimigos_liberar_tudo(ej);
        projeteis_inimigo_liberar_tudo(ej);
        ej->proximo_estado = ESTADO_MENU;
    }
}

/* --- Estado: CARTAS_UPGRADE ------------------------------------------------
 * Tela de escolha disparada a cada minuto cheio do cronograma. O tempo da
 * timeline NÃO avança enquanto este estado está ativo. O jogador aperta 1/2/3
 * pra escolher a carta; a escolha é aplicada e o controle volta pro combate. */
static void atualizar_cartas_upgrade(EstadoJogo *ej) {
    if (IsKeyDown(KEY_R)) {
        if (IsKeyPressed(KEY_ONE))   cartas_usar_dado(ej, 0);
        if (IsKeyPressed(KEY_TWO))   cartas_usar_dado(ej, 1);
        if (IsKeyPressed(KEY_THREE)) cartas_usar_dado(ej, 2);
    }

    
    int escolha = -1;
    if (!IsKeyDown(KEY_R)) {
        if      (IsKeyPressed(KEY_ONE))   escolha = 0;
        else if (IsKeyPressed(KEY_TWO))   escolha = 1;
        else if (IsKeyPressed(KEY_THREE)) escolha = 2;
    }

    if (escolha >= 0) {
        cartas_aplicar(ej, escolha);
        cronograma_consumir_carta_pendente(&ej->cronograma);
        ej->proximo_estado = ESTADO_COMBATE;
    }
}

/* --- Estado: GAME_OVER ----------------------------------------------------
 * Mostra score, seed e espera input. */
static void atualizar_game_over(EstadoJogo *ej) {
    if (IsKeyPressed(KEY_ENTER)) {
        /* Libera as listas antes de voltar ao menu — simétrico com a saída
         * pela pausa. O reset definitivo acontece em jogo_resetar_run() na
         * próxima run, mas limpar aqui evita segurar memória entre telas. */
        magias_liberar_tudo(ej);
        inimigos_liberar_tudo(ej);
        projeteis_inimigo_liberar_tudo(ej);
        ej->proximo_estado = ESTADO_MENU;
    }
}

/* --- Estado: VITORIA ------------------------------------------------------
 * Tela curta após derrotar o chefão. ENTER volta pro menu. Tela detalhada
 * (com biomassa total, recordes etc.) é polish pra outra sessão. */
static void atualizar_vitoria(EstadoJogo *ej) {
    if (IsKeyPressed(KEY_ENTER)) {
        ej->proximo_estado = ESTADO_MENU;
    }
}


/* ============================================================================
 * DESENHO (RENDERIZAÇÃO DO FRAME)
 * --------------------------------------------------------------------------
 * Chamada UMA VEZ POR FRAME, depois de atualizar. Só desenha — NÃO modifica
 * estado (por isso o ponteiro é "const"). Cada estado desenha uma coisa
 * diferente.
 *
 * NOTA SOBRE ACENTOS NAS STRINGS:
 *   A fonte default do Raylib é ASCII puro. Strings dentro de DrawText("...")
 *   ficam SEM acento ("ESPACO", "Pressione ENTER") porque acentos virariam
 *   quadradinhos na tela. Os comentários, esses sim, podem ter acento.
 * ========================================================================== */
static void jogo_desenhar(const EstadoJogo *ej) {
    switch (ej->estado_atual) {
        case ESTADO_MENU:
            desenhar_menu(ej);
            break;

        case ESTADO_OPCOES:
            desenhar_opcoes(ej);
            break;

        case ESTADO_LEADERBOARD:
            leaderboard_desenhar(&ej->salvamento, s_leaderboard_aba_biomassa);
            break;

        case ESTADO_HISTORICO:
            historico_desenhar(&ej->salvamento, s_historico_cursor);
            break;

        case ESTADO_INSERIR_SEED:
            desenhar_inserir_seed(ej);
            break;

        case ESTADO_REVELACAO_PROFECIA:
            profecia_desenhar(&ej->profecia);
            DrawText("Pressione ESPACO para comecar a onda",
                     LARGURA_TELA/2 - 260, ALTURA_TELA - 80, 22, GRAY);
            break;

        case ESTADO_COMBATE:
        case ESTADO_PAUSA:
            /* Tudo dentro de BeginMode2D é desenhado em COORD DE MUNDO:
             * a câmera aplica o offset automaticamente. Jogador, magias,
             * inimigos e obstáculos vivem no mundo.
             *
             * O case PAUSA reaproveita o render do COMBATE: o mundo continua
             * desenhado (estado da última frame antes da pausa), e em cima
             * pintamos o overlay no fim deste switch. */
            BeginMode2D(ej->camera);
                desenhar_grid_mundo(&ej->camera);
                obstaculos_desenhar(ej);
                magias_desenhar(ej);
                inimigos_desenhar(ej);
                projeteis_inimigo_desenhar(ej);
                jogador_desenhar(&ej->jogador);
            EndMode2D();
            /* HUD é coord de TELA (fixa, não rola com a câmera). */
            desenhar_hud(ej);

            /* Indicador do toggle de tiros: canto inferior esquerdo. ASCII
             * porque a fonte default do Raylib não renderiza acento. */
            DrawText(ej->tiros_ativos ? "[Q] Tiros: ON" : "[Q] Tiros: OFF",
                     10, ALTURA_TELA - 28, 18,
                     ej->tiros_ativos ? LIME : (Color){ 200, 100, 100, 255 });

            /* Overlay de pausa: tinta translúcida + texto centralizado. */
            if (ej->estado_atual == ESTADO_PAUSA) {
                DrawRectangle(0, 0, LARGURA_TELA, ALTURA_TELA,
                              (Color){ 0, 0, 0, 160 });
                DrawText("PAUSA",
                         LARGURA_TELA/2 - 90, ALTURA_TELA/2 - 80, 60, GOLD);
                DrawText("ESC para retomar",
                         LARGURA_TELA/2 - 110, ALTURA_TELA/2 + 10, 22, WHITE);
                DrawText("ENTER para voltar ao menu",
                         LARGURA_TELA/2 - 170, ALTURA_TELA/2 + 50, 20, GRAY);
            }
            break;

        case ESTADO_CARTAS_UPGRADE:
            DrawText("ESCOLHA UM UPGRADE",
                     LARGURA_TELA/2 - 180, 80, 32, GOLD);
            cartas_desenhar_ui(ej); /* stub */
            DrawText("(ESPACO pra continuar)",
                     LARGURA_TELA/2 - 150, ALTURA_TELA - 50, 18, GRAY);
            break;

        case ESTADO_GAME_OVER: {
            DrawText("GAME OVER", LARGURA_TELA/2 - 140, 200, 60, RED);
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "Pontuacao final: %d",
                     ej->jogador.biomassa);
            DrawText(buffer, LARGURA_TELA/2 - 130, 300, 28, GOLD);
            snprintf(buffer, sizeof(buffer), "Seed da run: %u",
                     ej->profecia.seed);
            DrawText(buffer, LARGURA_TELA/2 - 100, 350, 22, WHITE);
            DrawText("Pressione ENTER para voltar ao menu",
                     LARGURA_TELA/2 - 230, 440, 20, GRAY);
            break;
        }

        case ESTADO_VITORIA: {
            DrawText("VITORIA", LARGURA_TELA/2 - 130, 180, 70, GOLD);
            DrawText("Voce derrotou o chefao final!",
                     LARGURA_TELA/2 - 200, 290, 22, WHITE);
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "Pontuacao final: %d",
                     ej->jogador.biomassa);
            DrawText(buffer, LARGURA_TELA/2 - 130, 340, 28, GOLD);
            int min = (int)(ej->cronograma.tempo_decorrido / 60.0f);
            int seg = (int)ej->cronograma.tempo_decorrido % 60;
            snprintf(buffer, sizeof(buffer),
                     "Tempo: %02d:%02d   Seed: %u",
                     min, seg, ej->profecia.seed);
            DrawText(buffer, LARGURA_TELA/2 - 200, 390, 20, LIGHTGRAY);
            DrawText("Pressione ENTER para voltar ao menu",
                     LARGURA_TELA/2 - 230, 450, 20, GRAY);
            break;
        }
        default: break;
    }

    /* Overlay de debug — por cima de tudo. */
    if (ej->modo_debug) {
        DrawFPS(10, 10);
        DrawText("DEBUG [F1]", 10, 30, 14, LIME);
    }
}


/* ============================================================================
 * LIMPEZA FINAL
 * --------------------------------------------------------------------------
 * Chamada UMA VEZ no fim. Libera memória e salva progresso.
 * ========================================================================== */
static void jogo_finalizar(EstadoJogo *ej) {
    magias_liberar_tudo(ej);      /* stub — libera lista encadeada */
    inimigos_liberar_tudo(ej);    /* stub — libera lista encadeada */
    projeteis_inimigo_liberar_tudo(ej);  /* libera lista encadeada */
    salvamento_salvar(&ej->salvamento);  /* stub — grava arquivo */
}


/* ============================================================================
 * RESET DE RUN
 * --------------------------------------------------------------------------
 * Chamada em todo INÍCIO de run (depois da revelação da profecia). Devolve o
 * estado a um ponto consistente: listas vazias, jogador na origem com vida
 * cheia, câmera centrada no jogador. Não mexe em salvamento (meta), em
 * tempo_total (debug) nem na profecia (já gerada antes).
 * ========================================================================== */
static void jogo_resetar_run(EstadoJogo *ej) {
    magias_liberar_tudo(ej);
    inimigos_liberar_tudo(ej);
    projeteis_inimigo_liberar_tudo(ej);
    jogador_inicializar(&ej->jogador);
    ej->motor_profecia = (MotorProfecia){0};   /* zera timers/combo/buffs da run */
    ej->camera.target = ej->jogador.posicao;
    ej->tiros_ativos  = true;

    for (int k = 0; k < MAX_DADOS_JOGADOR; k++) {
        ej->dados_ativos[k].faces            = 6;
        ej->dados_ativos[k].ultimo_resultado = 0;
    }
}


/* ============================================================================
 * GRID DE REFERÊNCIA DO MUNDO
 * --------------------------------------------------------------------------
 * Desenha linhas finas em intervalos fixos, mas SÓ NA ÁREA VISÍVEL da câmera.
 * Isso dá sensação de "mundo infinito" (as linhas aparecem rolando conforme o
 * jogador anda) sem precisar renderizar milhões de linhas — só as que cabem
 * na tela a cada frame.
 *
 * Como deve ser chamada dentro de BeginMode2D, usamos coord de mundo: a área
 * visível em mundo vai de (target - tela/2/zoom) até (target + tela/2/zoom).
 * ========================================================================== */
static void desenhar_grid_mundo(const Camera2D *camera) {
    const float ESPACAMENTO = 128.0f;     /* distância entre linhas, em pixels de mundo */
    const Color COR_GRID    = (Color){ 30, 30, 45, 255 };  /* azul bem escuro, discreto */

    /* Limites visíveis em coord de mundo. zoom divide porque zoom 2 mostra
     * metade do mundo, zoom 0.5 mostra o dobro. */
    float meia_largura = (LARGURA_TELA / 2.0f) / camera->zoom;
    float meia_altura  = (ALTURA_TELA  / 2.0f) / camera->zoom;

    float esquerda = camera->target.x - meia_largura;
    float direita  = camera->target.x + meia_largura;
    float topo     = camera->target.y - meia_altura;
    float baixo    = camera->target.y + meia_altura;

    /* Alinha o início pra cair num múltiplo de ESPACAMENTO (efeito de grid
     * "fixo no mundo", não colado na câmera). */
    float primeiro_x = floorf(esquerda / ESPACAMENTO) * ESPACAMENTO;
    float primeiro_y = floorf(topo     / ESPACAMENTO) * ESPACAMENTO;

    /* Verticais */
    for (float x = primeiro_x; x <= direita; x += ESPACAMENTO) {
        DrawLineV((Vector2){ x, topo }, (Vector2){ x, baixo }, COR_GRID);
    }
    /* Horizontais */
    for (float y = primeiro_y; y <= baixo; y += ESPACAMENTO) {
        DrawLineV((Vector2){ esquerda, y }, (Vector2){ direita, y }, COR_GRID);
    }
}


/* ============================================================================
 * TELAS DE MENU — DESENHO
 * --------------------------------------------------------------------------
 * Todas em coord de tela (FORA de BeginMode2D). Layout simples: título no topo,
 * lista de itens centralizada, cursor "> " antes do item selecionado.
 * ========================================================================== */

static void desenhar_menu(const EstadoJogo *ej) {
    /* Título "AUGUR" no topo. */
    DrawText("AUGUR", LARGURA_TELA/2 - 140, 80, 100, GOLD);
    DrawText("Bullet Hell Roguelite",
             LARGURA_TELA/2 - 140, 190, 24, GRAY);

    /* Lista de itens. Reconstrói a janela visível (sem "Carregar Jogo" se não
     * há save). Cursor pinta de GOLD; outros itens em LIGHTGRAY. */
    ItemMenu mapa[MENU_TOTAL_ITENS];
    int n = montar_menu_visivel(ej, mapa);

    int x = LARGURA_TELA / 2 - 120;
    int y = 280;
    const int linha_altura = 38;

    for (int i = 0; i < n; i++) {
        const char *label = MENU_LABEL[mapa[i]];
        Color cor   = (i == s_menu_item) ? GOLD : LIGHTGRAY;
        const char *prefixo = (i == s_menu_item) ? "> " : "  ";
        char buf[64];
        snprintf(buf, sizeof(buf), "%s%s", prefixo, label);
        DrawText(buf, x, y + i * linha_altura, 28, cor);
    }

    DrawText("[UP/DOWN] navegar   [ENTER] selecionar",
             LARGURA_TELA/2 - 230, ALTURA_TELA - 70, 18, DARKGRAY);
    DrawText("PIF 2026.1 - CESAR School",
             LARGURA_TELA/2 - 130, ALTURA_TELA - 40, 16, DARKGRAY);
}


static void desenhar_opcoes(const EstadoJogo *ej) {
    DrawText("OPCOES", LARGURA_TELA/2 - 110, 60, 50, GOLD);
    DrawText("Configuracao de video",
             LARGURA_TELA/2 - 130, 130, 20, GRAY);

    int x = LARGURA_TELA / 2 - 180;
    int y = 200;
    const int linha_altura = 38;

    /* Resoluções: marca a aplicada no momento com "*". */
    int res_atual = config_video_indice_resolucao_atual(&ej->salvamento);
    for (int i = 0; i < RESOLUCOES_TOTAL; i++) {
        char buf[80];
        const char *marca = (i == res_atual) ? "*" : " ";
        snprintf(buf, sizeof(buf), "%s %dx%d",
                 marca,
                 (int)RESOLUCOES_DISPONIVEIS[i].x,
                 (int)RESOLUCOES_DISPONIVEIS[i].y);
        Color cor   = (i == s_opcoes_item) ? GOLD : LIGHTGRAY;
        const char *prefixo = (i == s_opcoes_item) ? "> " : "  ";
        char linha[96];
        snprintf(linha, sizeof(linha), "%s%s", prefixo, buf);
        DrawText(linha, x, y + i * linha_altura, 24, cor);
    }

    /* Fullscreen toggle. */
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "  Fullscreen: %s",
                 ej->salvamento.fullscreen ? "ON" : "OFF");
        int idx = RESOLUCOES_TOTAL;
        Color cor   = (idx == s_opcoes_item) ? GOLD : LIGHTGRAY;
        const char *prefixo = (idx == s_opcoes_item) ? "> " : "  ";
        char linha[80];
        snprintf(linha, sizeof(linha), "%s%s", prefixo, buf + 2);
        DrawText(linha, x, y + idx * linha_altura, 24, cor);
    }

    /* Voltar. */
    {
        int idx = RESOLUCOES_TOTAL + 1;
        Color cor   = (idx == s_opcoes_item) ? GOLD : LIGHTGRAY;
        const char *prefixo = (idx == s_opcoes_item) ? "> " : "  ";
        char linha[32];
        snprintf(linha, sizeof(linha), "%sVoltar", prefixo);
        DrawText(linha, x, y + idx * linha_altura + 20, 24, cor);
    }

    DrawText("[UP/DOWN] navegar   [ENTER] aplicar   [ESC] voltar",
             LARGURA_TELA/2 - 290, ALTURA_TELA - 50, 18, DARKGRAY);
}


static void desenhar_inserir_seed(const EstadoJogo *ej) {
    (void)ej;   /* não usa estado do jogo */

    DrawText("INSERIR SEED", LARGURA_TELA/2 - 180, 100, 50, GOLD);
    DrawText("Digite uma seed (0 a 4.294.967.295) e ENTER",
             LARGURA_TELA/2 - 300, 180, 22, GRAY);

    /* Caixa do input. */
    int caixa_w = 420;
    int caixa_h = 60;
    int caixa_x = LARGURA_TELA/2 - caixa_w/2;
    int caixa_y = 240;
    DrawRectangleLines(caixa_x, caixa_y, caixa_w, caixa_h, GRAY);

    /* Texto digitado. Cursor pulsa (visível em metade dos frames). */
    const char *texto = (s_seed_buffer_len > 0) ? s_seed_buffer : "";
    int texto_w = MeasureText(texto, 30);
    int texto_x = caixa_x + 12;
    int texto_y = caixa_y + 15;
    DrawText(texto, texto_x, texto_y, 30, WHITE);

    /* Cursor "|" pulsante. */
    if (((int)(GetTime() * 2.0)) % 2 == 0) {
        DrawText("|", texto_x + texto_w + 2, texto_y, 30, WHITE);
    }

    if (s_seed_buffer_len == 0) {
        DrawText("(vazio)", texto_x, texto_y + 4, 22, DARKGRAY);
    }

    DrawText("[0-9] digitar   [BACKSPACE] apagar   [ENTER] confirmar   [ESC] voltar",
             LARGURA_TELA/2 - 380, ALTURA_TELA - 50, 18, DARKGRAY);
}
