#include "../../Headers/draw.h"

// Desenha o caractere
void draw_tile(Game *g, int x, int y, int screen_x, int screen_y) {
  char tile = g->maze[y][x];

 if (!visible_to_pacman(g, x, y)) {
    mvaddch(screen_y, screen_x, ' ');
    return;
  }

  if (tile == 'X') {
    attron(COLOR_PAIR(6) | A_BOLD);
    mvaddch(screen_y, screen_x, ACS_CKBOARD);
    attroff(COLOR_PAIR(6) | A_BOLD);
  } else if (tile >= '1' && tile <= '6') {
    attron(COLOR_PAIR(5) | A_BOLD);
    mvaddch(screen_y, screen_x, tile);
    attroff(COLOR_PAIR(5) | A_BOLD);
  } else {
    mvaddch(screen_y, screen_x, '.');
  }
}

void draw_game(Game *g) {
  int x, y, i;
  int view_h = LINES - 4;
  int view_w = COLS - 2;
  int top = 2;
  int left;
  int start_y = 0;
  int start_x = 0;

  erase();

  if (view_h <= 0 || view_w <= 0) {
    mvprintw(1, 2, "Aumente o terminal.");
    refresh();
    return;
  }

  if (view_h > MAZE_SIZE)
    view_h = MAZE_SIZE;
  if (view_w > MAZE_SIZE)
    view_w = MAZE_SIZE;

  if (g->pacman.x < start_x)
    start_x = g->pacman.x;
  if (g->pacman.x >= start_x + view_w)
    start_x = g->pacman.x - view_w + 1;
  if (g->pacman.y < start_y)
    start_y = g->pacman.y;
  if (g->pacman.y >= start_y + view_h)
    start_y = g->pacman.y - view_h + 1;

  if (start_y + view_h > MAZE_SIZE)
    start_y = MAZE_SIZE - view_h;
  if (start_x + view_w > MAZE_SIZE)
    start_x = MAZE_SIZE - view_w;
  if (start_y < 0)
    start_y = 0;
  if (start_x < 0)
    start_x = 0;

  left = (COLS - view_w) / 2;

  attron(A_BOLD);
  mvprintw(0, 1,
           "Rodada: %d  Visao: %d  Pastilhas douradas: %d/%d  Score: %d",
           g->moves_count, g->vision_radius, g->prizes_collected, PRIZES, g->score);
  attroff(A_BOLD);

  for (x = 0; x < view_w; x++) {
    for (y = 0; y < view_h; y++)
      draw_tile(g, start_x + x, start_y + y, left + x, top + y);
  }

  for (i = 0; i < GHOSTS; i++) {
    int gx = g->ghosts[i].body.x - start_x;
    int gy = g->ghosts[i].body.y - start_y;

    if (gy >= 0 && gy < view_h && gx >= 0 && gx < view_w &&
        visible_to_pacman(g, g->ghosts[i].body.x, g->ghosts[i].body.y)) {
      attron(COLOR_PAIR(g->ghosts[i].color_pair) | A_BOLD);
      mvaddch(top + gy, left + gx, g->ghosts[i].symbol);
      attroff(COLOR_PAIR(g->ghosts[i].color_pair) | A_BOLD);
    }
  }

  attron(COLOR_PAIR(5) | A_BOLD);
  mvaddch(top + g->pacman.y - start_y, left + g->pacman.x - start_x, 'P');
  attroff(COLOR_PAIR(5) | A_BOLD);

  mvprintw(LINES - 2, 1, "Setas/WASD movem | r reinicia | q sai");
  refresh();
}

void show_end_screen(const char *title) {
  int center_y = LINES / 2;
  int center_x = COLS / 2;

  attron(A_BOLD);
  mvprintw(center_y - 1, center_x - (int)strlen(title) / 2, "%s", title);
  attroff(A_BOLD);
  //mvprintw(center_y + 1, 1, "%s | r reinicia | q sai", last_event);
  refresh();
}

void init_colors() {
  if (!has_colors())
    return;

  start_color();
  use_default_colors();
  init_pair(1, COLOR_RED, -1);
  init_pair(2, COLOR_BLUE, -1);
  init_pair(3, COLOR_GREEN, -1);
  init_pair(4, COLOR_YELLOW, -1);
  init_pair(5, COLOR_YELLOW, -1);
  init_pair(6, COLOR_CYAN, -1);
  init_pair(7, COLOR_WHITE, -1);
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