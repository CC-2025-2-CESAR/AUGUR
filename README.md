# AUGUR

**Bullet Hell Roguelite · Projeto PIF 2026.1 · CESAR School**

## Vídeo de gameplay

▶️ **[Assista ao jogo sendo executado no YouTube](https://www.youtube.com/watch?v=jXXlK0jgmD0)**

<a href="https://www.youtube.com/watch?v=jXXlK0jgmD0">
  <img src="https://img.youtube.com/vi/jXXlK0jgmD0/hqdefault.jpg" alt="AUGUR - video de gameplay no YouTube" width="480">
</a>

## A proposta

**AUGUR é um bullet hell roguelite onde cada partida é um quebra-cabeça diferente.**
No começo de toda run, uma **Profecia** é sorteada — três regras no formato
`[Elemento] + [Condição] + [Efeito]` (ex.: *"Fogo · Ao matar → Explosão em área"*)
que definem como aquele mundo funciona. Suas magias disparam **automaticamente**;
você foca em se mover e desviar dos projéteis inimigos.

A ideia central é que **não existe uma build ótima fixa**: como a Profecia muda a
cada run, a estratégia que venceu ontem pode ser inútil hoje. Quem entende melhor
os sistemas monta a build certa mais rápido — esse conhecimento é a verdadeira
progressão do jogo.

Cada run dura **5 minutos** (uma timeline contínua estilo *Vampire Survivors*):
a cada minuto cheio você escolhe uma **carta de upgrade**, e aos 5:00 aparece o
**chefão final**. Derrotá-lo é vitória; morrer encerra a run mostrando sua
**pontuação** (a biomassa acumulada matando inimigos). Toda run tem uma **seed**
visível — mesma seed gera a mesma profecia, então dá pra compartilhar e re-jogar
partidas marcantes.

> Referências de tom: *Vampire Survivors* (loop de sobrevivência), *Balatro* (peso
> de cada decisão), *Hades* (identidade da meta-progressão). O design completo está
> no GDD (`augur_gdd.pdf`).

## Como uma run acontece

```
Menu → (seed) → Revelação da Profecia → Escolha da Magia Inicial → Combate
                                                                      │
                        a cada 1 min ──► Carta de Upgrade ──► volta ao Combate
                                                                      │
                                          aos 5:00 ──► Chefão ──► Vitória / Game Over
```

1. **Profecia revelada** — leia as 3 regras da run.
2. **Magia inicial** — escolha 1 de 3 magias sorteadas (some um 4º elemento ao
   seu leque; pelo menos uma das opções tem sinergia com a profecia).
3. **Combate** — magias miram e atiram sozinhas; você se move pra desviar dos
   projéteis e dos golpes corpo a corpo telegrafados.
4. **Cartas** — a cada minuto, 3 cartas; pode gastar um **dado** pra rolar (melhorar
   ou piorar) o valor de uma carta antes de escolher.
5. **Chefão** aos 5:00, com fases por % de vida.

## Sistemas implementados

- **Profecia ativa em combate** — 6 Elementos × 4 Condições × 6 Efeitos (~3 milhões
  de combinações). O motor avalia as condições durante a run e dispara os efeitos,
  com a magnitude exibida no texto da revelação. Tunável em `profecia_efeitos.c`.
- **Poderes por elemento** — Gelo congela, Relâmpago salta entre inimigos, Veneno
  aplica DoT acumulativo, Fogo/Arcano/Sombra são projéteis puros.
- **Combos emergentes** — Fogo→Gelo = **Choque Térmico** (stun + próxima hit
  amplificada); Arcano em inimigo envenenado = dano dobrado.
- **Inimigos atiram** — ranged e chefão disparam um projétil padrão, tunável por tipo.
- **Golpe corpo a corpo telegrafado** — corpo a corpo e elite chegam perto, **param
  e carregam** o ataque (aviso visual) e então **avançam (lunge)** pra acertar; dá pra
  esquivar saindo durante o windup. **Encostar** num inimigo ainda causa um dano leve,
  com cooldown por inimigo (chip — não drena HP a cada frame).
- **Feedback de combate** — inimigos **piscam vermelho** ao levar dano e **amarelo**
  enquanto carregam o golpe; o jogador pisca ao tomar hit.
- **Escolha de magia inicial** e **histórico das últimas 10 seeds jogadas** (pra
  recarregar uma run boa sem precisar anotar a seed).
- **Cartas de upgrade + sistema de dados** — risco gerenciado: rolar pode melhorar
  ou piorar a carta.
- **Menu completo** — Novo Jogo, Carregar (última seed), Inserir Seed, Histórico,
  Leaderboard, Opções, Sair.
- **Leaderboard top-10** — por tempo (só vitórias) e por biomassa (todas), persistido
  em arquivo.
- **Sprites + tileset** — sheets direcionais (idle/walk/cast em 4 direções + hurt +
  death) pro jogador e inimigos; magias e tiros rotacionados pela velocidade; chão
  em tilemap de grama determinístico. Se um PNG faltar, cai em primitiva sem quebrar.
- **Vídeo adaptável** — 3 resoluções + fullscreen; a janela pode ser redimensionada
  e o **letterbox** (`RenderTexture2D`) escala tudo sem cropar nem distorcer.
- **Save em arquivo** — `saves/biomassa.dat` guarda config, última seed, leaderboards
  e histórico, com validação de versão.

> A **meta-progressão** (desbloqueios permanentes entre runs) está **fora de escopo**
> nesta versão — a biomassa é a pontuação da run, não moeda persistente.

## Equipe e obrigações

O projeto é dividido por responsabilidade pra cada um trabalhar no seu módulo sem
travar os outros. O contrato comum é o `tipos.h`.

| Dev | Frente | Pelo que é responsável | Arquivos (em `src/`) |
|-----|--------|------------------------|----------------------|
| **Arthur** (Dev 1) | Engine & Core | Game loop e máquina de estados, contrato de tipos, colisão, **motor de profecia**, engines de inimigos/magias/cronograma/projéteis de inimigo, combos, render de sprites, letterbox, leaderboard, config de vídeo, histórico, escolha de magia inicial e a integração geral. | `core/main.c`, `core/tipos.h`, `core/colisao`, `core/assets`, `entidades/jogador`, `entidades/inimigos`, `entidades/magias`, `entidades/projeteis_inimigo`, `sistemas/profecia`, `sistemas/combos`, `sistemas/cronograma`, `sistemas/config_video`, `sistemas/leaderboard`, `sistemas/historico`, `sistemas/magia_inicial` |
| **Sofia** (Dev 2) | Sistemas de jogo | **Cartas** de upgrade, **sistema de dados**, **salvamento** em arquivo e a **HUD** de combate (vida, tempo, biomassa, aviso de chefão). | `sistemas/cartas`, `sistemas/dados`, `sistemas/salvamento`, `interface/hud` |
| **Luísa** (Dev 3) | Conteúdo, balanceamento & arte | Tabelas de stats e **IA** dos inimigos, stats/auto-fire das magias e seus status, tiro de inimigo, magnitudes da profecia/combos, **timeline de spawns**, e todas as **sprite sheets + tileset**. | `entidades/inimigos_tipos`, `entidades/magias_tipos`, `entidades/magias_comportamento`, `entidades/projeteis_inimigo_tipos`, `sistemas/profecia_efeitos`, `sistemas/cronograma_eventos`, `assets/sprites/` |

> **Engine vs. conteúdo.** Tudo que termina em `_tipos.c`, `_eventos.c`,
> `_comportamento.c` ou `_efeitos.c` é conteúdo/balanceamento (Luísa): tabelas `const`
> que a engine (Arthur) só consome. Pra balancear ou criar algo novo, edita-se só
> essas tabelas — nenhum número de balanceamento mora na engine.

## Compilar e rodar

Requisitos: um compilador C (**GCC** ou **Clang**), **Raylib 5.5** e **GNU Make**.
O Makefile detecta o sistema (**Windows / Linux / macOS**) e linka as libs certas
do Raylib automaticamente.

Instalar o Raylib + make:

```bash
# Windows (terminal MSYS2 UCRT64)
pacman -S mingw-w64-ucrt-x86_64-raylib mingw-w64-ucrt-x86_64-make

# Linux (Debian/Ubuntu) — se o pacote do apt for antigo, compile o Raylib 5.5 da fonte
sudo apt install build-essential libraylib-dev

# macOS (Homebrew)
brew install raylib make
```

Compilar e jogar:

```bash
make          # compila (gera augur / augur.exe)
make run      # compila e roda
make clean
```

> No Windows, fora do MSYS2 (PowerShell/CMD), use **`mingw32-make`** no lugar de `make`.

## Estrutura de pastas

Cada subpasta de `src/` agrupa um propósito. O Makefile descobre os `.c`
automaticamente e adiciona cada subpasta ao `-I`, então os `#include` são por nome
simples (`#include "tipos.h"`).

```text
src/
├── core/         loop + contrato + colisão + carga de sprites
│   ├── main.c            game loop, máquina de estados, letterbox
│   ├── tipos.h           contrato entre os devs (todas as structs/enums)
│   ├── colisao.c/.h      colisão, dano único, riders e combos
│   └── assets.c/.h       carga e desenho das sprite sheets
├── entidades/    o que vive no mundo (ENGINE = Arthur, TABELA = Luísa)
│   ├── jogador.c/.h
│   ├── inimigos.c/.h            + inimigos_tipos.c/.h
│   ├── magias.c/.h              + magias_tipos.c/.h + magias_comportamento.c/.h
│   └── projeteis_inimigo.c/.h   + projeteis_inimigo_tipos.c/.h
├── sistemas/     regras do jogo
│   ├── profecia.c/.h            + profecia_efeitos.c/.h   (gerador + motor)
│   ├── combos.c/.h
│   ├── cronograma.c/.h          + cronograma_eventos.c/.h (timeline 5 min)
│   ├── cartas.c/.h · dados.c/.h · salvamento.c/.h         (Sofia)
│   ├── historico.c/.h · magia_inicial.c/.h
│   └── config_video.c/.h · leaderboard.c/.h
└── interface/
    └── hud.c/.h                  HUD de combate (Sofia)

assets/sprites/   sheets direcionais + tileset (Luísa)
docs/             guias internos (ambiente, sprites, dicionário)
saves/            progresso gerado em runtime
```

## Controles

| Contexto | Tecla | Ação |
|----------|-------|------|
| Menus | ↑/↓ ou W/S · ENTER/ESPAÇO · ESC | navegar · confirmar · voltar |
| Leaderboard | TAB | alterna Tempo / Biomassa |
| Inserir Seed | 0–9 · BACKSPACE · ENTER | digitar · apagar · confirmar |
| Combate | WASD / setas | mover |
| Combate | Q · ESC · F1 | liga/desliga tiros · pausa · debug |
| Combate | **F3** | pula pra próxima tela de cartas (skip de tempo) |
| Cartas | 1/2/3 · R + 1/2/3 | escolher carta · rolar dado na carta |

A janela pode ser **redimensionada/maximizada** (o letterbox escala sem cropar) e a
resolução fixa fica em **Opções**.

## Conceitos obrigatórios de PIF

| Conceito | Onde |
|----------|------|
| **Structs** | `tipos.h` — `Jogador`, `Inimigo`, `Magia`, `ProjetilInimigo`, `Profecia`, `MotorProfecia`, `Cronograma`, `EstadoJogo`, `DadosSalvos`, `Carta`, `Dado`, `EntradaLeaderboard`, `EntradaHistorico`, `OpcaoMagiaInicial`, … |
| **Ponteiros** | `EstadoJogo *ej` em quase toda função; `proxima`/`proximo` nos nós; **ponteiro duplo** `InimigoNo **` na remoção de mortos |
| **Alocação dinâmica** | `malloc`/`free` dos nós das 3 listas, nas engines `magias.c`, `inimigos.c` e `projeteis_inimigo.c` (cada `_spawnar*` aloca; cada morte e os `_liberar_tudo` no fim da run liberam) |
| **Listas encadeadas** | 3 listas, cada uma com seu nó (definido em `tipos.h`) e sua engine: **`MagiaNo`** — projéteis do jogador, em `src/entidades/magias.c`; **`InimigoNo`** — inimigos, em `src/entidades/inimigos.c`; **`ProjetilInimigoNo`** — tiros de inimigo, em `src/entidades/projeteis_inimigo.c`. Inserção O(1) na cabeça e remoção dos mortos com ponteiro duplo (`No **`) |
| **Matrizes** | Arrays em structs de `tipos.h`, consumidos pelas engines: `mods[3]` (`profecia.c`), `escolhas_upgrade[]` (`cartas.c`), `eventos[]` (`cronograma.c`), `top_tempo[]`/`top_biomassa[]` (`leaderboard.c`), `historico[]` (`historico.c`). E as tabelas `const` de conteúdo: `PARAMETROS_INIMIGO[]` (`inimigos_tipos.c`), `PARAMETROS_MAGIA[]` (`magias_tipos.c`), `COMPORTAMENTO_MAGIA[]` (`magias_comportamento.c`), `PARAMETROS_PROJETIL_INIMIGO[]` (`projeteis_inimigo_tipos.c`), `EVENTOS_CRONOGRAMA[]` (`cronograma_eventos.c`) |
| **Arquivo** | `salvamento.c` grava a `DadosSalvos` inteira em binário (`fwrite`/`fread`) em `saves/biomassa.dat`, com validação de versão |

> **Por que Raylib e não a cli-lib?** O enunciado permite outra biblioteca ("de
> responsabilidade do grupo"). Um bullet hell precisa renderizar dezenas de projéteis
> com colisão circular e câmera 2D rolando suave — inviável no terminal.

## Documentação

Guias em [`docs/`](docs/) (veja o [índice](docs/README.md)): setup do ambiente
([TUTORIAL_AMBIENTE.md](docs/TUTORIAL_AMBIENTE.md)), sprite sheets
([TUTORIAL_SPRITES.md](docs/TUTORIAL_SPRITES.md)) e o glossário de termos
([dicionario.md](docs/dicionario.md)).
