#include "game.h"
#include "client.h"
#include "draw.h"
#include "../../Headers/game_protocol.h"

int main() {

  bool running = true;
  char ch;
  
  const char *prize_files[PRIZES] = {"1.txt", "2.txt", "3.jpg",
                                     "4.jpg", "5.mp4", "6.mp4"};

  char matrix[MAZE_SIZE][MAZE_SIZE + 1]; 

  int socket = cria_socket() ;
  
  while (running) {

    /*recebe e desenha matriz*/
    receive_matrix(socket, matrix) ;
    draw_game_client(matrix) ;
  

    if (true) {

      show_end_screen("FIM DE JOGO");
      /*mensagem para o client*/
      send_end_screen(socket) ;
    }

    ch = getch();
    /*envia tecla */
    send_key(socket, &ch) ;

  }

  endwin();
  close(socket) ;

  return 0;
}
