#include <stdio.h>

#include "../../Headers/kermit.h"
#include "../../Headers/socket.h"
#include "../../Headers/timeout.h"

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

	unsigned char *bufferDeCaptura = malloc(MAX_FRAME_SIZE);
  if (!bufferDeCaptura) {
    perror("erro ao alocar buffer de captura\n") ;
  }

	// Loop de espera e tratamento
  while (1) {
		// Recebe mensagens e envia resposta
		int tamanhoCapturado = recebe_mensagem(socket, TIMEOUT_MILLIS, bufferDeCaptura, MAX_FRAME_SIZE);

		if (tamanhoCapturado > 0) {
			struct kermit *resposta = parsing_kermit(bufferDeCaptura, tamanhoCapturado);
			printf("MENSAGEM RECEBIDA, TIPO: %d\n", resposta->type);

			unsigned char *bufferDados = malloc(DEFAULT_MSG_SIZE);

			if (!bufferDados) {
				perror("Erro ao alocar mensagem\n");
				exit(1);
			}
			// Preenche o buffer de dados com uma mensagem aleatória
			memset(bufferDados, gera_byte_aleat(0,255), DEFAULT_MSG_SIZE);

			unsigned char *frameCompleto = buildFrame(bufferDados, DEFAULT_MSG_SIZE, 0,ACK_TYPE,1);

			send(socket, frameCompleto, DEFAULT_MSG_SIZE + 4, 0);

			free(bufferDados);
			free(frameCompleto);
		}

  }
	free(bufferDeCaptura);
  return 0;
}

