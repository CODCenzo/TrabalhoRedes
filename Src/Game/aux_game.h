#ifndef _GAME_AUX_H
#define _GAME_AUX_H

#include "game.h"

bool is_inside(int x, int y) ;
bool is_wall(Game *g, int x, int y) ;
bool is_free_cell(Game *g, int x, int y, int type) ;

// Muda as posicoes
void random_free_position(Game *g, int *x, int *y, int type) ;

void init_ghosts(Game *g) ;

char normalize_tile(char tile) ;

void reset_game_state(Game *g) ;

int try_move(Game *g, Actor *b, int dx, int dy) ;

int direction_from_key(int ch, int *dx, int *dy) ;

void rotate_left(int dx, int dy, int *out_dx, int *out_dy) ;

void rotate_right(int dx, int dy, int *out_dx, int *out_dy) ;

char **alloc_matrix(int l, int c) ;

void free_matrix(char **m, int l) ;

void free_game(Game *g) ;

#endif
