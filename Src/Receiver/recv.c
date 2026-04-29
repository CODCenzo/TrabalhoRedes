#include "../../Headers/kermit.h" 

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
    
  printf("Ouvindo a interface %s\n", argv[1]);

  // Escuta o socket em loop infinito
	loopDeCaptura(sock, buffer) ;

  close(sock);

  return 0;
}
