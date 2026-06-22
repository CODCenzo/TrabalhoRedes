
#include "../../Headers/kermit.h"
#include "../../Headers/protocol.h"
#include "../../Headers/socket.h"

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Uso: sudo %s <nome_da_interface>\n", argv[0]);
		printf("Exemplo: sudo %s eth0\n", argv[0]);
		return 1;
	}  

	int socket = cria_raw_socket(argv[1]);

  receive_file(socket);

  return 0;

}

