/* ============================================================================
 * inimigos.h - INTERFACE DA ENGINE DE INIMIGOS
 * ============================================================================
 *
 * RESPONSABILIDADE: Arthur (Dev 1) — engine.
 *
 * Esta camada lida com lista encadeada, push-out físico, render genérico
 * e dispatch de IA. Os DADOS dos tipos (vida, dano, IA específica) vivem
 * em src/entidades/inimigos_tipos.c, que a Luísa (Dev 3) edita.
 *
 * QUEM CHAMA O QUÊ:
 *   - main.c chama atualizar/desenhar/liberar uma vez por frame.
 *   - cronograma.c chama inimigos_spawnar_em quando a timeline pede.
 *   - colisao.c marca .vivo = false e .vida -= dano (não remove da lista).
 *
 * COMO A LUÍSA ADICIONA UM TIPO NOVO: vai em inimigos_tipos.c. NÃO precisa
 * mexer aqui.
 * ============================================================================ */

#ifndef INIMIGOS_H
#define INIMIGOS_H

#include "tipos.h"

/* Dispatch IA + movimento + push-out + remoção dos mortos. Uma chamada por
 * frame durante o combate. */
void inimigos_atualizar(EstadoJogo *ej);

/* Render circular usando cor da tabela de parâmetros. */
void inimigos_desenhar(const EstadoJogo *ej);

/* Libera todos os nós da lista. Chamada pelo main na finalização e quando
 * uma run termina (game over / vitória). */
void inimigos_liberar_tudo(EstadoJogo *ej);

/* Cria um inimigo na posição dada com os stats do tipo (lê PARAMETROS_INIMIGO).
 * Usada pelo cronograma e por testes. Ignora silenciosamente se a lista
 * estourar MAX_INIMIGOS. */
void inimigos_spawnar_em(EstadoJogo *ej, Vector2 posicao, TipoInimigo tipo);

/* Caminho ÚNICO de morte de inimigo. Idempotente (no-op se já morto).
 * Marca vivo=false e credita biomassa. Toda fonte de dano (colisão, DoT,
 * combo, explosão) DEVE matar por aqui — senão o kill não vira pontuação.
 * O free do nó continua sendo do PASS 3 de inimigos_atualizar. */
void inimigos_registrar_morte(EstadoJogo *ej, Inimigo *i);

/* Spawna um inimigo "aliado" temporário (EF_SPAWNA_ALIADO da profecia): não
 * fere o jogador, não é alvo das magias dele e some após `duracao` s. A
 * vida é `vida_frac` da vida de um corpo-a-corpo. Encapsula o spawn pra
 * pegar o nó certo com segurança (inimigos_spawnar_em insere na cabeça). */
void inimigos_spawnar_aliado(EstadoJogo *ej, Vector2 pos,
                             float vida_frac, float duracao);

#endif /* INIMIGOS_H */
