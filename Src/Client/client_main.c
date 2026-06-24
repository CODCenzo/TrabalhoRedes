/*
 * client.c — Cliente do Pacman no Escuro
 *
 * Usa ncurses (draw.c / draw.h) para renderizar o mapa recebido do servidor.
 * O cliente reconstrói um Game local apenas para poder chamar draw_game(),
 * que já sabe aplicar a janela de visão, colorir fantasmas, etc.
 *
 * Fluxo por rodada:
 *   1. Lê tecla do jogador (ncurses getch)
 *   2. Envia MSG_MOVE ao servidor
 *   3. Aguarda resposta:
 *        MSG_FILE_START / MSG_GHOST_ENCOUNTER → recebe arquivo, exibe, depois MSG_MAP
 *        MSG_MAP         → atualiza Game local e redesenha
 *        MSG_WIN / MSG_GAMEOVER → exibe tela final e sai
 */

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

/* ═══════════════════════════════════════════════════════════════════════════════
 * Reconstrução do Game local a partir de msg_mapa_t
 *
 * O servidor envia a matriz 40×40 já filtrada por build_client_matrix():
 *   ' '       → fora do raio de visão
 *   'X'       → parede
 *   '1'–'6'   → pastilha
 *   '.'       → corredor livre
 *   'R','B','G','Y' → fantasma
 *   'P'       → posição do Pacman
 *
 * Reconstruímos o Game para que draw_game() funcione sem modificação.
 * ═══════════════════════════════════════════════════════════════════════════════ */

/*
 * Preenche o Game local com os dados do mapa recebido.
 * Isso evita duplicar a lógica de renderização — draw_game() já está pronta.
 */
static void aplica_mapa(Game *g, const msg_mapa_t *mapa) {
    /* Atualiza contadores exibidos no HUD */
    g->vision_radius    = mapa->raio;
    g->prizes_collected = mapa->pastilhas_pegas;
    g->moves_count      = mapa->rodada;

    /* Posição do Pacman */
    g->pacman.x = mapa->pacman_col;
    g->pacman.y = mapa->pacman_linha;

    /* Zera posições dos fantasmas (serão preenchidas ao varrer a matriz) */
    for (int i = 0; i < GHOSTS; i++) {
        g->ghosts[i].body.x = -1;
        g->ghosts[i].body.y = -1;
    }

    /* Preenche g->maze e extrai posições dos fantasmas */
    for (int l = 0; l < MAZE_SIZE; l++) {
        for (int c = 0; c < MAZE_SIZE; c++) {
            char cel = (char)mapa->celulas[l * MAZE_SIZE + c];

            switch (cel) {
                case 'R':
                    g->ghosts[RED].body.x   = c;
                    g->ghosts[RED].body.y   = l;
                    g->maze[l][c] = '0';
                    break;
                case 'B':
                    g->ghosts[BLUE].body.x  = c;
                    g->ghosts[BLUE].body.y  = l;
                    g->maze[l][c] = '0';
                    break;
                case 'G':
                    g->ghosts[GREEN].body.x = c;
                    g->ghosts[GREEN].body.y = l;
                    g->maze[l][c] = '0';
                    break;
                case 'Y':
                    g->ghosts[YELLOW].body.x = c;
                    g->ghosts[YELLOW].body.y = l;
                    g->maze[l][c] = '0';
                    break;
                case 'P':
                    g->maze[l][c] = '0';   /* Pacman não fica no maze */
                    break;
                case ' ':
                    g->maze[l][c] = '0';   /* fora do raio → corredor (não será desenhado) */
                    break;
                case '.':
                    g->maze[l][c] = '0';   /* corredor visível */
                    break;
                default:
                    g->maze[l][c] = cel;   /* 'X', '1'–'6' */
                    break;
            }
        }
    }

    /*
     * draw_game chama visible_to_pacman() para filtrar células.
     * Como o servidor já aplicou a visão, marcamos as células ' ' como
     * fora do raio definindo vision_radius = raio recebido. A função
     * visible_to_pacman usa abs(pacman - célula) <= vision_radius,
     * que corresponde exatamente à janela do servidor.
     */
}

/* ═══════════════════════════════════════════════════════════════════════════════
 * Exibição de arquivo prêmio / encontro
 * ═══════════════════════════════════════════════════════════════════════════════ */

static void exibe_arquivo(const char *filepath, uint8_t ext) {
    char cmd[300];

    /* Sai do modo ncurses para poder exibir o arquivo normalmente */
    endwin();

    printf("\n  Arquivo recebido: %s\n", filepath);
    printf("  Pressione ENTER para visualizar...\n");
    getchar();

    switch (ext) {
        case 't':
            snprintf(cmd, sizeof(cmd), "cat '%s'", filepath);
            break;
        case 'j':
            snprintf(cmd, sizeof(cmd),
                     "eog '%s' 2>/dev/null || feh '%s' 2>/dev/null || display '%s' 2>/dev/null",
                     filepath, filepath, filepath);
            break;
        case 'm':
            snprintf(cmd, sizeof(cmd),
                     "vlc '%s' 2>/dev/null || ffplay -autoexit '%s' 2>/dev/null",
                     filepath, filepath);
            break;
        default:
            snprintf(cmd, sizeof(cmd), "cat '%s'", filepath);
            break;
    }

    system(cmd);

    printf("\n  Pressione ENTER para continuar o jogo...\n");
    getchar();

    /* Restaura ncurses */
    refresh();
    clear();
}

/* ═══════════════════════════════════════════════════════════════════════════════
 * Envio de movimento
 * ═══════════════════════════════════════════════════════════════════════════════ */

static int envia_movimento(int sock, int ch) {
    /* Converte código ncurses em direcao do protocolo */
    uint8_t dir;
    switch (ch) {
        case KEY_UP:    case 'w': case 'W': dir = DIR_UP;    break;
        case KEY_DOWN:  case 's': case 'S': dir = DIR_DOWN;  break;
        case KEY_LEFT:  case 'a': case 'A': dir = DIR_LEFT;  break;
        case KEY_RIGHT: case 'd': case 'D': dir = DIR_RIGHT; break;
        default: return 0; /* tecla sem direção: não envia */
    }

    msg_movimento_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.tipo    = MSG_MOVE;
    msg.direcao = dir;

    return send_buffer(sock, (const unsigned char *)&msg, sizeof(msg),
                       MSG_MOVE, DATA_TYPE);
}

/* ═══════════════════════════════════════════════════════════════════════════════
 * Recepção e despacho de resposta do servidor
 * ═══════════════════════════════════════════════════════════════════════════════ */

typedef enum {
    RESP_CONTINUA,
    RESP_VITORIA,
    RESP_GAMEOVER,
    RESP_ERRO
} resp_t;

/*
 * Recebe uma mensagem do servidor e processa de acordo com o tipo.
 * Retorna o estado resultante para o loop principal.
 */
static resp_t recebe_resposta(int sock, Game *g) {
    static unsigned char buf[sizeof(msg_mapa_t) + 64];
    size_t tam = 0;

    if (receive_buffer(sock, buf, sizeof(buf), &tam) == -1) {
        fprintf(stderr, "CLIENT: receive_buffer falhou\n");
        return RESP_ERRO;
    }

    if (tam == 0) {
        fprintf(stderr, "CLIENT: resposta vazia\n");
        return RESP_ERRO;
    }

    uint8_t tipo = buf[0];

    /* ── Mapa de visão ── */
    if (tipo == MSG_MAP) {
        if (tam < offsetof(msg_mapa_t, celulas)) {
            fprintf(stderr, "CLIENT: MSG_MAP truncado (%zu bytes)\n", tam);
            return RESP_ERRO;
        }
        aplica_mapa(g, (msg_mapa_t *)buf);
        draw_game(g);
        return RESP_CONTINUA;
    }

    /* ── Arquivo prêmio (pastilha dourada) ── */
    if (tipo == MSG_FILE_START) {
        if (tam < sizeof(msg_file_header_t)) {
            fprintf(stderr, "CLIENT: MSG_FILE_START truncado\n");
            return RESP_ERRO;
        }
        msg_file_header_t *hdr = (msg_file_header_t *)buf;
        uint8_t id  = hdr->id_pastilha;
        uint8_t ext = hdr->ext;

        char filepath[32];
        switch (ext) {
            case 't': snprintf(filepath, sizeof(filepath), "premio_%d.txt", id); break;
            case 'j': snprintf(filepath, sizeof(filepath), "premio_%d.jpg", id); break;
            case 'm': snprintf(filepath, sizeof(filepath), "premio_%d.mp4", id); break;
            default:  snprintf(filepath, sizeof(filepath), "premio_%d.bin", id); break;
        }

        printf("CLIENT: recebendo arquivo prêmio %s\n", filepath);
        if (receive_file(sock, filepath) == -1) {
            fprintf(stderr, "CLIENT: falha ao receber arquivo prêmio\n");
            return RESP_ERRO;
        }

        exibe_arquivo(filepath, ext);

        /*
         * O servidor envia o mapa atualizado após o arquivo prêmio
         * (exceto na rodada de vitória, onde vem MSG_WIN).
         * Chamada recursiva para processar essa próxima mensagem.
         */
        return recebe_resposta(sock, g);
    }

    /* ── Arquivo de encontro com fantasma ── */
    if (tipo == MSG_GHOST_ENCOUNTER) {
        printf("CLIENT: encontro com fantasma — recebendo arquivo\n");
        if (receive_file(sock, "encontro.jpg") == -1) {
            fprintf(stderr, "CLIENT: falha ao receber arquivo de encontro\n");
            return RESP_ERRO;
        }
        exibe_arquivo("encontro.jpg", 'j');

        /* Após o encontro o servidor envia MSG_GAMEOVER */
        return recebe_resposta(sock, g);
    }

    /* ── Vitória ── */
    if (tipo == MSG_WIN) {
        msg_status_t *s = (msg_status_t *)buf;
        show_end_screen("VOCÊ VENCEU! Parabéns!");
        mvprintw(LINES / 2 + 1, 2,
                 "Pastilhas: %d/%d  Rodadas: %d  | r reinicia | q sai",
                 s->pastilhas_pegas, PRIZES, s->rodada);
        refresh();
        return RESP_VITORIA;
    }

    /* ── Game over ── */
    if (tipo == MSG_GAMEOVER) {
        msg_status_t *s = (msg_status_t *)buf;
        show_end_screen("GAME OVER! Você foi pego por um fantasma.");
        mvprintw(LINES / 2 + 1, 2,
                 "Pastilhas: %d/%d  Rodadas: %d  | r reinicia | q sai",
                 s->pastilhas_pegas, PRIZES, s->rodada);
        refresh();
        return RESP_GAMEOVER;
    }

    fprintf(stderr, "CLIENT: tipo desconhecido 0x%02x (%zu bytes)\n", tipo, tam);
    return RESP_ERRO;
}

/* ═══════════════════════════════════════════════════════════════════════════════
 * Loop principal do jogo
 * ═══════════════════════════════════════════════════════════════════════════════ */

static void loop_jogo(int sock, Game *g) {

    /* Recebe o mapa inicial enviado pelo servidor ao iniciar */
    printf("CLIENT: aguardando estado inicial...\n");
    if (recebe_resposta(sock, g) != RESP_CONTINUA) {
        fprintf(stderr, "CLIENT: estado inicial inválido\n");
        return;
    }

    int rodando = 1;

    while (rodando) {
        int ch = getch();

        switch (ch) {
            case 'q': case 'Q':
                rodando = 0;
                break;

            case KEY_UP: case 'w': case 'W':
            case KEY_DOWN: case 's': case 'S':
            case KEY_LEFT: case 'a': case 'A':
            case KEY_RIGHT: case 'd': case 'D': {
                if (envia_movimento(sock, ch) == -1) {
                    fprintf(stderr, "CLIENT: falha ao enviar movimento\n");
                    rodando = 0;
                    break;
                }

                resp_t resp = recebe_resposta(sock, g);

                if (resp == RESP_VITORIA || resp == RESP_GAMEOVER) {
                    /* Aguarda 'r' ou 'q' na tela final */
                    int tecla;
                    while ((tecla = getch()) != 'q' && tecla != 'Q' &&
                           tecla != 'r' && tecla != 'R');
                    rodando = 0;
                } else if (resp == RESP_ERRO) {
                    rodando = 0;
                }
                break;
            }

            default:
                /* Tecla não reconhecida: ignora sem enviar ao servidor */
                break;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════════
 * main
 * ═══════════════════════════════════════════════════════════════════════════════ */

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

    // /* ── Inicializa ncurses ── */
    // initscr();
    // cbreak();
    // noecho();
    // keypad(stdscr, TRUE);
    // curs_set(0);
    // init_colors();

    // /* ── Inicializa o Game local (apenas para renderização) ── */
    // Game *g = init_game();
    // if (!g) {
    //     endwin();
    //     fprintf(stderr, "CLIENT: init_game falhou\n");
    //     close(sock);
    //     return EXIT_FAILURE;
    // }

    // /* Inicializa símbolos dos fantasmas para draw_game() colorir corretamente */
    // init_ghosts(g);

    // /* ── Loop do jogo ── */
    // loop_jogo(sock, g);

    // /* ── Limpeza ── */
    // free_game(g);
    // endwin();
    // close(sock);

    // printf("\nCLIENT: encerrado. Até mais!\n");

    send_file(sock, "../../Files/alanturing.txt", 4);

    return EXIT_SUCCESS;
}
