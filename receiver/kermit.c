#include "kermit.h" 

/*recebe o buffer e seu numero de bytes utilizados
  retorna dados na estrutura kermit*/
struct kermit parsing_kermit(unsigned char buffer[MAX_FRAME_SIZE], int tam) {

  struct kermit k ;
	uint8_t aux_seq ;

	// Separa o campo tamanhoMsg
  k.tam = buffer[1] ;
	k.tam = k.tam >> 3 ;

  aux_seq = buffer[1];

	//zera os 5 bits mais significativos
  for (int i = 3; i < 8; i++)
	  aux_seq &= ~(1 << i) ; 
  aux_seq = aux_seq << 5 ;

  k.seq = buffer[2] ;
  k.seq = k.seq >> 5 ;

	// Separa ambas as partes do campo sequência
	k.seq += aux_seq ;

	// Separada o campo type
	k.type = buffer[2] ;
  for (int i = 5; i < 8; i++)
	  k.type &= ~(1 << i) ; 
	
	unsigned char *bufferDadosRecebidos = malloc(k.tam);
	if (!bufferDadosRecebidos) {
		perror("parsing_kermit, erro ao alocar bufferDadosRecebidos\n");
	}
	memcpy(bufferDadosRecebidos, &buffer[3], k.tam);

	k.dados = bufferDadosRecebidos;

  k.crc = buffer[3 + k.tam] ;

	return k ;
}

/*
  loop de receptacao de informacoes
*/
int loop_recv(int sock, unsigned char bufferDeCaptura[MAX_FRAME_SIZE]) {

	ssize_t tamPacote ;
	uint8_t marcadorInicio ;

  if (!bufferDeCaptura) {
	  perror("erro parsing_frame\n") ;
		exit(1) ;
	}

  while (1) {
		//Limpa o buffer antes de cada tentatica de captura
		memset(bufferDeCaptura, 0, MAX_FRAME_SIZE);

    tamPacote = recv(sock, bufferDeCaptura, MAX_FRAME_SIZE, 0);
        
    if (tamPacote > 0) {

      // Lê o tipo do frame ethernet. Big-end -> Little-end
      marcadorInicio = bufferDeCaptura[0];

      if (marcadorInicio == 0x7e) {
        printf("\nPacote com Marcador 0x7e localizado! Tamanho do pacote: %zd\n", tamPacote);

				printf("\nAchei a minha própria mensagem\n");
				
				struct kermit k = parsing_kermit(bufferDeCaptura, tamPacote) ;
				// Debug
				printf("Parsed -> TAM: %u, SEQ: %u, TYPE: %u, CRC: %02X\n", k.tam, k.seq, k.type, k.crc);
				for (int i = 0; i < k.tam; i ++) {
					printf("%02x ", *(k.dados + i));
				}
				printf("\n");
				
				free(k.dados);
      }
    }
  }
 
  return 0 ;
}
