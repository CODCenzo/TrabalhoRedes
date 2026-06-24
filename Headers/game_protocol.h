#ifndef _GAME_PROTOCOL_H_
#define _GAME_PROTOCOL_H_

#include "kermit.h"
#include "protocol.h"
#include "../Src/Game/game.h"

int send_game_file (int socket, int type, int prize) ;

int receive_game_file (int socket, int type) ;

/*funcao que envia matriz*/
int send_matrix(int socket, char m[MAZE_SIZE][MAZE_SIZE + 1]) ;

/*recebe matriz*/
int receive_matrix(int socket, char m[MAZE_SIZE][MAZE_SIZE + 1]) ;

/*funcao que manda tecla*/
int send_key(int socket, char key) ;

/*funcao que recebe tecla*/
//int receive_key(int socket, char *key) ;

/*funcao que manda tela de fim*/
int send_end_screen(int socket) ;

/*funcao que recebe tela de fim*/
//int receive_end_screen(int socket) ;

int client_loop(int socket) ;

int client_checa_pacote(int socket);

int server_checa_pacote(int socket) ;

#endif