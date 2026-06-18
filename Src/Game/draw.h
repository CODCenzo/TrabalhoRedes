#ifndef _DRAW_H_
#define _DRAW_H_

#include "game.h"

void reset_game_state(Game *g) ;

// Desenha o caractere
void draw_tile(Game *g, int x, int y, int screen_x, int screen_y) ;

void draw_game(Game *g) ;

void show_end_screen(const char *title) ;

void init_colors() ;

#endif 
