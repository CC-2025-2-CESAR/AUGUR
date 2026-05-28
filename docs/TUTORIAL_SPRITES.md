# Guia de entrega de sprites — Luísa

Este doc é especificamente pra Luísa. Aqui está como organizar e entregar os PNGs das sprite sheets do AUGUR. **A integração no engine (carregar texturas, desenhar, animar) é trabalho de outro dev** — você não precisa mexer no código C.

Foque em: estrutura de pastas, formato dos arquivos, naming, e como cada sheet deve ser organizada.

## 1. Onde colocar as sprites

Todos os PNGs vão pra `assets/sprites/`. A pasta já existe no repo (com `.gitkeep`):

```
augur/
├── assets/
│   └── sprites/   ← aqui dentro
├── docs/
├── src/
└── ...
```

Crie subpastas se quiser organizar por categoria, ou deixe tudo em `sprites/` direto — fica a teu critério. O importante é o **nome do arquivo** seguir a convenção da §3.

## 2. Formato do arquivo

- **Sempre PNG.** O Raylib lê PNG nativamente.
- **Canal alpha obrigatório.** O fundo precisa ser **transparente**, nunca branco ou preto sólido.
- **Resolução nativa** (sem upscaling artificial). Se quiser entregar versões em 2x ou 4x pra preview, pode — mas o engine vai usar a versão base.
- **Pivot pensado pro centro da entidade.** O jogador, inimigos e magias são desenhados centrados na posição do objeto no jogo. Se você desenhar o personagem encostado no canto superior esquerdo do canvas, ele vai aparecer deslocado em tela. Mantenha o "centro de massa" da entidade no centro do canvas.

## 3. Convenção de naming (mapeada pro que você já entregou)

Os nomes ficam em **snake_case**, **ASCII** (sem acento), todos minúsculos.

### Jogador

| Arquivo | O que é |
|---|---|
| `augur_sheet.png` | Sprite sheet do personagem principal (Augur) com todas as animações |

### Inimigos

| Arquivo | O que é |
|---|---|
| `arauto_sheet.png`   | Sprite sheet do Arauto |
| `carnical_sheet.png` | Sprite sheet do Carnical |
| `oraculo_sheet.png`  | Sprite sheet do Oráculo |
| `vidente_sheet.png`  | Sprite sheet do Vidente |

### Magias (6 elementos)

| Arquivo | Elemento |
|---|---|
| `magia_fogo.png`      | Fogo |
| `magia_gelo.png`      | Gelo |
| `magia_relampago.png` | Relâmpago |
| `magia_veneno.png`    | Veneno |
| `magia_arcano.png`    | Arcano |
| `magia_sombra.png`    | Sombra |

### Background / mapa

| Arquivo | O que é |
|---|---|
| `tileset.png`     | Tileset principal (todos os tiles juntos numa grid) |
| `tile_*.png`      | Tiles individuais, se quiser entregar separados |

### Versões escaladas (opcional)

Se você entregar `augur_sheet_x4.png` ou `tileset_x2.png`, esses são versões pra preview — o engine vai usar **a versão base** (sem sufixo `_xN`). Mantenha as duas no zip se ajuda no seu fluxo de trabalho.

## 4. Tamanhos sugeridos

| Tipo de entidade | Frame individual |
|---|---|
| Augur (jogador) | 32×32 ou 64×64 |
| Inimigos comuns (arauto, vidente, oráculo) | 32×32 ou 64×64 |
| Inimigo grande / chefe (carnical) | 64×64 ou 128×128 |
| Magias / projéteis | 16×16 ou 24×24 |
| Tiles de mapa | 32×32 (padrão de tileset) |

Se o estilo do jogo ficar melhor com pixel art chunky, vai no 32×32. Se quiser detalhe, 64×64. Mantenha **consistência entre entidades do mesmo tipo** (todos os inimigos comuns no mesmo tamanho fica mais fácil pro engine alinhar).

## 5. Como organizar os frames de uma sprite sheet

Uma sprite sheet é um único PNG com várias frames lado a lado. A convenção que o engine vai esperar:

- **Linhas (rows) = animações diferentes** (idle, walk, attack, hit, death).
- **Colunas (columns) = frames de UMA animação**, em ordem.
- **Todos os frames têm o mesmo tamanho.** Se a frame é 32×32 e a animação tem 4 frames, a linha mede 128 px de largura.

### Layout sugerido para `augur_sheet.png`

| Linha | Animação |
|---|---|
| 0 | Idle (parado) |
| 1 | Walk (movendo) |
| 2 | Attack (lançando magia) |
| 3 | Hit (tomando dano) |
| 4 | Death (morrendo) |

Se uma animação tem menos frames que outra, deixa o resto da linha **vazio** (transparente) — não tem problema. O importante é não sobrar uma frame "meia preenchida".

### Para inimigos

Mesmo padrão. Mínimo recomendado:

| Linha | Animação |
|---|---|
| 0 | Idle |
| 1 | Walk |
| 2 | Attack (corpo a corpo ou ranged) |
| 3 | Death |

`hit` é opcional se você não quiser desenhar — o engine pode aplicar um flash branco como fallback.

### Para magias

Magias hoje são círculos coloridos no engine — você pode entregar:

- **PNG estático** (1 frame): o engine só usa essa imagem girando na direção do disparo.
- **PNG animado** (sheet com várias frames): cria um loop visual no projétil (ex.: chama tremulando).

Pra simplificar, **uma única frame por elemento já cobre tudo**. Se animar, segue o mesmo padrão (linhas = animações).

## 6. Checklist antes de commitar

Antes de mandar uma sprite pro repo:

- [ ] PNG com **fundo transparente** (não branco, não preto sólido).
- [ ] Nome em **snake_case ASCII**, sem acento nem espaço.
- [ ] Sprite sheet com **frames de mesmo tamanho** dentro do grid.
- [ ] **Pivot centralizado** (centro de massa no meio do canvas).
- [ ] Versão **base** (`*_sheet.png` sem sufixo `_xN`) existe — versões escaladas são opcionais.
- [ ] Arquivo dentro de `assets/sprites/`.

## 7. Onde tirar dúvidas

- Manda no grupo do Discord/WhatsApp do projeto — o Arthur ou a Sofia respondem rápido.
- Se quiser referência visual de como o Raylib trata sprites: <https://www.raylib.com/cheatsheet/cheatsheet.html> (seção "Texture Loading and Drawing"). **Você não precisa ler isso pra entregar**, é só pra curiosidade.

---

**Resumo do que você precisa fazer:**

1. Desenha as sprites em PNG com alpha.
2. Salva em `assets/sprites/` seguindo o naming da §3.
3. Confirma o checklist da §6.
4. Commita e abre PR.

O dev de engine vai puxar daí.
