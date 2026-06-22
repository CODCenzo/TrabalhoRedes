#ifndef _GAME_PROTOCOL_H_
#define _GAME_PROTOCOL_H_

#include "kermit.h"
#include "protocol.h"

// Mensagem cliente → servidor (sempre 31 bytes = MAX_DADOS)
typedef struct __attribute__((packed)) {
    uint8_t  tipo;      // MSG_MOVE
    uint8_t  direcao;   // DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT
    uint8_t  reservado[29]; // padding para completar 31 bytes
} msg_movimento_t;

// Mensagem servidor → cliente com o mapa visível
typedef struct __attribute__((packed)) {
    uint8_t  tipo;          // MSG_MAP
    uint8_t  raio;          // raio atual de visão
    uint8_t  pacman_x;      // posição atual do pacman
    uint8_t  pacman_y;
    uint8_t  pastilhas_pegas;
    uint8_t  rodada;
    // os bytes restantes contêm as células visíveis serializadas
    uint8_t  celulas[25];   // (2*raio_max+1)^2 no máximo
} msg_mapa_t;

typedef enum {
    RECV_MAP,
    RECV_FILE,
    RECV_GAMEOVER,
    RECV_WIN,
    RECV_ERROR
} recv_result_t;

typedef struct {
    char     mapa[40][40];
    uint8_t  pacman_x, pacman_y;
    uint8_t  raio_visao;
    uint8_t  rodadas_totais;
    uint8_t  pastilhas_pegas;
    // posições dos fantasmas
    uint8_t  fantasma_x[4], fantasma_y[4];
    uint8_t  dir_fantasma[4]; // direção atual de cada fantasma
} game_state_t;

// Recebe uma mensagem da camada de aplicação e identifica o que fazer com ela
recv_result_t receive_game_message(int socket, unsigned char *out_buf, size_t buf_size);

#endif