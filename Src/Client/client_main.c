#include <stdio.h>

#include "../../Headers/kermit.h"
#include "../../Headers/socket.h"
#include "../../Headers/protocol.h"

#define TAM_DADOS 31

// Retorna um valor de 8 bits entre min e max, incluindo eles mesmos
uint8_t gera_byte_aleat (uint8_t min, uint8_t max) {
  return rand() % (max - min +1) + min;
}

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Uso: sudo %s <nome_da_interface>\n", argv[0]);
		printf("Exemplo: sudo %s eth0\n", argv[0]);
		return 1;
	}  

	int socket = cria_raw_socket(argv[1]);
  
  // send_file(socket, "../../Files/stevejobs.txt", 5);

	// Loop principal do cliente
	// while (1) {
	// 		send_movimento(socket, ler_input_usuario());

	// 		switch (receive_game_message(socket, buf, sizeof(buf))) {
	// 				case RECV_MAP:      renderizar_mapa(buf);   break;
	// 				case RECV_FILE:     exibir_arquivo(buf);    break;
	// 				case RECV_GAMEOVER: mostrar_game_over();    return;
	// 				case RECV_WIN:      mostrar_vitoria();      return;
	// 				case RECV_ERROR:    /* tratar */            break;
	// 		}
	// }

	send_file(socket, "../../Files/alanturing.txt", 5);

  return 0;
}
