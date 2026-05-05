#include "../../Headers/kermit.h" 
#include "../../Headers/timeout.h" 


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
	
	// Cria uma cópia dos dados da mensagem 
	memcpy(k->dados, &bufferCapturado[3], k->tamDados);

	// Separa o CRC
  k->crc = bufferCapturado[3 + k->tamDados] ;

	return k;
}


/*
  *loop de receptacao de informacoes
  *aloca a estrutura kermit
  *recebe UM pacote
*/
struct kermit *loopDeCaptura(int sock) {

  int tamanhoCapturado ;
  struct kermit *pack ;

  unsigned char *bufferDeCaptura = malloc(MAX_FRAME_SIZE);
  if (!bufferDeCaptura) {

    perror("erro ao alocar buffer de captura\n") ;
    return NULL;
  }

  unsigned char *bufferDados = malloc(DEFAULT_MSG_SIZE);
  if (!bufferDados) {
    perror("Erro ao alocar mensagem\n");
    exit(1);
  }   

    // Loop de espera e tratamento
  while (1) {
    // Recebe mensagens e envia resposta
    tamanhoCapturado = recebe_mensagem(sock, TIMEOUT_MILLIS, bufferDeCaptura, MAX_FRAME_SIZE);

    if (tamanhoCapturado > 0) {

      pack = parsing_kermit(bufferDeCaptura, tamanhoCapturado);
      printf("MENSAGEM RECEBIDA, TIPO: %d\n", pack->type);

      break ;
    }   

  }
  free(bufferDados);

  free(bufferDeCaptura);
 
  return pack ;
}
