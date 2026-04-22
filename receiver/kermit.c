#include "kermit.h" 

/*recebe o buffer e seu numero de bytes utilizados
  retorna dados na estrutura kermit*/
struct kermit parsing_kermit(unsigned char buffer[TAM_FRAME], int tam) {

  struct kermit k ;
	uint8_t aux_seq ;

  k.tam = buffer[15] ;
	k.tam = k.tam >> 3 ;
	printf("k.tam: %u\n", (unsigned char) k.tam) ;

  aux_seq = buffer[15];

	//zera os 5 bits mais significativos
  for (int i = 3; i < 8; i++)
	  aux_seq &= ~(1 << i) ; 
  aux_seq = aux_seq << 5 ;

  k.seq = buffer[16] ;
  k.seq = k.seq >> 5 ;

	k.seq += aux_seq ;
	printf("k.seq %u\n", (unsigned char) k.seq) ;

	k.type = buffer[16] ;
  for (int i = 5; i < 7; i++)
	  k.type &= ~(1 << i) ; 
	
	printf("k.type %u\n", (unsigned char) k.type) ;

	k.dados = &buffer[17] ;

  k.crc = buffer[tam -2] ;

	return k ;
}

/*
  loop de receptacao de informacoes
*/
int loop_recv(int sock, unsigned char buffer[TAM_FRAME]) {

  int i, achouMensagem ;
	ssize_t tam ;
	unsigned short type ;

  if (!buffer) {

	  perror("erro parsing_frame\n") ;
		exit(1) ;
	}

  while (1) {

    tam = recv(sock, buffer, sizeof(unsigned char) *TAM_FRAME, 0);
        
    if (tam > 0) {

      // Lê o tipo do frame ethernet. Big-end -> Little-end
      type = ntohs(*(unsigned short *)(buffer + 12));

      if (type == 0x88B5) {
        printf("Pacote do tipo 0x88B5 localizado! Tamanho: %ld\n", tam);

				printf("\nAchei a minha própria mensagem\n");
				printf("ETHERNET FRAME(14 bytes): ");

				for (i = 0; i < 14; i ++) {
					printf("%02x ", buffer[i]);
				}

				printf("\nIMPRIMINDO PAYLOAD(%ld bytes): ", tam);
				for (i = 14; i < tam; i++) {
					printf("%02x ", buffer[i]);
				}
				printf("\n");

				parsing_kermit(buffer, TAM_FRAME) ;
      }
    }
  }
 
  return 0 ;
}
