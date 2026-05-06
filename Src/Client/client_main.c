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
  int n_read; 
  uint8_t seq, tipo_aux ;

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
    
    if (n_read < TAM_DADOS) {
      tipo_aux = FINAL_TYPE ;     
    }
    //crc nao implementado 
    sendMsg (sock, n_read, seq, tipo_aux, buffer, 0) ;

    seq++ ;
    if (seq == 64)
      seq = 0 ;

    printf("seq: %u \n", seq) ;

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

  send_file("../../Files/msg.txt", socket, DATA_TYPE) ;

  return 0;
}
