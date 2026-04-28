#include <stdlib.h> 
#include <stdio.h>
#include <string.h> 
#include <stdint.h>
#include <time.h>

#include "send.h"

int main(int argc, char *argv[]) {

	srand(time(NULL));

	//Valor usado para teste
	uint8_t tamanhoMensagem = 15;

	unsigned char *bufferMensagem = malloc(tamanhoMensagem);
	if (!bufferMensagem) {
		perror("Erro ao alocar mensagem\n");
		exit(1);
	}

	// Preenche o buffer de dados com uma mensagem aleatória
	memset(bufferMensagem, gera_byte_aleat(0,255), tamanhoMensagem);

	if (argc < 2) {
		printf("Uso: sudo %s <nome_da_interface>\n", argv[0]);
		printf("Exemplo: sudo %s eth0\n", argv[0]);
		return 1;
	}

	// Cria um socket a partir do nome da interface
	int sock = cria_raw_socket(argv[1]);

  /*numeros arbitrarios de teste*/
  unsigned char *frameCompleto = build_kermit(bufferMensagem, tamanhoMensagem, 2, 5, 6) ;
	if (!frameCompleto) {
		perror("ERRO AO CRIAR FRAME\n");
		exit(1);
	}

	printf("FRAME CONSTRUÍDO COM SUCESSO\n");
	unsigned int tamFrameCompleto = tamanhoMensagem + 4; 

	//SEND BUFFER
	if (send(sock, frameCompleto, sizeof(unsigned char) * tamFrameCompleto, 0) == -1) {
		perror("ERRO AO ENVIAR FRAME\n");
		exit(1);
	}

	printf("FRAME ENVIADO COM SUCESSO\n");
	
  close(sock);
	free(bufferMensagem);
	free(frameCompleto);

  return 0;
}


