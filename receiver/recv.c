#include "kermit.h" 

#define MAX_FRAME_SIZE 35

int main(int argc, char *argv[]) {

  int sock ;
	unsigned char buffer[MAX_FRAME_SIZE] ;

	// SOCKET
  if (argc < 2) {
    printf("Uso: sudo %s <nome_da_interface>\n", argv[0]);
    printf("Exemplo: sudo %s eth0\n", argv[0]);
    return 1;
  }

  // Cria um socket a partir do nome da interface
  sock = cria_raw_socket(argv[1]);

	// Preenche o vetor com o valor 1
  //memset(comparador, 1, sizeof(comparador));
    
  printf("Ouvindo a interface %s\n", argv[1]);

	loop_recv(sock, buffer) ;

  close(sock);

  return 0;
}
