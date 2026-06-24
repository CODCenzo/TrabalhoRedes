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

/*static const char *DEFAULT_UFPR[MAZE_SIZE] = {
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X0000X0000X00XXXXXX00XXXXXX00XXXXXX0000X",
  "X0000X0000X00X0000000X0000X00X0000X0000X",
  "X0000X0000X00X0000000X0000X00X0000X0000X",
  "X0000X0000X00X0000000X0000X00X0000X0000X",
  "X0000X0000X00X0000000X0000X00X0000X0000X",
  "X0000X0000X00X0000000X0000X00X0000X0000X",
  "X0000X0000X00XXXXX000X0XXXX00XX0XXX0000X",
  "X0000X0000X00X0000000X0000000XX00000000X",
  "X0000X0000X00X0000000X0000000X0X0000000X",
  "X0000X0000X00X0000000X0000000X0X0000000X",
  "X0000X0000X00X0000000X0000000X00X000000X",
  "X0000X0000X00X0000000X0000000X00X000000X",
  "X0000X0000X00X0000000X0000000X000X00000X",
  "X0000X0000X00X0000000X0000000X000X00000X",
  "X0000X0000X00X0000000X0000000X0000X0000X",
  "X0000XXXXXX00X0000000X0000000X0000X0000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "X00000000000000000000000000000000000000X",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
};*/

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

/*int load_csv_level(Game *g, const char *filepath) i*/
void load_level(Game *g, const char *filepath) ;

bool visible_to_pacman(Game *g, int x, int y) ;

int try_move(Game *g, Actor *b, int dx, int dy) ;

int direction_from_key(int ch, int *dx, int *dy) ;

void rotate_left(int dx, int dy, int *out_dx, int *out_dy) ;

void rotate_right(int dx, int dy, int *out_dx, int *out_dy) ;

void ghost_wall_rule(Game *g, Ghost *ghost, int prefer_left) ;

void ghost_random(Game *g, Ghost *ghost) ;

void update_ghosts(Game *g) ;

int check_collision(Game *g) ;

void collect_prize(Game *g) ;

int play_round(Game *g, int ch) ;

char **alloc_matrix(int l, int c) ;

void free_matrix(char **m, int l) ;

Game *init_game() ;

void free_game(Game *g) ;
#endif
