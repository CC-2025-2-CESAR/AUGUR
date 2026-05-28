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

    FILE *f = fopen(CAMINHO_SAVE, "rb");
    if (f == NULL) return; /* primeira run: fica com os zeros */

    fread(ds, sizeof(DadosSalvos), 1, f);
    fclose(f);
}

void salvamento_salvar(const DadosSalvos *ds) {
    if (ds == NULL) return;

    FILE *f = fopen(CAMINHO_SAVE, "wb");
    if (f == NULL) {
        fprintf(stderr, "[SALVAMENTO] Erro: nao foi possivel abrir '%s' para escrita.\n",
                CAMINHO_SAVE);
        return;
    }

    fwrite(ds, sizeof(DadosSalvos), 1, f);
    fclose(f);
}