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


char devolve_movimento(int tipoMsg) {

  switch (tipoMsg) {
    case MOVE_UP_TYPE:
      return 'w';
    case MOVE_DOWN_TYPE:
      return 's';
    case MOVE_LEFT_TYPE:
      return 'a';
    case MOVE_RIGHT_TYPE:
      return 'd';
  }

  return '1'; // Nenhum movimento válido detectado
}

int main(int argc, char *argv[]) {

  char **matrix  = alloc_matrix(MAZE_SIZE, MAZE_SIZE);
  char ch;

  const char *prize_files[PRIZES] = {"1.txt", "2.txt", "3.jpg",
                                     "4.jpg", "5.mp4", "6.mp4"};

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

  // Envia o estado inicial do labirinto assim que o cliente conecta
  printf("SERVER: Enviando tabuleiro inicial...\n");
  build_client_matrix_(g, matrix);
  enviar_tabuleiro_jogo(sock, matrix);

  uint8_t tipoMsgRecebida = 0;
  int game_state = 0; // 0 = ongoing, 1 = win, -1 = lose
  int prize ;

  printf("SERVER: Pronto e aguardando jogadas do cliente...\n");
    
  // Loop principal do servidor para escuta contínua de comandos
  while (running) {
    int resultado = servidor_receber_movimento(sock, &tipoMsgRecebida);
    ch = devolve_movimento(tipoMsgRecebida);

    if (resultado == 1) {
      printf("SERVER: Movimento detectado (Tipo: %d). Atualizando lógica do jogo...\n", tipoMsgRecebida);
             
      // funcao de envir premio dentro de play_round
      game_state = play_round(g, ch, &prize);

      if (prize != -1) {
        printf("SERVER: Prêmio coletado! Tipo: %d\n", prize);
        server_send_prize_collected(sock, DATA_TYPE, prize);
        send_file(sock, prize_files[prize - 1], DATA_TYPE);
      }
      else {
        printf("SERVER: Nenhum prêmio coletado nesta jogada.\n");
        server_send_prize_collected(sock, ERROR_TYPE, -1); ;
      }

      // Envia de volta a matriz atualizada pós-jogada para o cliente renderizar
      printf("SERVER: Enviando matriz atualizada para o cliente.\n");
      build_client_matrix_(g, matrix);
      enviar_tabuleiro_jogo(sock, matrix);
    }
    if (game_state != 0) {
      running = false; // Sai do loop se o jogo terminou
    }
  }

  close(sock);
  free_game(g); // Se houver função de desalocação
  free_matrix(matrix, MAZE_SIZE);

  return EXIT_SUCCESS;
}
