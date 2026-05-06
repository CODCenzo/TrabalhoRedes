#include <stdio.h>

#include "../../Headers/kermit.h"
#include "../../Headers/socket.h"

#define TAM_DADOS 31

// Retorna um valor de 8 bits entre min e max, incluindo eles mesmos
uint8_t gera_byte_aleat (uint8_t min, uint8_t max) {
  return rand() % (max - min +1) + min;
}

int send_file(const char *filepath, int sock, int tipo) {
  
  unsigned char buffer[TAM_DADOS] ;
 // unsigned char *frameAck ;
  FILE *f ;
  int n_read, seq, tipo_aux ;

  f = fopen(filepath, "rb") ;

  if (!sock || !filepath || !f) {
    perror("erro send_file\n") ;
    exit(1) ;
  }

  tipo_aux = tipo ;
  seq = 0 ;
  n_read = TAM_DADOS ;
  while (n_read == TAM_DADOS) {

    n_read = fread(buffer, 1, TAM_DADOS, f) ;

    for (int i = 0; i < TAM_DADOS; i++) {
      printf("%c", buffer[i]) ;
    }
    printf("\n") ;
    
    if (n_read < TAM_DADOS) {
      tipo_aux = FINAL_TYPE ;     
    }
    //crc nao implementado 
    sendMsg (sock, n_read, seq, tipo_aux, buffer, 0) ;

    /*seq++ ;
    if (seq == 64)
      seq = 0 ;*/

    printf("aqui\n") ;
  }

  fclose(f) ;
  return 0 ;
}

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Uso: sudo %s <nome_da_interface>\n", argv[0]);
		printf("Exemplo: sudo %s eth0\n", argv[0]);
		return 1;
	}  

	int socket = cria_raw_socket(argv[1]);

	//Loop de envio de mensagem
      /*//Send input and wait for ACK
			unsigned char *bufferDados = malloc(DEFAULT_MSG_SIZE);
			
			if (!bufferDados) {
			perror("Erro ao alocar mensagem\n");
			exit(1);
			}
			// Preenche o buffer de dados com uma mensagem aleatória
			memset(bufferDados, gera_byte_aleat(0,255), DEFAULT_MSG_SIZE);

			if (sendMsg(socket, DEFAULT_MSG_SIZE, 0, 10, bufferDados, 5) == -1) {
				printf("Erro ao enviar mensagem, máximo de tentativas excedido\n");
			}
			free(bufferDados);*/
  send_file("../../Files/msg.txt", socket, 5) ;

  return 0;
}
