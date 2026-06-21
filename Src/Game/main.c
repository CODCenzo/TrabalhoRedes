#include "game.h"

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
