/*arquivo que tenta enviar um buffer de 1's para o client*/

#include <stdlib.h> 
#include <string.h> 

#include "../socket.h"

int main(int argc, char *argv[]) {

  int sock, i ;
	unsigned char buffer[1600] ; // Buffer para enviar

	if (argc < 2) {
		printf("Uso: sudo %s <nome_da_interface>\n", argv[0]);
		printf("Exemplo: sudo %s eth0\n", argv[0]);
		return 1;
	}

	//Cria um socket a partir do nome da interface
	sock = cria_raw_socket(argv[1]);

  //enche o buffer
	for (i = 0; i < 1600; i++) 
	  buffer[i] = 1 ;
    
	send(sock, buffer, sizeof(unsigned char) *1600, 0) ;

  close(sock) ;
  return 0;
}


