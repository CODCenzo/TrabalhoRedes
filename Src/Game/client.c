#include "client.h"

void build_client_matrix(Game *g, char out[MAZE_SIZE][MAZE_SIZE + 1]) {
  int x, y, i;

  if (!g) {
    perror("erro build_client_matrix\n");
    exit(1);
  }

  for (y = 0; y < MAZE_SIZE; y++) {
    for (x = 0; x < MAZE_SIZE; x++) {
      if (!visible_to_pacman(g, x, y)) {
        out[y][x] = ' ';
      } else if (g->maze[y][x] == 'X') {
        out[y][x] = 'X';
      } else if (g->maze[y][x] >= '1' && g->maze[y][x] <= '6') {
        out[y][x] = g->maze[y][x];
      } else {
        out[y][x] = '.';
      }
    }

    out[y][MAZE_SIZE] = '\0';
  }

  for (i = 0; i < GHOSTS; i++) {
    int gx = g->ghosts[i].body.x;
    int gy = g->ghosts[i].body.y;

    if (is_inside(gx, gy) && visible_to_pacman(g, gx, gy)) {
      out[gy][gx] = g->ghosts[i].symbol;
    }
  }

  if (is_inside(g->pacman.x, g->pacman.y)) {
    out[g->pacman.y][g->pacman.x] = 'P';
  }
}

void draw_client_tile(char tile, int screen_x, int screen_y) {

  if (tile == ' ') {
    mvaddch(screen_y, screen_x, ' ');
  } 
  else if (tile == 'X') {
    attron(COLOR_PAIR(6) | A_BOLD);
    mvaddch(screen_y, screen_x, ACS_CKBOARD);
    attroff(COLOR_PAIR(6) | A_BOLD);
  } 
  else if (tile >= '1' && tile <= '6') {
    attron(COLOR_PAIR(5) | A_BOLD);
    mvaddch(screen_y, screen_x, tile);
    attroff(COLOR_PAIR(5) | A_BOLD);
  } 
  else if (tile == 'R') {
    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddch(screen_y, screen_x, tile);
    attroff(COLOR_PAIR(1) | A_BOLD);
  } 
  else if (tile == 'B') {
    attron(COLOR_PAIR(2) | A_BOLD);
    mvaddch(screen_y, screen_x, tile);
    attroff(COLOR_PAIR(2) | A_BOLD);
  } 
  else if (tile == 'G') {
    attron(COLOR_PAIR(3) | A_BOLD);
    mvaddch(screen_y, screen_x, tile);
    attroff(COLOR_PAIR(3) | A_BOLD);
  } 
  else if (tile == 'Y' || tile == 'P') {
    attron(COLOR_PAIR(5) | A_BOLD);
    mvaddch(screen_y, screen_x, tile);
    attroff(COLOR_PAIR(5) | A_BOLD);
  } 
  else {
    mvaddch(screen_y, screen_x, tile);
  }
}

void draw_game_client(char matrix[MAZE_SIZE][MAZE_SIZE + 1]) {
  int x, y;
  int top = 2;
  int left = (COLS - MAZE_SIZE) / 2;

  erase();

  for (y = 0; y < MAZE_SIZE; y++) {
    for (x = 0; x < MAZE_SIZE; x++) {
      draw_client_tile(matrix[y][x], left + x, top + y);
    }
  }

  refresh();
}