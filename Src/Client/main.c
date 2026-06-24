#include "../../Headers/game.h"
#include "../../Headers/client.h"
#include "../../Headers/draw.h"
#include "../../Headers/game_protocol.h"
#include "../../Headers/kermit.h"


int main(int argc, char **argv) {

  int socket = cria_raw_socket(argv[1]) ;
  if (socket < 0) {
    fprintf(stderr, "Erro ao criar socket\n");
    return -1;
  }

  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  timeout(-1);
  init_colors();
  
  client_loop(socket) ;

  endwin();
  close(socket) ;

  return 0;
}
