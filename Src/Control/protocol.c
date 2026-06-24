#include "../../Headers/protocol.h"
#include "../../Headers/kermit.h"

/* ─── wait_response ─────────────────────────────────────────────────────────── */

int wait_response(int socket, uint8_t msgSequence) {

    unsigned char bufferDeCaptura[MAX_FRAME_SIZE];
    int tamanhoCapturado = recebe_mensagem(socket, DEFAULT_TIMEOUT_MS,
                                           bufferDeCaptura, MAX_FRAME_SIZE);

    if (tamanhoCapturado == -1) {
        printf("WAIT_RESPONSE: TIMEOUT SEQ %d\n", msgSequence);
        return 2;
    }

    struct kermit *p = parsing_kermit(bufferDeCaptura, tamanhoCapturado);

    if (p == NULL) {
        printf("WAIT_RESPONSE: PARSING FALHOU SEQ %d\n", msgSequence);
        return -1;
    }

    /* Sequência errada – trata como NACK para forçar retransmissão */
    if (p->seq != msgSequence) {
        printf("WAIT_RESPONSE: SEQ ERRADA recebida=%d esperada=%d tipo=%d\n",
               p->seq, msgSequence, p->type);
        kermit_free(p);
        return NACK_TYPE;
    }

    /* ACK correto */
    if (p->type == ACK_TYPE) {
        //printf("WAIT_RESPONSE: ACK SEQ %d OK\n", p->seq);
        kermit_free(p);
        return ACK_TYPE;
    }

    /* Recebeu outra coisa (NACK explícito, pacote de dados fora de lugar…) */
    printf("WAIT_RESPONSE: RESPOSTA INESPERADA tipo=%d seq=%d\n", p->type, p->seq);
    kermit_free(p);
    return -1;
}

/* ─── send_packet_with_retry ────────────────────────────────────────────────── */

int send_packet_with_retry(int socket, int bytesLidos, uint8_t seq,
                           uint8_t type, unsigned char *buf) {
    for (int tentativas = 0; tentativas < MAX_TENTATIVAS_ENVIO; tentativas++) {

        if (sendMsg(socket, bytesLidos, seq, type, buf) == -1) {
            fprintf(stderr, "SEND_RETRY: ERRO AO ENVIAR SEQ %d\n", seq);
            return -1;          /* erro de sistema, não adianta retransmitir */
        }

        int resp = wait_response(socket, seq);
        if (resp == ACK_TYPE) {
            return 1;
        }

        printf("SEND_RETRY: tentativa %d/%d SEQ %d (resp=%d)\n",
               tentativas + 1, MAX_TENTATIVAS_ENVIO, seq, resp);
    }

    fprintf(stderr, "SEND_RETRY: TENTATIVAS ESGOTADAS SEQ %d\n", seq);
    return -1;
}

/* ─── send_buffer ───────────────────────────────────────────────────────────── */

/*
 * Divide `data` em fatias de MAX_DADOS bytes e envia cada uma como um pacote
 * Kermit independente via stop-and-wait.
 *
 * Layout dos tipos por pacote:
 *
 *   tamanho total == 0          → 1 pacote vazio, type = FINAL_TYPE
 *   tamanho total <= MAX_DADOS  → 1 pacote, type = FINAL_TYPE
 *   tamanho total >  MAX_DADOS  → N pacotes:
 *       pacote 0            type = firstType
 *       pacotes 1 .. N-2    type = midType
 *       pacote N-1          type = FINAL_TYPE
 *
 * A sequência é local a esta transferência e começa em 0, rolando módulo
 * SEQ_MODULO conforme o protocolo Kermit.
 */
int send_buffer(int socket, const unsigned char *data, size_t size,
                uint8_t firstType, uint8_t midType) {

    if (socket < 0 || data == NULL) {
        fprintf(stderr, "SEND_BUFFER: parametro invalido\n");
        return -1;
    }

    /* Caso especial: buffer vazio → manda um único pacote FINAL sem dados */
    if (size == 0) {
        printf("SEND_BUFFER: buffer vazio, enviando pacote FINAL\n");
        return send_packet_with_retry(socket, 0, 0, FINAL_TYPE, NULL);
    }

    /* Calcula o número de pacotes necessários */
    size_t totalPacotes = (size + MAX_DADOS - 1) / MAX_DADOS;

    //printf("SEND_BUFFER: %zu bytes → %zu pacote(s) de ate %d bytes\n",
    //       size, totalPacotes, MAX_DADOS);

    uint8_t seq = 0;
    size_t  offset = 0;

    for (size_t i = 0; i < totalPacotes; i++) {

        /* Quantos bytes neste pacote */
        size_t  fatia   = size - offset;
        uint8_t tamPkt  = (fatia > MAX_DADOS) ? (uint8_t)MAX_DADOS : (uint8_t)fatia;

        /* Determina o tipo do pacote */
        uint8_t tipo;
        if (i == totalPacotes - 1) {
            tipo = FINAL_TYPE;                 /* sempre, seja 1 ou N pacotes */
        } else if (i == 0) {
            tipo = firstType;
        } else {
            tipo = midType;
        }

        //printf("SEND_BUFFER: pacote %zu/%zu seq=%d tipo=%d tam=%d offset=%zu\n",
        //       i + 1, totalPacotes, seq, tipo, tamPkt, offset);

        /*
         * O cast de `const unsigned char *` para `unsigned char *` é necessário
         * porque send_packet_with_retry e sendMsg não declaram o buffer como const.
         * Os dados não são modificados internamente — apenas lidos para montar o frame.
         */
        if (send_packet_with_retry(socket, tamPkt, seq, tipo,
                                   (unsigned char *)(data + offset)) == -1) {
            fprintf(stderr, "SEND_BUFFER: falha no pacote %zu (seq=%d)\n", i, seq);
            return -1;
        }

        offset += tamPkt;
        seq     = (seq + 1) % SEQ_MODULO;
    }

    //printf("SEND_BUFFER: transferencia concluida (%zu bytes)\n", size);
    return 1;
}

/* ─── receive_buffer ────────────────────────────────────────────────────────── */

/*
 * Espelho de send_buffer: recebe pacotes Kermit em sequência e concatena os
 * dados em out_buf até receber um pacote com type == FINAL_TYPE.
 *
 * Envia ACK para cada pacote válido na ordem esperada.
 * Reenvia o ACK anterior se detectar retransmissão (seq == counter - 1).
 * Envia NACK se a sequência estiver completamente fora de ordem.
 */
int receive_buffer(int socket, unsigned char *out_buf,
                   size_t buf_capacity, size_t *out_size) {

    if (socket < 0 || out_buf == NULL || out_size == NULL) {
        fprintf(stderr, "RECEIVE_BUFFER: parametro invalido\n");
        return -1;
    }

    *out_size = 0;

    uint8_t counter      = 0;
    uint8_t auxType      = 0xFF;   /* sentinela: nenhum pacote recebido ainda */
    int     timeoutCount = 0;

    unsigned char bufferDeCaptura[MAX_FRAME_SIZE];

    do {
        int tamanhoCapturado = recebe_mensagem(socket, DEFAULT_TIMEOUT_MS,
                                               bufferDeCaptura, MAX_FRAME_SIZE);

        /* ── Timeout ── */
        if (tamanhoCapturado == -1) {
            //printf("RECEIVE_BUFFER: TIMEOUT seq esperada=%d (%d/%d)\n",
            //       counter, timeoutCount + 1, MAX_TIMEOUTS_SEGUIDOS);

            timeoutCount++;
            if (timeoutCount >= MAX_TIMEOUTS_SEGUIDOS) {
                fprintf(stderr, "RECEIVE_BUFFER: MAXIMO DE TIMEOUTS\n");
                return -1;
            }
            continue;
        }

        /* Pacote chegou: zera contador de timeouts consecutivos */
        timeoutCount = 0;

        struct kermit *p = parsing_kermit(bufferDeCaptura, tamanhoCapturado);
        if (p == NULL) {
            fprintf(stderr, "RECEIVE_BUFFER: parsing falhou seq esperada=%d\n", counter);
            return -1;
        }

        /* ── Sequência errada ── */
        if (p->seq != counter) {

            uint8_t seqAnterior = (counter - 1 + SEQ_MODULO) % SEQ_MODULO;

            if (p->seq == seqAnterior) {
                /* Remetente não recebeu o ACK anterior → reenvia */
                //printf("RECEIVE_BUFFER: ACK PERDIDO detectado, reenviando ACK seq=%d\n",
                //       p->seq);
                sendMsg(socket, 0, p->seq, ACK_TYPE, NULL);
            } else {
                //printf("RECEIVE_BUFFER: seq FORA DE ORDEM recebida=%d esperada=%d\n",
                //       p->seq, counter);
                sendMsg(socket, 0, counter, NACK_TYPE, NULL);
            }

            kermit_free(p);
            continue;
        }

        /* ── Pacote na ordem correta ── */

        /* Verifica se há espaço no buffer de saída */
        if (*out_size + p->tamDados > buf_capacity) {
            fprintf(stderr,
                    "RECEIVE_BUFFER: buffer cheio (capacidade=%zu, recebido=%zu, novo=%d)\n",
                    buf_capacity, *out_size, p->tamDados);
            kermit_free(p);
            return -1;
        }

        /* Copia os dados para o buffer de saída */
        if (p->tamDados > 0) {
            memcpy(out_buf + *out_size, p->dados, p->tamDados);
            *out_size += p->tamDados;
        }

        /* Envia ACK */
        sendMsg(socket, 0, counter, ACK_TYPE, NULL);
        //printf("RECEIVE_BUFFER: pacote seq=%d tipo=%d tam=%d total_acumulado=%zu\n",
        //       counter, p->type, p->tamDados, *out_size);

        auxType = p->type;
        counter = (counter + 1) % SEQ_MODULO;

        kermit_free(p);

    } while (auxType != FINAL_TYPE);

    //printf("RECEIVE_BUFFER: transferencia concluida (%zu bytes)\n", *out_size);
    return 1;
}

/* ─── send_file ─────────────────────────────────────────────────────────────── */

/*
 * Implementado sobre send_buffer: lê o arquivo inteiro para memória e delega.
 * Para arquivos muito grandes (vídeos) isso pode pressionar a memória disponível;
 * nesse caso considere um loop de leitura em chunks diretamente aqui.
 */
int send_file(int socket, const char *filepath, int fileType) {

    if (socket < 0 || filepath == NULL) {
        fprintf(stderr, "SEND_FILE: parametro invalido\n");
        return -1;
    }

    FILE *f = fopen(filepath, "rb");
    if (f == NULL) {
        perror("SEND_FILE: fopen");
        return -1;
    }

    /* Determina o tamanho do arquivo */
    fseek(f, 0, SEEK_END);
    long fileSizeSigned = ftell(f);
    rewind(f);

    if (fileSizeSigned < 0) {
        perror("SEND_FILE: ftell");
        fclose(f);
        return -1;
    }

    size_t fileSize = (size_t)fileSizeSigned;

    printf("SEND_FILE: %s (%zu bytes)\n", filepath, fileSize);

    /* Aloca buffer e lê o arquivo completo */
    unsigned char *buf = malloc(fileSize);
    if (buf == NULL) {
        fprintf(stderr, "SEND_FILE: malloc falhou para %zu bytes\n", fileSize);
        fclose(f);
        return -1;
    }

    size_t lido = fread(buf, 1, fileSize, f);
    fclose(f);

    if (lido != fileSize) {
        fprintf(stderr, "SEND_FILE: leitura incompleta %zu/%zu\n", lido, fileSize);
        free(buf);
        return -1;
    }

    /*
     * firstType = fileType  (indica ao receptor o tipo de conteúdo)
     * midType   = fileType  (mantém o mesmo indicador em todos os pacotes)
     * O último pacote sempre sai como FINAL_TYPE dentro de send_buffer.
     */
    int ret = send_buffer(socket, buf, fileSize, (uint8_t)fileType, (uint8_t)fileType);

    free(buf);
    return ret;
}

/* ─── receive_file ──────────────────────────────────────────────────────────── */

/*
 * Implementado sobre receive_buffer: aloca um buffer temporário de tamanho
 * máximo, recebe os dados e escreve no arquivo.
 *
 * MAX_RECEIVE_SIZE deve ser definido em kermit.h (ou aqui) com o tamanho
 * máximo esperado de uma transferência (ex: 10 MB para vídeos do trabalho).
 */
#ifndef MAX_RECEIVE_SIZE
#define MAX_RECEIVE_SIZE (10 * 1024 * 1024)   /* 10 MB */
#endif

int receive_file(int socket, const char *filepath) {

    if (socket < 0 || filepath == NULL) {
        fprintf(stderr, "RECEIVE_FILE: parametro invalido\n");
        return -1;
    }

    /* Aloca buffer temporário */
    unsigned char *buf = malloc(MAX_RECEIVE_SIZE);
    if (buf == NULL) {
        fprintf(stderr, "RECEIVE_FILE: malloc falhou\n");
        return -1;
    }

    size_t totalRecebido = 0;
    int ret = receive_buffer(socket, buf, MAX_RECEIVE_SIZE, &totalRecebido);

    if (ret == -1) {
        fprintf(stderr, "RECEIVE_FILE: receive_buffer falhou\n");
        free(buf);
        return -1;
    }

    /* Escreve os dados recebidos no arquivo de saída */
    FILE *f = fopen(filepath, "wb");
    if (f == NULL) {
        perror("RECEIVE_FILE: fopen");
        free(buf);
        return -1;
    }

    size_t escritos = fwrite(buf, 1, totalRecebido, f);
    fclose(f);
    free(buf);

    if (escritos != totalRecebido) {
        fprintf(stderr, "RECEIVE_FILE: escrita incompleta %zu/%zu\n",
                escritos, totalRecebido);
        return -1;
    }

    printf("RECEIVE_FILE: %s salvo (%zu bytes)\n", filepath, totalRecebido);
    return 1;
}
