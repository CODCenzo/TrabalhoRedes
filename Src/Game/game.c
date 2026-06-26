#include "../../Headers/game.h"
#include "../../Headers/default.h"

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

void load_level(Game *g, const char *filepath) {

  if(!g) {
    perror("erro load_level\n");
    exit(1);
  }

  if (filepath != NULL) //&& load_csv_level(g, filepath))
    return;

  load_default_level(g);
}

bool visible_to_pacman(Game *g, int x, int y) {

  if (abs(g->pacman.y - y) <= g->vision_radius)
    if (abs(g->pacman.x - x) <= g->vision_radius)
      return true;
  return false;
}

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
      return 1;
    }
  }
  return 0;
}

int collect_prize(Game *g) {

  char tile = g->maze[g->pacman.y][g->pacman.x];
  int num;

  if (tile >= '1' && tile <= '6') {

    g->maze[g->pacman.y][g->pacman.x] = '0';
    g->prizes_collected++;
    g->score += 100;

    num = (int)(tile - '0');
    //funcao que envia mensagem de premio coletado para o cliente
    return num;

  }
  return -1;
}

int play_round(Game *g, int ch, int *prize) {

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
  *prize = collect_prize(g);

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

Game *init_game(){
  Game *g;

  g = malloc(sizeof(Game)) ;

  g->maze = alloc_matrix(MAZE_SIZE, MAZE_SIZE);
  g->ghosts = malloc(sizeof(Ghost) *GHOSTS);
  //g->last_event = malloc(sizeof(char) *120);

  g->score = 0;
  g->moves_count = 0;
  g->vision_radius = 1; 
  g->prizes_collected = 0;
  g->green_prefers_left = 1;

  return g;
}

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
