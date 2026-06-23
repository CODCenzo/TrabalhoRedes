#include "game.h"
#include "client.h"
#include "draw.h"
#include "../../Headers/game_protocol.h"

int main(int argc, char **argv) {
  int ch;
  int result = 0;
  bool running = true;
  Game *g;
  const char *maze_file = argc > 1 ? argv[1] : NULL;
  
  //const char *prize_files[PRIZES] = {"1.txt", "2.txt", "3.jpg",
  //                                   "4.jpg", "5.mp4", "6.mp4"};

  char matrix[MAZE_SIZE][MAZE_SIZE + 1]; 

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

  int socket = cria_socket() ;
  
  while (running) {

    draw_game(g);
    /* manda matrixpara o client desenhar*/
    draw_game_client(matrix);
    send_matrix(socket, matrix) ;
  

    if (result < 0) {

      show_end_screen("FIM DE JOGO");
      /*mensagem para o client*/
      send_end_screen(socket) ;
    }
    else if (result > 0) {
      show_end_screen("VOCE VENCEU!");
      /*mensagem para o client*/
      send_end_screen(socket) ;
    }

    ch = getch();
    /*Pegar tecla do client*/
    receive_key(socket, &ch) ;

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
  close(socket) ;

  return 0;
}
