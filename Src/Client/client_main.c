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

static void desenhar_tabuleiro_ncurses(uint8_t tabuleiro[MAZE_SIZE][MAZE_SIZE]) {
    int top = 2;
    int left;

    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);
    init_colors();

    erase();

    if (LINES < MAZE_SIZE + 4 || COLS < MAZE_SIZE) {
        mvprintw(1, 2, "Terminal pequeno demais para mostrar o tabuleiro 40x40.");
        mvprintw(3, 2, "Aumente a janela e tente novamente.");
        mvprintw(LINES - 2, 2, "Pressione qualquer tecla para sair.");
        refresh();
        getch();
        endwin();
        return;
    }

    left = (COLS - MAZE_SIZE) / 2;

    attron(A_BOLD);
    mvprintw(0, 1, "CLIENT: Tabuleiro recebido");
    attroff(A_BOLD);

    for (int y = 0; y < MAZE_SIZE; y++) {
        for (int x = 0; x < MAZE_SIZE; x++) {
            char tile = (char)tabuleiro[y][x];

            if (tile == '\0') {
                tile = ' ';
            }

            if (tile == 'X') {
                attron(COLOR_PAIR(6) | A_BOLD);
                mvaddch(top + y, left + x, ACS_CKBOARD);
                attroff(COLOR_PAIR(6) | A_BOLD);
            } else if (tile >= '1' && tile <= '6') {
                attron(COLOR_PAIR(5) | A_BOLD);
                mvaddch(top + y, left + x, tile);
                attroff(COLOR_PAIR(5) | A_BOLD);
            } else {
                mvaddch(top + y, left + x, tile);
            }
        }
    }

    mvprintw(top + MAZE_SIZE + 1, 1, "Pressione qualquer tecla para continuar.");
    refresh();
    getch();
    endwin();
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

    uint8_t m[MAZE_SIZE][MAZE_SIZE]; 
    memset(m, 0, sizeof(m));

    // Agora m passa o endereço do bloco contínuo perfeitamente
    if (receber_tabuleiro_jogo(sock, m) == 1) {
        printf("CLIENT: Tabuleiro recebido e pronto para uso!\n");
        desenhar_tabuleiro_ncurses(m);
    } else {
        close(sock);
        return EXIT_FAILURE;
    }

    if (servidor_envia_game_show(sock, SHOW_TYPE) == 1) {
        printf("MOVIMENTO ENVIADO COM SUCESSO\n");
    }

    close(sock);
    return EXIT_SUCCESS;
}
