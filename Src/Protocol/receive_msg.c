#include "../../Headers/kermit.h" 

/*recebe o buffer e seu numero de bytes utilizados
  retorna dados na estrutura kermit*/
struct kermit *parsing_kermit(unsigned char bufferCapturado[MAX_FRAME_SIZE], int tamCaptura) {

  struct kermit *k = malloc(sizeof(struct kermit));
	uint8_t aux_seq ;

	// Separa o tamanhoMsg
  k->tamDados = bufferCapturado[1] ;
	k->tamDados = k->tamDados >> 3 ;

	// Separa os MSB do seq
  aux_seq = bufferCapturado[1];
	//zera os 5 bits mais significativos
  for (int i = 3; i < 8; i++)
	  aux_seq &= ~(1 << i) ; 
  aux_seq = aux_seq << 5 ;

	// Separa os LSB do seq
  k->seq = bufferCapturado[2] ;
  k->seq = k->seq >> 5 ;
	// Separa ambas as partes do campo sequência
	k->seq += aux_seq ;

	// Separada o campo type
	k->type = bufferCapturado[2] ;
  for (int i = 5; i < 8; i++)
	  k->type &= ~(1 << i) ; 
	
	unsigned char *bufferDadosRecebidos = malloc(k->tamDados);
	if (!bufferDadosRecebidos) {
		perror("parsing_kermit, erro ao alocar bufferDadosRecebidos\n");
	}

	// Cria uma cópia dos dados da mensagem 
	memcpy(bufferDadosRecebidos, &bufferCapturado[3], k->tamDados);
	k->dados = bufferDadosRecebidos;

	// Separa o CRC
  k->crc = bufferCapturado[3 + k->tamDados] ;

	return k;
}


/*
  loop de receptacao de informacoes
*/
struct kermit *loopDeCaptura(int sock) {

  unsigned char *bufferDeCaptura = malloc(MAX_FRAME_SIZE);
  if (!bufferDeCaptura) {
    perror("erro ao alocar buffer de captura\n") ;
    return NULL;
  }

  struct kermit *k = NULL;

	int tamPacote ;
	uint8_t marcadorInicio ;

  while (1) {
		// Limpa o buffer antes de cada tentativa de captura
		memset(bufferDeCaptura, 0, MAX_FRAME_SIZE);

    tamPacote = recv(sock, bufferDeCaptura, MAX_FRAME_SIZE, 0);
        
    if (tamPacote > 0) {

			// Verifica o marcador de início
      marcadorInicio = bufferDeCaptura[0];

      if (marcadorInicio == 0x7e) {
        printf("\nPacote com Marcador 0x7e localizado! Tamanho do pacote: %d\n", tamPacote);
				
				// Armazena os dados capturados na estrutura kermit
				k = parsing_kermit(bufferDeCaptura, tamPacote) ;

				// Debug
				printf("Parsed -> TAM: %u, SEQ: %u, TYPE: %u, CRC: %02X\n", k->tamDados, k->seq, k->type, k->crc);
				printf("Dados: ");
				for (int i = 0; i < k->tamDados; i ++) {
					printf("%02x ", *(k->dados + i));
				}
				printf("\n");
				break;
				
				// free(k->dados);
      }
    }
  }

  free(bufferDeCaptura);
 
  return k ;
}