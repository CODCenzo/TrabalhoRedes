#include <stdio.h>

#include "../../Headers/kermit.h"
#include "../../Headers/socket.h"
#include "../../Headers/timeout.h"

// Retorna um valor de 8 bits entre min e max, incluindo eles mesmos
uint8_t gera_byte_aleat (uint8_t min, uint8_t max) {
  return rand() % (max - min +1) + min;
}

int recive_file(int sock) {

  uint8_t cont ;
  uint8_t aux_type ;
  struct kermit *pack ;
  FILE *f ;
  const char *filepath = "saida.txt" ;
  //unsigned char *frameAck ;

  f = fopen(filepath, "w") ;
  if (!f) {
    perror("erro na abertura/criacao de arquivo \n") ;
    exit(1) ;
  }

  cont = 0 ;
  do {

    // Pega os pacotes
    do {
      pack = loopDeCaptura(sock) ; 
      printf("pack->seq: %u\n", (unsigned int) pack->seq) ;

      // Dropa o pacote
      if (cont != pack->seq)
        free(pack) ;

    } while (cont != pack->seq) ;

    print_kermit(*pack) ;

    // Escreve no arquivo saida.txt e dropa o pacote
    fwrite((char *) pack->dados, sizeof(char), pack->tamDados, f) ;

    aux_type = pack->type ;
    free(pack) ;
    cont++ ;

  } while (aux_type != FINAL_TYPE ) ;
  //tipo para fim da trasmissao == 16
  printf("cont: %d\n", cont) ;

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

