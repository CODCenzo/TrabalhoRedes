#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "socket.h"

// 4 bytes campos + 31 bytes de dados
#define MAX_FRAME_SIZE 35

struct kermit {
	uint8_t tamDados ; // 5 bits
	uint8_t seq ; // 6 bits
	uint8_t type ;// 5 bits
  unsigned char *dados ;
  uint8_t crc ; // 8 bits
} ;

/*
  recebe o buffer e o seu numero de bytes utilizados
  e retorna dados na estrutura
*/
struct kermit parsing_kermit(unsigned char buffer[MAX_FRAME_SIZE], int tamDados);

/*
  !em construcao
  loop de recebimento, recebe os dados e processa ;
*/
int loopDeCaptura(int sock, unsigned char bufferDeCaptura[MAX_FRAME_SIZE]);
