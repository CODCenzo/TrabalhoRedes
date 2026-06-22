#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "kermit.h"

#define MAX_TIMEOUTS_SEGUIDOS 25
#define DEFAULT_TIMEOUT_MS 300

/* ─── wait_response ────────────────────────────────────────────────────────────
 * Aguarda uma resposta ACK ou NACK para a sequência msgSequence.
 *
 * Retorna:
 *   ACK_TYPE  – ACK recebido com a sequência esperada
 *   NACK_TYPE – NACK recebido, ou sequência errada
 *   2         – timeout
 *  -1         – erro (pacote corrompido / parse falhou)
 */
int wait_response(int socket, uint8_t msgSequence);

/* ─── send_packet_with_retry ───────────────────────────────────────────────────
 * Envia um único pacote e aguarda ACK, repetindo até MAX_TENTATIVAS_ENVIO vezes.
 *
 * Retorna:
 *   1  – ACK recebido com sucesso
 *  -1  – falha de envio ou tentativas esgotadas
 */
int send_packet_with_retry(int socket, int bytesLidos, uint8_t seq,
                           uint8_t type, unsigned char *buf);

/* ─── send_buffer ──────────────────────────────────────────────────────────────
 * Envia um buffer já em memória em múltiplos pacotes Kermit (stop-and-wait).
 * O primeiro pacote usa `firstType` como TYPE; todos os intermediários usam
 * `midType`; o último pacote sempre usa FINAL_TYPE.
 *
 * Quando o buffer cabe em um único pacote, esse pacote é enviado com FINAL_TYPE
 * (ignorando firstType e midType).
 *
 * Parâmetros:
 *   socket    – descritor do socket
 *   data      – ponteiro para os dados a enviar (não pode ser NULL)
 *   size      – número de bytes em data (pode ser 0: envia um pacote vazio FINAL)
 *   firstType – TYPE do primeiro pacote (ex: MSG_MAP, MSG_FILE_START …)
 *   midType   – TYPE dos pacotes intermediários (ex: DATA_TYPE)
 *
 * Retorna:
 *   1  – todos os pacotes enviados e confirmados
 *  -1  – erro de parâmetro, envio ou tentativas esgotadas
 */
int send_buffer(int socket, const unsigned char *data, size_t size,
                uint8_t firstType, uint8_t midType);

/* ─── receive_buffer ───────────────────────────────────────────────────────────
 * Recebe uma sequência de pacotes Kermit e reconstrói os dados em `out_buf`.
 * Para quando recebe um pacote com type == FINAL_TYPE.
 *
 * Parâmetros:
 *   socket       – descritor do socket
 *   out_buf      – buffer de saída pré-alocado pelo chamador
 *   buf_capacity – tamanho máximo de out_buf em bytes
 *   out_size     – preenchido com o total de bytes escritos em out_buf
 *
 * Retorna:
 *   1  – recepção concluída com sucesso
 *  -1  – erro (timeout máximo, parsing falhou, buffer cheio)
 */
int receive_buffer(int socket, unsigned char *out_buf,
                   size_t buf_capacity, size_t *out_size);

/* ─── send_file ────────────────────────────────────────────────────────────────
 * Envia um arquivo completo em vários pacotes (stop-and-wait).
 *
 * Retorna:
 *   1  – sucesso
 *  -1  – erro
 */
int send_file(int socket, const char *filepath, int fileType);

/* ─── receive_file ─────────────────────────────────────────────────────────────
 * Recebe pacotes e reconstrói o arquivo em filepath.
 *
 * Retorna:
 *   1  – sucesso
 *  -1  – erro
 */
int receive_file(int socket, const char *filepath);

#endif /* PROTOCOL_H */
