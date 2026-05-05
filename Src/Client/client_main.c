#include <stdio.h>

#include "../../Headers/kermit.h"
#include "../../Headers/socket.h"

// Retorna um valor de 8 bits entre min e max, incluindo eles mesmos
uint8_t gera_byte_aleat (uint8_t min, uint8_t max) {
  return rand() % (max - min +1) + min;
}

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
			free(bufferDados);
    }
  }

  return 0;
}