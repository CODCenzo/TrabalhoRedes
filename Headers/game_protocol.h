#ifndef GAME_PROTOCOL_H
#define GAME_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

#include "game.h"
#include "kermit.h"
#include "protocol.h"

int enviar_tabuleiro_jogo(int socket, uint8_t tabuleiro[40][40]);

int receber_tabuleiro_jogo(int socket, uint8_t tabuleiro_destino[40][40]);

void imprimir_tabuleiro_jogo(uint8_t tabuleiro[40][40]);

int cliente_enviar_movimento(int socket, uint8_t tipo_movimento);

int servidor_receber_movimento(int socket, uint8_t *tipo_movimento_recebido);

#endif /* GAME_PROTOCOL_H */
