#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define TAM_FRAME 1514

struct kermit {

  uint8_t tam, seq, type ;
	unsigned int *dados ;
  uint8_t crc ;
} ;

extern int cria_raw_socket(char* nome_interface_rede);

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

/*recebe o buffer e seu numero de bytes utilizados
  retorna dados na estrutura kermit*/
struct kermit parsing_kermit(unsigned char buffer[TAM_FRAME], int tam) {

  struct kermit k ;
	uint8_t aux_seq ;

  k.tam = buffer[14] ;
	k.tam = k.tam >> 3 ;
	printf("k.tam: %u\n", (unsigned char) k.tam) ;

  aux_seq = buffer[14];

	//zera os 5 bits mais significativos
  for (int i = 3; i < 8; i++)
	  aux_seq &= ~(1 << i) ; 
  aux_seq = aux_seq << 5 ;

  k.seq = buffer[15] ;
  k.seq = k.seq >> 5 ;

	k.seq += aux_seq ;
	printf("k.seq %u\n", (unsigned char) k.seq) ;

	k.type = buffer[15] ;
  for (int i = 5; i < 7; i++)
	  k.type &= ~(1 << i) ; 
	
	printf("k.type %u\n", (unsigned char) k.type) ;

	return k ;
}

int main(int argc, char *argv[]) {

  int sock, i, achouMensagem ;
	unsigned char buffer[TAM_FRAME], comparador[TAM_FRAME] ;
	ssize_t tam ; 
	unsigned short type ; 

	//------------------------------------------
	//SOCKET
  if (argc < 2) {
    printf("Uso: sudo %s <nome_da_interface>\n", argv[0]);
    printf("Exemplo: sudo %s eth0\n", argv[0]);
    return 1;
  }

  //Cria um socket a partir do nome da interface
  sock = cria_raw_socket(argv[1]);

    //Preenche o vetor com o valor 1
  memset(comparador, 1, sizeof(comparador));
    
  printf("Ouvindo a interface %s... Pressione Ctrl+C para parar.\n", argv[1]);

	loop_recv(sock, buffer) ;

  close(sock);

  return 0;
}
