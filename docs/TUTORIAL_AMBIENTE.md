# Tutorial: Configurando o Ambiente de Desenvolvimento

Este guia cobre tudo que você precisa pra compilar e rodar um projeto em C com **raylib** do zero: instalação do GCC, Make, pkg-config e raylib — tanto no **Windows** quanto no **Linux**. Os exemplos abaixo são genéricos: se aplicam a qualquer projeto que use a mesma stack.

---

## Antes de começar — Configure o VS Code direito

Antes de qualquer instalação, faça isso no VS Code: instale a extensão **vscode-icons**. Ela adiciona ícones coloridos para cada tipo de arquivo e pasta, tornando a navegação no projeto muito mais fácil.

### Instalando o vscode-icons

1. Abra o VS Code
2. Vá em **Extensions** (atalho: `Ctrl+Shift+X`)
3. Pesquise por `vscode-icons`
4. Clique em **Install** na extensão da **VSCode Icons Team**

### Ativando os ícones

Após instalar, o VS Code vai perguntar se você quer ativar — clique em **Activate**. Se não aparecer a pergunta:

1. Abra a paleta de comandos: `Ctrl+Shift+P`
2. Digite: `File Icon Theme`
3. Selecione **VSCode Icons**

Pronto! Agora cada tipo de arquivo (`.c`, `.h`, `Makefile`, etc.) aparece com ícone distinto no explorador lateral.

---

## Índice

1. [Windows — Instalação via MSYS2](#1-windows--instalação-via-msys2)
2. [Linux — Instalação via apt](#2-linux--instalação-via-apt)
3. [Verificando a instalação](#3-verificando-a-instalação)
4. [Compilando um projeto](#4-compilando-um-projeto)
5. [Anatomia de um Makefile](#5-anatomia-de-um-makefile)
6. [Como criar um Makefile do zero](#6-como-criar-um-makefile-do-zero)
7. [Problemas comuns](#7-problemas-comuns)

---

## 1. Windows — Instalação via MSYS2

No Windows, a forma mais simples de ter GCC + Make + raylib funcionando é pelo **MSYS2**, que é um gerenciador de pacotes estilo Linux rodando no Windows.

### Passo 1 — Instalar o MSYS2

1. Acesse: https://www.msys2.org
2. Baixe e execute o instalador (`.exe`)
3. Siga a instalação padrão. O caminho padrão é `C:\msys64`
4. Ao final, o instalador abre o terminal **MSYS2 UCRT64** — pode fechar por enquanto

### Passo 2 — Abrir o terminal correto

> ⚠️ Atenção: existem vários terminais MSYS2. Use sempre o **MSYS2 UCRT64** (ícone roxo). Os outros (MINGW64, CLANG64, MSYS) usam runtimes diferentes e podem dar conflito.

Procure no menu iniciar por **"MSYS2 UCRT64"** e abra-o.

### Passo 3 — Atualizar o sistema

Antes de instalar qualquer coisa, atualize o sistema base:

```bash
pacman -Syu
```

Se o terminal fechar no meio do processo, abra de novo e rode novamente:

```bash
pacman -Su
```

### Passo 4 — Instalar GCC, Make e pkg-config

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-pkg-config
```

Confirme com `Y` quando perguntado.

### Passo 5 — Instalar o raylib

```bash
pacman -S mingw-w64-ucrt-x86_64-raylib
```

### Passo 6 — Adicionar o MSYS2 ao PATH do Windows

Para usar `gcc`, `mingw32-make` e etc. no **CMD ou PowerShell** (fora do terminal MSYS2):

1. Abra o **Painel de Controle** → Variáveis de Ambiente
   - Atalho: tecla Windows → pesquise "variáveis de ambiente" → clique em "Editar variáveis de ambiente do sistema"
2. Em **Variáveis do sistema**, selecione `Path` e clique em **Editar**
3. Clique em **Novo** e adicione:
   ```
   C:\msys64\ucrt64\bin
   ```
4. Clique OK em tudo e **reinicie o terminal**

> Após isso, `gcc`, `mingw32-make` e `pkg-config` funcionarão no CMD/PowerShell/VS Code normalmente. Repare que no UCRT64 o comando do Make é `mingw32-make`, não `make`.

---

## 2. Linux — Instalação via apt

No Linux (Ubuntu/Debian e derivados), tudo fica bem mais simples.

### Passo 1 — Atualizar os pacotes

```bash
sudo apt update
```

### Passo 2 — Instalar GCC, Make e pkg-config

```bash
sudo apt install gcc make pkg-config
```

### Passo 3 — Instalar dependências do raylib

```bash
sudo apt install libgl1-mesa-dev libxi-dev libxcursor-dev libxrandr-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```

### Passo 4 — Instalar o raylib

Existem duas formas. A mais simples é via apt (mas pode vir uma versão antiga):

```bash
sudo apt install libraylib-dev
```

Se a versão do apt for muito antiga (< 4.0), instale pelo código-fonte:

```bash
# Dependências de compilação
sudo apt install cmake git

# Clonar e compilar
git clone https://github.com/raysan5/raylib.git
cd raylib
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make
sudo make install
sudo ldconfig
```

---

## 3. Verificando a instalação

Após instalar tudo, verifique se está funcionando:

```bash
gcc --version
# deve mostrar algo como: gcc (Rev1, Built by MSYS2 project) 14.x.x

# Linux:
make --version
# Windows (UCRT64):
mingw32-make --version
# qualquer um deve mostrar: GNU Make 4.x

pkg-config --modversion raylib
# deve mostrar a versão instalada do raylib, ex: 5.5
```

Se algum comando não for reconhecido, verifique se o PATH está correto.

---

## 4. Compilando um projeto

Com tudo instalado, entre na pasta do projeto e rode:

```bash
# Linux
make
make run
make clean

# Windows (UCRT64)
mingw32-make
mingw32-make run
mingw32-make clean
```

O executável gerado fica na raiz do projeto, com `.exe` no Windows.

---

## 5. Anatomia de um Makefile

Um Makefile típico pra um projeto em C com raylib tem mais ou menos esta cara:

```makefile
CC := gcc                      # compilador que será usado
PKG_CONFIG := pkg-config       # ferramenta para encontrar flags de bibliotecas

# Descobre automaticamente todos os .c em src/ e subpastas
FONTES := $(wildcard src/*.c) $(wildcard src/*/*.c)

# Caminhos para os headers (exemplo genérico — adapte às pastas que você usar)
INCLUDES := -Isrc

# Converte lista de .c em lista de .o dentro da pasta build/
OBJETOS := $(patsubst src/%.c,build/%.o,$(FONTES))
```

O bloco `ifeq ($(OS),Windows_NT)` detecta o sistema operacional automaticamente e ajusta:
- O nome do executável (`.exe` no Windows)
- As flags de link (`-lopengl32 -lgdi32 -lwinmm` no Windows vs. só `-lm` no Linux)
- O comando pra rodar (`.\jogo.exe` no Windows vs. `./jogo` no Linux)

A vantagem é que o mesmo Makefile compila no Windows e no Linux sem alterações.

---

## 6. Como criar um Makefile do zero

Esta seção é um guia completo para quem **não tem nenhum Makefile** e quer criar um do zero para um projeto em C com raylib.

---

### Passo 1 — Criar o arquivo

O arquivo se chama exatamente `Makefile` (com M maiúsculo, sem extensão).

**No terminal**, dentro da pasta do seu projeto:

```bash
# Linux / MSYS2
touch Makefile

# Windows (CMD)
type nul > Makefile
```

**No VS Code**: clique em "Novo Arquivo", nomeie como `Makefile` e salve na raiz do projeto.

> ⚠️ **Atenção crítica**: o Makefile usa **TAB** para indentar os comandos, não espaços. Se você usar espaços, o Make vai dar erro. No VS Code, você pode garantir isso indo em View → Command Palette → "Change Indentation" → Tabs, ou configurando `"editor.insertSpaces": false` para arquivos Makefile.

---

### Passo 2 — Entender a anatomia

Um Makefile é uma lista de **regras**. Cada regra diz: *"para produzir X, execute este comando"*.

```
alvo: dependência1 dependência2
→TAB←	comando que produz o alvo
```

O Make só re-executa uma regra se alguma dependência for mais nova que o alvo. Isso é o que torna a compilação rápida: ele só recompila o que mudou.

**Variáveis** funcionam como atalhos:

```makefile
CC := gcc           # define a variável CC
$(CC)               # usa a variável CC (expande para "gcc")
```

**Comandos shell** podem ser embutidos com `$(shell ...)`:

```makefile
# Isso roda pkg-config no shell e usa a saída como valor
CFLAGS := $(shell pkg-config --cflags raylib)
```

---

### Passo 3 — Começar simples: um arquivo só

Suponha que sua estrutura é:

```
meu_projeto/
├── Makefile
└── main.c
```

Cole isso no seu `Makefile`:

```makefile
# O compilador a usar
CC := gcc

# Flags de compilação:
#   -Wall       → ativa todos os avisos úteis
#   -Wextra     → ativa avisos extras
#   -std=c11    → usa o padrão C11
#   -g          → inclui info de debug (útil para erros)
CFLAGS := -Wall -Wextra -std=c11 -g

# Libs a linkar (-lm é a biblioteca matemática, sempre boa ter)
LDFLAGS := -lm

# Nome do executável final
EXECUTAVEL := meu_jogo

.PHONY: all run clean

all: $(EXECUTAVEL)

$(EXECUTAVEL): main.c
	$(CC) $(CFLAGS) main.c -o $(EXECUTAVEL) $(LDFLAGS)

run: $(EXECUTAVEL)
	./$(EXECUTAVEL)

clean:
	rm -f $(EXECUTAVEL)
```

Salve e teste:

```bash
make        # compila
make run    # compila e roda
make clean  # apaga o executável
```

---

### Passo 4 — Adicionar o raylib

Agora vamos incluir o raylib. O `pkg-config` cuida de descobrir onde o raylib está instalado e quais flags usar — você não precisa saber o caminho na mão.

```makefile
CC := gcc

# pkg-config --cflags raylib → devolve algo como: -I/usr/include
# pkg-config --libs raylib   → devolve algo como: -lraylib
CFLAGS := -Wall -Wextra -std=c11 -g $(shell pkg-config --cflags raylib)
LDFLAGS := $(shell pkg-config --libs raylib) -lm

EXECUTAVEL := meu_jogo

.PHONY: all run clean

all: $(EXECUTAVEL)

$(EXECUTAVEL): main.c
	$(CC) $(CFLAGS) main.c -o $(EXECUTAVEL) $(LDFLAGS)

run: $(EXECUTAVEL)
	./$(EXECUTAVEL)

clean:
	rm -f $(EXECUTAVEL)
```

> **No Windows**, o raylib precisa de libs extras do sistema. Adicione `-lopengl32 -lgdi32 -lwinmm` no `LDFLAGS`:
>
> ```makefile
> LDFLAGS := $(shell pkg-config --libs raylib) -lopengl32 -lgdi32 -lwinmm -lm
> ```

---

### Passo 5 — Suporte a Windows e Linux ao mesmo tempo

Se o projeto precisa funcionar nos dois sistemas, use detecção automática de OS:

```makefile
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -g $(shell pkg-config --cflags raylib)

# Detecta o sistema operacional
ifeq ($(OS),Windows_NT)
    EXECUTAVEL := meu_jogo.exe
    LDFLAGS    := $(shell pkg-config --libs raylib) -lopengl32 -lgdi32 -lwinmm -lm
    RODAR      := .\meu_jogo.exe
else
    EXECUTAVEL := meu_jogo
    LDFLAGS    := $(shell pkg-config --libs raylib) -lm
    RODAR      := ./meu_jogo
endif

.PHONY: all run clean

all: $(EXECUTAVEL)

$(EXECUTAVEL): main.c
	$(CC) $(CFLAGS) main.c -o $(EXECUTAVEL) $(LDFLAGS)

run: $(EXECUTAVEL)
	$(RODAR)

clean:
	rm -f meu_jogo meu_jogo.exe
```

---

### Passo 6 — Projeto com múltiplos arquivos e pasta `src/`

Quando o projeto cresce, você organiza em pastas. Suponha esta estrutura genérica:

```
meu_projeto/
├── Makefile
├── src/
│   ├── main.c
│   ├── modulo_a.c
│   └── modulo_b.c
└── build/        ← criada automaticamente pelo Make
```

O Makefile abaixo descobre todos os `.c` em `src/` automaticamente:

```makefile
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -g \
          -Isrc \
          $(shell pkg-config --cflags raylib)

ifeq ($(OS),Windows_NT)
    EXECUTAVEL := meu_jogo.exe
    LDFLAGS    := $(shell pkg-config --libs raylib) -lopengl32 -lgdi32 -lwinmm -lm
    RODAR      := .\meu_jogo.exe
else
    EXECUTAVEL := meu_jogo
    LDFLAGS    := $(shell pkg-config --libs raylib) -lm
    RODAR      := ./meu_jogo
endif

# $(wildcard src/*.c) → lista todos os arquivos .c dentro de src/
FONTES  := $(wildcard src/*.c)

# $(patsubst padrão_de, padrão_para, lista)
# Converte cada "src/X.c" em "build/X.o"
OBJETOS := $(patsubst src/%.c, build/%.o, $(FONTES))

.PHONY: all run clean

all: $(EXECUTAVEL)

# Regra de link: junta todos os .o em um executável
# $@ = nome do alvo (meu_jogo / meu_jogo.exe)
# $^ = todas as dependências (todos os .o)
$(EXECUTAVEL): $(OBJETOS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Regra de compilação: compila cada .c em .o
# $< = primeira dependência (o arquivo .c correspondente)
# $@ = o .o que estamos gerando
build/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXECUTAVEL)
	$(RODAR)

clean:
	rm -rf build meu_jogo meu_jogo.exe
```

---

### Passo 7 — Subpastas dentro de `src/`

Se você organiza os módulos em subpastas (exemplo genérico: `src/modulo1/`, `src/modulo2/`), basta expandir o wildcard e os includes:

```makefile
# Descobre .c em src/ e em qualquer subpasta direta
FONTES := $(wildcard src/*.c) $(wildcard src/*/*.c)

# Um -I para cada subpasta, para que #include "header.h" funcione
INCLUDES := -Isrc -Isrc/modulo1 -Isrc/modulo2

# Usa os includes nas flags
CFLAGS := -Wall -Wextra -std=c11 -g $(INCLUDES) $(shell pkg-config --cflags raylib)

# A conversão de .c → .o continua a mesma
OBJETOS := $(patsubst src/%.c, build/%.o, $(FONTES))
```

A regra de compilação cria automaticamente as subpastas dentro de `build/`:

```makefile
build/%.o: src/%.c
	mkdir -p $(dir $@)           # cria build/modulo1/, build/modulo2/, etc.
	$(CC) $(CFLAGS) -c $< -o $@
```

---

### Referência rápida — variáveis automáticas do Make

| Variável | O que representa |
|---|---|
| `$@` | Nome do **alvo** da regra atual |
| `$<` | **Primeira** dependência |
| `$^` | **Todas** as dependências |
| `$(dir $@)` | Só o **diretório** do alvo (ex: `build/modulo1/`) |
| `$(wildcard padrão)` | Lista de arquivos que batem com o padrão |
| `$(patsubst de,para,lista)` | Substitui padrão em cada item da lista |
| `$(shell comando)` | Executa um comando shell e usa a saída |

### O que é `.PHONY`?

`.PHONY` declara alvos que **não são nomes de arquivo**. Se existir um arquivo chamado `clean` na pasta, o Make sem `.PHONY` não rodaria `make clean` (porque já "existe"). Com `.PHONY`, o Make sempre executa:

```makefile
.PHONY: all run clean
```

---

## 7. Problemas comuns

**`pkg-config: command not found`**
→ Instale o pkg-config (veja seção 1 ou 2) e reinicie o terminal.

**`raylib.h: No such file or directory`**
→ O raylib não foi encontrado pelo compilador. Verifique com `pkg-config --modversion raylib`. Se retornar erro, o raylib não está instalado corretamente.

**`make: *** No rule to make target`**
→ Você está rodando o `make` na pasta errada. Entre na pasta raiz do projeto (onde está o `Makefile`).

**Erros de link no Windows (`undefined reference to ...`)**
→ Adicione as libs do Windows nas flags: `-lopengl32 -lgdi32 -lwinmm`

**`make` não é reconhecido no Windows**
→ O PATH do MSYS2 não foi adicionado corretamente. Refaça o Passo 6 da seção 1 e reinicie o terminal. Lembre que no UCRT64 o comando é `mingw32-make`, não `make`.

**Jogo abre e fecha imediatamente**
→ Rode pelo terminal (`make run` ou `mingw32-make run`) para ver as mensagens de erro. Não dê duplo clique no `.exe`.
