/* ============================================================================
 * salvamento.c - STUB DO SISTEMA DE SAVE/LOAD
 * ============================================================================
 *
 * TODO: Dev 2 (Sofia) implementar este arquivo.
 *
 * Lembrete: o diretório "saves/" já existe no repositório. Mas em outras
 * máquinas pode não existir. Uma boa prática é usar _mkdir("saves") no
 * Windows antes de tentar abrir o arquivo (include <direct.h>).
 * ========================================================================== */

#include "salvamento.h"
#include <stdio.h>    /* FILE, fopen, fread, fwrite, fclose */
#include <string.h>   /* memset */

#define CAMINHO_SAVE "saves/biomassa.dat"

void salvamento_carregar(DadosSalvos *ds) {
    if (ds == NULL) return;

    memset(ds, 0, sizeof(DadosSalvos));
    ds->versao_save = SAVE_VERSAO_ATUAL;   /* default pra save inexistente */

    FILE *f = fopen(CAMINHO_SAVE, "rb");
    if (f == NULL) return;                 /* primeira run: fica com os zeros */

    fread(ds, sizeof(DadosSalvos), 1, f);
    fclose(f);

    /* Save antigo (sem campo de versão, ou de uma versão incompatível): zera
     * tudo e marca como save da versão atual. Garante que upgrades da struct
     * DadosSalvos não corrompam o jogo — pior caso, o jogador perde leaderboard
     * e config de vídeo, mas o jogo abre. */
    if (ds->versao_save != SAVE_VERSAO_ATUAL) {
        memset(ds, 0, sizeof(DadosSalvos));
        ds->versao_save = SAVE_VERSAO_ATUAL;
    }
}

void salvamento_salvar(const DadosSalvos *ds) {
    if (ds == NULL) return;

    /* Garante que o arquivo gravado SEMPRE traz a versão atual. Se um caller
     * der azar de passar uma struct com versao_save zerada (ex.: save fresco
     * que nunca passou por _carregar), evita gravar um "save inválido". */
    DadosSalvos copia = *ds;
    copia.versao_save = SAVE_VERSAO_ATUAL;

    FILE *f = fopen(CAMINHO_SAVE, "wb");
    if (f == NULL) {
        fprintf(stderr, "[SALVAMENTO] Erro: nao foi possivel abrir '%s' para escrita.\n",
                CAMINHO_SAVE);
        return;
    }

    fwrite(&copia, sizeof(DadosSalvos), 1, f);
    fclose(f);
}