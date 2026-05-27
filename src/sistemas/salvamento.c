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

void salvamento_carregar(DadosSalvos *ds) {
    /* TODO: Dev 2 — implementar leitura binária do arquivo.
     * Por enquanto, zera tudo (como se fosse uma sessão nova). */
    memset(ds, 0, sizeof(DadosSalvos));
}

void salvamento_salvar(const DadosSalvos *ds) {
    (void)ds;
    /* TODO: Dev 2 — implementar escrita binária no arquivo.
     * Não esquecer de fechar o FILE com fclose depois do fwrite! */
}
