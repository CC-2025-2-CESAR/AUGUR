# Issue para Sofia — Adaptar HUD ao novo sistema de Cronograma

## Contexto

O sistema de **ondas finitas** foi substituído por um **cronograma de 5 minutos** estilo Vampire Survivors:

- A struct `Onda` foi removida do `tipos.h`.
- Em `EstadoJogo`, o campo `onda_atual` virou `cronograma` (do tipo `Cronograma`).
- A run inteira é uma timeline contínua. Aos 5:00 surge o chefão; matá-lo encerra com vitória.
- A cada minuto cheio (1:00, 2:00, ...), a tela de cartas abre automaticamente.

Pra evitar quebrar a build, eu (Arthur) deixei um **stopgap de uma linha** em `src/interface/hud.c` — a função `hud_desenhar_onda()` mostra `MM:SS / 05:00` em vez de `Onda N`. Funcional, mas sem polish.

## O que precisa virar no HUD

A ideia é deixar o HUD mais alinhado com a fantasia VS-like:

1. **Tempo grande no topo**, formato `MM:SS / 05:00`. Já está no stopgap, mas pode ganhar fonte maior, brilho, tipografia.
2. **Cor mudando conforme o tempo se aproxima do chefão**:
   - 0:00 → 3:00: branco/dourado (calmo).
   - 3:00 → 4:00: amarelo (tensão crescendo).
   - 4:00 → 5:00: vermelho pulsando (rush final).
3. **Contagem regressiva pro chefão** (opcional, mas legal): nos últimos 30 segundos, um texto extra `CHEFAO EM XX` aparece em destaque.
4. **Indicador de "minuto atual"**: pode ser uma barrinha de progresso entre minutos cheios — visualiza quando a próxima carta vai sair.

## Onde ler os dados

Em vez de `ej->onda_atual.numero`, você usa o `Cronograma` em `ej->cronograma`. Os campos relevantes:

```c
// src/core/tipos.h
typedef struct {
    float            tempo_decorrido;        // segundos desde o início da run
    float            tempo_proxima_carta;    // segundos até o próximo trigger de carta
    bool             cartas_pendentes;       // engine setou (não use no HUD)
    bool             chefao_spawnado;        // se já apareceu o chefão
    bool             esperando_chefao_morrer;
    bool             vitoria;
    EventoCronograma eventos[MAX_EVENTOS_CRONOGRAMA];
    int              qtd_eventos;
} Cronograma;
```

E a constante `CRONOGRAMA_DURACAO_SEG` (= 300s, ou seja 5 min) está em `src/sistemas/cronograma.h`.

## Snippet pronto pra começar

```c
#include "cronograma.h"   /* pra ter CRONOGRAMA_DURACAO_SEG */

void hud_desenhar_onda(const EstadoJogo *ej) {
    float t = ej->cronograma.tempo_decorrido;
    int min = (int)(t / 60.0f);
    int seg = (int)t % 60;

    int total_min = (int)(CRONOGRAMA_DURACAO_SEG / 60.0f);

    char buf[48];
    snprintf(buf, sizeof(buf), "%02d:%02d / %02d:00", min, seg, total_min);

    Color cor = GOLD;
    if (t >= CRONOGRAMA_DURACAO_SEG - 60.0f) {
        // último minuto (4:00 → 5:00): vermelho pulsante
        cor = ((int)(t * 4) % 2) ? RED : MAROON;
    } else if (t >= CRONOGRAMA_DURACAO_SEG - 2.0f * 60.0f) {
        cor = YELLOW;   // penúltimo minuto (3:00 → 4:00)
    }

    DrawText(buf, LARGURA_TELA / 2 - 90, HUD_MARGEM,
             HUD_FONTE_GRANDE + 4, cor);

    // Contagem regressiva pro chefão (últimos 30s)
    float restante = CRONOGRAMA_DURACAO_SEG - t;
    if (restante > 0.0f && restante <= 30.0f && !ej->cronograma.chefao_spawnado) {
        char buf2[48];
        snprintf(buf2, sizeof(buf2), "CHEFAO EM %d", (int)ceilf(restante));
        DrawText(buf2, LARGURA_TELA / 2 - 90, HUD_MARGEM + 32,
                 HUD_FONTE_MEDIA, RED);
    }
}
```

(Pra `ceilf`: `#include <math.h>`.)

## Strings sem acento

A fonte default do Raylib é ASCII puro. Mantenha "CHEFAO" em vez de "CHEFÃO" dentro de `DrawText`. Comentários e nomes de variável podem ter acento normal.

## Quando estiver pronto

A engine de cronograma (5 min) já está na `main`. Faça uma branch a partir dela:

```bash
git checkout main
git pull
git checkout -b feat/hud-cronograma
# editar src/interface/hud.c e hud.h se precisar
git commit -m "feat(hud): exibe tempo da timeline e contagem regressiva pro chefao"
git push -u origin feat/hud-cronograma
```

PR: descreva o que mudou e cole um print do HUD novo. Bonito é jogador feliz.
