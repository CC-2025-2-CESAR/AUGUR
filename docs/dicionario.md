# Dicionário do AUGUR

Este documento existe pra desbloquear o vocabulário de jogos e de arquitetura de software que aparece o tempo todo no código, no README e nas conversas do grupo. Se você nunca jogou videogame ou nunca trabalhou com C, muitos desses termos parecem siglas aleatórias — eles têm significado bem específico. Aqui ficam todos juntos, em português claro, com o lugar do projeto onde cada um aparece.

> **Como usar:** abra esse arquivo lado a lado com o que você estiver lendo. Quando bater num termo estranho, dá `Ctrl+F` aqui pra achar a definição. Adicione termos novos conforme aparecerem.

---

## Sumário

- [Gêneros e estrutura macro do jogo](#gêneros-e-estrutura-macro-do-jogo)
- [Combate, IA e entidades](#combate-ia-e-entidades)
- [HUD e interface](#hud-e-interface)
- [Engine e renderização](#engine-e-renderização)
- [Procedural, física e geometria](#procedural-física-e-geometria)
- [Arquitetura e organização do código](#arquitetura-e-organização-do-código)

---

## Gêneros e estrutura macro do jogo

### Bullet Hell
**Tradução literal:** "inferno de balas". Subgênero de jogos de ação onde a tela fica cheia de projéteis inimigos e o jogador precisa desviar deles enquanto ataca de volta. O foco é em sobreviver à *quantidade* de coisas voando, não em mirar com precisão.

**No projeto:** AUGUR é um bullet hell — o jogador anda com WASD/setinhas esquivando, e as magias atacam automaticamente. Está descrito assim no `README.md` ("Bullet Hell Roguelite").

### Roguelite
Gênero em que cada partida (chamada **run**) começa do zero, é geralmente curta (10–25 minutos) e termina na morte do jogador ou na derrota do chefão final. A diferença pro "roguelike" puro é que entre runs você acumula uma progressão permanente — alguma coisa fica desbloqueada mesmo perdendo.

**No projeto:** AUGUR é um roguelite. Cada run gera uma profecia diferente (ver `profecia.c`). Na versão atual a parte "progressão permanente" está **fora de escopo**: a biomassa coletada matando inimigos é a **pontuação da run**, mostrada na tela de game over / vitória — não persiste entre runs.

### Run
Uma **partida individual** do jogo, do começo ao game over (ou vitória). Não confunda com "rodar o programa". Um jogador pode iniciar várias runs em uma mesma sessão de jogo.

**No projeto:** uma run dura no máximo 5 minutos. Vai do `ESTADO_MENU` (apertar ENTER) até `ESTADO_GAME_OVER` (morreu) ou `ESTADO_VITORIA` (derrotou o chefão aos 5:00).

### Wave / Onda
Um **grupo de inimigos** que aparecem no jogo durante um período. Termo clássico: nas ondas 1, 2, 3..., cada onda é mais difícil que a anterior. Quando todos morrem ou um timer acaba, geralmente abre uma tela de upgrade.

**No projeto:** AUGUR **não usa ondas finitas** mais. Foi substituído pelo conceito de **timeline / cronograma** (ver abaixo) — uma única run contínua de 5 minutos com fases sobrepostas. O termo "wave" ainda aparece em conversa como sinônimo de "fase intensa", mas no código a struct `Onda` não existe mais.

### Timeline / Cronograma
A **linha do tempo da run inteira**, em vez de ondas separadas. Estilo Vampire Survivors: o tempo conta de 0:00 a 5:00, e em momentos específicos novos tipos de inimigos começam a aparecer (ranged em 0:40, elites em 1:40, …).

**No projeto:** struct `Cronograma` em `tipos.h`, engine em `cronograma.c` e tabela declarativa de eventos em `cronograma_eventos.c`. Aos 5:00 a engine spawna o chefão final automaticamente.

### Meta-progressão
Progresso permanente do jogador que **persiste entre runs**, mesmo quando ele morre. É o que faz o roguelite ser viciante: cada morte ainda dá algum ganho.

**No projeto:** **fora de escopo na versão atual.** A biomassa existe (`Jogador.biomassa`) mas é usada como **pontuação da run**, exibida no game over / vitória — não persiste entre runs. `DadosSalvos` e `salvamento.c` continuam como stub dormente da Sofia; quando a meta-progressão voltar ao escopo, a moeda persistente entra aqui.

> **Vocabulário, não poder.** O design original do AUGUR é deliberado: a meta-progressão **não deixaria o jogador mais forte diretamente** — desbloquearia *opções* (novos dados, magias, mutações passivas, dicas de Profecia). Mantido aqui como referência de design do GDD; **não implementado** na versão atual.

---

## Combate, IA e entidades

### AoE (Area of Effect)
Ataque de **área** — afeta todo mundo dentro de um círculo/região, em vez de um alvo só. Útil contra muitos inimigos juntos.

**No projeto:** o efeito `EF_EXPLOSAO` em `tipos.h` é AoE. As tabelas em `profecia.c` chamam de "Explosao em area".

### Build
Em jogos com customização (RPGs, roguelites), uma **build** é a combinação específica de magias, upgrades e atributos que o jogador montou pra aquela run. Não é uma escolha única — é o pacote inteiro. "Build de fogo crítico", "build de tankar dano e refletir" etc.

**No projeto:** cada run AUGUR vira uma build emergente: a Profecia define o que sinergiza com o quê, e as cartas escolhidas a cada minuto cheio constroem o pacote final. Como a profecia é diferente toda run (`profecia.c`), a build ótima também muda — e é justamente isso que o GDD chama de "não existe meta fixo".

### Auto-fire
**Atirar automaticamente**, sem o jogador apertar tecla de tiro. A magia dispara sozinha quando o cooldown zera, geralmente mirando no alvo mais próximo.

**No projeto:** implementado em `magias_tipos.c`, função `magias_tipos_processar_auto_fire`. A engine chama uma vez por frame; ela checa o timer interno por elemento e cria projéteis quando bate o intervalo. Pode ser ligado/desligado pelo jogador com Q (ver Toggle).

### Boss / Chefão
**Inimigo gigante e excepcional**, geralmente no fim de uma fase. Tem muito mais HP, dano e às vezes fases por % de vida (muda de comportamento ao perder HP).

**No projeto:** `INIMIGO_CHEFE` no enum `TipoInimigo`. A engine spawna automaticamente aos 5:00 da timeline depois de **limpar a arena** (libera todos os inimigos vivos pra criar atmosfera de boss fight). Usa a IA `IA_BOSS_FASES` com 3 fases: acima de 50% velocidade base; abaixo de 50% acelera 1.25×; abaixo de 25% acelera 1.5× **e** spawna 1 minion (corpo a corpo) a cada 5s perto dele.

### Buff / Debuff
**Buff** = efeito *positivo* temporário (mais dano, mais velocidade, escudo). **Debuff** = efeito *negativo* (lento, queimando, com menos defesa).

**No projeto:** em `tipos.h` os efeitos `EF_VELOCIDADE_MAIS` e `EF_DANO_MAIS` são buffs no jogador. `EF_LENTIDAO` e `EF_VENENO` são debuffs em inimigos.

### Chase / IA Chase
Comportamento de inimigo mais simples possível: **andar em linha reta na direção do jogador**. Sem desvio, sem estratégia. Igual ao que você vê em Pac-Man com os fantasmas.

**No projeto:** valor `IA_CHASE` no enum `ComportamentoIA`. Função `ia_chase` em `inimigos_tipos.c`. Tem duas perturbações pra evitar fila indiana e linha reta:
1. **Offset angular fixo** por inimigo (~±23°, via hash do ponteiro): cada melee vem por um flanco persistente — uns pela direita, outros pela esquerda.
2. **Tangencial progressivo**: longe vem direto; abaixo de 220px começa a circular, com intensidade que cresce conforme se aproxima. Lado vem do mesmo hash, então é coerente com o flanco fixo. Efeito: na hora do encontro o melee "dança" um pouco antes de bater, dando frame extra pro jogador esquivar.

Inimigos do tipo `INIMIGO_CORPO_A_CORPO` e `INIMIGO_ELITE` usam essa IA.

### Kite / Kiter / Kiting
Comportamento oposto ao chase: o inimigo **mantém distância** do jogador. Se o jogador se aproxima, ele recua; se afasta demais, ele aproxima até voltar pra distância "ideal". Geralmente atira de longe. O termo vem de "kiting" em MMOs (segurar o inimigo "preso na linha do papagaio").

**No projeto:** valor `IA_KITER` no enum `ComportamentoIA`. Função `ia_kiter` em `inimigos_tipos.c`. **IA isolada com personalidade individual:** cada kiter combina dois componentes — um **radial** que mantém a distância ideal (240px ± 30) e um **tangencial fixo** (orbita um lado ou outro, escolhido por hash do ponteiro estável durante a vida do nó). Sem coordenação entre kiters: a formação emerge naturalmente porque metade orbita num sentido e metade no outro, espalhados em volta do jogador. `INIMIGO_A_DISTANCIA` é um kiter típico.

> Versão anterior tentou coordenação explícita (slots por ordenação angular num círculo orbital comum) mas ficou visualmente artificial — carrossel mecânico, raio crescendo ao spawnar mais um, slots saltando quando algum morria. A versão isolada é mais orgânica.

### Elite
**Versão buffada** de um inimigo comum: mesma "ideia" geral mas com mais HP, mais dano, mais velocidade e dropa mais recompensa. Tipicamente roxo/dourado pra destacar.

**No projeto:** `INIMIGO_ELITE` em `TipoInimigo`. Stats em `inimigos_tipos.c` na tabela `PARAMETROS_INIMIGO[]` (cor roxa; números são tunáveis pela Luísa). Na timeline de 5 min (em `cronograma_eventos.c`) os elites só começam a aparecer a partir de ~1:40 da run.

### Feiticeiro / Wizard
Termo do GDD pro jogador. AUGUR é mundo de magia — o protagonista não é um soldado, é um conjurador.

**No projeto:** a struct `Jogador` em `tipos.h` é o feiticeiro. Mecanicamente igual a qualquer "player" de bullet hell: posição, velocidade, raio, vida, `bonus_dano` (somado no dano de toda magia ao disparar). As **mutações passivas** da meta-progressão são referência do GDD; **fora de escopo na versão atual**.

### Formação / Cerco / Surround
Quando vários inimigos **se distribuem em volta do jogador** em vez de virem todos do mesmo lado. Fechando ângulos de fuga, viram pressão real: você não tem um "lado seguro" pra recuar. Em jogos com muitos inimigos é o que diferencia "burros me seguindo" de "inteligentes me cercando".

**No projeto:** os kiters (`IA_KITER`) formam o cerco **emergentemente**, sem coordenação explícita: cada kiter tem um **lado de orbita persistente** (horário ou anti-horário) escolhido por hash do ponteiro. Como o hash divide ~50/50, o resultado natural é que metade do grupo dança no flanco direito e metade no esquerdo. O push-out entre inimigos espalha os que orbitam pelo mesmo lado. Implementação em `ia_kiter` em `inimigos_tipos.c`.

### Grimório
Termo da fantasia: **livro de magias** do feiticeiro. Em jogos, costuma ser usado pro pool de magias disponíveis pro jogador.

**No projeto:** o "Grimório de Magias" é uma das categorias de meta-progressão do GDD — **fora de escopo na versão atual**. Hoje os elementos que o jogador dispara vêm direto dos 3 mods da Profecia (`magias_tipos.c` / `magias_comportamento.c`), sem desbloqueio persistente.

### Hit
Acerto. Tanto "o projétil acertou o inimigo" quanto "o inimigo me acertou" são "hits".

**No projeto:** as condições da profecia (enum `Condicao` em `tipos.h`, reorganizado conforme o GDD) usam isso: `COND_AO_ACERTAR` (toda magia que acerta), `COND_PRIMEIRA_HIT` (a 1ª da run), `COND_AO_MATAR` (inimigo morreu) e `COND_AO_RECEBER_DANO` (jogador tomou hit). O motor que dispara os efeitos fica em `profecia.c`.

### Combo
Sequência de acertos/abates dentro de uma janela de tempo curta. Geralmente recompensa o jogador (mais dano, gatilho de efeito).

**No projeto:** a condição `COND_EM_COMBO` em `tipos.h` dispara o efeito da profecia quando o jogador atinge o limiar de kills dentro da janela de combo. Limiar e janela são tunáveis pela Luísa em `profecia_efeitos.c` (`LIMIARES_CONDICAO`).

### Combo emergente
Interação entre dois sistemas (geralmente magias) que **cria um efeito não documentado** — o jogador descobre por experimentação. Não é uma feature listada em tutorial; é uma consequência natural das regras se combinarem. Quando o sistema permite combos emergentes, o jogador "inventa" estratégias e isso vira parte da identidade do jogo.

**No projeto:** **implementados** na engine (`combos.c`), com números tunáveis pela Luísa em `profecia_efeitos.c` (`MAGNITUDES_EFEITO`): **Choque Térmico** (Fogo depois Gelo no mesmo inimigo → stun + a próxima hit nele é amplificada) e **Arcano × envenenado** (adaptação do "Corrente Arcana + Nuvem Venenosa" ao modelo projétil+status: Arcano causa dano dobrado em inimigo sob efeito de veneno). A detecção fica em `colisao.c`, no ponto do acerto.

### Cooldown
**Tempo de espera** entre dois usos de uma habilidade. Aperto a magia → ela dispara → preciso esperar X segundos pra usar de novo.

**No projeto:** em `magias_tipos.c`, o campo `intervalo_disparo` da tabela `PARAMETROS_MAGIA` é o cooldown do auto-fire por elemento (ex.: relâmpago dispara a cada 0.30s, sombra a cada 0.65s).

### Dado / Roll / Risco gerenciado
**Dado** (no AUGUR) é um recurso ativo e limitado que o jogador escolhe gastar pra **rolar** (rerollar) o valor de uma carta de upgrade. O resultado pode ser melhor, igual ou pior que o valor base — não é RNG passivo, é **risco gerenciado**: você está aceitando uma faixa de resultados, não um número fixo.

Tipos planejados (GDD): `d6 Comum` (1–6 uniforme, inicial), `d6 Viciado` (sem 1, 2 ou 6 — seguro), `d4` (1–4, baixa variância), `d8` (1–8, alta variância), `d6 Maldito` (extremos mais frequentes), `d6 Profético` (mostra o resultado antes de confirmar).

**No projeto:** struct `Dado` em `tipos.h` (campos `faces`, `ultimo_resultado`). O array `EstadoJogo.dados_ativos[MAX_DADOS_JOGADOR]` guarda os 2 dados que o jogador trouxe pra run. Implementação completa em `dados.c` é responsabilidade da Sofia.

### Dash
**Investida rápida** numa direção, geralmente curta e com cooldown. Em jogos top-down funciona como esquiva ofensiva.

**No projeto:** a condição `COND_NO_DASH` em `tipos.h` está reservada pra quando essa mecânica for adicionada — ainda não tem dash implementado.

### DoT (Damage over Time)
Dano **contínuo ao longo do tempo**, não num pulso só. Ex.: queimadura que tira 5 HP por segundo durante 4 segundos.

**No projeto:** `EF_VENENO` em `tipos.h` é um DoT. O comentário no enum literalmente diz `/* DoT */`.

### Hitbox
Área **invisível** que define onde uma entidade pode ser atingida. Em jogos 2D simples, costuma ser um círculo ou retângulo.

**No projeto:** o `raio` em `Jogador`, `Inimigo` e `Magia` (em `tipos.h`) é a hitbox circular. A colisão em `colisao.c` usa esses raios pra decidir se duas coisas se tocaram.

### Horda
**Grupo grande de inimigos vindo ao mesmo tempo.** Em bullet hell e roguelite, hordas crescem em densidade conforme a run avança — começa com poucos, termina com a tela cheia.

**No projeto:** a `cronograma_eventos.c` modela hordas de forma declarativa. Cada linha da tabela diz "do tempo X ao tempo Y, spawnar tipo T a cada Z segundos". Sobreposição de eventos = horda mais densa (na timeline de 5 min: por volta de 2:40 o melee acelera pra 0.8s enquanto elites já spawnam, criando pressão simultânea). Os tempos são tunáveis pela Luísa.

### HP (Health Points)
**Pontos de vida.** Quando chega a zero, a entidade morre.

**No projeto:** campos `vida` e `vida_maxima` em `Jogador` e `Inimigo` (`tipos.h`). A barra colorida no canto da tela é a representação visual do HP do jogador (ver `hud.c`).

### Ignite (Igníção / Queimar)
**Status de fogo** que aplica DoT (ver acima) — o alvo "pega fogo" e perde HP por alguns segundos. Muitas vezes pode propagar ou empilhar.

**No projeto:** mencionado nos comentários do GDD como possível efeito do dado em `magias.c::sandbox` ("d6 saiu 6 = +30 dano + ignite passivo").

### Knockback
**Empurrão** que um ataque dá no alvo. Ataque com knockback pra fora afasta o inimigo; sem knockback ele "fica colado" em você.

**No projeto:** não implementado. O contato jogador-inimigo em `colisao.c` aplica dano puro + push-out físico (o inimigo bloqueia o jogador), sem knockback.

### Mutação passiva
Bônus permanente do **feiticeiro** que vale em todas as runs, independente da Profecia. Ao contrário de uma carta de upgrade (que vive só na run atual), uma mutação fica desbloqueada pra sempre depois que você gasta biomassa nela.

**No projeto:** **fora de escopo na versão atual** (depende de meta-progressão/save persistente, que não está no escopo agora). Mantido como referência de design do GDD.

### Minion / Reforço
Inimigo **secundário invocado por outro inimigo** (geralmente um chefão ou elite). Aparece em fases finais pra aumentar pressão — em vez de só lidar com o boss, você precisa lidar com o boss + os bichinhos que ele chama.

**No projeto:** o chefão (`IA_BOSS_FASES`) spawna 1 corpo a corpo a cada 5s **abaixo de 25% de vida**. Implementação em `ia_boss_fases` em `inimigos_tipos.c`, com timer interno em `static float`.

### Perturbação angular / Variação angular
Pequeno **deslocamento aleatório** aplicado à direção que um inimigo segue. Sem isso, todos viajam exatamente em linha reta até o jogador e formam fila indiana. Com uma perturbação fixa por inimigo (~±20°), cada um aproxima por um flanco diferente, dispersando naturalmente.

**No projeto:** a `ia_chase` em `inimigos_tipos.c` aplica um offset angular de até ±0.4 rad (~23°) na direção até o jogador. O valor é estável durante a vida do inimigo porque vem de um hash do endereço do nó (ver "Hash de ponteiro / Pseudo-id").

### Piercing (Penetrante)
Atributo de projétil que **atravessa o primeiro alvo** em vez de sumir. Útil pra hit em fila.

**No projeto:** o comentário em `colisao.c` cita explicitamente: o `break` foi omitido na colisão magia-vs-inimigo "pra deixar brecha pra piercing shots".

### Projétil
Qualquer coisa que **voa pela tela** atacando — bala, flecha, bola de fogo. Em código, costuma ter posição, velocidade e tempo de vida.

**No projeto:** struct `Magia` em `tipos.h` é um projétil. A lista encadeada `magias_cabeca` em `EstadoJogo` guarda todos os projéteis ativos.

### Spawn / Spawnar
Verbo: **fazer aparecer** uma entidade no mundo (inimigo, projétil, item). Substantivo: o local/momento em que ela aparece.

**No projeto:** quem decide *quando* spawnar é o cronograma (`cronograma.c`), que lê a tabela em `cronograma_eventos.c` e chama `inimigos_spawnar_em()` (engine, em `inimigos.c`) na hora certa, em uma posição aleatória na borda da tela do jogador.

### Stun (Atordoamento)
Status que **impede o alvo de agir** por um curto período (não anda, não ataca). Bom contra inimigos perigosos.

**No projeto:** ainda não tem stun na implementação. O GDD menciona "Choque Térmico" como combo Bola de Fogo + Nova de Gelo: stun de 2s.

### Tick
Cada **iteração regular** de um efeito que tem duração. Um veneno de "5 dano por segundo durante 4s" tem 4 ticks. Também é usado como sinônimo de "frame" em alguns contextos.

**No projeto:** a condição `COND_A_CADA_5S` em `tipos.h` é um tick a cada 5 segundos.

### Zona morta (Deadzone)
**Faixa de valores onde o sistema ignora a entrada** ou para de agir. Em IA, é uma região perto do alvo onde o inimigo simplesmente para em vez de tentar correções minúsculas (que só causariam jitter). Em controles físicos (joystick), é o intervalo perto do centro onde pequenos desvios não contam, evitando movimento parasita.

**No projeto:** a `ia_kiter` em `inimigos_tipos.c` para o inimigo se a distância até o slot do círculo for menor que 6 pixels — sem isso, o kiter ficaria oscilando 1px pra lá e pra cá quando perto demais do alvo.

---

## HUD e interface

### HUD (Heads-Up Display)
Camada de **informação fixa na tela**, sobreposta ao jogo: barra de vida, contador de munição, mapinha. Geralmente nos cantos.

**No projeto:** módulo `hud.c` da Sofia. Mostra barra de vida no canto superior esquerdo, tempo da run no topo central (`MM:SS / 05:00`), e moedinha de biomassa (pontuação da run) abaixo da barra.

### Indicador (de toggle)
Pequeno texto/ícone **mostrando o estado de uma opção** que o jogador pode mudar. Aparece pra dar feedback de que a tecla teve efeito.

**No projeto:** `[Q] Tiros: ON` (verde) ou `[Q] Tiros: OFF` (vermelho) no canto inferior esquerdo durante o combate. Lê `ej->tiros_ativos`.

### Overlay
Algo desenhado **por cima** do que já está na tela. Tipicamente semi-transparente — você ainda enxerga o jogo atrás.

**No projeto:** o overlay de pausa escurece a tela com um retângulo preto a cerca de 60% de opacidade pra dar contraste com o texto "PAUSA" por cima. Também usado pelo modo debug do F1.

### Pausa (Pause)
Estado em que o **mundo congela**: tempo não avança, inimigos não se movem, projéteis param. Geralmente abre um menu por cima.

**No projeto:** `ESTADO_PAUSA` na máquina de estados. Acionado por ESC durante o combate. Tecnicamente o `switch` em `jogo_atualizar` simplesmente não chama `cronograma_atualizar`, `magias_atualizar` etc. quando o estado é `ESTADO_PAUSA` — é assim que tudo congela. ESC retoma; ENTER volta ao menu.

### Polish
Ajustes finais que **fazem o jogo "sentir bem"**: timing certo das animações, partículas, screenshake, balanceamento de números, feedback sonoro. Não muda o jogo no nível de regras — muda como o jogo "soa". É a fase final do desenvolvimento, depois que tudo já funciona mecanicamente.

**No projeto:** a semana 5 do cronograma do GDD (26/05–31/05) é "Polish & Entrega" — screenshake em explosões, partículas de morte, balanceamento de dificuldade, gravação do vídeo. Hoje (07/05) ainda estamos em fase de feature; polish vem depois.

### Score
**Pontuação.** Em roguelites geralmente é "até onde você chegou" + abates + tempo.

**No projeto:** o GDD prevê **top 10 scores em arquivo** (`scores.dat`) com seed visível, pra o jogador poder reproduzir runs interessantes. Implementação em `salvamento.c` é responsabilidade da Sofia. O campo `DadosSalvos.melhor_onda` em `tipos.h` é placeholder; o nome ficou de quando ainda existia o conceito de "onda" — vai virar `melhor_tempo` ou similar quando o módulo for refeito.

### Screenshake
Efeito de **tremer a câmera** durante eventos impactantes (explosão, hit forte, morte). Dá peso ao impacto sem precisar de dano numérico maior.

**No projeto:** ainda não implementado. Listado pro polish (semana 5 do GDD) — partículas de morte e screenshake em explosões. Implementação típica: durante alguns frames depois do evento, somar um pequeno offset aleatório a `camera.target` antes de renderizar.

### Toggle
**Botão liga/desliga.** Apertar uma vez liga, apertar de novo desliga. Diferente de um botão que só dispara enquanto pressionado.

**No projeto:** Q é um toggle dos tiros automáticos. A lógica em `main.c` é literalmente `ej->tiros_ativos = !ej->tiros_ativos` quando Q é apertado. F1 é outro toggle (modo debug).

---

## Engine e renderização

### atan2 / Ângulo polar
**`atan2(y, x)`** é uma função matemática (em `<math.h>`) que devolve o **ângulo em radianos** correspondente ao vetor `(x, y)`, no intervalo `(-π, π]`. Diferente de `atan(y/x)`, ela considera o sinal de cada componente — distingue, por exemplo, segundo de quarto quadrante.

**Ângulo polar** é o ângulo de um ponto em relação a uma origem (geralmente o jogador). Útil pra organizar inimigos em volta dele de forma coerente: quem está em "30°" fica à direita-em-baixo, quem está em "180°" fica à esquerda, etc.

**No projeto:** a `ia_kiter` em `inimigos_tipos.c` usa `atan2f` pra calcular o ângulo polar de cada kiter ao redor do jogador, e ordena os kiters por esse ângulo pra atribuir slots numa formação circular (ver "Formação / Cerco").

### BeginMode2D / EndMode2D
Funções do Raylib que **abrem e fecham um bloco com câmera 2D ativa**. Tudo desenhado entre os dois é interpretado em **coordenadas de mundo** (a câmera aplica offset/zoom). Fora do bloco, tudo é coord de tela.

**No projeto:** em `main.c`, o bloco `BeginMode2D(ej->camera) ... EndMode2D()` engloba o desenho do grid, jogador, inimigos, magias e obstáculos. O HUD vem **depois** do `EndMode2D` porque deve ficar fixo na tela.

### CLI / Linha de comando
**Command Line Interface.** Programa que roda no terminal e desenha "gráficos" usando caracteres (ASCII art) na grade do terminal, em vez de uma janela gráfica com pixels.

**No projeto:** o spec do PIF cita a **CLI-LIB** (`github.com/tgfb/cli-lib/`) como biblioteca padrão sugerida — ela cuida de manipulação de tela, teclado e timer no terminal. AUGUR optou por **Raylib** (também permitido pelo spec: "É permitido usar outra biblioteca para jogos, entretanto seu uso é de responsabilidade do grupo"). Raylib oferece janela gráfica de verdade, melhor pra um bullet hell que precisa renderizar várias dezenas de projéteis e inimigos com colisão circular precisa.

### Camera2D
Estrutura do Raylib que **define um ponto de vista**. Tem `target` (o que ela aponta), `offset` (onde o target aparece na tela), `zoom` e `rotation`. A câmera segue o jogador atualizando `target = jogador.posicao` a cada frame.

**No projeto:** `EstadoJogo.camera` em `tipos.h`. Inicializada em `main.c::jogo_inicializar` com `offset` no centro da tela e `target` na posição do jogador.

### Coord de mundo × Coord de tela
- **Coord de mundo:** posição "real" da entidade no universo do jogo. Não muda quando a câmera anda. Ex.: o jogador está em (1500, -200) no mundo.
- **Coord de tela:** posição em pixels da janela. (0, 0) é o canto superior esquerdo da tela. Muda conforme você vê o mundo.

A câmera converte de uma pra outra. HUD e menus usam coord de tela. Jogador, inimigos, magias e obstáculos usam coord de mundo.

**No projeto:** o comentário longo em `tipos.h::EstadoJogo.camera` explica isso.

### Delta time / `delta_tempo`
**Tempo decorrido entre o frame atual e o frame anterior**, em segundos. Geralmente um número pequeno tipo `0.0166` (=1/60 a 60fps).

Por que importa: se você fizer `posicao.x += 5` por frame, a velocidade depende do FPS. Se fizer `posicao.x += velocidade * delta_tempo`, a velocidade fica constante mesmo se o FPS oscilar.

**No projeto:** `EstadoJogo.delta_tempo` em `tipos.h`, atualizado a cada frame por `GetFrameTime()` em `main.c::jogo_atualizar`. Usado em quase toda função `_atualizar`.

### Frame
**Um único quadro/imagem** desenhado na tela. O jogo atualiza estado e renderiza um frame, depois outro, depois outro... Igual aos quadros de um filme.

**No projeto:** o loop em `main.c` é o "loop de frames". Cada volta = 1 frame. `SetTargetFPS(60)` pede ao Raylib pra rodar a 60 frames por segundo.

### FPS (Frames Per Second)
**Quantos frames são desenhados por segundo.** Padrão pra jogos é 60. Acima disso é mais fluido; abaixo de 30 é desconfortável.

**No projeto:** `FPS_ALVO = 60` em `tipos.h`. F1 liga o overlay de debug que mostra o FPS atual com `DrawFPS()`.

### Top-down
Câmera vista **de cima pra baixo**, em ângulo fixo. Você enxerga o personagem como se fosse uma vista de mapa. Vampire Survivors, Diablo (em parte), Hotline Miami são top-down.

**No projeto:** AUGUR é top-down. Daí o jogador ser desenhado como um círculo simples e a câmera nunca girar.

### Viewport
A **área visível** da tela do jogo. Em AUGUR, é uma janela de 1280×720 pixels. O viewport é o "buraco" através do qual você vê o mundo (que é maior/infinito).

**No projeto:** `LARGURA_TELA` e `ALTURA_TELA` em `tipos.h` definem o viewport. A função `desenhar_grid_mundo` em `main.c` calcula o viewport visível em coordenadas de mundo pra desenhar só as linhas que estão na tela.

---

## Procedural, física e geometria

### Bioma
**Tema visual + comportamental** de um pedaço do mundo. Floresta, deserto, caverna são biomas. Em jogos procedurais, o bioma define cor de fundo, tipos de obstáculo possíveis, paleta dos inimigos.

**No projeto:** o GDD cita "bioma + obstáculos" como dado por seed. Hoje o jogo tem só um bioma genérico (grid escuro + obstáculos genéricos). Quando os obstáculos forem portados do sandbox, vão criar a sensação de bioma — árvores em densidade alta = floresta, pedras em densidade alta = caverna, etc.

### Big O / O(n²) / Complexidade
Notação matemática pra estimar **quanto tempo um algoritmo leva conforme o tamanho da entrada cresce**. `O(n)` = roda 1 vez pra cada item. `O(n²)` = pra cada item, roda outra passada de N itens (loop dentro de loop) — fica caro rápido. `O(1)` = tempo fixo, não depende do N.

Por que importa em jogo: se um sistema é O(n²) e você tem 100 inimigos, são 10.000 checagens por frame. Aceitável a 60fps. Se vira 1000 inimigos, vira 1.000.000 checagens — começa a engasgar.

**No projeto:** o push-out entre inimigos em `inimigos.c` é O(n²) — pra cada par de inimigos vivos checa se estão sobrepostos. Funciona bem porque `MAX_INIMIGOS = 128` (limite de segurança) mantém o pior caso ainda dentro do orçamento.

### Determinístico
Algo que produz **sempre o mesmo resultado** dado a mesma entrada. Oposto de aleatório de verdade.

**No projeto:** a profecia é determinística a partir da seed. Mesma seed → mesmas 3 regras geradas, sempre. Permite replay e debug.

### Hash de ponteiro / Pseudo-id
Truque pra dar **identidade estável** a um objeto sem precisar de um campo `id` numérico explícito: pega o endereço do ponteiro (`uintptr_t`) e passa por uma função de hash (multiplicação por uma constante grande tipo a constante de Knuth `2654435761`). O resultado é um número que parece aleatório mas é **determinístico durante a vida daquele objeto** — perfeito pra "personalidade" estável (offsets, fases, cores variantes) sem inflar a struct.

**No projeto:** `hash_pointer_para_unitario()` em `inimigos_tipos.c`. Usada em `ia_chase` pra gerar uma perturbação angular única por inimigo (ver "Perturbação angular"), evitando precisar adicionar campo `id` na struct `Inimigo`.

### Hitbox
Ver **Hitbox** em [Combate, IA e entidades](#combate-ia-e-entidades).

### Overlap
**Sobreposição.** Quando duas formas (círculos, caixas) ocupam parte do mesmo espaço. É o sinal de que houve colisão.

**No projeto:** `colisao.c` mede `overlap = soma_raios − distancia` quando o jogador encosta num inimigo. Esse valor é quanto eles estão "afundados" um no outro, e é exatamente quanto o jogador precisa ser empurrado pra fora.

### Procedural / proceduralmente
Conteúdo **gerado por algoritmo**, não desenhado à mão. Em vez de ter 100 mapas pré-feitos, o jogo escreve um mapa novo a cada partida usando regras + um número aleatório.

**No projeto:** profecia, layout de obstáculos e timeline de spawns são procedurais. Tudo a partir de uma seed.

### Push-out
Técnica de colisão: quando duas formas se sobrepõem, **empurra uma pra fora** pela quantidade exata do overlap, na direção do vetor entre os centros. Resultado: as formas ficam só se tocando, não dentro uma da outra.

**No projeto:**
- `colisao.c` faz push-out **assimétrico** do jogador contra inimigos (jogador é empurrado, inimigo fica parado).
- `inimigos.c` faz push-out **simétrico O(n²)** entre todos os pares de inimigos (cada um anda metade do overlap), evitando que se empilhem em cima do jogador.
- `obstaculos.c` (a portar) deve fazer push-out do jogador e dos inimigos contra árvores e pedras.

### Seed
**Número que alimenta um gerador procedural.** Mesma seed → mesma sequência de números aleatórios → mesmo conteúdo gerado. Tipo um "código de barras" da run.

**No projeto:** `unsigned int seed` em `Profecia` (`tipos.h`). O ENTER no menu sorteia uma seed nova com `rand()`, e ela é usada tanto pela `profecia_gerar` quanto pela `obstaculos_gerar`. A seed também é mostrada nas telas de game over e vitória pro jogador anotar.

---

## Arquitetura e organização do código

### Coordenação / IA coordenada vs IA isolada
Numa **IA isolada**, cada inimigo decide o que fazer olhando só o jogador, sem se importar com os outros inimigos. Resultado: comportamento "burro em massa" — todos cumprem a mesma regra, todos vão pro mesmo ponto, todos viram fila indiana.

Numa **IA coordenada**, cada inimigo lê (ou compartilha) o estado dos outros pra dividir tarefas. Custo: complexidade extra (precisa iterar a lista, pode ser O(N²)). Ganho: comportamento que parece inteligente — cercar, flanquear, formar parede.

**No projeto:** atualmente todas as IAs em `inimigos_tipos.c` são **isoladas** com personalidade individual. Cada inimigo deriva valores estáveis (offset angular, lado de orbita) de um hash do seu ponteiro, sem olhar os outros. A formação emerge do conjunto, não de coordenação explícita. Uma versão coordenada do kiter foi testada (slots num círculo orbital comum, ordenação angular) mas ficou visualmente mecânica e foi revertida. O push-out físico entre inimigos faz o trabalho que normalmente exigiria steering coordenado.

### Engine vs Conteúdo
**Engine** = o motor do jogo: alocação de memória, loops, render genérico, dispatch de comportamentos. Não fala "fogo dá 18 de dano" — fala "leia o dano da tabela e aplique".

**Conteúdo** = os números e regras específicas: stats por inimigo, parâmetros por magia, timeline de spawns. Não fala "como" alocar ou renderizar — fala "esse boss tem 1500 de HP".

A vantagem de separar: pra mudar balance ou criar tipo novo, você mexe só no conteúdo, sem risco de quebrar a engine. Pra otimizar render ou trocar lib, mexe só na engine, sem mudar números.

**No projeto:**
- **Engine (Arthur):** `inimigos.c`, `magias.c`, `cronograma.c`, `colisao.c`.
- **Conteúdo (Luísa):** `inimigos_tipos.c`, `magias_tipos.c`, `cronograma_eventos.c`.

### Data-driven design
Estilo de programação onde o **comportamento é controlado por dados em tabela** (estruturas, listas, arquivos), não por código duro. Adicionar uma feature vira "adicionar uma linha na tabela" em vez de "escrever uma nova função".

**No projeto:** todas as três tabelas dos arquivos `*_tipos.c` e `*_eventos.c` seguem esse padrão. A engine itera sobre arrays const e aplica os parâmetros.

### Tabela declarativa
Uma **lista de structs `const`** que descreve "o que existe" sem dizer "como funciona". O código que **lê** essa lista decide o como. Estilo de programação muito usado em jogos.

**No projeto:** `PARAMETROS_INIMIGO[]`, `PARAMETROS_MAGIA[]`, `EVENTOS_CRONOGRAMA[]`. Cada uma é um array de structs `const`.

### Dispatcher / Dispatch
Função que recebe um valor (geralmente um enum), olha qual é, e **chama a função apropriada**. Tipicamente um `switch` que delega.

```c
void dispatcher(Tipo t) {
    switch (t) {
        case TIPO_A: faz_a(); break;
        case TIPO_B: faz_b(); break;
        // ...
    }
}
```

A graça é que quem chama o dispatcher não precisa saber qual variante é qual — só passa o valor e segue a vida.

**No projeto:** `inimigos_tipos_executar_ia` é o dispatcher de IA. Olha `PARAMETROS_INIMIGO[i->tipo].comportamento` e chama `ia_chase`, `ia_kiter` ou `ia_boss_fases` conforme o caso. Adicionar uma IA inédita = adicionar um `case` aqui.

### Stub
Função que **existe mas ainda não foi implementada**. Tipicamente vazia ou só com `return 0`. Permite que o resto do código compile enquanto alguém ainda não terminou aquela parte.

**No projeto:** os arquivos `obstaculos.c`, `dados.c`, `salvamento.c` ainda têm stubs. As funções são declaradas no header e o `.c` implementa versões vazias com `(void)ej;` pra silenciar o warning de parâmetro não usado.

### Lookup table / Indexar
**Acessar uma tabela usando um índice direto**, sem loop. Ex.: `PARAMETROS_INIMIGO[INIMIGO_ELITE]` te devolve os stats do elite em O(1) — instantâneo.

**No projeto:** as tabelas `PARAMETROS_INIMIGO[]` e `PARAMETROS_MAGIA[]` são lookup tables. A ordem das linhas tem que casar com a ordem do enum correspondente — é o que faz o índice funcionar.

### Cap / Limite
Um **teto** pra impedir que algo cresça sem controle. Importante em jogo pra preservar performance mesmo no pior caso.

**No projeto:** `MAX_INIMIGOS = 128` e `MAX_PROJETEIS = 256` em `tipos.h`. As funções `inimigos_spawnar_em` e `magias_spawnar` checam o cap antes de alocar, evitando que a lista cresça infinitamente e trave a CPU.

### Máquina de estados / FSM (Finite State Machine)
Padrão onde o programa só pode estar em **um estado por vez** (menu, combate, pausa, ...) e existem regras claras pra trocar de um pro outro. Evita o spaghetti de "if dentro de if dentro de if".

**No projeto:** o enum `EstadoAtual` em `tipos.h` lista todos os estados (`ESTADO_MENU`, `ESTADO_COMBATE`, `ESTADO_PAUSA`, `ESTADO_VITORIA`, etc.). O `switch` em `jogo_atualizar` é o dispatcher da FSM. As transições são sempre via `ej->proximo_estado = ...`.

### Reset de estado / Reset de run
Devolver o estado do jogo a um **ponto consistente conhecido**. Em roguelite, é o que acontece quando você começa uma nova run depois do game over: jogador na origem com vida cheia, inimigos limpos, timers zerados, listas liberadas. Sem reset adequado, sobras da run anterior contaminam a nova run (memória vazada, posições estranhas, vida negativa).

**No projeto:** `jogo_resetar_run()` em `main.c`. Chamada no início de toda revelação de profecia (ponto único de "começou uma run nova"). Libera as listas encadeadas via `magias_liberar_tudo` / `inimigos_liberar_tudo`, chama `jogador_inicializar` pra resetar posição/vida, e reposiciona a câmera. **Não** mexe em meta-progressão (`salvamento`) nem em `tempo_total` (que é debug-only).

### Variável estática (`static` dentro de função)
Variável local declarada com `static` dentro de uma função. Diferente de uma local normal, ela **mantém o valor entre chamadas**: a função sai, volta a ser chamada e a variável continua de onde parou. É memória compartilhada por todas as invocações daquela função — útil pra acumuladores e timers internos quando você não quer adicionar campo na struct.

**Cuidado:** se a função puder ser chamada por múltiplos "donos" simultaneamente (várias instâncias), a variável vira recurso compartilhado e pode dar bug.

**No projeto:** `ia_boss_fases` em `inimigos_tipos.c` usa `static float timer_minions` pra acumular o tempo entre spawns de minions na fase final do chefão. Ok porque a engine garante no máximo 1 chefão por run (cronograma só spawna 1 e desativa todos os outros eventos quando ele entra).

### Lista encadeada (linked list)
Estrutura de dados onde cada **nó tem um ponteiro pro próximo**. Diferente de array porque dá pra inserir e remover em O(1) sem realocar. Custo: percorrer é O(n) (não dá pra "pular pro item 50"). Ideal pra coisas que nascem e morrem o tempo todo.

**No projeto:** `MagiaNo` e `InimigoNo` em `tipos.h`. Inserção sempre na cabeça (`novo->proximo = cabeca; cabeca = novo`). Remoção dos mortos com **ponteiro duplo** (`InimigoNo **atual`) pra tratar a cabeça e os outros nós com a mesma lógica.

---

## Quando aparecer um termo novo

Adicione aqui assim que encontrar. Manter este documento atualizado é responsabilidade do grupo inteiro — se você buscou e não achou, esse termo provavelmente vai aparecer várias outras vezes.
