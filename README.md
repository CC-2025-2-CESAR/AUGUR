# AUGUR

Bullet Hell Roguelite · Projeto PIF 2026.1 · CESAR School

## Sobre o jogo

Em AUGUR, cada run começa com uma **Profecia** — três modificadores `[Elemento] + [Condição] + [Efeito]` gerados proceduralmente que definem as regras daquela run. Magias disparam automaticamente enquanto você esquiva de projéteis inimigos. A run é uma timeline de **5 minutos** estilo Vampire Survivors: a cada minuto cheio você escolhe cartas de upgrade, e aos 5:00 surge o chefão final. Derrotá-lo encerra a run com vitória; ao morrer, sua **pontuação** (biomassa acumulada matando inimigos) aparece na tela final.

Cada run tem uma **seed** visível na tela. Como a profecia é gerada de forma determinística a partir da seed, você pode compartilhar runs marcantes com amigos: mesma seed = mesma profecia = mesmo puzzle inicial. É o ponto de partida pra replays, debug e desafios entre jogadores.

### Sistemas implementados

- **Profecia ativa em combate.** 6 Elementos × 4 Condições × 6 Efeitos (pool encolhido pra ficar legível). O motor avalia as 3 condições durante a run e dispara os efeitos; magnitudes e limiares são 100% tunáveis em `profecia_efeitos.c`. O texto da revelação mostra a magnitude explícita (`Explosao de Fogo (40 dano, raio 150)`) — sem jargão.
- **Escolha da Magia Inicial.** Depois da revelação, o jogador escolhe 1 de 3 magias sorteadas; pelo menos uma das opções é **sinergética** (elemento bate com algum mod da profecia). A magia escolhida vira um 4º elemento no auto-fire, somando ao ciclo dos 3 elementos da profecia.
- **Poderes por elemento.** Cada magia tem um comportamento de status: Gelo congela, Relâmpago salta entre inimigos próximos, Veneno aplica DoT acumulativo, Fogo/Arcano/Sombra são projéteis puros. Tudo tunável em `magias_comportamento.c`.
- **Combos emergentes.** Fogo → Gelo = **Choque Térmico** (stun + próxima hit amplificada); Arcano em inimigo envenenado = dano dobrado.
- **Inimigos atiram.** Ranged e chefão disparam um projétil padrão (não-elemental), tunável por tipo em `projeteis_inimigo_tipos.c`.
- **Pontuação.** A biomassa coletada vira a pontuação final mostrada no game over / vitória.
- **Menu inicial completo.** Novo Jogo, Carregar Jogo (replay da última seed), Inserir Seed manual, **Histórico** (lista das últimas 10 seeds jogadas), Leaderboard, Opções e Sair. Navegação com setas + ENTER.
- **Histórico de seeds.** Persiste as últimas 10 seeds jogadas (mais recente em cima) com resultado V/D, tempo e biomassa. Clica em ENTER pra recarregar qualquer uma — útil pra revisitar uma profecia interessante sem precisar anotar.
- **Leaderboard top-10.** Duas tabelas: por tempo (só vitórias, mais rápido primeiro) e por biomassa (vitórias e derrotas, maior pontuação primeiro). Persiste entre execuções; TAB alterna abas.
- **Opções de vídeo.** 3 resoluções (1280×720, 1600×900, 1920×1080) e toggle de fullscreen. As preferências persistem no save.
- **Seed manual.** Tela "Inserir Seed" aceita um número decimal e abre uma run determinística — basta compartilhar a seed pra um amigo jogar a mesma profecia.
- **Skip de evento (F3).** Durante o combate, F3 pula pro próximo minuto cheio (próxima tela de cartas). Útil pra testar builds rápido. Se já estiver no último minuto, vai direto pro chefão.

> A meta-progressão (desbloqueios persistentes entre runs) está **fora de escopo na versão atual** — a biomassa é usada como pontuação da run, não como moeda persistente. O design completo de referência (loop, Profecias, Dados, magias, meta-progressão) está no GDD: **`augur_gdd.pdf`**.

## Equipe

| Dev | Responsabilidade | Módulos |
|-----|------------------|---------|
| Arthur (Dev 1) | Engine & Core | `main.c`, `tipos.h`, `jogador`, `profecia` (+ motor), `colisao`, engine de `inimigos`/`magias`/`cronograma`/`projeteis_inimigo`, `combos`, `obstaculos` |
| Sofia (Dev 2) | Sistemas de Jogo | `cartas`, `dados`, `salvamento`, `hud` |
| Luísa (Dev 3) | Conteúdo / Balanceamento | `inimigos_tipos`, `magias_tipos`, `magias_comportamento`, `projeteis_inimigo_tipos`, `profecia_efeitos`, `cronograma_eventos` |

## Documentação

Tudo em [`docs/`](docs/) — veja o [índice](docs/README.md) pra navegação rápida. Os guias principais:

| Guia | Quando usar |
|---|---|
| [TUTORIAL_AMBIENTE.md](docs/TUTORIAL_AMBIENTE.md) | Setup do ambiente (MSYS2/Linux), instalação do Raylib e make. Leitura obrigatória no primeiro clone. |
| [TUTORIAL_SPRITES.md](docs/TUTORIAL_SPRITES.md) | Guia da Luísa pra entregar as sprite sheets — pasta, naming, formato PNG, layout de frames, checklist. Sem código (a integração no engine fica em tarefa separada). |
| [dicionario.md](docs/dicionario.md) | Glossário de termos de jogos (AoE, DoT, kiting…) e da arquitetura do AUGUR (motor de profecia, riders, push-out…). |

## Requisitos

- GCC 15+ via MSYS2 UCRT64
- Raylib 5.5
- GNU Make 4+

## Instalação rápida do ambiente

Abra o terminal **MSYS2 UCRT64** e rode:

```bash
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-raylib
pacman -S mingw-w64-ucrt-x86_64-make
```

Valide:

```bash
pkg-config --cflags --libs raylib
gcc --version
mingw32-make --version
```

## Como compilar e rodar

No PowerShell:

```powershell
mingw32-make
.\augur.exe
mingw32-make clean
```

No terminal MSYS2:

```bash
make
make run
make clean
```

## Estrutura de pastas

O código é organizado em módulos por responsabilidade. Cada subpasta de `src/` agrupa arquivos com propósito relacionado, deixando claro pra cada dev onde encostar.

```text
Jogo-PIF/
|-- src/
|   |-- core/                          <- motor: loop, contrato (tipos.h), colisão
|   |   |-- main.c                     <- game loop e máquina de estados
|   |   |-- tipos.h                    <- contrato entre devs (todas as structs)
|   |   `-- colisao.c/.h               <- colisão + dano único, riders e combos
|   |
|   |-- entidades/                     <- coisas que vivem no mundo
|   |   |-- jogador.c/.h               <- movimento, HP e direção
|   |   |-- inimigos.c/.h              <- ENGINE de inimigos + status (Arthur)
|   |   |-- inimigos_tipos.c/.h        <- TABELA + IA por tipo (Luísa)
|   |   |-- magias.c/.h                <- ENGINE de projéteis (Arthur)
|   |   |-- magias_tipos.c/.h          <- TABELA por elemento + auto-fire (Luísa)
|   |   |-- magias_comportamento.c/.h  <- RIDER de status por elemento (Luísa)
|   |   |-- projeteis_inimigo.c/.h     <- ENGINE do tiro de inimigo (Arthur)
|   |   |-- projeteis_inimigo_tipos.c/.h <- TABELA do tiro por tipo (Luísa)
|   |   `-- obstaculos.c/.h            <- árvores e pedras (Arthur)
|   |
|   |-- sistemas/                      <- regras e lógica do jogo
|   |   |-- profecia.c/.h              <- gerador + MOTOR de efeitos (Arthur)
|   |   |-- profecia_efeitos.c/.h      <- magnitudes/limiares + combos (Luísa)
|   |   |-- combos.c/.h                <- ENGINE dos combos emergentes (Arthur)
|   |   |-- cronograma.c/.h            <- ENGINE da timeline 5min (Arthur)
|   |   |-- cronograma_eventos.c/.h    <- TABELA de eventos da timeline (Luísa)
|   |   |-- cartas.c/.h                <- sistema de upgrade (Sofia)
|   |   |-- dados.c/.h                 <- sistema de dados (Sofia)
|   |   |-- salvamento.c/.h            <- save/load em arquivo (Sofia)
|   |   |-- config_video.c/.h          <- resolução e fullscreen (Arthur)
|   |   |-- leaderboard.c/.h           <- top-10 de runs (Arthur)
|   |   |-- historico.c/.h             <- últimas 10 seeds jogadas (Arthur)
|   |   `-- magia_inicial.c/.h         <- sorteio + render da escolha (Arthur)
|   |
|   `-- interface/                     <- UI e HUD
|       `-- hud.c/.h                   <- HUD durante combate (Sofia)
|
|-- assets/                            <- sprites, sons e fontes
|-- build/                             <- arquivos .o gerados pelo make
|-- saves/                             <- progresso gerado em runtime
|-- docs/                              <- guias internos pro grupo
|   |-- README.md                      <- indice dos docs
|   |-- TUTORIAL_AMBIENTE.md           <- setup do MSYS2/Linux + Makefile do zero
|   |-- TUTORIAL_SPRITES.md            <- como adicionar sprites no lugar das primitivas
|   |-- dicionario.md                  <- glossario de termos de jogos e arquitetura
|   `-- issue-hud-cronograma.md        <- task tecnica pra Sofia (HUD VS-like)
|-- Makefile
`-- README.md
```

> **Engine vs. conteúdo.** A regra é: tudo que termina em `_tipos.c`, `_eventos.c`, `_comportamento.c` ou `_efeitos.c` é **conteúdo/balanceamento da Luísa** — tabelas de stats, IA, timeline de spawns, status por elemento, magnitudes da profecia e dos combos. A engine (Arthur) só consome essas tabelas. Pra balancear ou criar coisa nova, a Luísa edita apenas esses arquivos de conteúdo — nenhum número de balanceamento mora na engine.

> Os `#include` continuam sendo por nome simples (ex.: `#include "tipos.h"`) porque o Makefile adiciona cada subpasta de `src/` ao `-I` do compilador. Você não precisa escrever `#include "core/tipos.h"`.

## Como adicionar conteúdo (guia da Luísa)

Toda mudança de balanceamento ou conteúdo novo acontece em **três arquivos**:

| O que você quer | Onde editar |
|-----------------|-------------|
| Stats e IA de inimigos | `src/entidades/inimigos_tipos.c` |
| Stats e auto-fire de magias | `src/entidades/magias_tipos.c` |
| Status por elemento (congela/chain/veneno) | `src/entidades/magias_comportamento.c` |
| Tiro de inimigo (dano, alcance, ligar/desligar por tipo) | `src/entidades/projeteis_inimigo_tipos.c` |
| Magnitudes/limiares da profecia e dos combos | `src/sistemas/profecia_efeitos.c` |
| Quando spawnar o quê (timeline de 5 min) | `src/sistemas/cronograma_eventos.c` |

A engine (`inimigos.c`, `magias.c`, `cronograma.c`, `projeteis_inimigo.c`, `profecia.c`, `combos.c`) lê dessas tabelas. Você não precisa mexer em alocação, lista encadeada, push-out, render, motor de profecia ou detecção de combo — tudo isso já está pronto.

### Adicionar um inimigo novo (ex.: elite à distância)

1. Em `src/core/tipos.h`, no enum `TipoInimigo`, adicione um valor novo:
   ```c
   typedef enum {
       INIMIGO_CORPO_A_CORPO,
       INIMIGO_A_DISTANCIA,
       INIMIGO_ELITE,
       INIMIGO_ELITE_DISTANCIA,   /* novo */
       INIMIGO_CHEFE
   } TipoInimigo;
   ```
2. Em `src/entidades/inimigos_tipos.c`, adicione uma linha em `PARAMETROS_INIMIGO[]` na **mesma ordem do enum**:
   ```c
   {
       .vida_base = 70, .dano = 16, .velocidade_movimento = 80.0f,
       .raio = 14.0f, .raio_visual = 14.0f,
       .cor = (Color){80, 200, 220, 255},
       .recompensa_biomassa = 30,
       .comportamento = IA_KITER,    /* reaproveita a IA do ranged base */
   },
   ```
3. Pronto. Pra fazer ele aparecer numa fase, adicione uma linha em `cronograma_eventos.c`.

### Mexer na timeline da run

Cada linha em `EVENTOS_CRONOGRAMA[]` é uma fase declarativa: "do tempo X ao tempo Y, spawnar tipo T a cada Z segundos". Tempos em segundos. A run dura 5 min: o chefão é spawnado pela engine automaticamente aos **5:00** (`CRONOGRAMA_DURACAO_SEG`) e todos os outros eventos param — não precisa cadastrar o chefão. Mantenha o último `tempo_fim` em `5.0f * 60.0f` (300s).

```c
{ 240.0f, 5.0f * 60.0f, INIMIGO_ELITE_DISTANCIA, 12.0f, 0.0f, false },
```

### Mexer numa magia existente

Em `magias_tipos.c`, ache a linha do elemento (ordem do enum) e ajuste dano, cooldown ou cor. Pra mudar o jeito que o jogador atira (ex.: spread em leque), edite a função `magias_tipos_processar_auto_fire`.

### Adicionar uma IA inédita

1. Em `tipos.h`, novo valor no enum `ComportamentoIA`.
2. Em `inimigos_tipos.c`, crie uma função `static void ia_minha_ia(Inimigo *i, EstadoJogo *ej)` que escreve em `i->velocidade`.
3. Adicione um `case` em `inimigos_tipos_executar_ia`.

A função recebe `EstadoJogo *ej`, então tem acesso à lista inteira de inimigos via `ej->inimigos_cabeca` — isso permite **IAs coordenadas** (cada inimigo decide olhando o que os outros estão fazendo). Exemplo já no projeto: `ia_kiter` itera a lista pra contar quantos kiters vivos existem e descobrir seu índice ordenado por ângulo polar, daí cada um ocupa um slot único num círculo orbital cercando o jogador. Outras coordenações possíveis: cargas em V, parede defendendo o boss, divisão de flancos entre dois grupos. Custo: O(N²) por frame se cada inimigo iterar a lista — aceitável até umas dezenas de inimigos.

Pra IAs **isoladas** com leve variedade entre indivíduos sem precisar de campo `id`: use `hash_pointer_para_unitario(i)` (já em `inimigos_tipos.c`) — devolve um valor estável em `[-1, 1]` derivado do endereço do nó, que serve de "personalidade" do inimigo (offset angular, fase de timer, jitter de velocidade).

## Convenções de código C

### Header (`.h`) é o "cartão de visita" do módulo

Cada módulo tem um par `nome.c` + `nome.h`:

- **`.h`** — DECLARA o que o módulo expõe pra fora: protótipos das funções públicas, structs, enums, constantes. Nada de implementação aqui.
- **`.c`** — IMPLEMENTA o que o `.h` declarou. É onde o código de verdade vive.

Por que separar?

- Outros arquivos só precisam incluir o `.h` pra usar as funções — sem ver os detalhes da implementação.
- O compilador valida o tipo dos argumentos na hora da chamada, evitando bugs.
- Recompila mais rápido: mudar só o `.c` não obriga recompilar quem usa o módulo.

### `#include`-guard (`#ifndef`/`#define`/`#endif`)

Todo header começa com este padrão:

```c
#ifndef NOME_H
#define NOME_H

/* ... conteúdo do header ... */

#endif /* NOME_H */
```

Garante que o conteúdo do header só seja lido **uma única vez** pelo compilador, mesmo que vários arquivos o incluam (direta ou indiretamente). Sem isso, daria erro de "redefinição" de tipos.

### `EstadoJogo *ej` em todo lugar

A struct raiz `EstadoJogo` (definida em `tipos.h`) carrega TODO o estado da run: jogador, listas de inimigos/magias/projéteis de inimigo, profecia, motor de profecia, cronograma, etc. Quase toda função do projeto recebe um `EstadoJogo *ej` por parâmetro. Vantagens:

- Zero variáveis globais espalhadas pelo código.
- Qualquer função vê o contexto inteiro.
- Fica óbvio quem depende de quê (basta olhar o `ej->...` no corpo da função).

## Menu e fluxo de telas

O jogo começa no menu principal. Cada item leva a um caminho:

```text
MENU ┬─ Novo Jogo ─────────► REVELACAO ─► ESCOLHA_MAGIA ─► COMBATE ─┬─► CARTAS_UPGRADE ─► COMBATE ...
     │                                                               ├─► GAME_OVER ──────► MENU
     │                                                               └─► VITORIA ────────► MENU
     ├─ Carregar Jogo ─────► REVELACAO  (replay da ultima seed jogada)
     ├─ Inserir Seed ──────► INSERIR_SEED ─► REVELACAO  (com a seed digitada)
     ├─ Historico  ────────► HISTORICO ───► REVELACAO  (recarrega seed da lista)
     ├─ Leaderboard ───────► LEADERBOARD ──► MENU
     ├─ Opcoes ────────────► OPCOES ───────► MENU
     └─ Sair
```

- **Novo Jogo:** sorteia uma seed aleatória.
- **Carregar Jogo:** só aparece se já houve pelo menos uma run; reutiliza a última seed.
- **Inserir Seed:** input numérico (0 a 4.294.967.295). Mesma seed → mesma profecia → mesmo mapa.
- **Histórico:** só aparece se já houve pelo menos uma run. Lista as últimas 10 seeds jogadas com resultado (V/D), tempo e biomassa. ENTER recarrega a selecionada.
- **REVELACAO:** mostra os 3 mods da profecia com magnitude explícita; ESPAÇO/ENTER avança.
- **ESCOLHA_MAGIA:** 3 cards de magia sorteados; pelo menos 1 é sinergético com a profecia. ←/→ ou 1/2/3 escolhem; ENTER confirma.
- **Leaderboard:** TAB alterna entre tabela "Tempo" (só vitórias) e "Biomassa" (todas as runs).
- **Opções:** lista de resoluções + toggle de fullscreen. As mudanças se aplicam na hora e persistem no save.

## Controles

### No menu e nas telas
| Tecla | Ação |
|-------|------|
| ↑ / ↓ ou W / S | Navegar entre itens (menu, Opções, Histórico) |
| ← / → ou A / D | Navegar entre cards (Escolha de Magia Inicial) |
| 1 / 2 / 3 | (Escolha de Magia) salta direto pro card |
| ENTER ou ESPAÇO | Confirmar / Selecionar |
| TAB | (Leaderboard) alterna entre Tempo e Biomassa |
| BACKSPACE | (Inserir Seed) apagar último dígito |
| ESC | Voltar pra tela anterior |

### Durante o combate
| Tecla | Ação |
|-------|------|
| WASD ou setinhas | Mover jogador |
| Q | Liga/desliga tiros automáticos |
| ESC | Pausar / Retomar |
| F1 | Alternar modo debug |
| **F3** | **Pular pra próxima tela de cartas (skip de tempo)** |
| ENTER | (Pausa / Game Over / Vitória) confirmar / voltar ao menu |
| 1 / 2 / 3 | (Cartas de Upgrade) escolher carta |
| R + 1/2/3 | (Cartas de Upgrade) rolar dado na carta |

## Conceitos obrigatórios de PIF implementados

| Conceito | Onde |
|----------|------|
| Structs | `tipos.h` — `Jogador`, `Inimigo`, `Magia`, `ProjetilInimigo`, `Profecia`, `MotorProfecia`, `Cronograma`, `EstadoJogo`, `DadosSalvos`, `EntradaLeaderboard`, `EntradaHistorico`, `OpcaoMagiaInicial`, `Carta`, `Dado`, `Obstaculo`, `EventoCronograma`, `ParametrosInimigo`, `ParametrosMagia` |
| Ponteiros | `EstadoJogo *ej` em quase toda função; `proxima`/`proximo` nos nós das listas; ponteiro duplo `InimigoNo **` na remoção de mortos em `inimigos.c` |
| Alocação dinâmica | `malloc`/`free` em `MagiaNo` (`magias.c`), `InimigoNo` (`inimigos.c`) e `ProjetilInimigoNo` (`projeteis_inimigo.c`); `free` ao morrer o nó e `_liberar_tudo` ao encerrar a run |
| Listas encadeadas | `MagiaNo` (projéteis do jogador), `InimigoNo` (inimigos) e `ProjetilInimigoNo` (tiro de inimigo) — inserção na cabeça em O(1), remoção com ponteiro duplo |
| Matrizes | `mods[3]` (profecia), `escolhas_upgrade[CARTAS_POR_ESCOLHA]`, `opcoes_magia[3]` (escolha de magia inicial), `obstaculos[MAX_OBSTACULOS]`, `dados_ativos[MAX_DADOS_JOGADOR]`, `eventos[MAX_EVENTOS_CRONOGRAMA]`, `top_tempo[LEADERBOARD_TAM]` e `top_biomassa[LEADERBOARD_TAM]` (leaderboards), `historico[HISTORICO_SEEDS_TAM]` (seeds passadas); tabelas `PARAMETROS_INIMIGO[]`, `PARAMETROS_MAGIA[]`, `COMPORTAMENTO_MAGIA[]`, `PARAMETROS_PROJETIL_INIMIGO[]`, `EVENTOS_CRONOGRAMA[]`, `RESOLUCOES_DISPONIVEIS[]`, `NOMES_MAGIA[ELEMENTO_TOTAL][6]` (matriz de nomes por elemento × raridade) |
| Arquivo | `salvamento.c` (`saves/biomassa.dat`) — **cumprido.** Persiste a `DadosSalvos` inteira em binário via `fwrite`/`fread`, incluindo config de vídeo, última seed jogada (Carregar Jogo), os dois leaderboards top-10 e o histórico das últimas 10 seeds. Validação de versão (`SAVE_VERSAO_ATUAL`) descarta saves antigos automaticamente. |

> **Sobre a CLI-LIB.** O spec do PIF cita a [`cli-lib`](https://github.com/tgfb/cli-lib/) como biblioteca padrão sugerida (terminal-based). AUGUR optou por **Raylib** — também permitido pelo enunciado: "É permitido usar outra biblioteca para jogos, entretanto seu uso é de responsabilidade do grupo". A justificativa é técnica: bullet hell precisa renderizar dezenas de projéteis com colisão circular precisa, e câmera 2D rolando suavemente pelo mundo — coisas que ficam pesadas no terminal.
