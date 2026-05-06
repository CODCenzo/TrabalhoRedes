#include <stdio.h>

#include "../../Headers/kermit.h"
#include "../../Headers/socket.h"
#include "../../Headers/timeout.h"

// Retorna um valor de 8 bits entre min e max, incluindo eles mesmos
uint8_t gera_byte_aleat (uint8_t min, uint8_t max) {
  return rand() % (max - min +1) + min;
}

int recive_file(int sock) {

  struct kermit *pack ;
  FILE *f ;
  const char *filepath = "saida.txt" ;
  //unsigned char *frameAck ;

  f = fopen(filepath, "w") ;
  if (!f) {
    perror("erro na abertura/criacao de arquivo \n") ;
    exit(1) ;
  }

  do {

    // Pega os pacotes
    pack = loopDeCaptura(sock) ;

    // Escreve no arquivo saida.txt 
    fwrite((char *) pack->dados, sizeof(char), pack->tamDados, f) ;

    /*frameAck = buildFrame(NULL, 0, pack->seq, ACK_TYPE, 1);
    send(sock, frameAck, MIN_FRAME_SIZE, 0) ;*/

    printf("aqui\n") ;

  } while (pack->type != FINAL_TYPE ) ;
  //tipo para fim da trasmissao == 16

  return 0 ;
}


int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Uso: sudo %s <nome_da_interface>\n", argv[0]);
		printf("Exemplo: sudo %s eth0\n", argv[0]);
		return 1;
	}  

	int socket = cria_raw_socket(argv[1]);

  recive_file(socket) ;

  return 0;

  
}

