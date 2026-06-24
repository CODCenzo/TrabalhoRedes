#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "../../Headers/kermit.h"
#include "../../Headers/protocol.h"
#include "../../Headers/game_protocol.h"
#include "../../Headers/game.h"
#include "../../Headers/draw.h"


int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Uso: %s <interface-de-rede>\n", argv[0]);
    fprintf(stderr, "Exemplo: %s eth0\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *interface = argv[1];

  int sock = cria_raw_socket((char *)interface);
  if (sock < 0) {
    fprintf(stderr, "CLIENT: falha ao criar socket em %s\n", interface);
    return EXIT_FAILURE;
  }

  uint8_t m[40][40]; 
  uint8_t seq_mov = 0; // Controle de sequência para os pacotes de movimento
    
  memset(m, 0, sizeof(m));

  // 1. Recebe o tabuleiro inicial enviado pelo servidor
  if (receber_tabuleiro_jogo(sock, m) == 1) {
    printf("CLIENT: Tabuleiro inicial recebido com sucesso!\n");
    imprimir_tabuleiro_jogo(m);
  } else {
    fprintf(stderr, "CLIENT: Erro ao receber o tabuleiro inicial.\n");
    close(sock);
    return EXIT_FAILURE;
  }

  char input;
  do {
    // Captura a entrada do usuário (ajuste caso getch() dependa de inicialização específica)
    input = getch(); 
    int moveu = 0;

    switch (input) {
      case 'w':
        moveu = cliente_enviar_movimento(sock, MOVE_UP_TYPE, &seq_mov);
        break;
      case 'a':
        // CORRIGIDO: Era MOVE_DOWN_TYPE no seu código original
        moveu = cliente_enviar_movimento(sock, MOVE_LEFT_TYPE, &seq_mov); 
        break;
      case 's':
        moveu = cliente_enviar_movimento(sock, MOVE_DOWN_TYPE, &seq_mov);
        break;
      case 'd':
        moveu = cliente_enviar_movimento(sock, MOVE_RIGHT_TYPE, &seq_mov);
        break;
      case 'q':
        printf("CLIENT: Encerrando o jogo...\n");
        break;
      default:
        break;
    }

    // 2. Se um comando válido foi enviado, aguarda o servidor processar e devolver a matriz atualizada
    if (moveu == 1) {
      printf("CLIENT: Comando aceito. Aguardando atualização do mapa...\n");
      if (receber_tabuleiro_jogo(sock, m) == 1) {
        imprimir_tabuleiro_jogo(m);
      }
    }

  } while (input != 'q');

  close(sock);
  return EXIT_SUCCESS;
}
