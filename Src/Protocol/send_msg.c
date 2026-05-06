#include "../../Headers/kermit.h"
#include <time.h>
#include "../../Headers/timeout.h"

void imprimeFrame (unsigned char *bufferFrame, int tamFrameCompleto) {
	printf("IMPRIMINDO FRAME ENVIADO: \n");
	for (int i = 0; i < tamFrameCompleto; i ++) {
		printf("%02x ", bufferFrame[i]);
	}
	printf("\n");
}

// Constrói e preenche o frame. O tamanhoFrame min é 4 sempre e o máximo é 35
unsigned char* buildFrame(unsigned char *bufferDados, uint8_t tamDados, uint8_t seq,
                 uint8_t type, uint8_t crc) {

	//Verifica os parâmetros da função
	if (!bufferDados && tamDados > 0 ) {
		perror("Erro buildFrame, buffer vazio\n");
		return NULL;
	} 
	else if (tamDados > 31) {
		perror("Erro buildFrame, o tamanho da mensagem é maior que 31\n");
		return NULL;
	}
	else if (seq > 63) {
		perror("Erro buildFrame, o campo sequência é maior que 63\n");
		return NULL;
	}
	else if (type > 31) {
		perror("Erro buildFrame, o campo type é maior que 31\n");
		return NULL;
	}

	unsigned char *bufferFrame = malloc (tamDados + 4);
	if (!bufferFrame) {
		perror("Erro buildFrame, erro ao alocar frame\n");
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

// Envia mensagem e espera por ACK
// Falta implementar validação(CRC)
int sendMsg (int socket, uint8_t tamDados, uint8_t sequencia, uint8_t tipo, unsigned char *dadosMsg, uint8_t crc) {
	
	// Constrói o frame para o envio
	unsigned char *frameCompleto = buildFrame(dadosMsg, tamDados, sequencia, tipo, crc);
	unsigned int tamFrameCompleto = tamDados + 4; 
	if (!frameCompleto) {
		perror("ERRO AO CRIAR FRAME\n");
		return -1;
	}
	printf("FRAME CONSTRUÍDO COM SUCESSO\n");

	//DADOS:
	// Min = 0, max = 31
	// Não é possível enviar mensagens pelo LO menores que 10 (10 +4 para o frame)
	unsigned char *bufferCaptura = malloc(MAX_FRAME_SIZE);
	if (!bufferCaptura) {
		perror("Erro ao alocar buffer de captura\n");
		return -1;
	}

	//int tentativasEnvio = 0;
	//do {
		if (send(socket, frameCompleto, tamFrameCompleto, 0) == -1) {
			perror("ERRO AO ENVIAR FRAME\n");
			return -1;
		}
		printf("FRAME ENVIADO COM SUCESSO!\n");
		// imprimeFrame(frameCompleto, tamFrameCompleto);

		// Lógica para receber resposta
		//int tamanhoCapturado = recebe_mensagem(socket, TIMEOUT_MILLIS, bufferCaptura, MAX_FRAME_SIZE);

		// Analise da resposta
		//if (tamanhoCapturado > 0) {
			//struct kermit *resposta = parsing_kermit(bufferCaptura, tamanhoCapturado);

			//if (resposta->type == ACK_TYPE && resposta->seq == sequencia) {
				//printf("ACK RECEBIDO\n");
				//free(frameCompleto);
				//return 1;
			//} else if (resposta->type == NACK_TYPE) {
			//	printf("NACK RECEBIDO, ENVIANDO MSG NOVAMENTE\n");
			//}
		//} else {
			//printf("NENHUM DADO FOI LIDO (TIMEOUT), ENVIANDO MSG NOVAMENTE\n");
		//}
		//tentativasEnvio ++;
	//} while (tentativasEnvio < MAX_TENTATIVAS_ENVIO);

	free(frameCompleto);
	free(bufferCaptura);

  return -1;
}
