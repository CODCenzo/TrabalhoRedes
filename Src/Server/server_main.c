/*
 * server.c — Servidor do Pacman no Escuro
 *
 * Toda a lógica do jogo vive em game.c / aux_game.c (Game, Actor, Ghost,
 * play_round, update_ghosts, collect_prize, check_collision, …).
 * Este arquivo cuida exclusivamente de:
 *   - Carregar o nível e inicializar o Game
 *   - Converter o estado do Game em mensagens de rede (game_protocol.h)
 *   - Loop de rodadas via raw socket (protocol.h / kermit.h)
 */

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

#define ARQUIVO_ENCONTRO "encontro.jpg"

/* ═══════════════════════════════════════════════════════════════════════════════
 * Carregamento do CSV
 *
 * O servidor usa a função load_level() de game.c para o labirinto padrão.
 * Esta função complementa com a leitura de CSV externo, normalizando cada
 * célula com normalize_tile() de aux_game.c.
 * ═══════════════════════════════════════════════════════════════════════════════ */

/*
 * Lê o labirinto de um arquivo CSV com separador ';' e preenche g->maze.
 * Usa normalize_tile() para aceitar variações ('x','#' → 'X', etc.).
 * Retorna 1 em sucesso, -1 em erro.
 */
static int load_csv_level(Game *g, const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        perror("SERVER: fopen labirinto");
        return -1;
    }

    char linha[512];
    int  l = 0;
    int has_ghosts[GHOSTS]  = {0, 0, 0, 0};
    int has_prizes[PRIZES]  = {0, 0, 0, 0, 0, 0};
    bool has_pacman = false;

    init_ghosts(g);
    reset_game_state(g);

    while (fgets(linha, sizeof(linha), f) && l < MAZE_SIZE) {
        int c = 0;
        char *tok = strtok(linha, ";\n\r");

        while (tok && c < MAZE_SIZE) {
            char cell = normalize_tile(tok[0]);

            /* Identifica entidades e anota posição; limpa a célula do maze */
            switch (tok[0]) {
                case 'P': case 'p':
                    g->pacman.x = c;
                    g->pacman.y = l;
                    has_pacman  = true;
                    cell = '0';
                    break;
                case 'R':
                    g->ghosts[RED].body.x   = c;
                    g->ghosts[RED].body.y   = l;
                    has_ghosts[RED]         = 1;
                    cell = '0';
                    break;
                case 'B':
                    g->ghosts[BLUE].body.x  = c;
                    g->ghosts[BLUE].body.y  = l;
                    has_ghosts[BLUE]        = 1;
                    cell = '0';
                    break;
                case 'G':
                    g->ghosts[GREEN].body.x = c;
                    g->ghosts[GREEN].body.y = l;
                    has_ghosts[GREEN]       = 1;
                    cell = '0';
                    break;
                case 'Y':
                    g->ghosts[YELLOW].body.x = c;
                    g->ghosts[YELLOW].body.y = l;
                    has_ghosts[YELLOW]       = 1;
                    cell = '0';
                    break;
                default:
                    if (tok[0] >= '1' && tok[0] <= '6')
                        has_prizes[tok[0] - '1'] = 1;
                    break;
            }

            g->maze[l][c] = cell;
            tok = strtok(NULL, ";\n\r");
            c++;
        }

        /* Preenche colunas faltantes */
        while (c < MAZE_SIZE) g->maze[l][c++] = '0';
        l++;
    }

    fclose(f);

    if (l < MAZE_SIZE) {
        fprintf(stderr, "SERVER: CSV incompleto (%d linhas)\n", l);
        return -1;
    }

    /* Sorteia aleatoriamente os objetos que não vieram no CSV */
    place_objects(g, has_pacman, has_ghosts, has_prizes);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════════
 * Serialização do mapa para envio ao cliente
 *
 * Usa build_client_matrix() de game.c — ela aplica a janela de visão,
 * insere fantasmas e Pacman na posição correta e retorna uma matriz 40x41.
 * O servidor empacota esse resultado na msg_mapa_t e envia via send_buffer.
 * ═══════════════════════════════════════════════════════════════════════════════ */

static int envia_mapa(int sock, Game *g) {
    /*
     * build_client_matrix produz um mapa 40x(40+1):
     *   ' '  → fora do raio de visão
     *   'X'  → parede visível
     *   '1'–'6' → pastilha visível
     *   '.'  → corredor visível
     *   'R','B','G','Y' → fantasma visível
     *   'P'  → posição do Pacman
     *
     * Empacotamos as 40 strings (sem o '\0') em celulas[], row-major.
     * O cliente já conhece o encoding — os mesmos CELL_* de game_protocol.h.
     */
    char matriz[MAZE_SIZE][MAZE_SIZE + 1];
    build_client_matrix(g, matriz);

    msg_mapa_t mapa;
    memset(&mapa, 0, sizeof(mapa));

    mapa.tipo            = MSG_MAP;
    mapa.pacman_linha    = (uint8_t)g->pacman.y;
    mapa.pacman_col      = (uint8_t)g->pacman.x;
    mapa.raio            = (uint8_t)g->vision_radius;
    mapa.pastilhas_pegas = (uint8_t)g->prizes_collected;
    mapa.rodada          = (uint8_t)g->moves_count;

    for (int l = 0; l < MAZE_SIZE; l++)
        for (int c = 0; c < MAZE_SIZE; c++)
            mapa.celulas[l * MAZE_SIZE + c] = (uint8_t)matriz[l][c];

    /*
     * Tamanho enviado: cabeçalho (6 bytes) + todas as células (40*40 = 1600 bytes).
     * send_buffer fragmenta automaticamente em pacotes de MAX_DADOS bytes.
     */
    size_t tam = offsetof(msg_mapa_t, celulas) + MAZE_SIZE * MAZE_SIZE;

    printf("SERVER: enviando mapa raio=%d pacman=(%d,%d)\n",
           g->vision_radius, g->pacman.x, g->pacman.y);

    return send_buffer(sock, (const unsigned char *)&mapa, tam,
                       MSG_MAP, DATA_TYPE);
}

/* ═══════════════════════════════════════════════════════════════════════════════
 * Envio de arquivos prêmio e de encontro
 * ═══════════════════════════════════════════════════════════════════════════════ */

/*
 * Determina a extensão do arquivo de prêmio pelo id da pastilha.
 * Retorna o caractere de extensão ('t', 'j', 'm') ou 0 em erro.
 */
static uint8_t ext_para_pastilha(uint8_t id) {
    if (id == 1 || id == 2) return 't';
    if (id == 3 || id == 4) return 'j';
    if (id == 5 || id == 6) return 'm';
    return 0;
}

/*
 * Envia o header de arquivo seguido do conteúdo binário.
 * O cliente usa o header para saber o nome/extensão antes de receber os dados.
 */
static int envia_arquivo(int sock, uint8_t msg_tipo, uint8_t id_pastilha,
                         uint8_t ext, const char *filepath) {
    msg_file_header_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.tipo        = msg_tipo;
    hdr.id_pastilha = id_pastilha;
    hdr.ext         = ext;

    printf("SERVER: enviando header tipo=0x%02x id=%d ext=%c\n",
           msg_tipo, id_pastilha, ext);

    if (send_buffer(sock, (const unsigned char *)&hdr, sizeof(hdr),
                    msg_tipo, DATA_TYPE) == -1) {
        fprintf(stderr, "SERVER: falha ao enviar header\n");
        return -1;
    }

    printf("SERVER: enviando arquivo '%s'\n", filepath);
    return send_file(sock, filepath, msg_tipo);
}

static int envia_arquivo_premio(int sock, uint8_t id_pastilha) {
    uint8_t ext = ext_para_pastilha(id_pastilha);
    if (!ext) {
        fprintf(stderr, "SERVER: id de pastilha inválido: %d\n", id_pastilha);
        return -1;
    }

    char filepath[16];
    /* Os arquivos ficam no diretório de trabalho do servidor: 1.txt, 3.jpg, 5.mp4 … */
    switch (ext) {
        case 't': snprintf(filepath, sizeof(filepath), "%d.txt", id_pastilha); break;
        case 'j': snprintf(filepath, sizeof(filepath), "%d.jpg", id_pastilha); break;
        case 'm': snprintf(filepath, sizeof(filepath), "%d.mp4", id_pastilha); break;
        default:  return -1;
    }

    return envia_arquivo(sock, MSG_FILE_START, id_pastilha, ext, filepath);
}

static int envia_arquivo_encontro(int sock) {
    return envia_arquivo(sock, MSG_GHOST_ENCOUNTER, 0, 'j', ARQUIVO_ENCONTRO);
}

/* ═══════════════════════════════════════════════════════════════════════════════
 * Mensagens de status (game over / vitória)
 * ═══════════════════════════════════════════════════════════════════════════════ */

static int envia_status(int sock, uint8_t tipo, Game *g) {
    msg_status_t status;
    memset(&status, 0, sizeof(status));
    status.tipo            = tipo;
    status.pastilhas_pegas = (uint8_t)g->prizes_collected;
    status.rodada          = (uint8_t)g->moves_count;

    printf("SERVER: enviando status tipo=0x%02x\n", tipo);

    return send_buffer(sock, (const unsigned char *)&status, sizeof(status),
                       tipo, DATA_TYPE);
}

/* ═══════════════════════════════════════════════════════════════════════════════
 * Recepção do movimento do cliente
 * ═══════════════════════════════════════════════════════════════════════════════ */

/*
 * Aguarda e decodifica um MSG_MOVE do cliente.
 * Converte o campo direcao (DIR_UP/DOWN/LEFT/RIGHT = 'w'/'s'/'a'/'d')
 * no código de tecla esperado por direction_from_key() de aux_game.c.
 *
 * Retorna o código de tecla ou -1 em erro.
 */
static int recebe_movimento(int sock) {
    unsigned char buf[sizeof(msg_movimento_t) + 32];
    size_t tam = 0;

    if (receive_buffer(sock, buf, sizeof(buf), &tam) == -1) {
        fprintf(stderr, "SERVER: receive_buffer falhou\n");
        return -1;
    }

    if (tam < sizeof(msg_movimento_t)) {
        fprintf(stderr, "SERVER: MSG_MOVE truncado (%zu bytes)\n", tam);
        return -1;
    }

    msg_movimento_t *msg = (msg_movimento_t *)buf;

    if (msg->tipo != MSG_MOVE) {
        fprintf(stderr, "SERVER: tipo inesperado 0x%02x (esperado MSG_MOVE)\n",
                msg->tipo);
        return -1;
    }

    printf("SERVER: movimento recebido '%c'\n", msg->direcao);

    /*
     * direction_from_key() aceita 'w','s','a','d' (e KEY_UP etc. do ncurses).
     * Como o servidor não usa ncurses, apenas passamos o char diretamente —
     * os cases 'w'/'W', 's'/'S', 'a'/'A', 'd'/'D' batem exatamente.
     */
    return (int)msg->direcao;
}

/* ═══════════════════════════════════════════════════════════════════════════════
 * Loop principal do jogo
 *
 * Usa play_round() de game.c como única fonte de verdade sobre o estado do jogo.
 * play_round() retorna:
 *   0   → rodada normal (continua)
 *   1   → vitória (6 pastilhas coletadas)
 *  -1   → game over (colisão com fantasma)
 * ═══════════════════════════════════════════════════════════════════════════════ */

static void loop_jogo(int sock, Game *g) {

    /* Estado da pastilha antes de cada rodada — para detectar coleta */
    char mapa_antes[MAZE_SIZE][MAZE_SIZE];

    /* Envia o mapa inicial para o cliente poder exibir o estado de partida */
    printf("SERVER: enviando estado inicial\n");
    if (envia_mapa(sock, g) == -1) {
        fprintf(stderr, "SERVER: falha ao enviar mapa inicial\n");
        return;
    }

    while (1) {

        /* ── 1. Recebe o movimento do cliente ── */
        int ch = recebe_movimento(sock);
        if (ch == -1) {
            fprintf(stderr, "SERVER: erro ao receber movimento. Encerrando.\n");
            return;
        }

        /* ── 2. Salva snapshot do maze para detectar coleta de pastilha ── */
        for (int l = 0; l < MAZE_SIZE; l++)
            memcpy(mapa_antes[l], g->maze[l], MAZE_SIZE);

        /* ── 3. Processa a rodada completa via play_round() ── */
        int resultado = play_round(g, ch);

        /* ── 4. Detecta se alguma pastilha foi coletada nesta rodada ── */
        uint8_t id_pastilha_coletada = 0;
        for (int l = 0; l < MAZE_SIZE && !id_pastilha_coletada; l++) {
            for (int c = 0; c < MAZE_SIZE && !id_pastilha_coletada; c++) {
                char antes = mapa_antes[l][c];
                char depois = g->maze[l][c];
                if (antes >= '1' && antes <= '6' && depois == '0') {
                    id_pastilha_coletada = (uint8_t)(antes - '0');
                }
            }
        }

        /* ── 5. Despacha resposta conforme resultado ── */

        if (resultado == 1) {
            /* Vitória */
            if (id_pastilha_coletada) {
                envia_arquivo_premio(sock, id_pastilha_coletada);
            }
            envia_status(sock, MSG_WIN, g);
            printf("SERVER: jogo encerrado — VITÓRIA em %d rodadas\n",
                   g->moves_count);
            return;
        }

        if (resultado == -1) {
            /* Game over por colisão */
            envia_arquivo_encontro(sock);
            envia_status(sock, MSG_GAMEOVER, g);
            printf("SERVER: jogo encerrado — GAME OVER na rodada %d\n",
                   g->moves_count);
            return;
        }

        /* Rodada normal: envia arquivo prêmio se coletou pastilha */
        if (id_pastilha_coletada) {
            printf("SERVER: pastilha %d coletada (%d/%d)\n",
                   id_pastilha_coletada, g->prizes_collected, PRIZES);
            if (envia_arquivo_premio(sock, id_pastilha_coletada) == -1) {
                fprintf(stderr, "SERVER: falha ao enviar arquivo prêmio\n");
                return;
            }
        }

        /* Envia o mapa atualizado */
        if (envia_mapa(sock, g) == -1) {
            fprintf(stderr, "SERVER: falha ao enviar mapa\n");
            return;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════════
 * main
 * ═══════════════════════════════════════════════════════════════════════════════ */

int main(int argc, char *argv[]) {

    srand((unsigned int)time(NULL));

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <interface> [labirinto.csv]\n", argv[0]);
        fprintf(stderr, "Exemplo: %s eth0 labirinto.csv\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *interface = argv[1];
    const char *arq_mapa  = (argc >= 3) ? argv[2] : NULL;

    /* ── Cria o raw socket ── */
    int sock = cria_raw_socket((char *)interface);
    if (sock < 0) {
        fprintf(stderr, "SERVER: falha ao criar socket em %s\n", interface);
        return EXIT_FAILURE;
    }
    printf("SERVER: socket criado (fd=%d)\n", sock);

    /* ── Inicializa o Game usando as funções de game.c ── */
    Game *g = init_game();
    if (!g) {
        fprintf(stderr, "SERVER: init_game falhou\n");
        close(sock);
        return EXIT_FAILURE;
    }

    if (arq_mapa) {
        printf("SERVER: carregando CSV '%s'\n", arq_mapa);
        if (load_csv_level(g, arq_mapa) == -1) {
            fprintf(stderr, "SERVER: falha no CSV, usando labirinto padrão\n");
            load_default_level(g);
        }
    } else {
        printf("SERVER: usando labirinto padrão\n");
        load_default_level(g);
    }

    printf("SERVER: Pacman em (%d,%d), raio=%d\n",
           g->pacman.x, g->pacman.y, g->vision_radius);
    printf("SERVER: aguardando cliente...\n\n");

    /* ── Loop do jogo ── */
    loop_jogo(sock, g);

    free_game(g);
    
    close(sock);
    printf("\nSERVER: encerrado.\n");
    return EXIT_SUCCESS;
}
