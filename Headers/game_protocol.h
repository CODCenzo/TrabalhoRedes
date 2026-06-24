#ifndef GAME_PROTOCOL_H
#define GAME_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

#include "game.h"
#include "kermit.h"
#include "protocol.h"

int send_matrix(int socket, char m[MAZE_SIZE][MAZE_SIZE + 1]);

int enviar_tabuleiro_jogo(int socket, uint8_t tabuleiro[40][40]);

int receber_tabuleiro_jogo(int socket, uint8_t tabuleiro_destino[40][40]);


#endif /* GAME_PROTOCOL_H */
