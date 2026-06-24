#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "game.h"

void build_client_matrix(Game *g, char out[MAZE_SIZE][MAZE_SIZE + 1]) ;
void draw_client_tile(char tile, int screen_x, int screen_y) ;
void draw_game_client(char matrix[MAZE_SIZE][MAZE_SIZE + 1]) ;


#endif
