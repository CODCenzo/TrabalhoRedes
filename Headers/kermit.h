#ifndef _KERMIT_H_
#define _KERMIT_H_

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <sys/time.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// 4 bytes campos + 31 bytes de dados
#define MAX_FRAME_SIZE 35
#define MIN_FRAME_SIZE 4
#define MAX_TENTATIVAS_ENVIO 5
#define MAX_DADOS 31
#define MIN_DADOS 10

#define ACK_TYPE 0
#define NACK_TYPE 1
#define DATA_TYPE 4
#define FINAL_TYPE 16

#define DEFAULT_MSG_SIZE 10

#define TIMEOUT_MILLIS 200

struct kermit {
	uint8_t tamDados ; // 5 bits
	uint8_t seq ; // 6 bits
	uint8_t type ;// 5 bits
  unsigned char *dados ;
  uint8_t crc ; // 8 bits
} ;

void print_kermit(struct kermit *k);

void imprimeFrame (unsigned char *bufferFrame, int tamFrameCompleto);

// Aloca e preenche uma estrutura kermit com os dados do buffer 
struct kermit *parsing_kermit(unsigned char *bufferCapturado, int tamCaptura);

// Constrói e preenche o frame. O tamanhoFrame min é 4 sempre e o máximo é 35
// Falta implementar CRC
unsigned char* buildFrame(unsigned char *bufferDados, uint8_t tamDados, uint8_t seq,
                          uint8_t type, uint8_t crc);

// Contrói o frame e envia pelo socket. Não trata de ACK ou NACK
// Não é possível enviar mensagens menores que 14
int sendMsg (int socket, uint8_t tamDados, uint8_t sequencia, uint8_t tipo, 
            unsigned char *dadosMsg, uint8_t crc);

// Retorna o tempo do sistema em milissegundos
long long timestamp();

// Verifica tamanho do buffer e marcador inicíal
int protocolo_e_valido(unsigned char* buffer, int tamanho_buffer);

// Loop que lê o socket em busca de pacotes válidos
// retorna -1 se deu timeout, ou quantidade de bytes lidos
int recebe_mensagem(int soquete, int timeoutMillis, unsigned char* buffer, int tamanho_buffer);

#endif
