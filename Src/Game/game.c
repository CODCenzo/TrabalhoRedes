#include "game.h"
#include "aux_game.h"

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

void collect_prize(Game *g) {
  char tile = g->maze[g->pacman.y][g->pacman.x];

  if (tile >= '1' && tile <= '6') {

    g->maze[g->pacman.y][g->pacman.x] = '0';
    g->prizes_collected++;
    g->score += 100;
    /*funcao que avisa que vai vir um arquivo*/
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
