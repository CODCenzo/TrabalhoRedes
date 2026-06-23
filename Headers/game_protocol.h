#ifndef _GAME_PROTOCOL_H_
#define _GAME_PROTOCOL_H_

#include "kermit.h"
#include "protocol.h"

typedef enum {
    RECV_MAP,
    RECV_FILE,
    RECV_GAMEOVER,
    RECV_WIN,
    RECV_ERROR
} recv_result_t;


#endif