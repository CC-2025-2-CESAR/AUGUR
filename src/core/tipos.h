/* ============================================================================
 * tipos.h - CONTRATO ENTRE OS DEVS
 * ============================================================================
 *
 * Este é o arquivo mais importante do projeto. Aqui ficam TODAS as structs,
 * enums e constantes que os 3 devs compartilham. Se você é o Dev 2 ou Dev 3
 * lendo isso: não escreva suas próprias versões das structs — use as daqui.
 *
 * POR QUE TUDO EM UM ARQUIVO SÓ?
 *   - Evita dependências circulares (um header que inclui outro que inclui...).
 *   - Qualquer módulo só precisa dar #include "tipos.h" pra ver tudo.
 *   - É o "contrato": quando esse arquivo está estável, os 3 podem codar em
 *     paralelo sem esperar ninguém.
 *
 * REGRA: Mudou alguma struct aqui? AVISE O GRUPO antes de commitar. Todo
 * mundo depende desse arquivo.
 *
 * ATUALIZACAO (engine Arthur): enums Condicao/Efeito reordenados conforme o
 * GDD; Inimigo/Magia ganharam campos de status; nova lista ProjetilInimigo;
 * novo MotorProfecia. Numeros de balanceamento NAO ficam aqui — vivem nos
 * arquivos de conteudo da Luisa (magias_comportamento.c, profecia_efeitos.c,
 * projeteis_inimigo_tipos.c). Quem indexar nomes em profecia.c atualiza junto.
 * ========================================================================== */

#ifndef TIPOS_H
#define TIPOS_H

#include "raylib.h"   /* Vector2, Rectangle, Color vêm dela */
#include <stdbool.h>  /* bool, true, false */
#include <stddef.h>   /* NULL — usado nas cabeças das listas encadeadas */

/* ============================================================================
 * CONSTANTES GLOBAIS
 * --------------------------------------------------------------------------
 * Qualquer "número mágico" que vários arquivos usam vira #define aqui.
 * Assim muda em um lugar só.
 * ========================================================================== */
#define LARGURA_TELA      1280
#define ALTURA_TELA       720
#define FPS_ALVO          60

#define MAX_PROJETEIS     256   /* teto de segurança pra lista de magias */
#define MAX_PROJETEIS_INIMIGO 256 /* teto da lista de projéteis de inimigo */
#define MAX_INIMIGOS      128   /* teto de segurança pra lista de inimigos */
#define MAX_OBSTACULOS     40   /* teto de obstáculos do mapa por run */
#define CARTAS_POR_ESCOLHA 3    /* quantas cartas aparecem entre ondas */
#define MAX_DADOS_JOGADOR 2     /* quantos dados o jogador leva por run */

/* ============================================================================
 * ENUMS — "Int com nome"
 * --------------------------------------------------------------------------
 * Enum = tipo que é internamente um int, mas com nomes legíveis. Em vez de
 * escrever "if (estado == 2)", você escreve "if (estado == COMBATE)".
 * Bem mais fácil de ler e difícil de errar.
 * ========================================================================== */

/* Estados da máquina de estados do jogo.
 * Main.c tem um switch que decide o que rodar baseado nesse valor. */
typedef enum {
    ESTADO_MENU,                /* tela inicial */
    ESTADO_REVELACAO_PROFECIA,  /* mostra os 3 modificadores sorteados */
    ESTADO_COMBATE,             /* timeline rolando, inimigos spawnando */
    ESTADO_PAUSA,               /* ESC durante o combate; mundo congelado */
    ESTADO_CARTAS_UPGRADE,      /* a cada minuto: jogador escolhe upgrade */
    ESTADO_GAME_OVER,           /* morreu, mostra score e seed */
    ESTADO_VITORIA,             /* matou o chefão final aos 5:00 */
    ESTADO_SAIR                 /* sinaliza main pra fechar a janela */
} EstadoAtual;

/* Comportamento de IA de um inimigo.
 * Cada valor mapeia 1:1 a uma função em inimigos_tipos.c (a Luísa adiciona
 * casos novos no dispatch quando quiser uma IA inédita). */
typedef enum {
    IA_CHASE,                   /* anda direto na direção do jogador */
    IA_KITER,                   /* mantém distância e atira projéteis */
    IA_BOSS_FASES               /* chefão com fases por % de vida */
} ComportamentoIA;

/* 6 elementos do jogo. Ordem importa: é usada pra indexar tabelas de nomes.
 * ELEMENTO_TOTAL no final é truque comum: vira a contagem automaticamente. */
typedef enum {
    ELEMENTO_FOGO,
    ELEMENTO_GELO,
    ELEMENTO_RELAMPAGO,
    ELEMENTO_VENENO,
    ELEMENTO_ARCANO,
    ELEMENTO_SOMBRA,
    ELEMENTO_TOTAL
} Elemento;

/* Tipos de inimigos. Dev 3 usa isso pra decidir IA e sprite. */
typedef enum {
    INIMIGO_CORPO_A_CORPO,
    INIMIGO_A_DISTANCIA,
    INIMIGO_ELITE,
    INIMIGO_CHEFE
} TipoInimigo;

/* Gatilhos das profecias. "Quando X acontece, dispare o efeito".
 * Dev 3 precisa checar essas condições no loop de combate. */
typedef enum {
    COND_AO_MATAR,          /* inimigo morreu */
    COND_AO_RECEBER_DANO,   /* jogador tomou hit */
    COND_A_CADA_10S,        /* timer interno por mod: dispara a cada 10s */
    COND_EM_COMBO,          /* contador de combo atingiu o limiar */
    COND_AO_ROLAR_DADO,     /* jogador rolou dado (gancho Sofia/dados) */
    COND_AO_CURAR,          /* jogador foi curado */
    COND_PRIMEIRA_HIT,      /* a 1ª magia que acerta na run */
    COND_VIDA_ABAIXO_X,     /* HP do jogador < X% (X tunável pela Luísa) */
    COND_AO_ACERTAR,        /* extra renomeável: toda magia que acerta */
    COND_INICIO_RUN,        /* extra renomeável: uma vez no começo da run */
    COND_TOTAL
} Condicao;

/* Efeitos que podem ser disparados pelas profecias. */
typedef enum {
    EF_EXPLOSAO,            /* dano em área no contexto */
    EF_CURA,                /* recupera HP do jogador */
    EF_DUPLICA_PROJETIL,    /* próximos disparos saem duplicados */
    EF_CONGELA,             /* congela inimigos */
    EF_DROPA_DADO,          /* gancho Sofia/dados — no-op seguro por ora */
    EF_MAIS2_PROX_ROLL,     /* gancho Sofia/dados — no-op seguro por ora */
    EF_DANO_TRIPLO,         /* próxima hit do jogador triplica */
    EF_SPAWNA_ALIADO,       /* spawna inimigo fraco aliado temporário */
    EF_REDUZ_COOLDOWN,      /* reduz cooldown de disparo por um tempo */
    EF_IGNITE_PASSIVO,      /* aplica DoT (ignite) no alvo */
    EF_ESCUDO,              /* extra renomeável: anula o próximo hit no jogador */
    EF_ROUBO_DE_VIDA,       /* extra renomeável: lifesteal por um tempo */
    EF_TOTAL
} Efeito;

/* Tipos de obstáculo do mapa. Cada tipo desenha de um jeito (árvore tem
 * tronco + copa, pedra é um círculo cinza com volume) e tem range de raio
 * próprio. Luísa decide a lista final na implementação. */
typedef enum {
    OBSTACULO_ARVORE,
    OBSTACULO_PEDRA,
    OBSTACULO_TIPO_TOTAL
} TipoObstaculo;

/* Tipos de cartas de upgrade. Dev 2 popula e aplica. */
typedef enum {
    CARTA_DANO_UP,
    CARTA_VIDA_UP,
    CARTA_VELOCIDADE_UP,
    CARTA_MAIS_MAGIAS,
    CARTA_RECARGA_DADO,     /* devolve um dado pro jogador */
    CARTA_TOTAL
} TipoCarta;


/* ============================================================================
 * STRUCTS DO JOGO
 * --------------------------------------------------------------------------
 * Toda entidade importante tem sua própria struct. Passamos elas por ponteiro
 * nas funções (Jogador *j), o que é mais rápido que copiar e permite que a
 * função modifique os campos.
 * ========================================================================== */

/* -------------------- JOGADOR --------------------
 * `bonus_dano` é somado ao dano-base de toda magia no momento da criação do
 * projétil (a Luísa lê este campo em magias) — nunca itere a lista de
 * projéteis em voo pra aplicar bônus, eles morrem e o bônus se perde. */
typedef struct {
    Vector2 posicao;        /* x, y na tela. Vector2 é do Raylib. */
    Vector2 velocidade;     /* usado pra mover de forma suave */
    float   raio;           /* pra colisão circular */
    int     vida;           /* HP atual */
    int     vida_maxima;    /* HP teto */
    float   velocidade_movimento;  /* pixels por segundo */
    int     biomassa;       /* moeda da meta-progressão */
    int     bonus_dano;     /* somado no dano-base de toda magia disparada */
} Jogador;


/* -------------------- MAGIAS (LISTA ENCADEADA) --------------------
 * POR QUE LISTA ENCADEADA?
 *   - Projéteis nascem e morrem o tempo todo (spawn e free constante).
 *   - Quantidade varia: ora 2, ora 150.
 *   - Lista encadeada atende o REQUISITO OBRIGATÓRIO de PIF de listas.
 *   - Cada magia vira um "nó" alocado com malloc e liberado com free.
 * ---------------------------------------------------------------- */

/* Dados de UMA magia/projétil individual. */
typedef struct {
    Vector2  posicao;
    Vector2  velocidade;    /* direção e velocidade do projétil */
    float    dano;
    float    tempo_de_vida; /* em segundos; projétil some quando chega a 0 */
    float    raio;          /* raio de colisão (preenchido a partir do elemento) */
    Elemento elemento;
    bool     viva;          /* se false, será removida no próximo frame */
    int      saltos_restantes; /* hops de chain do Relâmpago; 0 = sem chain */
    bool     ja_acertou;    /* guarda contra reprocessar o mesmo projétil */
} Magia;

/* Nó da lista encadeada. Cada nó carrega uma Magia e aponta pro próximo. */
typedef struct MagiaNo {
    Magia           dados;
    struct MagiaNo *proxima;  /* NULL = fim da lista */
} MagiaNo;


/* -------------------- INIMIGOS (LISTA ENCADEADA) --------------------
 * Mesma lógica das magias: spawn/despawn constante, quantidade variável.
 * ------------------------------------------------------------------ */
typedef struct {
    Vector2     posicao;
    Vector2     velocidade;
    float       raio;
    int         vida;
    int         vida_maxima;
    float       dano;                /* dano que causa ao encostar */
    float       velocidade_movimento;
    TipoInimigo tipo;
    int         recompensa_biomassa; /* quantas biomassas dropa ao morrer */
    bool        vivo;

    /* --- Status aplicado por magias/combos/profecia ---
     * A engine aplica (colisao.c) e expira (inimigos.c) estes campos. As
     * MAGNITUDES vivem em magias_comportamento.c / profecia_efeitos.c. */
    float congelado_tempo;           /* s; >0 zera a velocidade no update */
    float veneno_tempo;              /* s restantes do DoT */
    float veneno_dps;                /* dano/s do DoT (escala com stacks) */
    int   veneno_stacks;             /* stacks ativos (teto tunável) */
    float veneno_acumulado;          /* acumulador fracionário (não trunca int vida) */
    float marca_termica_tempo;       /* janela do combo Choque Térmico (marca de Fogo) */
    float proxima_hit_multiplicador; /* mult. da PRÓXIMA hit recebida (1.0 = normal) */
    bool  aliado;                    /* spawnado por EF_SPAWNA_ALIADO: não fere o jogador */
    float vida_aliado_restante;      /* s de vida de um aliado (0 = não expira) */
    float timer_disparo;             /* cooldown do disparo de projétil deste inimigo */
} Inimigo;

typedef struct InimigoNo {
    Inimigo            dados;
    struct InimigoNo  *proximo;
} InimigoNo;


/* -------------------- PROJÉTEIS DE INIMIGO (LISTA ENCADEADA) --------------------
 * Tiro padrão (NÃO-elemental) que inimigos ranged e o chefão disparam no
 * jogador. Mesma mecânica de lista das magias (malloc/free). Os stats vivem
 * em projeteis_inimigo_tipos.c — a Luísa tuna por TipoInimigo (nerf/buff,
 * e futuramente pode torná-lo elemental sem mexer na engine). */
typedef struct {
    Vector2 posicao;
    Vector2 velocidade;
    float   dano;
    float   tempo_de_vida;   /* em segundos; some quando chega a 0 */
    float   raio;
    Color   cor;
    bool    vivo;            /* se false, removido no próximo frame */
} ProjetilInimigo;

typedef struct ProjetilInimigoNo {
    ProjetilInimigo            dados;
    struct ProjetilInimigoNo  *proximo;
} ProjetilInimigoNo;


/* -------------------- TABELAS DE PARÂMETROS (DEV 3) --------------------
 * Estas duas structs são usadas em tabelas const indexadas por TipoInimigo
 * e Elemento, respectivamente. A Luísa preenche essas tabelas em
 * inimigos_tipos.c e magias_tipos.c — é o ponto único onde stats vivem.
 *
 * Quem usa: a engine (inimigos.c, magias.c) lê desses arrays na hora de
 * spawnar, mover, desenhar e disparar. Mudou um valor na tabela? Próximo
 * spawn já pega o novo. Ideal pra balanceamento.
 * ----------------------------------------------------------------------- */
typedef struct {
    int             vida_base;
    float           dano;
    float           velocidade_movimento;
    float           raio;                /* colisão */
    float           raio_visual;         /* desenho (pode != colisão) */
    Color           cor;
    int             recompensa_biomassa;
    ComportamentoIA comportamento;
} ParametrosInimigo;

typedef struct {
    float dano_base;
    float velocidade_projetil;
    float tempo_de_vida;
    float raio_projetil;
    float intervalo_disparo;             /* segundos entre disparos automáticos */
    Color cor;
} ParametrosMagia;


/* -------------------- TIMELINE (CRONOGRAMA + EVENTOS) --------------------
 * O jogo NÃO tem "ondas finitas" no estilo arena clássico. A run inteira é
 * uma timeline contínua de 5 minutos, modelada à la Vampire Survivors:
 *
 *   - O Cronograma guarda o tempo total decorrido desde o início da run.
 *   - A tabela EVENTOS_CRONOGRAMA[] (em src/sistemas/cronograma_eventos.c)
 *     descreve, declarativamente, "do minuto X ao minuto Y, spawnar inimigos
 *     do tipo T a cada Z segundos". Múltiplos eventos podem estar ativos
 *     ao mesmo tempo (e.g., melee + ranged simultâneos a partir dos 2:00).
 *   - Aos 5:00, a engine spawna 1 chefão e para os outros eventos. Quando
 *     o chefão morre, o jogo transiciona pra ESTADO_VITORIA.
 *   - A cada minuto inteiro (1:00, 2:00, …), a tela de cartas abre e o
 *     tempo congela; o jogador escolhe um upgrade e a run continua.
 *
 * A Luísa edita SÓ a tabela EVENTOS_CRONOGRAMA[]. A engine cuida do resto.
 * ----------------------------------------------------------------------- */

/* Um evento agendado: "do tempo_inicio ao tempo_fim, spawnar tipo a cada X
 * segundos". A engine faz uma cópia interna ANTES de mexer em timer_interno
 * e ativo, então a tabela declarativa em cronograma_eventos.c pode ser const. */
typedef struct {
    float        tempo_inicio_seg;   /* quando o evento começa a spawnar */
    float        tempo_fim_seg;      /* quando para; INFINITY = nunca */
    TipoInimigo  tipo;               /* qual inimigo é spawnado */
    float        intervalo_spawn;    /* segundos entre cada spawn */
    float        timer_interno;      /* acumulador (gerenciado pela engine) */
    bool         ativo;              /* engine liga/desliga conforme o tempo */
} EventoCronograma;

#define MAX_EVENTOS_CRONOGRAMA 32    /* teto de eventos copiados pra runtime */

typedef struct {
    float            tempo_decorrido;        /* segundos desde o início da run */
    float            tempo_proxima_carta;    /* dispara cartas neste valor */
    bool             cartas_pendentes;       /* engine setou, main consome */
    bool             chefao_spawnado;        /* já criou o chefão final? */
    bool             esperando_chefao_morrer;
    bool             vitoria;                /* chefão derrotado */
    EventoCronograma eventos[MAX_EVENTOS_CRONOGRAMA];
    int              qtd_eventos;
} Cronograma;


/* -------------------- PROFECIA (O CORAÇÃO DO JOGO) --------------------
 * Uma profecia é composta por 3 modificadores independentes. Cada um
 * combina [Elemento] + [Condição] + [Efeito], por exemplo:
 *   "Fogo | Ao matar -> Explosão"
 *
 * Com 6 elementos × 10 condições × 12 efeitos = 720 combinações por modificador.
 * 3 modificadores = 720^3 = ~370 milhões de combinações teóricas.
 *
 * Como é gerado proceduralmente a partir de uma seed (unsigned int), duas
 * runs com a mesma seed produzem a mesma profecia — reprodutível pra debug
 * e pra compartilhar runs interessantes com amigos.
 * -------------------------------------------------------------------- */
typedef struct {
    Elemento elemento;
    Condicao condicao;
    Efeito   efeito;
} Modificador;

typedef struct {
    Modificador mods[3];       /* sempre 3 modificadores por profecia */
    unsigned int seed;         /* guardada pra debug e replay */
} Profecia;


/* -------------------- MOTOR DE PROFECIA (RUNTIME) --------------------
 * Estado vivo que o motor (profecia.c) usa pra avaliar Condições e aplicar
 * Efeitos durante o combate. Zerado a cada run (jogo_resetar_run). As
 * MAGNITUDES e LIMIARES ficam em profecia_efeitos.c — a Luísa tuna lá.
 * -------------------------------------------------------------------- */
typedef struct {
    float timer_cond[3];          /* COND_A_CADA_10S: acumulador por mod */
    float cooldown_global_disparo;/* cooldown unico entre disparos (round-robin) */
    int   proximo_slot_disparo;   /* indice 0..2 do proximo mod a disparar */
    bool  cond_vida_armada[3];    /* debounce de COND_VIDA_ABAIXO_X por mod */
    int   combo_contador;         /* kills dentro da janela atual */
    float combo_janela_restante;  /* s restantes da janela de combo */
    bool  primeira_hit_consumida; /* COND_PRIMEIRA_HIT dispara só 1x por run */
    bool  inicio_run_disparado;   /* COND_INICIO_RUN dispara só 1x por run */
    float dano_triplo_proxima;    /* >1 => próxima hit do jogador multiplica */
    int   duplica_proximos;       /* nº de disparos a duplicar (EF_DUPLICA_PROJETIL) */
    float escudo_ativo;           /* >0 => próximo hit no jogador é anulado */
    float roubo_vida_tempo;       /* >0 => dano em inimigo cura o jogador */
    float reduz_cooldown_tempo;   /* >0 => cooldown de disparo reduzido */
    bool  em_aplicar_efeito;      /* guard de reentrância no dispatch */
    int   pending_dado_drop;      /* gancho Sofia/dados: dados a dropar (no-op) */
    int   pending_bonus_roll;     /* gancho Sofia/dados: +N no próximo roll (no-op) */
} MotorProfecia;


/* -------------------- OBSTÁCULOS DO MAPA (DEV 3) --------------------
 * Objetos fixos no mundo (árvores, pedras...). Bloqueiam jogador e inimigos
 * (push-out) mas não causam dano. Layout deve ser gerado UMA vez no início
 * da run, determinístico a partir da seed da profecia (mesma seed = mesmo
 * mapa, pra debug/replay).
 *
 * Como são fixos durante a run (não spawnam nem somem em tempo de combate),
 * usamos um ARRAY simples no EstadoJogo com qtd_obstaculos marcando quantos
 * slots estão preenchidos. Não precisa de lista encadeada aqui.
 * ----------------------------------------------------------------- */
typedef struct {
    Vector2       posicao;
    float         raio;
    TipoObstaculo tipo;
} Obstaculo;


/* -------------------- CARTAS E DADOS (DEV 2) --------------------
 * Cartas de upgrade que aparecem entre ondas. Dados são rolados pra
 * modificar o valor das cartas.
 * --------------------------------------------------------------- */
typedef struct {
    TipoCarta tipo;
    int       raridade;          /* 0=comum, 1=rara, 2=lendária */
    int       valor;             /* ex.: +10 de dano */
    char      nome[64];
    char      descricao[256];
} Carta;

typedef struct {
    int faces;                   /* d6 = 6, d20 = 20 */
    int ultimo_resultado;        /* pra mostrar na tela após rolar */
} Dado;


/* -------------------- DADOS SALVOS (DEV 2) --------------------
 * Persistem entre runs. Dev 2 salva/carrega de saves/biomassa.dat
 * usando fwrite/fread (REQUISITO OBRIGATÓRIO de PIF: arquivo).
 * ------------------------------------------------------------- */
typedef struct {
    int  biomassa_total;            /* moeda acumulada em todas as runs */
    int  runs_completadas;
    int  melhor_onda;               /* maior onda alcançada até hoje */
    int  profecias_desbloqueadas[20]; /* MATRIZ — requisito obrigatório */
    char nome_jogador[32];
} DadosSalvos;


/* ============================================================================
 * ESTADO DO JOGO (STRUCT RAIZ)
 * --------------------------------------------------------------------------
 * Essa é a struct mãe. Ela carrega absolutamente tudo do estado atual.
 * Passamos ponteiro dela (EstadoJogo *ej) pra todas as funções do jogo.
 *
 * POR QUE USAR ESSA STRUCT RAIZ?
 *   - Zero variáveis globais espalhadas pelo código.
 *   - Toda função vê o contexto inteiro e pode modificar o que precisa.
 *   - Fácil de salvar/carregar: só serializa essa struct.
 *   - Fica óbvio quem depende de quê.
 * ========================================================================== */
typedef struct {
    /* --- Máquina de estados --- */
    EstadoAtual estado_atual;
    EstadoAtual proximo_estado;   /* buffer de transição pra trocar entre frames */

    /* --- Entidades principais --- */
    Jogador      jogador;
    Profecia     profecia;
    MotorProfecia motor_profecia;    /* runtime do motor de efeitos da profecia */
    Cronograma   cronograma;         /* timeline da run (substituiu Onda) */
    DadosSalvos  salvamento;

    /* --- Camera 2D ---
     * O jogador vive em um mundo infinito (sem bordas). A câmera segue o
     * jogador: camera.target = jogador.posicao. offset é o ponto da tela onde
     * o target aparece (centro da tela = player centralizado). Tudo que for
     * desenhado dentro de BeginMode2D(camera)/EndMode2D é interpretado em
     * coordenadas de mundo; o que fica fora é coord de tela (HUD, menus). */
    Camera2D  camera;

    /* --- Listas encadeadas (cabeças) ---
     * Começam em NULL = lista vazia. Dev 3 adiciona nós com malloc. */
    MagiaNo   *magias_cabeca;
    InimigoNo *inimigos_cabeca;
    ProjetilInimigoNo *projeteis_inimigo_cabeca;

    /* --- Obstáculos do mapa ---
     * Array fixo populado uma vez no início da run (determinístico a partir
     * da seed da profecia). qtd_obstaculos é quantos slots estão realmente
     * preenchidos. Bloqueiam o jogador e os inimigos. */
    Obstaculo obstaculos[MAX_OBSTACULOS];
    int       qtd_obstaculos;

    /* --- Opções de upgrade mostradas no estado CARTAS_UPGRADE ---
     * MATRIZ de 3 cartas (requisito obrigatório). */
    Carta     escolhas_upgrade[CARTAS_POR_ESCOLHA];

    /* --- Dados que o jogador escolheu levar nessa run --- */
    Dado      dados_ativos[MAX_DADOS_JOGADOR];

    /* --- Tempo e frames --- */
    float     delta_tempo;        /* segundos desde o último frame */
    float     tempo_total;        /* tempo acumulado da run */
    int       contador_frames;

    /* --- Debug --- */
    bool      modo_debug;         /* F1 alterna; mostra FPS e info extra */

    /* --- Toggles do jogador durante o combate --- */
    bool      tiros_ativos;       /* Q alterna; quando false, auto-fire pausa */
} EstadoJogo;

#endif /* TIPOS_H */
