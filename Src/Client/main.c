#include "game.h"
#include "client.h"
#include "draw.h"
#include "../../Headers/game_protocol.h"

int main(int argc, char **argv) {

  int socket = cria_socket(argv[1]) ;
  if (socket < 0) {
    fprintf(stderr, "Erro ao criar socket\n");
    return -1;
  }
  
  client_loop(socket) ;

  endwin();
  close(socket) ;

  return 0;
}
