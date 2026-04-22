#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define TAM_FRAME 1514

extern int cria_raw_socket(char* nome_interface_rede);

/*
  funcao que retorna apenas o payload

  ideia final seria retornar os dados do kermit 
	ou mais informacoes, segregando o frame ethernet
*/
int parsing_frame(int sock, unsigned char buffer[TAM_FRAME], 
                  unsigned char comparador[TAM_FRAME]) {

  int i, achouMensagem ;
	ssize_t tam ;
	unsigned short type ;

  if (!buffer) {

	  perror("erro parsing_frame\n") ;
		exit(1) ;
	}

  while (1) {
    // recv coloca os dados lidos do socket no buffer e retorna o tamanho lido,
    // em caso de erro retorna -1
    // recv retorna o tamanho do pacote recebido
    tam = recv(sock, buffer, sizeof(unsigned char) *TAM_FRAME, 0);
        
    if (tam > 0) {

      // Lê o tipo do frame ethernet. Big-end -> Little-end
      type = ntohs(*(unsigned short *)(buffer + 12));

      if (type == 0x88B5) {
        printf("Pacote do tipo 0x88B5 localizado! Tamanho: %ld\n", tam);

        //Se a o buffer for igual ao comparador, então a mensagem chegou
        /*achouMensagem = 1;
        for (i = 14; i < tam; i++) {
          if (buffer[i] != comparador[i]) {
            achouMensagem = 0;
          }
        }*/

        // 

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
      }
    }
  }
 
  return 0 ;
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

	parsing_frame(sock, buffer, comparador) ;

  close(sock);

  return 0;
}
