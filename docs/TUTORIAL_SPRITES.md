# Tutorial — Sprites do AUGUR

Esse doc cobre tanto a **entrega das sprite sheets** (parte da Luísa) quanto a **integração no engine** (parte do dev). As duas partes vivem juntas aqui pra que a Luísa veja o que precisa entregar e o dev integrador veja onde o trabalho dela se encaixa.

A entrega atual (`assets/sprites/`) e o módulo `src/core/assets.{c,h}` já estão prontos — esse tutorial documenta como tudo funciona e como estender pra novos personagens ou novos inimigos.

---

## Parte 1 — Entrega das sprite sheets

### 1.1 Onde colocar os arquivos

Tudo vai em `assets/sprites/`, organizado por categoria:

```
assets/sprites/
├── personagem/
│   └── augur_sheet.png       (+ augur.json — opcional, metadata)
├── inimigos/
│   ├── carnical_sheet.png    (corpo a corpo) + carnical.json
│   ├── vidente_sheet.png     (a distância)   + vidente.json
│   ├── arauto_sheet.png      (elite, 40×40)  + arauto.json
│   └── oraculo_sheet.png     (chefe, 64×64)  + oraculo.json
├── magias/
│   ├── magia_fogo.png        (16×16 ou 24×24)
│   ├── magia_gelo.png
│   ├── magia_relampago.png
│   ├── magia_veneno.png
│   ├── magia_arcano.png
│   └── magia_sombra.png
└── background/
    ├── tileset.png           (atlas com N tiles em 1 linha)
    ├── tile_*.png            (versões individuais dos tiles, opcional)
    └── tiles.json            (pesos sugeridos pra sorteio)
```

Naming é **obrigatório em ASCII**, **snake_case**, **sem acento**. O engine carrega por path literal (em `src/core/assets.c`); renomear quebra a carga.

### 1.2 Formato

- **PNG com canal alpha** (transparência). Sem alpha, o sprite vira retângulo opaco no jogo.
- **Fundo transparente, nunca branco/preto sólido.**
- **Cores integradas**: paleta consistente com o tema (capuz creme, manto preto-azul, olhos ciano pro AUGUR; cinza-rocha + brasa pro carniçal; etc.).

### 1.3 Layout das sprite sheets de personagem/inimigo

Todas as sheets seguem **o mesmo layout**: 14 linhas (rows), cada linha = uma animação, e cada coluna = um frame da animação. A altura da sheet é `frame_h × 14`.

| Row  | Animação            | Frames sugeridos | FPS sugerido |
|------|---------------------|------------------|--------------|
| 0    | `idle_down`         | 6                | 6            |
| 1    | `idle_up`           | 6                | 6            |
| 2    | `idle_left`         | 6                | 6            |
| 3    | `idle_right`        | 6                | 6            |
| 4    | `walk_down`         | 8                | 10           |
| 5    | `walk_up`           | 8                | 10           |
| 6    | `walk_left`         | 8                | 10           |
| 7    | `walk_right`        | 8                | 10           |
| 8    | `cast_down`         | 6                | 12           |
| 9    | `cast_up`           | 6                | 12           |
| 10   | `cast_left`         | 6                | 12           |
| 11   | `cast_right`        | 6                | 12           |
| 12   | `hurt` (omni)       | 4                | 12           |
| 13   | `death` (omni)      | 8                | 8            |

Hurt e death **não têm versão por direção** — usam a linha única, omnidirecional.

**Tamanhos por entidade** (já mapeados no engine):

| Entidade        | `frame_w × frame_h` |
|-----------------|---------------------|
| Jogador (augur) | 32 × 32             |
| Carniçal        | 32 × 32             |
| Vidente Falso   | 32 × 32             |
| Arauto (elite)  | 40 × 40             |
| Oráculo (chefe) | 64 × 64             |

### 1.4 Sprites de magia (estáticos)

Magias usam **1 frame único** (não animado): só um PNG de 16×16 ou 24×24 com a "ponta" do projétil apontando pra **direita** (+X). O engine rotaciona em runtime com `atan2(velocidade)`, então o sprite só precisa estar orientado pra direita; ele gira naturalmente conforme a magia voa.

### 1.5 Tileset do chão

`tileset.png` é um atlas horizontal: N tiles de 32×32 lado a lado em uma única linha. Todos os tiles compartilham a mesma borda de "rejunte" pra que qualquer combinação encaixe sem emenda visível.

Tiles atuais (`tile_*.png` individuais + `tileset.png` atlas):

| Tile               | Peso aproximado | Uso |
|--------------------|-----------------|-----|
| `tile_plain`       | 60%             | base — chão limpo |
| `tile_plain2`      | 20%             | variação da base |
| `tile_crack`       | 10%             | lajota rachada |
| `tile_moss`        | 5%              | musgo/desgaste |
| `tile_rubble`      | 1%              | escombro |
| `tile_rune_cyan`   | 1%              | runa ciano (raro) |
| `tile_gold_mosaic` | 1%              | mosaico dourado (raro) |
| `tile_bird_glyph`  | 1%              | glifo de ave (raro) |
| `tile_fissure_glow`| 1%              | fissura com luz ciano (raro) |
| `tile_tiles_quad`  | <1%             | quebra escala (4 lajotas menores) |

Os pesos vivem inline no engine (`src/core/main.c`, função `desenhar_chao_mundo`). Pra mudar o "tom" do chão, ajuste os buckets lá.

### 1.6 Checklist antes de commitar

- [ ] PNG com canal alpha; fundo transparente.
- [ ] Naming em `snake_case` ASCII (sem acento), conforme tabela 1.1.
- [ ] Sheet de personagem/inimigo tem 14 rows na ordem da tabela 1.3.
- [ ] Todos os frames dentro de uma sheet têm o mesmo tamanho.
- [ ] Magia aponta pra direita no PNG (engine rotaciona).
- [ ] Tile é 32×32 e bate a borda com os outros tiles do tileset.
- [ ] `mingw32-make && ./augur` mostra a sheet renderizando.

---

## Parte 2 — Integração no engine

A integração está **pronta** em `src/core/assets.{c,h}` e nos três `_desenhar` (jogador, inimigos, magias) + `desenhar_chao_mundo` em `main.c`. Esta seção explica como o sistema funciona pra que quem quiser estender (ex.: adicionar um inimigo novo, animação extra, ou versão direcional pra 8 sentidos) consiga.

### 2.1 Módulo `assets` (texturas globais)

`src/core/assets.h` declara:

```c
typedef struct {
    Texture2D jogador;
    Texture2D inimigos[ASSETS_NUM_INIMIGOS];   /* 4: indexado por TipoInimigo */
    Texture2D magias[ELEMENTO_TOTAL];          /* 6: indexado por Elemento */
    Texture2D tileset;
} Assets;

extern Assets g_assets;
```

`g_assets` é variável global única, populada por `assets_carregar()` em `jogo_inicializar` (depois de `InitWindow`) e liberada por `assets_liberar()` em `jogo_finalizar` (antes de `CloseWindow`).

Se um PNG não estiver no disco, `LoadTexture` retorna textura com `id == 0`. Os `_desenhar` testam isso e **caem em fallback** (círculo + outline pro jogador/inimigos, bolinha colorida pras magias, grid de linhas pro chão). Excluir 1 PNG nunca derruba o jogo.

### 2.2 `MetaSheet` — metadata por sprite sheet

```c
typedef struct {
    int frame_w, frame_h;
    int idle_row_base;  int idle_n;   float idle_fps;
    int walk_row_base;  int walk_n;   float walk_fps;
    int cast_row_base;  int cast_n;   float cast_fps;
    int hurt_row;       int hurt_n;   float hurt_fps;
    int death_row;      int death_n;  float death_fps;
} MetaSheet;

extern MetaSheet META_JOGADOR;
extern MetaSheet META_INIMIGO[ASSETS_NUM_INIMIGOS];
```

`idle_row_base` é a linha onde começa `idle_down`; `idle_down/up/left/right` ocupam linhas `idle_row_base + 0..3`. Mesmo padrão pra walk e cast. `hurt_row` e `death_row` apontam pra linha única (omnidirecional).

A tabela de magnitudes vive em `src/core/assets.c` — bate exatamente com a tabela 1.3. Pra adicionar uma sheet nova (ex.: 5° tipo de inimigo), incremente `ASSETS_NUM_INIMIGOS`, adicione a entrada em `META_INIMIGO[]` e a chamada `LoadTexture` em `assets_carregar`.

### 2.3 Enums de animação e direção (`tipos.h`)

```c
#define ANIM_IDLE   0
#define ANIM_WALK   1
#define ANIM_CAST   2
#define ANIM_HURT   3
#define ANIM_DEATH  4

#define DIR_DOWN    0
#define DIR_UP      1
#define DIR_LEFT    2
#define DIR_RIGHT   3
```

São os índices que `desenhar_sheet` mapeia pra `(row, fps, n_frames)` baseado no `MetaSheet`.

### 2.4 Campos novos em `Jogador` e `Inimigo`

`Jogador` (tipos.h):
```c
int   direcao_atual;
int   animacao_atual;
float animacao_tempo;
float hurt_tempo_restante;   /* >0: força HURT até zerar */
float cast_tempo_restante;   /* >0: força CAST até zerar */
```

`Inimigo` (tipos.h):
```c
int   direcao_atual;
int   animacao_atual;
float animacao_tempo;
float morrendo_tempo;        /* >0: anim DEATH rolando antes do free */
```

`animacao_tempo` zera na transição entre animações (pra cada uma começar do frame 0). `hurt_tempo_restante` e `cast_tempo_restante` são timers que prendem a animação em HURT/CAST até expirarem.

### 2.5 `desenhar_sheet()` — o helper único de desenho

```c
void desenhar_sheet(Texture2D sheet, const MetaSheet *meta,
                    Vector2 pos, int direcao, int animacao,
                    float tempo, float escala, Color tint);
```

Mapeia `(animacao, direcao)` pra `(row, n, fps)` e calcula `frame = (tempo * fps) % n`. Desenha centralizado em `pos` com `DrawTexturePro`. No-op se `sheet.id == 0`.

Uso típico (em `jogador_desenhar`):
```c
Texture2D tex = g_assets.jogador;
if (tex.id == 0) { /* fallback DrawCircle */ return; }
float escala = (j->raio * 2.0f) / (float)META_JOGADOR.frame_w;
desenhar_sheet(tex, &META_JOGADOR, j->posicao,
               j->direcao_atual, j->animacao_atual,
               j->animacao_tempo, escala, WHITE);
```

A escala é calculada pra que o sprite ocupe diâmetro igual a `2 * raio` — mantém leitura visual coerente com a hitbox circular.

### 2.6 Atualização de direção + animação

**Jogador** (`jogador_atualizar`):
- Direção: eixo dominante do vetor de input (W/A/S/D). Mantém última se input zerou.
- Animação: prioridade `HURT > CAST > (WALK se vel² > 1) > IDLE`. `hurt_tempo_restante` é setado em `jogador_sofrer_dano`; `cast_tempo_restante` é setado em `magias_disparar_elemento`.

**Inimigo** (`inimigos_atualizar` PASS 1, depois de integrar posição):
- Direção: eixo dominante do vetor `(jogador.posicao - inimigo.posicao)` — sempre olha pro jogador.
- Animação: `WALK` se está se movendo (`vel² > 1`), `IDLE` se parado.

### 2.7 Animação de morte (inimigos)

`inimigos_registrar_morte` agora marca `morrendo_tempo = 0.6f`, `animacao_atual = ANIM_DEATH`, zera velocidade e dá `vivo = false`. **NÃO** chama `free` — o `PASS 3` do `inimigos_atualizar` é que faz isso, mas só quando `morrendo_tempo <= 0`. Enquanto isso, ele acumula `animacao_tempo` pra que a animação de death role corretamente.

Resultado: o inimigo joga a animação completa antes de sumir. O `contar_nos` continua incluindo morrendo na lista, então `MAX_INIMIGOS` respeita a "fila de morte" e não tem spawn descontrolado durante hordas grandes.

### 2.8 Magias — sprite estático rotacionado

```c
float angulo_deg = atan2f(mg->velocidade.y, mg->velocidade.x) * RAD2DEG;
float lado = mg->raio * 2.0f;
Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
Rectangle dst = { mg->posicao.x, mg->posicao.y, lado, lado };
Vector2 origem = { lado * 0.5f, lado * 0.5f };
DrawTexturePro(tex, src, dst, origem, angulo_deg, WHITE);
```

`atan2` dá o ângulo da velocidade em radianos; `RAD2DEG` converte porque `DrawTexturePro` espera graus. Origem no centro do projétil pra rotacionar ao redor de si mesmo.

### 2.9 Tileset do chão (`desenhar_chao_mundo` em main.c)

A função substitui o `desenhar_grid_mundo` antigo. Algoritmo:

1. Calcula a área visível em coordenadas de mundo (baseado em `camera.target`, `camera.zoom` e `LARGURA_TELA/ALTURA_TELA`).
2. Pra cada célula 32×32 na área visível, calcula `hash = (x * 73856093) ^ (y * 19349663)` (primos clássicos pra hash espacial).
3. `bucket = hash % 100` decide qual tile usar:
   - 0..59 → `tile_plain` (idx 0)
   - 60..79 → `tile_plain2` (idx 1)
   - 80..89 → `tile_crack` (idx 2)
   - 90..94 → `tile_moss` (idx 3)
   - 95..99 → raros (índices variáveis)
4. Hash é **determinístico** em `(x, y)` — mesma posição no mundo sempre tem o mesmo tile, então o chão não "pisca" entre frames.

Fallback se `tileset.png` faltar: grid de linhas escuras 128×128 (comportamento original).

### 2.10 Letterbox — render em framebuffer fixo

Resolução do jogo é **sempre 1280×720 logicamente**, independente do tamanho da janela. O loop principal (`main.c`) tem 2 passes:

```c
/* PASS 1: desenha tudo no render_target (1280x720) */
BeginTextureMode(ej.render_target);
    ClearBackground(BLACK);
    jogo_desenhar(&ej);
EndTextureMode();

/* PASS 2: escala uniformemente o framebuffer pra janela */
BeginDrawing();
    ClearBackground(BLACK);
    int jw = GetScreenWidth(), jh = GetScreenHeight();
    float escala = fminf((float)jw/LARGURA_TELA, (float)jh/ALTURA_TELA);
    float dw = LARGURA_TELA * escala, dh = ALTURA_TELA * escala;
    float dx = (jw - dw) * 0.5f, dy = (jh - dh) * 0.5f;
    /* src.height NEGATIVO inverte Y (RenderTexture sai com Y invertido OpenGL) */
    Rectangle src = { 0, 0, (float)LARGURA_TELA, -(float)ALTURA_TELA };
    Rectangle dst = { dx, dy, dw, dh };
    DrawTexturePro(ej.render_target.texture, src, dst, (Vector2){0,0}, 0, WHITE);
EndDrawing();
```

`SetConfigFlags(FLAG_WINDOW_RESIZABLE)` é chamado **antes** de `InitWindow`, e `SetTextureFilter(TEXTURE_FILTER_POINT)` no render target mantém pixel art nítido em qualquer escala.

**Consequências**:
- Mudar resolução no menu de Opções continua chamando `SetWindowSize`, mas agora o conteúdo escala sozinho — nada cropa.
- Usuário pode arrastar a borda da janela ou maximizar. Janelas com aspect ratio diferente de 16:9 ganham barras pretas (sem distorção).
- Todo código de UI/HUD/câmera continua usando `LARGURA_TELA/ALTURA_TELA` hardcoded — sem refactor necessário em outras funções.

### 2.11 Tabela de integração — arquivo → função → `g_assets.X`

| Arquivo                          | Função                  | Textura usada                  | Fallback                  |
|----------------------------------|-------------------------|--------------------------------|---------------------------|
| `src/entidades/jogador.c`        | `jogador_desenhar`      | `g_assets.jogador`             | `DrawCircleV` + indicador |
| `src/entidades/inimigos.c`       | `inimigos_desenhar`     | `g_assets.inimigos[i->tipo]`   | `DrawCircleV` com alpha   |
| `src/entidades/magias.c`         | `magias_desenhar`       | `g_assets.magias[mg->elemento]`| `DrawCircleV` colorido    |
| `src/core/main.c`                | `desenhar_chao_mundo`   | `g_assets.tileset`             | Grid de linhas 128×128    |

### 2.12 Makefile

`src/core/assets.c` é pego automaticamente pelo `$(wildcard src/*/*.c)`. Nenhuma mudança no Makefile foi necessária.

---

## Onde tirar dúvidas

- Raylib cheatsheet: <https://www.raylib.com/cheatsheet/cheatsheet.html> (seção "Texture Loading and Drawing", "Render Texture").
- Discord do grupo pra alinhar nomes/tamanhos antes de gerar uma sheet nova.
- `LEIAME.md` original da Luísa (no kit de sprites) traz contexto narrativo das paletas e direções de design.
