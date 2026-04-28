#include <stdlib.h> 
#include <stdio.h>
#include <string.h> 
#include <stdint.h>
#include <time.h>
#include "../util/socket.h"

#include "send.h"

// Retorna um valor de 8 bits entre min e max, incluindo eles mesmos
uint8_t gera_byte_aleat (uint8_t min, uint8_t max) {
  return rand() % (max - min +1) + min;
}

// Constrói e preenche o frame.
// nao aloca o buffer
unsigned char* build_kermit(unsigned char *buffer_dados, uint8_t tamMsg, uint8_t seq,
                 uint8_t type, uint8_t crc) {

	//Verifica os parâmetros da função
	if (!buffer_dados) {
		perror("Erro build_kermit, buffer vazio\n");
		return NULL;
	} 
	else if (tamMsg > 31) {
		perror("Erro build_kermit, o tamanho da mensagem não é compatível\n");
		return NULL;
	}
	else if (seq > 63) {
		perror("Erro build_kermit, o campo sequência não é compatível\n");
		return NULL;
	}
	else if (type > 31) {
		perror("Erro build_kermit, o campo type não é compatível\n");
		return NULL;
	}

	unsigned char *bufferFrame = malloc (tamMsg + 4);
	if (!bufferFrame) {
		perror("Erro build_kermit, erro ao alocar frame\n");
		return NULL;
	}

	// Marcador de inicio(8 bits), 01111110 ou 7e
	memset(bufferFrame, 0x7e, 1) ;

  // Tamanho do campo dados (5 bits)
	uint8_t aux_tamMsg = tamMsg ;
  aux_tamMsg = aux_tamMsg << 3 ;

	// Campo sequencia (6 bits)
	uint8_t aux_seq1 = seq ;
  for (int i = 0; i < 3; i++)
		aux_seq1 &= ~(1 << i)  ;

	aux_seq1 = aux_seq1 >> 3 ;

  // Monta o byte na forma TAM(5 bits)/SEQ(3 bits MSB)
	bufferFrame[1] = aux_seq1 + aux_tamMsg ;
	//printf("VALOR %X\n", bufferFrame[1]) ; 

  // Apaga os 3 bits mais significativos
	uint8_t aux_seq2 = seq ;
  for (int i = 3; i < 6; i++)
		aux_seq2 &= ~(1 << i)  ;
  
	aux_seq2 = aux_seq2 << 5 ;

	// Campo tipo (5 bits)
	uint8_t aux_type = type ;
	
	// Monta o byte na forma SEQ(3 bits LSB)/TYPE(5 bits)
	bufferFrame[2] = aux_seq2 + aux_type ;
	//printf("VALOR %X\n", bufferFrame[2]) ; 

	// Campo Dados (tam bytes)
	memcpy(bufferFrame + 3, buffer_dados ,tamMsg);

	// Campo CRC(8bits)
	bufferFrame[3 + tamMsg] = crc;

  return bufferFrame;
}

