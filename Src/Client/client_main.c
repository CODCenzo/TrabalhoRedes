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

#define printf(...) ((void)0)
#define fprintf(...) ((void)0)

static void preparar_ncurses_cliente(void) {
  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  timeout(-1);
  init_colors();
}

static void imprimir_tabuleiro_ncurses(uint8_t tabuleiro[MAZE_SIZE][MAZE_SIZE]) {
  int top = 2;
  int left;

  erase();

  if (LINES < MAZE_SIZE + top + 2 || COLS < MAZE_SIZE) {
    mvprintw(1, 2, "Aumente o terminal para exibir o tabuleiro 40x40.");
    refresh();
    return;
  }

  left = (COLS - MAZE_SIZE) / 2;

  attron(A_BOLD);
  mvprintw(0, 1, "Cliente - Tabuleiro recebido do servidor");
  attroff(A_BOLD);

  for (int y = 0; y < MAZE_SIZE; y++) {
    for (int x = 0; x < MAZE_SIZE; x++) {
      draw_client_tile((char)tabuleiro[y][x], left + x, top + y);
    }
  }

  mvprintw(LINES - 2, 1, "WASD/Setas movem | q sai");
  refresh();
}

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
  (void)seq_mov;
    
  memset(m, 0, sizeof(m));

  // 1. Recebe o tabuleiro inicial enviado pelo servidor
  if (receber_tabuleiro_jogo(sock, m) == 1) {
    printf("CLIENT: Tabuleiro inicial recebido com sucesso!\n");
    preparar_ncurses_cliente();
    imprimir_tabuleiro_ncurses(m);
  } else {
    fprintf(stderr, "CLIENT: Erro ao receber o tabuleiro inicial.\n");
    close(sock);
    return EXIT_FAILURE;
  }

  int input;
  do {
    input = getch();
    int moveu = 0;

    switch (input) {
      case 'w':
      case 'W':
      case KEY_UP:
        moveu = cliente_enviar_movimento(sock, MOVE_UP_TYPE);
        break;
      case 'a':
      case 'A':
      case KEY_LEFT:
        moveu = cliente_enviar_movimento(sock, MOVE_LEFT_TYPE); 
        break;
      case 's':
      case 'S':
      case KEY_DOWN:
        moveu = cliente_enviar_movimento(sock, MOVE_DOWN_TYPE);
        break;
      case 'd':
      case 'D':
      case KEY_RIGHT:
        moveu = cliente_enviar_movimento(sock, MOVE_RIGHT_TYPE);
        break;
      case 'q':
      case 'Q':
        printf("CLIENT: Encerrando o jogo...\n");
        break;
      default:
        break;
    }
    //sleep(1);

    // 2. Se um comando válido foi enviado, aguarda o servidor processar e devolver a matriz atualizada
    if (moveu == 1) {
      printf("CLIENT: Comando aceito. Aguardando atualização do mapa...\n");
      if (receber_tabuleiro_jogo(sock, m) == 1) {
        imprimir_tabuleiro_ncurses(m);
      }
    }

  } while (input != 'q' && input != 'Q');

  show_end_screen("Fim de Jogo");
  getch();
  endwin();

  close(sock);
  return EXIT_SUCCESS;
}
