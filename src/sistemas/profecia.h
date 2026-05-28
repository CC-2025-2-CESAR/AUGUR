/* ============================================================================
 * profecia.h - INTERFACE DO GERADOR PROCEDURAL DE PROFECIAS
 * ============================================================================
 *
 * Uma profecia é composta por 3 modificadores do tipo
 *   [Elemento] + [Condição] + [Efeito].
 *
 * Ex.: "Fogo | Ao matar -> Explosão em área"
 *
 * O gerador recebe uma SEED (número unsigned int) e produz SEMPRE a mesma
 * profecia pra aquela seed — isso é chamado de "determinismo procedural".
 * Permite debug reproduzível e compartilhar runs com amigos ("joga a seed
 * 4821, a profecia é absurda").
 * ========================================================================== */

#ifndef PROFECIA_H
#define PROFECIA_H

#include "tipos.h"

/* Gera uma profecia completa a partir da seed.
 * Escreve os 3 modificadores em p->mods e guarda a seed em p->seed. */
void profecia_gerar(Profecia *p, unsigned int seed);

/* Desenha a profecia na tela (usado no estado REVELACAO_PROFECIA). */
void profecia_desenhar(const Profecia *p);

/* Funções de lookup: converte enum em texto legível.
 * Úteis pra outros módulos que queiram mostrar nomes (HUD, debug). */
const char *elemento_nome(Elemento e);
const char *condicao_nome(Condicao c);
const char *efeito_nome(Efeito e);


/* ============================================================================
 * MOTOR DE PROFECIA (engine — Arthur)
 * --------------------------------------------------------------------------
 * O motor avalia as Condições dos 3 mods e aplica os Efeitos correspondentes
 * durante o combate. As MAGNITUDES vivem em profecia_efeitos.c (Luísa tuna).
 *
 * Quem chama o quê:
 *   - main.c: profecia_evento_inicio_run (ao começar a run) e
 *     profecia_motor_atualizar (todo frame de combate).
 *   - inimigos.c: profecia_evento_ao_matar (caminho único de morte).
 *   - colisao.c: profecia_evento_ao_acertar / _ao_receber_dano.
 *   - sistema de dados (Sofia, futuro): profecia_evento_ao_rolar_dado.
 * ========================================================================== */

/* Tick por frame: condições de tempo (A cada 10s), vida baixa, decaimento
 * dos buffs temporários e janela de combo. */
void profecia_motor_atualizar(EstadoJogo *ej);

/* Reseta o disparo único e aplica COND_INICIO_RUN. Chamar uma vez ao
 * iniciar a run (o reset dos campos do motor é feito em jogo_resetar_run). */
void profecia_evento_inicio_run(EstadoJogo *ej);

/* Gatilhos pontuais. ctx é a posição-contexto (onde aplicar AoE etc.). */
void profecia_evento_ao_matar(EstadoJogo *ej, Inimigo *morto);
void profecia_evento_ao_receber_dano(EstadoJogo *ej);
void profecia_evento_ao_acertar(EstadoJogo *ej, Vector2 ctx);
void profecia_evento_ao_curar(EstadoJogo *ej);
void profecia_evento_ao_rolar_dado(EstadoJogo *ej); /* gancho Sofia */

/* Cura o jogador (clamp em vida_maxima) e dispara COND_AO_CURAR. Usado por
 * EF_CURA e pelo lifesteal. A reentrância é cortada pelo guard do motor. */
void profecia_curar_jogador(EstadoJogo *ej, int qtd);

/* Aplica UM efeito no contexto dado. Tem guard de reentrância: um efeito
 * que mata (explosão) não recursiona em "Ao matar" infinitamente. */
void profecia_aplicar_efeito(EstadoJogo *ej, Efeito ef, Vector2 ctx);

#endif /* PROFECIA_H */
