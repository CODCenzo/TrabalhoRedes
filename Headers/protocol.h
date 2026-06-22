#include "kermit.h"

#define MAX_TIMEOUTS_SEGUIDOS 25

// Espera por resposta do tipo ACK ou NACK.
// Retorna ACK_TYPE, NACK_TYPE, 2 em caso de timeout e -1 em ERRO
int wait_response (int socket, uint8_t msgSequence);

// Envia o pacote diversas vezes.
// Retorna 1 em sucesso, -1 em falha ou após MAX_TENTATIVAS.
int send_packet_with_retry(int socket, int bytesLidos, uint8_t seq, uint8_t type, 
                            unsigned char *buf);

// Envia um arquivo completo em várias mensagens. Lógica stop-and-wait.
// Retorna 1 em caso de sucesso e -1 para erro.
int send_file (int socket, const char *filepath, int fileType);

// Cria um novo arquivo a partir das mensagens recebidas.
// Retorna 1 em caso de sucesso e -1 para erro.
int receive_file (int socket, const char *filepath);