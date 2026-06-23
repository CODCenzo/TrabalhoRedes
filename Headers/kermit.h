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

#define MI 0x7e // Marcador de início
#define SEQ_MODULO 64 // Limite da sequencia de mensagens

#define MAX_FRAME_SIZE 35 // 4 bytes campos + 31 bytes de dados
#define MIN_FRAME_SIZE 14 // 4 bytes campos + 10 bytes de dados ou padding

#define MAX_TENTATIVAS_ENVIO 25

#define MAX_DADOS 31
#define MIN_DADOS 10

#define ACK_TYPE 0
#define NACK_TYPE 1
#define FINAL_TYPE 16

#define MOVE_UP_TYPE 12
#define MOVE_DOWN_TYPE 13
#define MOVE_RIGHT_TYPE 10
#define MOVE_LEFT_TYPE 11

#define DEFAULT_CRC 1

#define DEFAULT_MSG_SIZE 10

int cria_raw_socket(char* nome_interface_rede);

struct kermit {
	uint8_t tamDados ; // 5 bits
	uint8_t seq ; // 6 bits
	uint8_t type ;// 5 bits
  unsigned char *dados ;
  uint8_t crc ; // 8 bits
} ;

void print_kermit(struct kermit *k);

void kermit_free(struct kermit *k);

void imprimeFrame (unsigned char *bufferFrame, int tamFrameCompleto);

// Utiliza o PG=0x07 para calcular o CRC do buffer.
uint8_t calculaCRC8(const unsigned char *data, int tamData);

// Aloca e preenche o frame com seguindo o protocolo.
// Retorna char*, e NULL em caso de erro
unsigned char* buildFrame(unsigned char *bufferDados, uint8_t tamDados, uint8_t seq,
                          uint8_t type);

// Aloca e preenche uma estrutura kermit com os dados do buffer capturado.
// Retorna kermit*, e NULL em caso de erro
struct kermit *parsing_kermit(unsigned char *bufferCapturado, int tamCaptura);

// Contrói o frame e envia pelo socket. Não trata de ACK ou NACK
// Retorna 1, e em caso de erro -1
int sendMsg (int socket, uint8_t tamDados, uint8_t sequencia, uint8_t tipo, 
            unsigned char *dadosMsg);

// Retorna o tempo do sistema em milissegundos
long long timestamp();

// Verifica a validade do pacote capturado a partir do MI e CRC.
// Retorna 1, e 0 caso seja inválido
int protocolo_e_valido(unsigned char* buffer, int tamanho_buffer);

// Loop que lê o socket em busca de pacotes válidos
// retorna -1 se deu timeout, ou quantidade de bytes lidos
int recebe_mensagem(int soquete, int timeoutMillis, unsigned char* buffer, 
                    int tamanho_buffer);

#endif
