#include "../../Headers/kermit.h"
#include <time.h>

// Constrói e preenche o frame. O tamanhoFrame min é 4 sempre e o máximo é 35
unsigned char* buildMsg(unsigned char *bufferDados, uint8_t tamDados, uint8_t seq,
                 uint8_t type, uint8_t crc) {

	//Verifica os parâmetros da função
	if (!bufferDados) {
		perror("Erro buildMsg, buffer vazio\n");
		return NULL;
	} 
	else if (tamDados > 31) {
		perror("Erro buildMsg, o tamanho da mensagem é maior que 31\n");
		return NULL;
	}
	else if (seq > 63) {
		perror("Erro buildMsg, o campo sequência é maior que 63\n");
		return NULL;
	}
	else if (type > 31) {
		perror("Erro buildMsg, o campo type é maior que 31\n");
		return NULL;
	}

	unsigned char *bufferFrame = malloc (tamDados + 4);
	if (!bufferFrame) {
		perror("Erro buildMsg, erro ao alocar frame\n");
		return NULL;
	}

	// Byte 0: Marcador de inicio(8 bits), 01111110 ou 7e
	memset(bufferFrame, 0x7e, 1) ;

	// Byte 1: TAM(5 bits MSB) | SEQ Alta(3 bits LSB)
  bufferFrame[1] = ((tamDados & 0x1F) << 3) | ((seq >> 3) & 0x07);

  // Byte 2: SEQ Baixa(3 bits MSB) | TYPE(5 bits LSB)
  bufferFrame[2] = ((seq & 0x07) << 5) | (type & 0x1F);

	// Campo Dados
	memcpy(bufferFrame + 3, bufferDados ,tamDados);

	// Campo CRC (8bits)
	bufferFrame[3 + tamDados] = crc;

  return bufferFrame;
}

// Retorna um valor de 8 bits entre min e max, incluindo eles mesmos
uint8_t gera_byte_aleat (uint8_t min, uint8_t max) {
  return rand() % (max - min +1) + min;
}

// Constrói e envia a mensagem.
int sendMsg (int socket, uint8_t tamDados, uint8_t sequencia, uint8_t tipo, unsigned char *dadosMsg, uint8_t crc) {
	srand(time(NULL));

	//DADOS:
	// Min = 0, max = 31
	// Não é possível enviar mensagens pelo LO menores que 10 (10 +4 para o frame)

  unsigned char *frameCompleto = buildMsg(dadosMsg, tamDados, sequencia, tipo, crc) ;

	if (!frameCompleto) {
		perror("ERRO AO CRIAR FRAME\n");
		return -1;
	}

	printf("FRAME CONSTRUÍDO COM SUCESSO\n");
	unsigned int tamFrameCompleto = tamDados + 4; 

	if (send(socket, frameCompleto, tamFrameCompleto, 0) == -1) {
		perror("ERRO AO ENVIAR FRAME\n");
		return -1;
	}

	printf("FRAME ENVIADO COM SUCESSO!\n");
	printf("IMPRIMINDO FRAME ENVIADO: \n");
	
	for (int i = 0; i < tamFrameCompleto; i ++) {
		printf("%02x ", frameCompleto[i]);
	}
	printf("\n");
	
	free(frameCompleto);

  return 0;
}


