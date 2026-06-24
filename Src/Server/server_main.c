#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "../../Headers/kermit.h"
#include "../../Headers/protocol.h"
#include "../../Headers/game_protocol.h"
#include "../../Headers/game.h"
#include "../../Headers/draw.h"

int main(int argc, char *argv[]) {
  srand((unsigned int)time(NULL));

  if (argc < 2) {
    fprintf(stderr, "Uso: %s <interface> [labirinto.csv]\n", argv[0]);
    fprintf(stderr, "Exemplo: %s eth0 labirinto.csv\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *interface = argv[1];

  int sock = cria_raw_socket((char *)interface);
  if (sock < 0) {
    fprintf(stderr, "SERVER: falha ao criar socket em %s\n", interface);
    return EXIT_FAILURE;
  }
  printf("SERVER: socket criado (fd=%d)\n", sock);

  bool running = true;
  Game *g = init_game();

  srand(300);
  load_level(g, NULL);

  // 1. Envia o estado inicial do labirinto assim que o cliente conecta
  printf("SERVER: Enviando tabuleiro inicial...\n");
  enviar_tabuleiro_jogo(sock, g->maze);

  uint8_t tipoMsgRecebida = 0;
  uint8_t seq_esperada_mov = 0; // Sincronizador de sequência do servidor

  printf("SERVER: Pronto e aguardando jogadas do cliente...\n");
    
  // 2. Loop principal do servidor para escuta contínua de comandos
  while (running) {
    int resultado = servidor_receber_movimento(sock, &tipoMsgRecebida);

    if (resultado == 1) {
      printf("SERVER: Movimento detectado (Tipo: %d). Atualizando lógica do jogo...\n", tipoMsgRecebida);
            
      // computar_movimento_personagem(g, tipoMsgRecebida); 

      // 3. Envia de volta a matriz atualizada pós-jogada para o cliente renderizar
      printf("SERVER: Enviando matriz atualizada para o cliente.\n");
      enviar_tabuleiro_jogo(sock, g->maze);
    }
        
    // if (g->player_ganhou) running = false;
  }

  close(sock);
  // free_game(g); // Se houver função de desalocação
    return EXIT_SUCCESS;
}
