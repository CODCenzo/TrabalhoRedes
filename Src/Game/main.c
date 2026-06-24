#include "../../Headers/game.h"
#include "../../Headers/client.h"
#include "../../Headers/draw.h"
#include "../../Headers/socket.h"
#include "../../Headers/game_protocol.h"
#include "../../Headers/protocol.h"

int main(int argc, char **argv) {
  int aux;
  char ch;
  int result = 0;
  bool running = true;
  Game *g;
  const char *maze_file = argc > 2 ? argv[2] : NULL;
  
  char matrix[MAZE_SIZE][MAZE_SIZE + 1]; 

  g = init_game();

  //srand((unsigned int)time(NULL));
  srand(300);
  // initscr();
  // raw();
  // noecho();
  // curs_set(0);
  // keypad(stdscr, TRUE);
  // timeout(-1);
  // init_colors();
  load_level(g, maze_file);

  int socket = cria_raw_socket(argv[1]) ;
  if (socket < 0) {
    fprintf(stderr, "Erro ao criar socket\n");
    return -1;
  }
  
  while (running) {

    //draw_game(g);
    /* manda matrixpara o client desenhar*/

    build_client_matrix(g, matrix);
    // draw_game_client(matrix);
    send_matrix(socket, matrix) ;
  

    if (result < 0) {

      // show_end_screen("FIM DE JOGO");
      /*mensagem para o client*/
      //send_end_screen(socket) ;
    }
    else if (result > 0) {
      // show_end_screen("VOCE VENCEU!");
      /*mensagem para o client*/
      //send_end_screen(socket) ;
    }

    //aux = server_checa_pacote(socket) ;

    /*if (aux == MOVE_UP_TYPE) {
      ch = 'w';
    }
    else if (aux == MOVE_DOWN_TYPE) {
      ch = 's';
    }
    else if (aux == MOVE_LEFT_TYPE) {
      ch = 'a';
    }
    else if (aux == MOVE_RIGHT_TYPE) {
      ch = 'd';
    }
    else if (aux == QUIT_TYPE) {
      ch = 'q';
    }
    else if (aux == RESTART_TYPE) {
      ch = 'r';
    }

    /Pegar tecla do client/
    //ch = getch();
         
    if (ch == 'q' || ch == 'Q') {
      running = false;
    } 
    else if (ch == 'r' || ch == 'R') {
      load_level(g, maze_file);
      result = 0;
    } 
    else if (result == 0) {
      result = play_round(g, ch);
    }*/ 

    running = false;
  }

  // endwin();
  free_game(g) ;
  close(socket) ;

  return 0;
}
