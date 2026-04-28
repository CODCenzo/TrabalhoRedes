#include <stdlib.h> 
#include <stdio.h>
#include <string.h> 
#include <stdint.h>
#include <time.h>

#include "send.h"

#define BUFFER_SIZE 255

int main(int argc, char *argv[]) {

  FILE *f ;
  long msg_size, left, n_buf, n_read ;
  unsigned char *pkt ;

	srand(time(NULL));

	int8_t tam_buffer = 255;

	unsigned char *buffer = malloc(sizeof(unsigned char) * tam_buffer);
	if (!buffer) {
		perror("Erro ao alocar mensagem\n");
		exit(1);
	}

	if (argc < 2) {
		printf("Uso: sudo %s <nome_da_interface>\n", argv[0]);
		printf("Exemplo: sudo %s eth0\n", argv[0]);
		return 1;
	}

  /*implementar aqui a logica de mandar msg continuamente*/

	// Cria um socket a partir do nome da interface
	int sock = cria_raw_socket(argv[1]);

  f = fopen("msg.txt", "rb") ;
  if (!f) {
    perror("erro ao abrir arquivo\n") ;
    exit(1) ;
  }
  fseek(f, 0, SEEK_END) ;
  msg_size = ftell(f) ;
  fseek(f, 0, SEEK_SET) ;
  
  left = msg_size ;
  while (left > 0) {

    memset(buffer, 0, BUFFER_SIZE) ;
    n_buf = (left > BUFFER_SIZE) ? BUFFER_SIZE : left ;
    n_read = fread(buffer, 1, n_buf, f) ;
    if (n_read == 0) break ;

    pkt = build_kermit(buffer, n_buf, 1, 1, 1) ;

    //manda o pacote 
    if (send(sock, pkt, n_buf +27, 0) == -1) {
      perror("ERRO AO ENVIAR FRAME\n");
      exit(1);
    }

    left -= n_buf ;
  }
  fclose(f) ;

	printf("FRAME ENVIADO COM SUCESSO\n");
  /*logica terminaria aqui */
	
  close(sock);
	free(buffer);

  return 0;
}


