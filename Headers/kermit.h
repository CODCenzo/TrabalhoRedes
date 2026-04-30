#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// 4 bytes campos + 31 bytes de dados
#define MAX_FRAME_SIZE 35

struct kermit {
	uint8_t tamDados ; // 5 bits
	uint8_t seq ; // 6 bits
	uint8_t type ;// 5 bits
  unsigned char *dados ;
  uint8_t crc ; // 8 bits
} ;

// Cria uma estrutura kermit a partir do buffer capturado
struct kermit *parsing_kermit(unsigned char bufferCapturado[MAX_FRAME_SIZE], int tamCaptura); 

// Loop infinito que filtra as mensagens
// Retorna NULL em caso de timeout ou erro
struct kermit *loopDeCaptura(int sock);

// Constrói e preenche o frame
// Retorna um buffer com a mensagem completa
unsigned char* build_kermit(unsigned char *buffer_dados, uint8_t tamMsg, uint8_t seq,
                 uint8_t type, uint8_t crc) ; 

// Envia a mensagem seguindo o protocolo kermit
int sendMsg (int socket, uint8_t tamDados, uint8_t sequencia, uint8_t tipo, unsigned char *dadosMsg, uint8_t crc);

// Retorna um valor de 8 bits entre min e max, incluindo eles mesmos
uint8_t gera_byte_aleat (uint8_t min, uint8_t max) ;
