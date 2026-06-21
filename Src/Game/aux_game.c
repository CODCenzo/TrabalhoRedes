#include "aux_game.h"

bool is_inside(int x, int y) {

  if(y >= 0 && y < MAZE_SIZE && x >= 0 && x < MAZE_SIZE) {
    return true ;
  }
  return false;
}

bool is_wall(Game *g, int x, int y) {

  if (!g) {
    perror("erro is_wall\n");
    exit(1);
  }

  if(!is_inside(x, y) || g->maze[y][x] == 'X')
    return true;
  return false;
}

bool is_free_cell(Game *g, int x, int y, int type) {
  int i;

  if(!g) {
    perror("erro is_free_cell\n");
    exit(1);
  }

  if (is_wall(g, x, y))
    return false;
  if (g->pacman.y == y && g->pacman.x == x && type != PACMAN)
    return false;

  for (i = 0; i < GHOSTS; i++) {

    if (type != i)
      if (g->ghosts[i].body.x == x && g->ghosts[i].body.y == y)
        return false;
  }

  if (g->maze[y][x] == '0') {
    return true ;
  }
  
  perror("caso nao definido\n");
  return false;
}

// Muda as posicoes
void random_free_position(Game *g, int *x, int *y, int type) {

  int attempts;

  if(!g) {
    perror("erro random_free_position\n");
    exit(1);
  }

  for (attempts = 0; attempts < 5000; attempts++) {
    *y = rand() % MAZE_SIZE;
    *x = rand() % MAZE_SIZE;

    if (is_free_cell(g, *x, *y, type)) {
      return;
    }
  }

  for (*x = 0; *x < MAZE_SIZE; (*x)++) {
    for (*y = 0; *y < MAZE_SIZE; (*y)++) {
      if (is_free_cell(g, *x, *y, type))
        return;
    }
  }

  //nao ha mais espaco
  return;
}

void init_ghosts(Game *g) {
  g->ghosts[RED] = (Ghost){{0, 0, -1, 0}, 1, 'R'};
  g->ghosts[BLUE] = (Ghost){{0, 0, -1, 0}, 2, 'B'};
  g->ghosts[GREEN] = (Ghost){{0, 0, -1, 0}, 3, 'G'};
  g->ghosts[YELLOW] = (Ghost){{0, 0, -1, 0}, 4, 'Y'};
}

char normalize_tile(char tile) {
  if (tile == 'X' || tile == 'x' || tile == '#')
    return 'X';
  if (tile >= '1' && tile <= '6')
    return tile;
  return '0';
}

void reset_game_state(Game *g) {
  g->score = 0;
  g->moves_count = 0;
  g->vision_radius = 1;
  g->prizes_collected = 0;
  g->green_prefers_left = 1;
}

int try_move(Game *g, Actor *b, int dx, int dy) {
  int nx = b->x + dx;
  int ny = b->y + dy;

  if (is_wall(g, nx, ny))
    return 0;

  b->x = nx;
  b->y = ny;
  b->dx = dx;
  b->dy = dy;
  return 1;
}

int direction_from_key(int ch, int *dx, int *dy) {
  switch (ch) {
  case KEY_UP:
  case 'w':
  case 'W':
    *dy = -1;
    *dx = 0;
    return 1;
  case KEY_DOWN:
  case 's':
  case 'S':
    *dy = 1;
    *dx = 0;
    return 1;
  case KEY_LEFT:
  case 'a':
  case 'A':
    *dy = 0;
    *dx = -1;
    return 1;
  case KEY_RIGHT:
  case 'd':
  case 'D':
    *dy = 0;
    *dx = 1;
    return 1;
  default:
    return 0;
  }
}

void rotate_left(int dx, int dy, int *out_dx, int *out_dy) {
  *out_dx = dy;
  *out_dy = -dx;
}

void rotate_right(int dx, int dy, int *out_dx, int *out_dy) {
  *out_dy = dx;
  *out_dx = -dy;
}

char **alloc_matrix(int l, int c) {
  char **m;

  m = malloc(sizeof(char*) * l) ;

  for (int i = 0; i < l; i++)
    m[i] = malloc(sizeof(char) * c) ;

  return m ;
}

void free_matrix(char **m, int l) {

  if(!m) {
    perror("erro free_matrix\n");
    exit(1);
  }

  for(int i = 0; i < l; i++){
    free(m[i]);
  }
  free(m);
}

void free_game(Game *g) {
   
  if(!g) {
    perror("erro free_game\n");
    exit(1);
  }

  free_matrix(g->maze, MAZE_SIZE);
  free(g->ghosts);

  free(g);
}
