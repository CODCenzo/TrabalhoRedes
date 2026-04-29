#include "../../Headers/kermit_builder.h"

int sendMsg (int socket, uint8_t tamDados, uint8_t sequencia, uint8_t tipo, unsigned char *DadosMsg, uint8_t crc) {
	srand(time(NULL));

	//DADOS:
	// Min = 0, max = 31
	// Não é possível enviar mensagens pelo LO menores que 10 (10 +4 para o frame)

	unsigned char *bufferDados = malloc(tamDados);
	if (!bufferDados) {
		perror("Erro ao alocar mensagem\n");
		exit(1);
	}

	// Preenche o buffer de dados com uma mensagem aleatória
	memset(bufferDados, gera_byte_aleat(0,255), tamDados);

	// Cria um socket a partir do nome da interface
	int sock = cria_raw_socket(argv[1]);

  /*numeros arbitrarios de teste*/
  unsigned char *frameCompleto = build_kermit(bufferDados, tamDados, 2, 5, 6) ;
	if (!frameCompleto) {
		perror("ERRO AO CRIAR FRAME\n");
		exit(1);
	}

	printf("FRAME CONSTRUÍDO COM SUCESSO\n");
	unsigned int tamFrameCompleto = tamDados + 4; 

	// Envia o frame comleto
	if (send(sock, frameCompleto, tamFrameCompleto, 0) == -1) {
		perror("ERRO AO ENVIAR FRAME\n");
		exit(1);
	}

	printf("FRAME ENVIADO COM SUCESSO!\n");
	printf("IMPRIMINDO FRAME ENVIADO: \n");
	for (int i = 0; i < tamFrameCompleto; i ++) {
		printf("%02x ", frameCompleto[i]);
	}
	printf("\n");
	
  close(sock);
	free(bufferDados);
	free(frameCompleto);

  return 0;
}


