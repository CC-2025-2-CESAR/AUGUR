# ============================================================================
# Makefile - AUGUR  (multiplataforma: Windows / Linux / macOS)
# ----------------------------------------------------------------------------
# Estrutura modular de src/:
#   core/       loop + contrato (tipos.h) + colisão + assets
#   entidades/  jogador, inimigos, magias, projéteis de inimigo
#   sistemas/   profecia, cronograma, cartas, dados, salvamento, etc.
#   interface/  hud
#
# Descobre todos os .c automaticamente e adiciona cada subpasta ao -I, então os
# #include continuam por nome simples (ex.: #include "tipos.h").
#
# Detecta o sistema operacional e ajusta o executável e as libs do Raylib:
#   - Windows: opengl32/gdi32/winmm   - Linux: GL/X11/pthread/dl/rt
#   - macOS:   frameworks Cocoa/OpenGL/IOKit/CoreVideo/CoreAudio
# Usa pkg-config quando disponível (com fallback pra -lraylib).
# ============================================================================

CC := gcc
PKG_CONFIG := pkg-config

# Descobre todos os .c em src/ e em qualquer subpasta direta de src/.
FONTES := $(wildcard src/*.c) $(wildcard src/*/*.c)

# -I para cada módulo, pra que #include "header.h" funcione sem o caminho completo.
INCLUDES := -Isrc -Isrc/core -Isrc/entidades -Isrc/sistemas -Isrc/interface

OBJETOS := $(patsubst src/%.c,build/%.o,$(FONTES))

CFLAGS_BASE := -Wall -Wextra -std=c11 -g $(INCLUDES)

# Flags do Raylib via pkg-config. Se o .pc não existir na máquina, cai no
# fallback -lraylib (assume a lib instalada no caminho padrão do compilador).
RAYLIB_CFLAGS := $(shell $(PKG_CONFIG) --cflags raylib 2>/dev/null)
RAYLIB_LIBS   := $(shell $(PKG_CONFIG) --libs raylib 2>/dev/null)
ifeq ($(strip $(RAYLIB_LIBS)),)
    RAYLIB_LIBS := -lraylib
endif

ifeq ($(OS),Windows_NT)
    # ---------------------------- Windows (MSYS2 UCRT64) --------------------
    EXECUTAVEL := augur.exe
    CFLAGS  := $(CFLAGS_BASE) $(RAYLIB_CFLAGS)
    LDFLAGS := $(RAYLIB_LIBS) -lopengl32 -lgdi32 -lwinmm -lm

    # No terminal MSYS2/Git Bash, MSYSTEM vem definido e os comandos sao POSIX.
    # Fora dele (PowerShell/CMD), usamos cmd.exe pra nao depender do /usr/bin.
    ifneq ($(strip $(MSYSTEM)),)
        COMANDO_EXECUTAR := ./$(EXECUTAVEL)
        CRIAR_PASTA = mkdir -p "$(1)"
        REMOVER_GERADOS := rm -rf build augur augur.exe
    else
        SHELL := cmd.exe
        .SHELLFLAGS := /C
        COMANDO_EXECUTAR := .\$(EXECUTAVEL)
        CRIAR_PASTA = if not exist "$(subst /,\,$(1))" mkdir "$(subst /,\,$(1))"
        REMOVER_GERADOS := if exist build rmdir /S /Q build & if exist augur.exe del /Q augur.exe & if exist augur del /Q augur
    endif
else
    # ---------------------------- Unix (Linux / macOS) ----------------------
    EXECUTAVEL := augur
    COMANDO_EXECUTAR := ./augur
    CFLAGS  := $(CFLAGS_BASE) $(RAYLIB_CFLAGS)
    CRIAR_PASTA = mkdir -p "$(1)"
    REMOVER_GERADOS := rm -rf build augur augur.exe

    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        # macOS: o Raylib precisa dos frameworks do sistema pra linkar.
        LDFLAGS := $(RAYLIB_LIBS) -framework CoreVideo -framework IOKit -framework Cocoa -framework OpenGL -framework CoreAudio -lm
    else
        # Linux (e outros Unix): GL + libs de sistema que o Raylib usa.
        LDFLAGS := $(RAYLIB_LIBS) -lGL -lm -lpthread -ldl -lrt -lX11
    endif
endif

.PHONY: all run executar clean limpar

all: $(EXECUTAVEL)

$(EXECUTAVEL): $(OBJETOS)
	$(CC) $(OBJETOS) -o $@ $(LDFLAGS)
	@echo Base compilada com sucesso.

build/%.o: src/%.c
	@$(call CRIAR_PASTA,$(dir $@))
	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXECUTAVEL)
	$(COMANDO_EXECUTAR)

executar: run

clean:
	@$(REMOVER_GERADOS)

limpar: clean
