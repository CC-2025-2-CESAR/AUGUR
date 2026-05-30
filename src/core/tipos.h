/* ============================================================================
 * tipos.h - CONTRATO ENTRE OS DEVS
 * ----------------------------------------------------------------------------
 * Reúne TODAS as structs, enums e constantes compartilhadas. Cada módulo só
 * inclui "tipos.h" pra enxergar tudo, sem dependência circular. Números de
 * balanceamento NÃO vivem aqui — ficam nos arquivos de conteúdo da Luísa
 * (magias_comportamento.c, profecia_efeitos.c, etc.).
 * ========================================================================== */

#ifndef TIPOS_H
#define TIPOS_H

#include "raylib.h"   /* Vector2, Rectangle, Color, Texture2D */
#include <stdbool.h>
#include <stddef.h>   /* NULL */

/* ============================================================================
 * CONSTANTES GLOBAIS — números mágicos usados por vários arquivos.
 * ========================================================================== */
#define LARGURA_TELA      1280   /* resolução-base do render; pode ser sobrescrita pelo save */
#define ALTURA_TELA       720
#define FPS_ALVO          60

/* Versão do formato do save. Se o disco trouxer outro valor, salvamento_carregar
 * zera tudo — evita lixo binário ao mudar o layout de DadosSalvos. */
#define SAVE_VERSAO_ATUAL 3

#define LEADERBOARD_TAM     10  /* top-10 nas duas tabelas (tempo + biomassa) */
#define HISTORICO_SEEDS_TAM 10  /* últimas 10 seeds jogadas */
#define SEED_MAX_DIGITOS    10  /* unsigned int 32-bit cabe em 10 dígitos decimais */

#define MAX_PROJETEIS         256  /* teto da lista de magias do jogador */
#define MAX_PROJETEIS_INIMIGO 256  /* teto da lista de projéteis de inimigo */
#define MAX_INIMIGOS          128  /* teto da lista de inimigos */
#define CARTAS_POR_ESCOLHA      3  /* quantas cartas aparecem por tela de upgrade */
#define MAX_DADOS_JOGADOR        2  /* quantos dados o jogador leva por run */

/* Animações e direções das sprite sheets (consumidos pelo módulo assets).
 * Layout fixo das sheets: rows 0..3 = idle_{down,up,left,right}, 4..7 = walk_*,
 * 8..11 = cast_*, 12 = hurt (omni), 13 = death (omni). Direção é o offset
 * somado ao row-base em desenhar_sheet. */
#define ANIM_IDLE   0
#define ANIM_WALK   1
#define ANIM_CAST   2
#define ANIM_HURT   3
#define ANIM_DEATH  4

#define DIR_DOWN    0
#define DIR_UP      1
#define DIR_LEFT    2
#define DIR_RIGHT   3

/* ============================================================================
 * ENUMS — int com nome legível (em vez de "estado == 2", "estado == COMBATE").
 * O sufixo _TOTAL no fim de um enum vira a contagem automaticamente.
 * ========================================================================== */

/* Estados da máquina de estados. main.c tem um switch que despacha por aqui. */
typedef enum {
    ESTADO_MENU,                    /* tela inicial */
    ESTADO_OPCOES,                  /* config de vídeo: resolução, fullscreen */
    ESTADO_LEADERBOARD,             /* tabelas top-10 (tempo e biomassa) */
    ESTADO_HISTORICO,               /* lista das últimas seeds jogadas */
    ESTADO_INSERIR_SEED,            /* input de seed manual */
    ESTADO_REVELACAO_PROFECIA,      /* mostra os 3 modificadores sorteados */
    ESTADO_ESCOLHA_MAGIA_INICIAL,   /* depois da profecia: escolhe 1 de 3 magias */
    ESTADO_COMBATE,                 /* timeline rolando, inimigos spawnando */
    ESTADO_PAUSA,                   /* ESC no combate; mundo congelado */
    ESTADO_CARTAS_UPGRADE,          /* a cada minuto: jogador escolhe upgrade */
    ESTADO_GAME_OVER,               /* morreu; mostra score e seed */
    ESTADO_VITORIA,                 /* derrotou o chefão final aos 5:00 */
    ESTADO_SAIR                     /* sinaliza main pra fechar a janela */
} EstadoAtual;

/* Comportamento de IA. Cada valor mapeia 1:1 a uma função em inimigos_tipos.c. */
typedef enum {
    IA_CHASE,                   /* anda direto no jogador */
    IA_KITER,                   /* mantém distância e atira */
    IA_BOSS_FASES               /* chefão com fases por % de vida */
} ComportamentoIA;

/* 6 elementos. A ordem indexa as tabelas de nomes/stats. */
typedef enum {
    ELEMENTO_FOGO,
    ELEMENTO_GELO,
    ELEMENTO_RELAMPAGO,
    ELEMENTO_VENENO,
    ELEMENTO_ARCANO,
    ELEMENTO_SOMBRA,
    ELEMENTO_TOTAL
} Elemento;

/* Tipos de inimigo — decidem IA, stats e sprite. */
typedef enum {
    INIMIGO_CORPO_A_CORPO,
    INIMIGO_A_DISTANCIA,
    INIMIGO_ELITE,
    INIMIGO_CHEFE
} TipoInimigo;

/* Condições da profecia: "quando X acontece, dispara o efeito". Pool enxuto
 * (4) pra cada profecia ser um puzzle legível. */
typedef enum {
    COND_AO_MATAR,          /* inimigo morreu */
    COND_AO_RECEBER_DANO,   /* jogador tomou hit */
    COND_AO_ACERTAR,        /* magia acertou um inimigo */
    COND_A_CADA_N_SEG,      /* timer interno por mod: dispara a cada N segundos */
    COND_TOTAL
} Condicao;

/* Efeitos disparados pelas profecias. Pool enxuto (6); cada um tem magnitude
 * explícita no texto da profecia (vive em profecia_efeitos.c). */
typedef enum {
    EF_EXPLOSAO,            /* dano em área no contexto */
    EF_CURA,                /* recupera HP do jogador */
    EF_ESCUDO,              /* anula o próximo hit no jogador */
    EF_IGNITE,              /* aplica DoT no alvo */
    EF_CONGELAR,            /* congela inimigos no contexto */
    EF_DUPLICA_PROJETIL,    /* próximos N disparos saem duplicados */
    EF_TOTAL
} Efeito;

/* Cartas de upgrade (Dev 2 popula e aplica). */
typedef enum {
    CARTA_DANO_UP,
    CARTA_VIDA_UP,
    CARTA_VELOCIDADE_UP,
    CARTA_RECARGA_DADO,     /* devolve um dado pro jogador */
    CARTA_TOTAL
} TipoCarta;


/* ============================================================================
 * STRUCTS DO JOGO — passadas por ponteiro (Jogador *j) pra evitar cópia e
 * permitir que a função modifique o original.
 * ========================================================================== */

/* JOGADOR. `bonus_dano` é somado ao dano-base de toda magia no nascimento do
 * projétil (lido em magias.c) — nunca itere projéteis em voo pra aplicar bônus. */
typedef struct {
    Vector2 posicao;
    Vector2 velocidade;
    float   raio;                   /* hitbox circular */
    int     vida;
    int     vida_maxima;
    float   velocidade_movimento;   /* pixels por segundo */
    int     biomassa;               /* pontuação da run */
    int     bonus_dano;             /* somado no dano de toda magia disparada */

    /* Visual (módulo assets): direção pra onde olha + animação atual. */
    int     direcao_atual;          /* DIR_DOWN/UP/LEFT/RIGHT */
    int     animacao_atual;         /* ANIM_IDLE/WALK/CAST/HURT/DEATH */
    float   animacao_tempo;         /* s acumulados na animação */
    float   hurt_tempo_restante;    /* >0: força HURT até zerar */
    float   cast_tempo_restante;    /* >0: força CAST até zerar */
} Jogador;


/* MAGIAS — lista encadeada (nascem/morrem o tempo todo, quantidade variável).
 * Cada nó é malloc/free; cumpre o requisito de listas encadeadas do PIF. */
typedef struct {
    Vector2  posicao;
    Vector2  velocidade;
    float    dano;
    float    tempo_de_vida;     /* s; o projétil some quando chega a 0 */
    float    raio;              /* colisão (vem do elemento) */
    Elemento elemento;
    bool     viva;             /* false = removida no próximo frame */
    int      saltos_restantes; /* hops de chain do Relâmpago; 0 = sem chain */
    bool     ja_acertou;       /* guarda contra reprocessar o mesmo projétil */
} Magia;

typedef struct MagiaNo {
    Magia           dados;
    struct MagiaNo *proxima;   /* NULL = fim da lista */
} MagiaNo;


/* INIMIGOS — lista encadeada, mesma lógica das magias. */
typedef struct {
    Vector2     posicao;
    Vector2     velocidade;
    float       raio;
    int         vida;
    int         vida_maxima;
    float       dano;                /* dano ao encostar no jogador */
    float       velocidade_movimento;
    TipoInimigo tipo;
    int         recompensa_biomassa; /* biomassa que dropa ao morrer */
    bool        vivo;

    /* Status aplicado por magias/combos/profecia. A engine aplica (colisao.c) e
     * expira (inimigos.c) estes campos; as magnitudes vivem no conteúdo da Luísa. */
    float congelado_tempo;           /* s; >0 zera a velocidade no update */
    float veneno_tempo;              /* s restantes do DoT */
    float veneno_dps;                /* dano/s do DoT (escala com stacks) */
    int   veneno_stacks;
    float veneno_acumulado;          /* acumulador fracionário (não trunca o int vida) */
    float marca_termica_tempo;       /* janela do combo Choque Térmico (marca de Fogo) */
    float proxima_hit_multiplicador; /* mult. da PRÓXIMA hit recebida (1.0 = normal) */
    float timer_disparo;             /* cooldown do tiro deste inimigo */

    /* Visual (módulo assets). */
    int   direcao_atual;
    int   animacao_atual;
    float animacao_tempo;
    float morrendo_tempo;            /* >0: anim DEATH rolando antes do free */
} Inimigo;

typedef struct InimigoNo {
    Inimigo            dados;
    struct InimigoNo  *proximo;
} InimigoNo;


/* PROJÉTEIS DE INIMIGO — lista encadeada. Tiro padrão que ranged e chefão
 * disparam no jogador; stats vivem em projeteis_inimigo_tipos.c. */
typedef struct {
    Vector2 posicao;
    Vector2 velocidade;
    float   dano;
    float   tempo_de_vida;
    float   raio;
    Color   cor;
    bool    vivo;
    int     tipo_origem;     /* TipoInimigo que disparou — escolhe o sprite */
} ProjetilInimigo;

typedef struct ProjetilInimigoNo {
    ProjetilInimigo            dados;
    struct ProjetilInimigoNo  *proximo;
} ProjetilInimigoNo;


/* TABELAS DE PARÂMETROS (Dev 3) — arrays const indexados por TipoInimigo e
 * Elemento. A engine lê desses arrays na hora de spawnar/disparar; mudou um
 * valor, o próximo spawn já pega. Ideal pra balanceamento. */
typedef struct {
    int             vida_base;
    float           dano;
    float           velocidade_movimento;
    float           raio;                /* colisão */
    float           raio_visual;         /* desenho (pode diferir da colisão) */
    Color           cor;
    int             recompensa_biomassa;
    ComportamentoIA comportamento;
} ParametrosInimigo;

typedef struct {
    float dano_base;
    float velocidade_projetil;
    float tempo_de_vida;
    float raio_projetil;
    float intervalo_disparo;             /* s entre disparos automáticos */
    Color cor;
} ParametrosMagia;


/* TIMELINE (cronograma + eventos). A run é uma timeline contínua de 5 min
 * (estilo Vampire Survivors), não ondas finitas. A tabela EVENTOS_CRONOGRAMA[]
 * (cronograma_eventos.c) descreve "do tempo X ao Y, spawna tipo T a cada Z s".
 * Aos 5:00 a engine spawna o chefão e para o resto. A Luísa edita só a tabela. */
typedef struct {
    float        tempo_inicio_seg;   /* quando começa a spawnar */
    float        tempo_fim_seg;      /* quando para; INFINITY = nunca */
    TipoInimigo  tipo;
    float        intervalo_spawn;    /* s entre cada spawn */
    float        timer_interno;      /* acumulador (gerenciado pela engine) */
    bool         ativo;
} EventoCronograma;

#define MAX_EVENTOS_CRONOGRAMA 32

typedef struct {
    float            tempo_decorrido;        /* s desde o início da run */
    float            tempo_proxima_carta;    /* dispara cartas neste valor */
    bool             cartas_pendentes;       /* engine setou, main consome */
    bool             chefao_spawnado;
    bool             esperando_chefao_morrer;
    bool             vitoria;
    EventoCronograma eventos[MAX_EVENTOS_CRONOGRAMA];
    int              qtd_eventos;
} Cronograma;


/* PROFECIA — o coração do jogo. 3 modificadores [Elemento]+[Condição]+[Efeito].
 * Gerada deterministicamente da seed: 6 elementos × 4 condições × 6 efeitos =
 * 144 combinações por mod (~3 milhões na profecia inteira). Mesma seed = mesma
 * profecia, então dá pra reproduzir e compartilhar runs. */
typedef struct {
    Elemento elemento;
    Condicao condicao;
    Efeito   efeito;
} Modificador;

typedef struct {
    Modificador mods[3];
    unsigned int seed;
} Profecia;


/* MOTOR DE PROFECIA — estado vivo que profecia.c usa pra avaliar Condições e
 * aplicar Efeitos durante o combate. Zerado a cada run (jogo_resetar_run). */
typedef struct {
    float timer_cond[3];           /* COND_A_CADA_N_SEG: acumulador por mod */
    float cooldown_global_disparo; /* cooldown único entre disparos (round-robin) */
    int   prox_slot_disparo;       /* índice 0-2 do próximo mod a disparar */
    int   duplica_proximos;        /* disparos a duplicar (EF_DUPLICA_PROJETIL) */
    float escudo_ativo;            /* >0 => próximo hit no jogador é anulado (EF_ESCUDO) */
    bool  em_aplicar_efeito;       /* guard de reentrância no dispatch */
} MotorProfecia;


/* CARTAS E DADOS (Dev 2). Cartas aparecem entre minutos; dados rolam pra
 * modificar o valor de uma carta. */
typedef struct {
    TipoCarta tipo;
    int       raridade;          /* 0=comum .. 5=lendária */
    int       valor;             /* ex.: +10 de dano */
    char      nome[64];
    char      descricao[256];
} Carta;

typedef struct {
    int faces;                   /* d6 = 6 */
    int ultimo_resultado;        /* mostrado após rolar; 0 = ainda carregado */
} Dado;


/* Uma linha do top-10. Slot livre = ocupado=false. */
typedef struct {
    int          pontuacao;         /* biomassa coletada na run */
    float        tempo_segundos;    /* duração; só vale se venceu=true */
    unsigned int seed;
    bool         venceu;
    bool         ocupado;
} EntradaLeaderboard;


/* Uma linha do histórico de seeds (índice 0 = mais recente). Guarda toda run
 * que chegou ao fim, pra recarregar uma seed antiga sem precisar anotar. */
typedef struct {
    unsigned int seed;
    bool         venceu;
    float        tempo_segundos;
    int          pontuacao;
    bool         ocupado;
} EntradaHistorico;


/* DADOS SALVOS (Dev 2) — persistem entre runs. Gravados inteiros via fwrite em
 * saves/biomassa.dat (requisito de arquivo do PIF). Campo versao_save é o gate
 * de compatibilidade: layout diferente no disco => zera tudo no carregamento. */
typedef struct {
    int  versao_save;

    /* Progressão (campos originais da Sofia). */
    int  biomassa_total;
    int  runs_completadas;
    int  melhor_onda;
    int  profecias_desbloqueadas[20]; /* MATRIZ — requisito obrigatório */
    char nome_jogador[32];

    /* Config de vídeo. */
    int  largura_tela;              /* 0 = usar o default */
    int  altura_tela;
    bool fullscreen;

    /* "Carregar Jogo" (replay da última seed). */
    unsigned int ultima_seed;
    bool         tem_ultima_seed;

    /* Leaderboards. */
    EntradaLeaderboard top_tempo[LEADERBOARD_TAM];     /* só vitórias; tempo crescente */
    EntradaLeaderboard top_biomassa[LEADERBOARD_TAM];  /* todas; pontuação decrescente */

    /* Histórico de seeds jogadas (mais recente em [0]). */
    EntradaHistorico historico[HISTORICO_SEEDS_TAM];
    int              historico_qtd;
} DadosSalvos;


/* Uma das 3 opções da tela de escolha de magia inicial. A escolhida vira o 4º
 * elemento do auto-fire. Pelo menos 1 das 3 é sinergética (bate com um mod). */
typedef struct {
    Elemento  elemento;
    int       raridade;             /* 0..5, mesma curva das cartas */
    bool      sinergetica;          /* elemento bate com algum mod da profecia */
    char      nome[24];             /* "Bola de Fogo", "Lanca Gelada"... */
} OpcaoMagiaInicial;


/* ============================================================================
 * ESTADO DO JOGO (STRUCT RAIZ) — carrega todo o estado atual. Passada por
 * ponteiro (EstadoJogo *ej) pra quase toda função: zero variável global, e
 * qualquer função enxerga o contexto inteiro.
 * ========================================================================== */
typedef struct {
    /* Máquina de estados. proximo_estado é o buffer de transição (a troca
     * efetiva acontece no fim do frame, evitando rodar meio-update no estado
     * errado). */
    EstadoAtual estado_atual;
    EstadoAtual proximo_estado;

    /* Entidades e sistemas principais. */
    Jogador       jogador;
    Profecia      profecia;
    MotorProfecia motor_profecia;
    Cronograma    cronograma;
    DadosSalvos   salvamento;

    /* Câmera 2D: segue o jogador (target = jogador.posicao) com offset no centro
     * da tela. O que é desenhado dentro de BeginMode2D/EndMode2D usa coord de
     * mundo; fora disso é coord de tela (HUD, menus). */
    Camera2D  camera;

    /* Cabeças das listas encadeadas (NULL = vazia). */
    MagiaNo           *magias_cabeca;
    InimigoNo         *inimigos_cabeca;
    ProjetilInimigoNo *projeteis_inimigo_cabeca;

    /* MATRIZ de 3 cartas mostradas no estado CARTAS_UPGRADE. */
    Carta     escolhas_upgrade[CARTAS_POR_ESCOLHA];

    /* Tela ESCOLHA_MAGIA_INICIAL. */
    OpcaoMagiaInicial opcoes_magia[3];
    int               opcao_magia_selecionada;   /* 0..2 */
    Elemento          magia_inicial_escolhida;   /* aplicada no auto-fire (4º slot) */
    bool              magia_inicial_definida;

    /* Dados que o jogador leva nesta run. */
    Dado      dados_ativos[MAX_DADOS_JOGADOR];

    /* Tempo e frames. */
    float     delta_tempo;        /* s desde o último frame */
    float     tempo_total;
    int       contador_frames;

    bool      modo_debug;         /* F1 alterna; mostra FPS */
    bool      tiros_ativos;       /* Q alterna; pausa o auto-fire */

    /* Letterbox: tudo é renderizado neste framebuffer fixo (LARGURA×ALTURA) e
     * depois copiado escalado pra janela, mantendo o aspect ratio (nunca cropa). */
    RenderTexture2D render_target;
} EstadoJogo;

#endif /* TIPOS_H */
