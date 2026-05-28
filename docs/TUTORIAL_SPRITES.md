# Tutorial: como adicionar sprites no AUGUR

Hoje o jogo renderiza tudo com primitivas (`DrawCircleV`, `DrawRectangle`). Este tutorial mostra como trocar essas primitivas por sprites PNG sem quebrar nada — e qual o caminho mais limpo de organizar isso no projeto.

## 1. Onde colocar os arquivos

Crie os PNGs em `assets/sprites/`. A pasta já existe no repositório (com `.gitkeep`). Use nomes descritivos e em ASCII (sem acento, espaço ou maiúscula misturada):

```
assets/sprites/
├── jogador.png
├── inimigo_corpo_a_corpo.png
├── inimigo_a_distancia.png
├── inimigo_elite.png
├── inimigo_chefe.png
├── magia_fogo.png
├── magia_gelo.png
├── magia_arcano.png
├── obstaculo_arvore.png
└── obstaculo_pedra.png
```

A regra: **um sprite por enum**. Se o `TipoInimigo` tem 4 valores, são 4 sprites. Se adicionar um inimigo novo no enum, adiciona o sprite com o mesmo padrão de nome.

## 2. Formato recomendado

- **PNG com canal alpha** (transparência). O Raylib lê PNG nativamente — não precisa de lib extra.
- **Tamanhos por tipo de entidade**:
  - Jogador e inimigos: **32×32** ou **64×64**.
  - Magias/projéteis: **16×16** ou **24×24** (são pequenos na tela).
  - Obstáculos: **48×48** ou **64×64**.
  - Chefão: **128×128** (é maior na tela).
- **Pivot**: o sprite é desenhado a partir do canto superior esquerdo por padrão. Pra centralizar, subtraia `largura/2, altura/2` na hora de desenhar (ver §4).

## 3. Como carregar os sprites no código

A forma limpa é criar um módulo `assets` centralizado que carrega tudo uma vez no `jogo_inicializar` e libera no `jogo_finalizar`. Não carregue sprites dentro das funções `_desenhar` — `LoadTexture` lê o arquivo do disco e enviaria pra GPU a cada frame.

### 3.1 — Criar `src/core/assets.h`

```c
#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"
#include "tipos.h"

/* Sprites carregados na inicialização. Acesso direto (struct global). */
typedef struct {
    Texture2D jogador;
    Texture2D inimigos[4];          /* indexado por TipoInimigo */
    Texture2D magias[ELEMENTO_TOTAL];
    Texture2D obstaculos[OBSTACULO_TIPO_TOTAL];
} Assets;

extern Assets g_assets;

/* Chamar uma vez em jogo_inicializar. Tenta carregar cada sprite; se falhar
 * (arquivo inexistente), deixa texture.id = 0 — _desenhar checa isso e cai
 * em fallback. */
void assets_carregar(void);

/* Chamar uma vez em jogo_finalizar. Libera todas as texturas. */
void assets_liberar(void);

#endif /* ASSETS_H */
```

### 3.2 — Criar `src/core/assets.c`

```c
#include "assets.h"

Assets g_assets = {0};

void assets_carregar(void) {
    g_assets.jogador = LoadTexture("assets/sprites/jogador.png");

    g_assets.inimigos[INIMIGO_CORPO_A_CORPO] =
        LoadTexture("assets/sprites/inimigo_corpo_a_corpo.png");
    g_assets.inimigos[INIMIGO_A_DISTANCIA]   =
        LoadTexture("assets/sprites/inimigo_a_distancia.png");
    g_assets.inimigos[INIMIGO_ELITE]         =
        LoadTexture("assets/sprites/inimigo_elite.png");
    g_assets.inimigos[INIMIGO_CHEFE]         =
        LoadTexture("assets/sprites/inimigo_chefe.png");

    g_assets.magias[ELEMENTO_FOGO]       = LoadTexture("assets/sprites/magia_fogo.png");
    g_assets.magias[ELEMENTO_GELO]       = LoadTexture("assets/sprites/magia_gelo.png");
    g_assets.magias[ELEMENTO_RELAMPAGO]  = LoadTexture("assets/sprites/magia_relampago.png");
    g_assets.magias[ELEMENTO_VENENO]     = LoadTexture("assets/sprites/magia_veneno.png");
    g_assets.magias[ELEMENTO_ARCANO]     = LoadTexture("assets/sprites/magia_arcano.png");
    g_assets.magias[ELEMENTO_SOMBRA]     = LoadTexture("assets/sprites/magia_sombra.png");

    g_assets.obstaculos[OBSTACULO_ARVORE] = LoadTexture("assets/sprites/obstaculo_arvore.png");
    g_assets.obstaculos[OBSTACULO_PEDRA]  = LoadTexture("assets/sprites/obstaculo_pedra.png");
}

void assets_liberar(void) {
    UnloadTexture(g_assets.jogador);
    for (int i = 0; i < 4; i++) UnloadTexture(g_assets.inimigos[i]);
    for (int i = 0; i < ELEMENTO_TOTAL; i++) UnloadTexture(g_assets.magias[i]);
    for (int i = 0; i < OBSTACULO_TIPO_TOTAL; i++) UnloadTexture(g_assets.obstaculos[i]);
}
```

> **Importante:** `LoadTexture` exige que `InitWindow` já tenha rolado. Chame `assets_carregar()` DENTRO de `jogo_inicializar`, não antes do `InitWindow`.

### 3.3 — Plugar no `main.c`

```c
#include "assets.h"
/* ... */
static void jogo_inicializar(EstadoJogo *ej) {
    /* ... linhas existentes ... */
    assets_carregar();   /* NOVA */
}

static void jogo_finalizar(EstadoJogo *ej) {
    /* ... linhas existentes ... */
    assets_liberar();    /* NOVA */
}
```

## 4. Como integrar com as funções `_desenhar`

Cada entidade já tem uma função `_desenhar` (`jogador_desenhar`, `inimigos_desenhar`, `magias_desenhar` etc.). A integração é trocar `DrawCircleV` por `DrawTexturePro` ou `DrawTextureV`, mantendo um fallback caso o sprite não tenha carregado.

### Padrão de fallback

```c
static void desenhar_um_inimigo(const Inimigo *i) {
    Texture2D tex = g_assets.inimigos[i->tipo];

    /* texture.id == 0 = sprite não carregou (arquivo inexistente).
     * Cai no fallback de primitiva pra não quebrar o jogo. */
    if (tex.id == 0) {
        DrawCircleV(i->posicao, i->raio_visual, i->cor);
        return;
    }

    /* DrawTexturePro permite escala, rotação e pivot. Aqui centralizamos no
     * meio da entidade: source = sprite inteiro, dest centrado em i->posicao
     * com tamanho igual ao raio_visual * 2. */
    Rectangle src  = (Rectangle){ 0, 0, (float)tex.width, (float)tex.height };
    Rectangle dest = (Rectangle){
        i->posicao.x, i->posicao.y,
        i->raio_visual * 2.0f, i->raio_visual * 2.0f
    };
    Vector2 origem = (Vector2){ i->raio_visual, i->raio_visual };
    DrawTexturePro(tex, src, dest, origem, 0.0f, WHITE);
}
```

`WHITE` no último argumento = tinta neutra (o sprite aparece com cores originais). Pra aplicar tinte (ex.: piscar vermelho ao tomar dano), troca por `(Color){255,100,100,255}`.

### Onde aplicar no AUGUR

Os arquivos com função `_desenhar` que vão receber sprites:

| Arquivo | Função | O que renderizar |
|---|---|---|
| `src/entidades/jogador.c` | `jogador_desenhar` | `g_assets.jogador` |
| `src/entidades/inimigos.c` | `inimigos_desenhar` | `g_assets.inimigos[tipo]` |
| `src/entidades/magias.c` | `magias_desenhar` | `g_assets.magias[elemento]` |
| `src/entidades/projeteis_inimigo.c` | `projeteis_inimigo_desenhar` | (mantém primitiva ou novo sprite) |
| `src/entidades/obstaculos.c` | `obstaculos_desenhar` | `g_assets.obstaculos[tipo]` |

## 5. Rotação de sprites pra magias

Magias têm `velocidade` (Vector2), então dá pra orientar o sprite na direção do projétil:

```c
float angulo = atan2f(m->velocidade.y, m->velocidade.x) * RAD2DEG;
DrawTexturePro(tex, src, dest, origem, angulo, WHITE);
```

`RAD2DEG` é macro do Raylib. O sprite deve estar desenhado "apontando pra direita" no arquivo PNG (lado positivo do X) pra rotação fazer sentido.

## 6. Animação simples (opcional)

Se algum sprite for um *sprite sheet* (várias frames lado a lado num PNG), use `source` pra recortar a frame atual:

```c
int frame_atual = ((int)(GetTime() * 8.0f)) % 4;   /* 8 fps, 4 frames */
Rectangle src = (Rectangle){
    frame_atual * 32, 0,   /* x do frame, y=0 */
    32, 32                 /* tamanho de uma frame */
};
DrawTexturePro(tex, src, dest, origem, 0.0f, WHITE);
```

## 7. Atualizar o Makefile (não precisa por enquanto)

Como `LoadTexture` lê os arquivos em tempo de execução pelo caminho `assets/sprites/jogador.png` relativo ao binário, **basta rodar o jogo a partir da raiz do repo**. Se quiser empacotar tudo numa pasta de release, copie `assets/` pra perto do `.exe`:

```makefile
# Sugestão pra um target "release" futuro
release: $(EXECUTAVEL)
	mkdir -p release
	cp $(EXECUTAVEL) release/
	cp -r assets/ release/
```

Não é obrigatório agora — o `mingw32-make` atual já funciona porque o `.exe` é gerado na raiz do repo, onde `assets/` está.

## 8. Convenções e checklist

Antes de commitar sprites:

- [ ] PNG com fundo transparente (não branco).
- [ ] Nome em ASCII, snake_case, sem acento.
- [ ] Tamanho aproximado do `raio_visual` da entidade (não muito maior — pixel art exagerado fica feio).
- [ ] Pivot pensado pro centro (não pra um canto).
- [ ] Adicionou em `assets_carregar` e `assets_liberar` se for uma entidade nova.
- [ ] `mingw32-make` builda limpo (`LoadTexture` em sprite inexistente só loga warning — não quebra).
- [ ] Testou que o fallback de primitiva ainda funciona deletando temporariamente o `.png` e abrindo o jogo.

## 9. Onde ler mais

- Raylib: <https://www.raylib.com/cheatsheet/cheatsheet.html> — seção "Texture Loading and Drawing".
- Exemplos oficiais: <https://github.com/raysan5/raylib/tree/master/examples/textures>.
