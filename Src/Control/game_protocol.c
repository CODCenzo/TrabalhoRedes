#include "../../Headers/game_protocol.h"


int send_game_file (int socket, int type, int prize) {

  const char *prize_files[6] = {"1.txt", "2.txt", "3.jpg",
                                "4.jpg", "5.mp4", "6.mp4"} ;

  sendMsg(socket, 0, 0, INITIAL_FILE_TYPE, NULL) ;
  sendMsg(socket, 0, 0, type, NULL) ;
  
  send_file(socket, prize_files[prize], type) ;

  return 0 ;
}

int recive_game_file (int socket) {

  int type ;
  struct kermit *k ;
  unsigned char buff[33] ;
  

  const char *prize_files[6] = {"1.txt", "2.txt", "3.jpg",
                                "4.jpg", "5.mp4", "6.mp4"} ;

  recebe_mensagem(socket, TIMEOUT_MILLIS, buff, 33) ;

  k = parsing_kermit(buff, 33) ;

  receive_file(socket, prize_files[k->type]) ;

  return 0 ;
}