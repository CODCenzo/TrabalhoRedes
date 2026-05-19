#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAZE_SIZE 40
#define GHOSTS 4
#define PRIZES 6
#define MIN_COLS 30
#define MIN_LINES 12

enum { RED = 0, BLUE = 1, GREEN = 2, YELLOW = 3 };

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
  "X0000X0000X00XXXXX000XXXXXX00XXXXXX0000X",
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

static char maze[MAZE_SIZE][MAZE_SIZE + 1];
static Actor pacman;
static Ghost ghosts[GHOSTS];
static int score;
static int moves_count;
static int vision_radius;
static int prizes_collected;
static int green_prefers_left = 1;
static char last_event[120] = "Pegue as pastilhas 1..6.";
static const char *prize_files[PRIZES] = {"1.txt", "2.txt", "3.jpg",
                                          "4.jpg", "5.mp4", "6.mp4"};

static int is_inside(int y, int x) {
  return y >= 0 && y < MAZE_SIZE && x >= 0 && x < MAZE_SIZE;
}

static int is_wall(int y, int x) {
  return !is_inside(y, x) || maze[y][x] == 'X';
}

static int is_free_cell(int y, int x) {
  int i;

  if (is_wall(y, x))
    return 0;
  if (pacman.y == y && pacman.x == x)
    return 0;
  for (i = 0; i < GHOSTS; i++) {
    if (ghosts[i].body.y == y && ghosts[i].body.x == x)
      return 0;
  }
  return maze[y][x] == '0';
}

static void random_free_position(int *y, int *x) {
  int attempts;

  for (attempts = 0; attempts < 5000; attempts++) {
    *y = rand() % MAZE_SIZE;
    *x = rand() % MAZE_SIZE;
    if (is_free_cell(*y, *x))
      return;
  }

  for (*y = 0; *y < MAZE_SIZE; (*y)++) {
    for (*x = 0; *x < MAZE_SIZE; (*x)++) {
      if (is_free_cell(*y, *x))
        return;
    }
  }

  *y = 1;
  *x = 1;
  maze[*y][*x] = '0';
}

static void init_ghosts(void) {
  ghosts[RED] = (Ghost){{0, 0, -1, 0}, 1, 'R'};
  ghosts[BLUE] = (Ghost){{0, 0, -1, 0}, 2, 'B'};
  ghosts[GREEN] = (Ghost){{0, 0, -1, 0}, 3, 'G'};
  ghosts[YELLOW] = (Ghost){{0, 0, -1, 0}, 4, 'Y'};
}

static char normalize_tile(char tile) {
  if (tile == 'X' || tile == 'x' || tile == '#')
    return 'X';
  if (tile >= '1' && tile <= '6')
    return tile;
  return '0';
}

static void reset_game_state(void) {
  score = 0;
  moves_count = 0;
  vision_radius = 1;
  prizes_collected = 0;
  green_prefers_left = 1;
  strcpy(last_event, "Pegue as pastilhas 1..6.");
}

static void place_missing_actors_and_prizes(int has_pacman, int has_ghosts[GHOSTS],
                                            int has_prizes[PRIZES]) {
  int i, y, x;

  if (!has_pacman) {
    random_free_position(&pacman.y, &pacman.x);
  }

  for (i = 0; i < GHOSTS; i++) {
    if (!has_ghosts[i]) {
      random_free_position(&ghosts[i].body.y, &ghosts[i].body.x);
    }
  }

  for (i = 0; i < PRIZES; i++) {
    if (!has_prizes[i]) {
      random_free_position(&y, &x);
      maze[y][x] = (char)('1' + i);
    }
  }
}

static void load_default_level(void) {
  int y, x;
  int has_ghosts[GHOSTS] = {0, 0, 0, 0};
  int has_prizes[PRIZES] = {0, 0, 0, 0, 0, 0};

  init_ghosts();
  reset_game_state();

  for (y = 0; y < MAZE_SIZE; y++) {
    for (x = 0; x < MAZE_SIZE; x++) {
      maze[y][x] = DEFAULT_UFPR[y][x] == 'X' ? 'X' : '0';
    }
    maze[y][MAZE_SIZE] = '\0';
  }

  pacman.y = pacman.x = -1;
  place_missing_actors_and_prizes(0, has_ghosts, has_prizes);
}

static int load_csv_level(const char *filepath) {
  FILE *file;
  char line[512];
  int y = 0;
  int has_pacman = 0;
  int has_ghosts[GHOSTS] = {0, 0, 0, 0};
  int has_prizes[PRIZES] = {0, 0, 0, 0, 0, 0};

  if (filepath == NULL)
    return 0;

  file = fopen(filepath, "r");
  if (file == NULL)
    return 0;

  init_ghosts();
  reset_game_state();

  while (y < MAZE_SIZE && fgets(line, sizeof(line), file) != NULL) {
    char *token = strtok(line, ";\n\r");
    int x = 0;

    while (x < MAZE_SIZE) {
      char tile = token != NULL ? token[0] : 'X';

      if (tile == 'P') {
        pacman.y = y;
        pacman.x = x;
        has_pacman = 1;
        maze[y][x] = '0';
      } else if (tile == 'R' || tile == 'B' || tile == 'G' || tile == 'Y') {
        int id = tile == 'R' ? RED : tile == 'B' ? BLUE : tile == 'G' ? GREEN : YELLOW;
        ghosts[id].body.y = y;
        ghosts[id].body.x = x;
        has_ghosts[id] = 1;
        maze[y][x] = '0';
      } else {
        maze[y][x] = normalize_tile(tile);
        if (maze[y][x] >= '1' && maze[y][x] <= '6')
          has_prizes[maze[y][x] - '1'] = 1;
      }

      x++;
      token = strtok(NULL, ";\n\r");
    }
    maze[y][MAZE_SIZE] = '\0';
    y++;
  }

  fclose(file);
  if (y != MAZE_SIZE)
    return 0;

  place_missing_actors_and_prizes(has_pacman, has_ghosts, has_prizes);
  return 1;
}

static void load_level(const char *filepath) {
  if (filepath != NULL && load_csv_level(filepath))
    return;

  load_default_level();
}

static void init_colors(void) {
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

static int visible_to_pacman(int y, int x) {
  return abs(pacman.y - y) <= vision_radius && abs(pacman.x - x) <= vision_radius;
}

static void draw_tile(int y, int x, int screen_y, int screen_x) {
  char tile = maze[y][x];

  if (!visible_to_pacman(y, x)) {
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

static void draw_game(void) {
  int y, x, i;
  int view_h = LINES - 5;
  int view_w = COLS - 2;
  int top = 2;
  int left;
  int start_y;
  int start_x;

  erase();

  if (LINES < MIN_LINES || COLS < MIN_COLS) {
    mvprintw(1, 2, "Aumente o terminal para pelo menos %dx%d.", MIN_COLS,
             MIN_LINES);
    refresh();
    return;
  }

  if (view_h > MAZE_SIZE)
    view_h = MAZE_SIZE;
  if (view_w > MAZE_SIZE)
    view_w = MAZE_SIZE;

  start_y = pacman.y - view_h / 2;
  start_x = pacman.x - view_w / 2;
  if (start_y < 0)
    start_y = 0;
  if (start_x < 0)
    start_x = 0;
  if (start_y + view_h > MAZE_SIZE)
    start_y = MAZE_SIZE - view_h;
  if (start_x + view_w > MAZE_SIZE)
    start_x = MAZE_SIZE - view_w;

  left = (COLS - view_w) / 2;

  attron(A_BOLD);
  mvprintw(0, 1,
           "Rodada: %d  Visao: %d  Pastilhas douradas: %d/%d  Score: %d",
           moves_count, vision_radius, prizes_collected, PRIZES, score);
  attroff(A_BOLD);

  for (y = 0; y < view_h; y++) {
    for (x = 0; x < view_w; x++)
      draw_tile(start_y + y, start_x + x, top + y, left + x);
  }

  for (i = 0; i < GHOSTS; i++) {
    int gy = ghosts[i].body.y - start_y;
    int gx = ghosts[i].body.x - start_x;

    if (gy >= 0 && gy < view_h && gx >= 0 && gx < view_w &&
        visible_to_pacman(ghosts[i].body.y, ghosts[i].body.x)) {
      attron(COLOR_PAIR(ghosts[i].color_pair) | A_BOLD);
      mvaddch(top + gy, left + gx, ghosts[i].symbol);
      attroff(COLOR_PAIR(ghosts[i].color_pair) | A_BOLD);
    }
  }

  attron(COLOR_PAIR(5) | A_BOLD);
  mvaddch(top + pacman.y - start_y, left + pacman.x - start_x, 'C');
  attroff(COLOR_PAIR(5) | A_BOLD);

  mvprintw(LINES - 2, 1, "Setas/WASD movem | r reinicia | q sai");
  mvprintw(LINES - 1, 1, "%s", last_event);
  refresh();
}

static int try_move(Actor *actor, int dy, int dx) {
  int ny = actor->y + dy;
  int nx = actor->x + dx;

  if (is_wall(ny, nx))
    return 0;

  actor->y = ny;
  actor->x = nx;
  actor->dy = dy;
  actor->dx = dx;
  return 1;
}

static int direction_from_key(int ch, int *dy, int *dx) {
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

static void rotate_left(int dy, int dx, int *out_dy, int *out_dx) {
  *out_dy = -dx;
  *out_dx = dy;
}

static void rotate_right(int dy, int dx, int *out_dy, int *out_dx) {
  *out_dy = dx;
  *out_dx = -dy;
}

static void ghost_wall_rule(Ghost *ghost, int prefer_left) {
  int left_dy, left_dx, right_dy, right_dx;
  int dirs[4][2];
  int i;

  rotate_left(ghost->body.dy, ghost->body.dx, &left_dy, &left_dx);
  rotate_right(ghost->body.dy, ghost->body.dx, &right_dy, &right_dx);

  if (prefer_left) {
    dirs[0][0] = left_dy;
    dirs[0][1] = left_dx;
    dirs[2][0] = right_dy;
    dirs[2][1] = right_dx;
  } else {
    dirs[0][0] = right_dy;
    dirs[0][1] = right_dx;
    dirs[2][0] = left_dy;
    dirs[2][1] = left_dx;
  }
  dirs[1][0] = ghost->body.dy;
  dirs[1][1] = ghost->body.dx;
  dirs[3][0] = -ghost->body.dy;
  dirs[3][1] = -ghost->body.dx;

  for (i = 0; i < 4; i++) {
    if (try_move(&ghost->body, dirs[i][0], dirs[i][1]))
      return;
  }
}

static void ghost_random(Ghost *ghost) {
  const int dirs[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
  int options[4];
  int count = 0;
  int i, pick;

  for (i = 0; i < 4; i++) {
    if (!is_wall(ghost->body.y + dirs[i][0], ghost->body.x + dirs[i][1]))
      options[count++] = i;
  }
  if (count == 0)
    return;

  pick = options[rand() % count];
  try_move(&ghost->body, dirs[pick][0], dirs[pick][1]);
}

static void update_ghosts(void) {
  ghost_wall_rule(&ghosts[RED], 1);
  ghost_wall_rule(&ghosts[BLUE], 0);
  ghost_wall_rule(&ghosts[GREEN], green_prefers_left);
  green_prefers_left = !green_prefers_left;
  ghost_random(&ghosts[YELLOW]);
}

static int check_collision(void) {
  int i;

  for (i = 0; i < GHOSTS; i++) {
    if (pacman.y == ghosts[i].body.y && pacman.x == ghosts[i].body.x) {
      snprintf(last_event, sizeof(last_event),
               "PacMan encontrou o fantasma %c. Arquivo: encontro.txt",
               ghosts[i].symbol);
      return 1;
    }
  }
  return 0;
}

static void collect_prize(void) {
  char tile = maze[pacman.y][pacman.x];

  if (tile >= '1' && tile <= '6') {
    int id = tile - '1';
    maze[pacman.y][pacman.x] = '0';
    prizes_collected++;
    score += 100;
    snprintf(last_event, sizeof(last_event), "Pastilha %c coletada. Premio: %s",
             tile, prize_files[id]);
  }
}

static int play_round(int ch) {
  int dy, dx;

  if (!direction_from_key(ch, &dy, &dx))
    return 0;

  if (!try_move(&pacman, dy, dx)) {
    strcpy(last_event, "Movimento bloqueado por parede.");
    return 0;
  }

  moves_count++;
  vision_radius = 1 + moves_count / 5;
  collect_prize();

  if (check_collision())
    return -1;

  update_ghosts();
  if (check_collision())
    return -1;

  if (prizes_collected >= PRIZES) {
    strcpy(last_event, "Fase concluida: 6 pastilhas douradas coletadas.");
    return 1;
  }

  return 0;
}

static void show_end_screen(const char *title) {
  int center_y = LINES / 2;
  int center_x = COLS / 2;

  attron(A_BOLD);
  mvprintw(center_y - 1, center_x - (int)strlen(title) / 2, "%s", title);
  attroff(A_BOLD);
  mvprintw(center_y + 1, 1, "%s | r reinicia | q sai", last_event);
  refresh();
}

int main(int argc, char **argv) {
  int ch;
  int result = 0;
  int running = 1;
  const char *maze_file = argc > 1 ? argv[1] : NULL;

  srand((unsigned int)time(NULL));
  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  timeout(-1);
  init_colors();
  load_level(maze_file);

  while (running) {
    draw_game();
    if (result < 0)
      show_end_screen("FIM DE JOGO");
    else if (result > 0)
      show_end_screen("VOCE VENCEU!");

    ch = getch();

    if (ch == 'q' || ch == 'Q') {
      running = 0;
    } else if (ch == 'r' || ch == 'R') {
      load_level(maze_file);
      result = 0;
    } else if (result == 0) {
      result = play_round(ch);
    }
  }

  endwin();
  return 0;
}
