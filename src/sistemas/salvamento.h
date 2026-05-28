/* ============================================================================
 * salvamento.h - INTERFACE DO SISTEMA DE SAVE/LOAD
 * ============================================================================
 *
 * RESPONSABILIDADE: Dev 2 (Sofia)
 *
 * Atende o REQUISITO OBRIGATÓRIO de PIF: leitura e escrita em arquivo.
 *
 * O que salva: struct DadosSalvos (definida em tipos.h) em formato BINÁRIO.
 * Arquivo: saves/biomassa.dat
 *
 * Por que binário e não texto?
 *   - Mais simples de implementar (fwrite/fread de uma só vez).
 *   - Menor no disco.
 *   - Desvantagem: não dá pra abrir no bloco de notas. Pra debug, adicionar
 *     uma função de "imprimir salvamento" que printa no terminal.
 *
 * COMO IMPLEMENTAR:
 *   Salvar:  FILE *f = fopen("saves/biomassa.dat", "wb");
 *            fwrite(ds, sizeof(DadosSalvos), 1, f);
 *            fclose(f);
 *
 *   Carregar: FILE *f = fopen("saves/biomassa.dat", "rb");
 *             if (f) { fread(ds, sizeof(DadosSalvos), 1, f); fclose(f); }
 *             else   { // arquivo não existe, usar valores padrão
 * ========================================================================== */

#ifndef SALVAMENTO_H
#define SALVAMENTO_H

#include "tipos.h"

/* Lê saves/biomassa.dat e preenche ds. Se o arquivo não existir, zera tudo. */
void salvamento_carregar(DadosSalvos *ds);

/* Grava ds em saves/biomassa.dat. Chamada ao fechar o jogo. */
void salvamento_salvar(const DadosSalvos *ds);

#endif /* SALVAMENTO_H */
