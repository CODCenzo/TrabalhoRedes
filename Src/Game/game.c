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

static const char *DEFAULT_UFPR[MAZE_SIZE] = {
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
};

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

void place_objects(Game *g, bool has_pacman, 
                   int has_ghosts[GHOSTS], int has_prizes[PRIZES]) {
  
  int i, x, y;

  if (!g) {
    perror("erro place_obj\n");
    exit(1);
  }

  if (!has_pacman) {
    random_free_position(g, &g->pacman.x, &g->pacman.y, PACMAN);
  }

  for (i = 0; i < GHOSTS; i++) {
    if (has_ghosts[i] == 0) {
      random_free_position(g, &g->ghosts[i].body.x, &g->ghosts[i].body.y, i);
    }
  }

  for (i = 0; i < PRIZES; i++) {
    if (!has_prizes[i]) {
      random_free_position(g, &x, &y, i +5);
      g->maze[y][x] = (char)('1' + i);
    }
  }
}

void load_default_level(Game *g) {
  int x, y;
  int has_ghosts[GHOSTS] = {0, 0, 0, 0};
  int has_prizes[PRIZES] = {0, 0, 0, 0, 0, 0};

  init_ghosts(g);
  reset_game_state(g);

  for (y = 0; y < MAZE_SIZE; y++) {
    for (x = 0; x < MAZE_SIZE; x++) {
      g->maze[y][x] = DEFAULT_UFPR[y][x] == 'X' ? 'X' : '0';
    }
  }

  place_objects(g, false, has_ghosts, has_prizes);
}

/*int load_csv_level(Game *g, const char *filepath) {
  FILE *file;
  char line[512];
  int y = 0;
  int x ;
  bool has_pacman = false;
  int has_ghosts[GHOSTS] = {0, 0, 0, 0};
  int has_prizes[PRIZES] = {0, 0, 0, 0, 0, 0};

  if (filepath == NULL)
    return 0;

  file = fopen(filepath, "r");
  if (file == NULL)
    return 0;

  init_ghosts(g);
  reset_game_state(g);

  while (y < MAZE_SIZE && fgets(line, sizeof(line), file) != NULL) {
    char *token = strtok(line, ";\n\r");
    x = 0;

    while (x < MAZE_SIZE) {
      char tile = token != NULL ? token[0] : 'X';

      if (tile == 'P') {
        g->pacman.y = y;
        g->pacman.x = x;
        has_pacman = true;
        g->maze[y][x] = '0';
      } else if (tile == 'R' || tile == 'B' || tile == 'G' || tile == 'Y') {
        int id = tile == 'R' ? RED : tile == 'B' ? BLUE : tile == 'G' ? GREEN : YELLOW;
        g->ghosts[id].body.y = y;
        g->ghosts[id].body.x = x;
        has_ghosts[id] = 1;
        g->maze[y][x] = '0';
      } else {
        g->maze[y][x] = normalize_tile(tile);
        if (g->maze[y][x] >= '1' && g->maze[y][x] <= '6')
          has_prizes[g->maze[y][x] - '1'] = 1;
      }

      x++;
      token = strtok(NULL, ";\n\r");
    }
    y++;
  }

  fclose(file);
  if (x != MAZE_SIZE)
    return 0;

  place_objects(g, has_pacman, has_ghosts, has_prizes);
  return 1;
}*/

void load_level(Game *g, const char *filepath) {

  if(!g) {
    perror("erro load_level\n");
    exit(1);
  }

  if (filepath != NULL) //&& load_csv_level(g, filepath))
    return;

  load_default_level(g);
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

bool visible_to_pacman(Game *g, int x, int y) {

  if (abs(g->pacman.y - y) <= g->vision_radius)
    if (abs(g->pacman.x - x) <= g->vision_radius)
      return true;
  return false;
}

void draw_tile(Game *g, int x, int y, int screen_x, int screen_y) {
  char tile = g->maze[y][x];

 /*if (!visible_to_pacman(g, x, y)) {
    mvaddch(screen_y, screen_x, ' ');
    return;
  }*/

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
  //mvprintw(LINES - 1, 1, "%s", last_event);
  refresh();
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

void ghost_wall_rule(Game *g, Ghost *ghost, int prefer_left) {
  int left_dy, left_dx, right_dy, right_dx;
  int dirs[4][2];
  int i;

  rotate_left(ghost->body.dx, ghost->body.dy, &left_dx, &left_dy);
  rotate_right(ghost->body.dx, ghost->body.dy, &right_dx, &right_dy);

  if (prefer_left) {
    dirs[0][0] = left_dx;
    dirs[0][1] = left_dy;
    dirs[2][0] = right_dx;
    dirs[2][1] = right_dy;
  } else {
    dirs[0][0] = right_dx;
    dirs[0][1] = right_dy;
    dirs[2][0] = left_dx;
    dirs[2][1] = left_dy;
  }
  dirs[1][0] = ghost->body.dx;
  dirs[1][1] = ghost->body.dy;
  dirs[3][0] = -ghost->body.dx;
  dirs[3][1] = -ghost->body.dy;

  for (i = 0; i < 4; i++) {
    if (try_move(g, &ghost->body, dirs[i][0], dirs[i][1]))
      return;
  }
}

void ghost_random(Game *g, Ghost *ghost) {
  const int dirs[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
  int options[4];
  int count = 0;
  int i, pick;

  for (i = 0; i < 4; i++) {
    if (!is_wall(g, ghost->body.x + dirs[i][0], ghost->body.y + dirs[i][1])) {

      options[count] = i;
      count++;
    }
  }
  if (count == 0)
    return;

  pick = options[rand() % count];
  try_move(g, &ghost->body, dirs[pick][0], dirs[pick][1]);
}

void update_ghosts(Game *g) {
  ghost_wall_rule(g, &g->ghosts[RED], 1);
  ghost_wall_rule(g, &g->ghosts[BLUE], 0);
  ghost_wall_rule(g, &g->ghosts[GREEN], g->green_prefers_left);
  g->green_prefers_left = !g->green_prefers_left;
  ghost_random(g, &g->ghosts[YELLOW]);
}

int check_collision(Game *g) {
  int i;

  if (!g) {
    perror("erro check_collision\n");
    exit(1);
  }

  for (i = 0; i < GHOSTS; i++) {
    if (g->pacman.y == g->ghosts[i].body.y && g->pacman.x == g->ghosts[i].body.x) {
     // snprintf(last_event, sizeof(last_event),
     //          "PacMan encontrou o fantasma %c. Arquivo: encontro.txt",
     //          ghosts[i].symbol);
      return 1;
    }
  }
  return 0;
}

void collect_prize(Game *g) {
  char tile = g->maze[g->pacman.y][g->pacman.x];

  if (tile >= '1' && tile <= '6') {
    //int id = tile - '1';
    g->maze[g->pacman.y][g->pacman.x] = '0';
    g->prizes_collected++;
    g->score += 100;
    //snprintf(last_event, sizeof(last_event), "Pastilha %c coletada. Premio: %s",
    //         tile, prize_files[id]);
  }
}

int play_round(Game *g, int ch) {

  int dx, dy;

  if (!g) {
    perror("erro play_round\n");
    exit(1);
  }

  if (!direction_from_key(ch, &dx, &dy))
    return 0;

  if (try_move(g, &g->pacman, dx, dy) == 0) {
    //strcpy(last_event, "Movimento bloqueado por parede.");
    return 0;
  }

  g->moves_count++;
  //cada 5 movimentos amplia 1 de visao 
  g->vision_radius = 1 + g->moves_count / 5;
  collect_prize(g);

  // Condicao de derrota
  if (check_collision(g))
    return -1;

  update_ghosts(g);
  if (check_collision(g) == 1)
    return -1;

  if (g->prizes_collected >= PRIZES) {
    //strcpy(last_event, "Fase concluida: 6 pastilhas douradas coletadas.");
    return 1;
  }

  return 0;
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

Game *init_game(){
  Game *g;

  g = malloc(sizeof(Game)) ;

  g->maze = alloc_matrix(MAZE_SIZE, MAZE_SIZE);
  g->ghosts = malloc(sizeof(Ghost) *GHOSTS);
  //g->last_event = malloc(sizeof(char) *120);

  g->score = 0;
  g->moves_count = 0;
  g->vision_radius = 100; 
  g->prizes_collected = 0;
  g->green_prefers_left = 1;

  return g;
}

void free_game(Game *g) {
   
  if(!g) {
    perror("erro free_game\n");
    exit(1);
  }

  free_matrix(g->maze, MAZE_SIZE);
  free(g->ghosts);
  //free(g->last_event);

  free(g);
}


int main(int argc, char **argv) {
  int ch;
  int result = 0;
  bool running = true;
  Game *g;
  const char *maze_file = argc > 1 ? argv[1] : NULL;
  
  //const char *prize_files[PRIZES] = {"1.txt", "2.txt", "3.jpg",
  //                                   "4.jpg", "5.mp4", "6.mp4"};

  g = init_game();

  //srand((unsigned int)time(NULL));
  srand(300);
  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  timeout(-1);
  init_colors();
  load_level(g, maze_file);

  while (running) {

    draw_game(g);

    if (result < 0)
      show_end_screen("FIM DE JOGO");
    else if (result > 0)
      show_end_screen("VOCE VENCEU!");

    ch = getch();

    if (ch == 'q' || ch == 'Q') {
      running = false;
    } 
    else if (ch == 'r' || ch == 'R') {
      load_level(g, maze_file);
      result = 0;
    } 
    else if (result == 0) {
      result = play_round(g, ch);
    }
  }

  endwin();
  free_game(g) ;

  return 0;
}
