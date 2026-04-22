#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "socket.h"

#define TAM_FRAME 1514

struct kermit {

	uint8_t tam, seq, type ;
  unsigned char *dados ;
  uint8_t crc ;
} ;

/*recebe o buffer e o seu numero de bytes utilizados
  e retorna dados na estrutura*/
struct kermit parsing_kermit(unsigned char buffer[TAM_FRAME], int tam) ;

/*
  !em construcao
  loop de recebimento, recebe os dados e processa ;
*/
int loop_recv(int sock, unsigned char buffer[TAM_FRAME]) ;
