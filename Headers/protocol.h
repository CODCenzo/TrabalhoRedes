#include "kermit.h"

#define MAX_TIMEOUTS_SEGUIDOS 100

// Espera por resposta do tipo ACK ou NACK.
// Retorna ACK_TYPE, NACK_TYPE, 2 em caso de timeout e -1 em ERRO
int wait_response (int socket, uint8_t msgSequence);

// Envia um arquivo completo em várias mensagens. Lógica stop-and-wait.
// Retorna 1 em caso de sucesso e -1 para erro.
int send_file (int socket, const char *filepath, int fileType);

// Cria um novo arquivo a partir das mensagens recebidas.
// Retorna 1 em caso de sucesso e -1 para erro.
int receive_file (int socket);