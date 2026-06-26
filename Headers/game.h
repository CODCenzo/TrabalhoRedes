#ifndef _GAME_H_
#define _GAME_H_

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAZE_SIZE 40
#define GHOSTS 4
#define PRIZES 6

enum { PACMAN = -1, RED = 0, BLUE = 1, GREEN = 2, YELLOW = 3,
       P1 = 4, P2 = 5, P3 = 6, P4 = 7, P5 = 8, P6 = 9 };

typedef struct {
  int y;
  int x;
  int dy;
  int dx;
} Actor;

typedef struct {
  Actor body;
  int color_pair;
  char symbol;
} Ghost;

typedef struct {
  Actor pacman;
  Ghost *ghosts;
  char **maze;
  char **prizes_files;
  int score;
  int moves_count;
  int vision_radius;
  int prizes_collected;
  int green_prefers_left;
} Game;

bool is_inside(int x, int y) ;

bool is_wall(Game *g, int x, int y) ;

bool is_free_cell(Game *g, int x, int y, int type) ;

// Muda as posicoes
void random_free_position(Game *g, int *x, int *y, int type) ;

void init_ghosts(Game *g) ;

char normalize_tile(char tile) ;

void reset_game_state(Game *g) ;

void place_objects(Game *g, bool has_pacman, 
                   int has_ghosts[GHOSTS], int has_prizes[PRIZES]) ;

void load_default_level(Game *g) ;

void load_level(Game *g, const char *filepath) ;

bool visible_to_pacman(Game *g, int x, int y) ;

void build_client_matrix(Game *g, char out[MAZE_SIZE][MAZE_SIZE + 1]) ;

int try_move(Game *g, Actor *b, int dx, int dy) ;

int direction_from_key(int ch, int *dx, int *dy) ;

void rotate_left(int dx, int dy, int *out_dx, int *out_dy) ;

void rotate_right(int dx, int dy, int *out_dx, int *out_dy) ;

void ghost_wall_rule(Game *g, Ghost *ghost, int prefer_left) ;

void ghost_random(Game *g, Ghost *ghost) ;

void update_ghosts(Game *g) ;

int check_collision(Game *g) ;

int collect_prize(Game *g) ;

int play_round(Game *g, int ch, int *prize) ;

char **alloc_matrix(int l, int c) ;

void free_matrix(char **m, int l) ;

Game *init_game() ;

void free_game(Game *g) ;
#endif
