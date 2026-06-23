#ifndef GAME_PROTOCOL_H
#define GAME_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

/* ═══════════════════════════════════════════════════════════════════════════════
 * Constantes do labirinto e do jogo
 * ═══════════════════════════════════════════════════════════════════════════════ */

#define MAPA_LINHAS         40
#define MAPA_COLUNAS        40
#define TOTAL_PASTILHAS     6
#define RAIO_INICIAL        1
#define RODADAS_POR_RAIO    5   /* a cada N rodadas o raio de visão aumenta 1 */

/* Células do labirinto (mesmo encoding do CSV) */
#define CELL_EMPTY          '0'
#define CELL_WALL           'X'
#define CELL_PACMAN         'P'
#define CELL_GHOST_RED      'R'
#define CELL_GHOST_BLUE     'B'
#define CELL_GHOST_GREEN    'G'
#define CELL_GHOST_YELLOW   'Y'
#define CELL_PILL_1         '1'   /* pastilha → arquivo 1.txt  */
#define CELL_PILL_2         '2'   /* pastilha → arquivo 2.txt  */
#define CELL_PILL_3         '3'   /* pastilha → arquivo 3.jpg  */
#define CELL_PILL_4         '4'   /* pastilha → arquivo 4.jpg  */
#define CELL_PILL_5         '5'   /* pastilha → arquivo 5.mp4  */
#define CELL_PILL_6         '6'   /* pastilha → arquivo 6.mp4  */

/* ═══════════════════════════════════════════════════════════════════════════════
 * Tipos de mensagem da camada de aplicação
 * (transportados no campo TYPE do frame Kermit ou no primeiro byte de DADOS)
 * ═══════════════════════════════════════════════════════════════════════════════ */

/* Kermit reserva TYPE em 5 bits (0-31).
 * ACK=0, NACK=1, FINAL=0x1F são do protocolo de transporte.
 * Os tipos abaixo usam valores 2-14 para a camada de aplicação. */

#define MSG_MOVE            0x02  /* cliente → servidor: intenção de movimento  */
#define MSG_MAP             0x03  /* servidor → cliente: janela de visão do mapa */
#define MSG_FILE_START      0x04  /* servidor → cliente: início de arquivo prêmio */
#define MSG_GHOST_ENCOUNTER 0x05  /* servidor → cliente: início de arquivo encontro */
#define MSG_GAMEOVER        0x06  /* servidor → cliente: fim de jogo (morreu)    */
#define MSG_WIN             0x07  /* servidor → cliente: vitória (6 pastilhas)   */
#define DATA_TYPE           0x08  /* pacotes intermediários de send_buffer        */

/* ═══════════════════════════════════════════════════════════════════════════════
 * Direções de movimento
 * ═══════════════════════════════════════════════════════════════════════════════ */

#define DIR_UP              'w'
#define DIR_DOWN            's'
#define DIR_LEFT            'a'
#define DIR_RIGHT           'd'
#define DIR_NONE            0

/* ═══════════════════════════════════════════════════════════════════════════════
 * Structs de mensagem — todas com __attribute__((packed)) para serialização direta
 * ═══════════════════════════════════════════════════════════════════════════════ */

/*
 * msg_movimento_t  (cliente → servidor)
 * Tamanho fixo: 31 bytes (= MAX_DADOS), conforme especificado no enunciado.
 */
typedef struct __attribute__((packed)) {
    uint8_t tipo;           /* MSG_MOVE                             */
    uint8_t direcao;        /* DIR_UP / DIR_DOWN / DIR_LEFT / DIR_RIGHT */
    uint8_t _pad[29];       /* padding para completar MAX_DADOS     */
} msg_movimento_t;

/*
 * msg_mapa_t  (servidor → cliente)
 * Cabeçalho de 6 bytes + células da janela visível em ordem row-major.
 * Tamanho das células: (2*raio+1)^2, máximo (2*7+1)^2 = 225 bytes.
 * Total máximo: 6 + 225 = 231 bytes → cabe em múltiplos pacotes via send_buffer.
 */
typedef struct __attribute__((packed)) {
    uint8_t tipo;               /* MSG_MAP                          */
    uint8_t pacman_linha;       /* posição linha do Pacman no mapa  */
    uint8_t pacman_col;         /* posição coluna do Pacman no mapa */
    uint8_t raio;               /* raio de visão atual              */
    uint8_t pastilhas_pegas;    /* quantas pastilhas já coletadas   */
    uint8_t rodada;             /* número da rodada atual           */
    uint8_t celulas[225];       /* janela visível, row-major        */
} msg_mapa_t;

/*
 * msg_status_t  (servidor → cliente)
 * Mensagem simples de controle: game over ou vitória.
 */
typedef struct __attribute__((packed)) {
    uint8_t tipo;               /* MSG_GAMEOVER ou MSG_WIN          */
    uint8_t pastilhas_pegas;
    uint8_t rodada;
    uint8_t _pad[28];
} msg_status_t;

/*
 * msg_file_header_t  (servidor → cliente)
 * Primeiro pacote de uma transferência de arquivo prêmio ou encontro.
 * O conteúdo binário do arquivo segue nos pacotes subsequentes via send_buffer.
 */
typedef struct __attribute__((packed)) {
    uint8_t  tipo;              /* MSG_FILE_START ou MSG_GHOST_ENCOUNTER */
    uint8_t  id_pastilha;       /* qual pastilha (1-6) ou 0 p/ fantasma */
    uint8_t  ext;               /* extensão: 't'=txt, 'j'=jpg, 'm'=mp4  */
    uint8_t  _pad[28];
} msg_file_header_t;

/* ═══════════════════════════════════════════════════════════════════════════════
 * Tamanho máximo do buffer de recepção de arquivo
 * ═══════════════════════════════════════════════════════════════════════════════ */

#define MAX_RECEIVE_SIZE    (10 * 1024 * 1024)   /* 10 MB */

#endif /* GAME_PROTOCOL_H */
