#include <stdio.h>

#include "../../Headers/kermit.h"
#include "../../Headers/socket.h"

#define ACK_TYPE 0
#define NACK_TYPE 0

#define DEFAULT_MSG_SIZE 10


int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Uso: sudo %s <nome_da_interface>\n", argv[0]);
		printf("Exemplo: sudo %s eth0\n", argv[0]);
		return 1;
	}  

	int socket = cria_raw_socket(argv[1]);

  int input;

	//Loop de envio de mensagem
  while (1) {
    scanf("%d", &input);

    if (input == 1) {
      //Send input and wait for ACK
				unsigned char *bufferDados = malloc(10);
				
				if (!bufferDados) {
				perror("Erro ao alocar mensagem\n");
				exit(1);
				}
				// Preenche o buffer de dados com uma mensagem aleatória
				memset(bufferDados, gera_byte_aleat(0,255), 10);

				if (sendMsg(socket, DEFAULT_MSG_SIZE, 0, 10, bufferDados, 0) == -1) {
					perror("ERRO AO ENVIAR MENSAGEM\n");
				}	else {
					// Espera do ACK
					struct kermit *k = loopDeCaptura(socket);
					if (k->type == ACK_TYPE) {
						perror("ACK RECEBIDO\n");
						break;
					}
				}

    }
  }


  return 0;
}