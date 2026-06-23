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

/*funcao que envia matriz*/
int send_matrix(int socket, char m[MAZE_SIZE][MAZE_SIZE + 1]) {

  char *buf ;
  uint32_t size = MAZE_SIZE * (MAZE_SIZE + 1) * sizeof(char) ;

  buf = &m[0][0] ;

  if (socket < 0 == NULL) {
    fprintf(stderr, "SEND_Matrix: parametro invalido\n");
    return -1;
  }

  int ret = send_buffer(socket, buf, size, (uint8_t)DATA_TYPE, (uint8_t)DATA_TYPE);
  printf("SEND_Matrix: enviado %d\n", ret);

  free(buf);
  return ret;
}

/*recebe matriz*/
/*a matriz passada recebe os dados*/
int receive_matrix(int socket, char m[MAZE_SIZE][MAZE_SIZE + 1]) {
  
  char *buf ;
  uint32_t size = MAZE_SIZE * (MAZE_SIZE + 1) * sizeof(char) ;
  size_t received_size = 0 ;

  buf = &m[0][0] ;

  if (socket < 0 == NULL) {
    fprintf(stderr, "RECEIVE_Matrix: parametro invalido\n");
    return -1;
  }

  int ret = receive_buffer(socket, buf, size, &received_size);
  printf("RECEIVE_Matrix: recebido %d \n", ret);

  free(buf);
  return ret;
}

/*funcao que manda tecla*/
int send_key(int socket, char key) {

  unsigned char *buf ;
  uint32_t size = sizeof(int) ;
  uint8_t key_type;

  buf = (unsigned char *)key ;

  if (socket < 0 == NULL) {
    fprintf(stderr, "SEND_KEY: parametro invalido\n");
    return -1;
  }

  if(key == 'w' || key == 'W') {
    key_type = MOVE_UP_TYPE ;
  }
  else if(key == 'a' || key == 'A') {
    key_type = MOVE_LEFT_TYPE ;
  }
  else if(key == 's' || key == 'S') {
    key_type = MOVE_DOWN_TYPE ;
  }
  else if(key == 'd' || key == 'D') {
    key_type = MOVE_RIGHT_TYPE ;
  }
  else if(key == 'q' || key == 'Q') {
    key_type = QUIT_TYPE ;
  }
  else if(key == 'r' || key == 'R') {
    key_type = RESTART_TYPE ;
  }
  else {
    fprintf(stderr, "SEND_KEY: tecla invalida\n");
    return -1;
  }

  int ret = send_buffer(socket, buf, size, (uint8_t)key_type, (uint8_t)key_type);
  printf("SEND_KEY: enviado %d\n", ret);

  free(buf);
  return ret;
}

/*funcao que recebe tecla*/
int receive_key(int socket, char *key) {

  unsigned char *buf ;
  uint32_t size = sizeof(int) ;
  size_t received_size = 0 ;

  buf = (unsigned char *)key ;

  if (socket < 0 == NULL) {
    fprintf(stderr, "RECEIVE_KEY: parametro invalido\n");
    return -1;
  }

  int ret = receive_buffer(socket, buf, size, &received_size);
  printf("RECEIVE_KEY: recebido %d \n", ret);

  free(buf);
  return ret;
}

/*funcao que manda tela de fim*/
int send_end_screen(int socket) {

  unsigned char *buf ;


  if (socket < 0 == NULL) {
    fprintf(stderr, "SEND_END_SCREEN: parametro invalido\n");
    return -1;
  }

  int ret = send_buffer(socket, buf, 0, (uint8_t)FINAL_TYPE, (uint8_t)FINAL_TYPE);
  printf("SEND_END_SCREEN: enviado %d\n", ret);

  free(buf);
  return ret;
}

/*funcao que recebe tela de fim*/
int receive_end_screen(int socket) {

  unsigned char *buf ;
  size_t received_size = 0 ;

  if (socket < 0 == NULL) {
    fprintf(stderr, "RECEIVE_END_SCREEN: parametro invalido\n");
    return -1;
  }

  int ret = receive_buffer(socket, buf, 0, &received_size);
  printf("RECEIVE_END_SCREEN: recebido %d \n", ret);

  free(buf);
  return ret;
}


