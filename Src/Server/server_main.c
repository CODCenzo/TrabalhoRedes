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
    // const char *arq_mapa  = (argc >= 3) ? argv[2] : NULL;

    /* ── Cria o raw socket ── */
    int sock = cria_raw_socket((char *)interface);
    if (sock < 0) {
        fprintf(stderr, "SERVER: falha ao criar socket em %s\n", interface);
        return EXIT_FAILURE;
    }
    printf("SERVER: socket criado (fd=%d)\n", sock);


    int ch;
    int result = 0;
    bool running = true;
    Game *g;

    g = init_game();

    //srand((unsigned int)time(NULL));
    srand(300);
    // initscr();
    // raw();
    // noecho();
    // curs_set(0);
    // keypad(stdscr, TRUE);
    // timeout(-1);
    // init_colors();
    load_level(g, NULL);

    // enviar_tabuleiro_jogo(sock, g->maze);

    uint8_t *tipoMsg;
   if (client_receber_game_show(sock, tipoMsg) == 1) {
    printf("MOVIMENTO RECEBIDO COM SUCESSO\n");
   }  

  close(sock);
  return EXIT_SUCCESS;
}
