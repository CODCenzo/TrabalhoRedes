#include "../../Headers/kermit_builder.h"

// Retorna um valor de 8 bits entre min e max, incluindo eles mesmos
uint8_t gera_byte_aleat (uint8_t min, uint8_t max) {
  return rand() % (max - min +1) + min;
}

// Constrói e preenche o frame. O tamanhoFrame min é 4 sempre e o máximo é 35
unsigned char* build_kermit(unsigned char *bufferDados, uint8_t tamDados, uint8_t seq,
                 uint8_t type, uint8_t crc) {

	//Verifica os parâmetros da função
	if (!bufferDados) {
		perror("Erro build_kermit, buffer vazio\n");
		return NULL;
	} 
	else if (tamDados > 31) {
		perror("Erro build_kermit, o tamanho da mensagem é maior que 31\n");
		return NULL;
	}
	else if (seq > 63) {
		perror("Erro build_kermit, o campo sequência é maior que 63\n");
		return NULL;
	}
	else if (type > 31) {
		perror("Erro build_kermit, o campo type é maior que 31\n");
		return NULL;
	}

	unsigned char *bufferFrame = malloc (tamDados + 4);
	if (!bufferFrame) {
		perror("Erro build_kermit, erro ao alocar frame\n");
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

