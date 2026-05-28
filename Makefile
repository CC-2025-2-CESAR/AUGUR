# ============================================================================
# Makefile - AUGUR
# ============================================================================
#
# Estrutura modular:
#   src/core/         loop principal, contrato (tipos.h), colisão
#   src/entidades/    jogador, inimigos, magias, obstáculos
#   src/sistemas/     profecia, onda, cartas, dados, salvamento
#   src/interface/    hud
#
# Cada módulo é uma pasta. O Makefile descobre automaticamente todos os
# .c e adiciona cada subpasta no -I do compilador, então os #include
# continuam podendo ser por nome simples (ex.: #include "tipos.h").
# ============================================================================

CC := gcc
PKG_CONFIG := pkg-config

# Descobre todos os .c em src/ e em qualquer subpasta direta de src/.
FONTES := $(wildcard src/*.c) $(wildcard src/*/*.c)

# -I para cada módulo, pra que #include "header.h" funcione sem precisar
# escrever o caminho completo.
INCLUDES := -Isrc -Isrc/core -Isrc/entidades -Isrc/sistemas -Isrc/interface

OBJETOS := $(patsubst src/%.c,build/%.o,$(FONTES))

ifeq ($(OS),Windows_NT)
    EXECUTAVEL := augur.exe
    CFLAGS := -Wall -Wextra -std=c11 -g $(INCLUDES) $(shell $(PKG_CONFIG) --cflags raylib)
    LDFLAGS := $(shell $(PKG_CONFIG) --libs raylib) -lopengl32 -lgdi32 -lwinmm -lm

    # No terminal MSYS2/Git Bash, MSYSTEM vem definido e os comandos sao POSIX.
    # Fora dele, usamos cmd.exe para o PowerShell/CMD nao depender do /usr/bin.
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
    EXECUTAVEL := augur
    COMANDO_EXECUTAR := ./augur
    CFLAGS := -Wall -Wextra -std=c11 -g $(INCLUDES) $(shell $(PKG_CONFIG) --cflags raylib)
    LDFLAGS := $(shell $(PKG_CONFIG) --libs raylib) -lm
    CRIAR_PASTA = mkdir -p "$(1)"
    REMOVER_GERADOS := rm -rf build augur augur.exe
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
