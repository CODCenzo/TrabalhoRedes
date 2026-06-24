#include "../../Headers/game_protocol.h"
#include "../Game/client.h"
#include "../Game/game.h"
#include "../Game/draw.h"



int send_game_file (int socket, int type, int prize) {

  const char *prize_files[6] = {"1.txt", "2.txt", "3.jpg",
                                "4.jpg", "5.mp4", "6.mp4"} ;

  sendMsg(socket, 0, 0, INITIAL_FILE_TYPE, NULL) ;
  sendMsg(socket, 0, 0, type, NULL) ;
  
  send_file(socket, prize_files[prize], type) ;

  return 0 ;
}

int receive_game_file (int socket, int type) {

  const char *prize_files[6] = {"1.txt", "2.txt", "3.jpg",
                                "4.jpg", "5.mp4", "6.mp4"} ;


  receive_file(socket, prize_files[type]) ;

  return 0 ;
}

/*funcao que envia matriz*/
int send_matrix(int socket, char m[MAZE_SIZE][MAZE_SIZE + 1]) {

  unsigned char *buf ;
  uint32_t size = MAZE_SIZE * (MAZE_SIZE + 1) * sizeof(char) ;

  buf = (unsigned char *)&m[0][0] ;

  if (socket < 0 || m == NULL) {
    fprintf(stderr, "SEND_Matrix: parametro invalido\n");
    return -1;
  }

  int ret = send_buffer(socket, buf, size, (uint8_t)DATA_TYPE, (uint8_t)DATA_TYPE);
  printf("SEND_Matrix: enviado %d\n", ret);

  return ret;
}

/*recebe matriz*/
/*a matriz passada recebe os dados*/
int receive_matrix(int socket, char m[MAZE_SIZE][MAZE_SIZE + 1]) {
  
  unsigned char *buf ;
  uint32_t size = MAZE_SIZE * (MAZE_SIZE + 1) * sizeof(char) ;
  size_t received_size = 0 ;

  buf = (unsigned char *)&m[0][0] ;

  if (socket < 0 || m == NULL) {
    fprintf(stderr, "RECEIVE_Matrix: parametro invalido\n");
    return -1;
  }

  int ret = receive_buffer(socket, buf, size, &received_size);
  printf("RECEIVE_Matrix: recebido %d \n", ret);

  return ret;
}

/*funcao que manda tecla*/
int send_key(int socket, char key) {

  unsigned char *buf ;
  uint32_t size = sizeof(int) ;
  uint8_t key_type;

  buf = (unsigned char *)&key ;

  if (socket < 0) {
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

  return ret;
}

/*funcao que recebe tecla*/
/*int receive_key(int socket, char *key) {

  unsigned char *buf ;
  uint32_t size = sizeof(int) ;
  size_t received_size = 0 ;

  buf = (unsigned char *)key ;

  if (socket < 0 ) {
    fprintf(stderr, "RECEIVE_KEY: parametro invalido\n");
    return -1;
  }

  int ret = receive_buffer(socket, buf, size, &received_size);
  printf("RECEIVE_KEY: recebido %d \n", ret);

  return ret;
}*/

/*funcao que manda tela de fim*/
int send_end_screen(int socket) {

  
  //unsigned char buf[MAX_FRAME_SIZE] ;


  if (socket < 0) {
    fprintf(stderr, "SEND_END_SCREEN: parametro invalido\n");
    return -1;
  }

  int ret = sendMsg(socket, 0, 0, SHOW_TYPE, NULL) ;
  
  //arrumar depois
  wait_response(socket, 0) ;


  printf("SEND_END_SCREEN: enviado %d\n", ret);

  return ret;
}

/*funcao que recebe tela de fim*/
/*int receive_end_screen(int socket) {

  unsigned char *buf ;
  size_t received_size = 0 ;

  if (socket < 0 ) {
    fprintf(stderr, "RECEIVE_END_SCREEN: parametro invalido\n");
    return -1;
  }

  int ret = receive_buffer(socket, buf, 0, &received_size);
  printf("RECEIVE_END_SCREEN: recebido %d \n", ret);

  free(buf);
  return ret;
}*/

int client_checa_pacote(int socket) {

  struct kermit *k ;
  int ret ;

  unsigned char buf[MAX_FRAME_SIZE] ;

  if (socket < 0 ) {
    fprintf(stderr, "CHECA_PACOTE: parametro invalido\n");
    return -1;
  }
  
  do {
    ret = recebe_mensagem(socket, TIMEOUT_MILLIS, buf, MAX_FRAME_SIZE);
  } while (ret == -1) ;

  k = parsing_kermit(buf, ret) ;
  sendMsg(socket, 0, k->seq, k->type, NULL);
  printf("CHECA_PACOTE: recebido %d \n", k->type);

  switch (k->type) {
    case SHOW_TYPE:
      return SHOW_TYPE ;
    case DATA_TYPE:
      return DATA_TYPE ;
    default:
      return -1 ;
  }

  free(buf);
  return ret;
}

int server_checa_pacote(int socket) {

  struct kermit *k ;
  int ret ;

  unsigned char buf[MAX_FRAME_SIZE] ;

  if (socket < 0) {
    fprintf(stderr, "CHECA_PACOTE: parametro invalido\n");
    return -1;
  }
  
  do {
    ret = recebe_mensagem(socket, TIMEOUT_MILLIS, buf, MAX_FRAME_SIZE);
  } while (ret == -1) ;

  k = parsing_kermit(buf, ret) ;
  sendMsg(socket, 0, k->seq, k->type, NULL);
  printf("CHECA_PACOTE: recebido %d \n", k->type);

  switch (k->type) {
    case MOVE_UP_TYPE:
      return MOVE_UP_TYPE ;
    case MOVE_DOWN_TYPE:
      return MOVE_DOWN_TYPE ;
    case MOVE_LEFT_TYPE:
      return MOVE_LEFT_TYPE ;
    case MOVE_RIGHT_TYPE:
      return MOVE_RIGHT_TYPE ;
    case QUIT_TYPE:
      return QUIT_TYPE ;
    case RESTART_TYPE:
      return RESTART_TYPE ;
    default:
      return -1 ;
  }

  free(buf);
  return ret;
}


int client_loop(int socket) {

  int type ;
  char matrix[MAZE_SIZE][MAZE_SIZE + 1] ;

  do {
    type = client_checa_pacote(socket) ;

    switch (type)
    {
    case DATA_TYPE:
      receive_matrix(socket, matrix) ;
      draw_game_client(matrix) ;
      break;
    
    default:
      break;
    }
  } while (type != SHOW_TYPE) ;

  show_end_screen("FIM DE JOGO");

  return 0 ;
}

/*int server_loop(int socket) {

  int type ;
  char matrix[MAZE_SIZE][MAZE_SIZE + 1] ;

  do {
    type = server_checa_pacote(socket) ;

    switch (type)
    {
    case MOVE_UP_TYPE:
      return MOVE_UP_TYPE ;
      break;
    case MOVE_DOWN_TYPE:
      return MOVE_DOWN_TYPE ;
      break;
    case MOVE_LEFT_TYPE:
      return MOVE_LEFT_TYPE ;
      break;
    case MOVE_RIGHT_TYPE:
      return MOVE_RIGHT_TYPE ;
      break;
    case QUIT_TYPE:
      return QUIT_TYPE ;
      break;
    case RESTART_TYPE:
      return RESTART_TYPE ;
      break;
    
    default:
      break;
    }
  } while (type != QUIT_TYPE) ;


  return 0 ;
}*/
