#include "../../Headers/game_protocol.h"

int enviar_tabuleiro_jogo(int socket, uint8_t tabuleiro[40][40]) {
    printf("[SERVER] Preparando envio do tabuleiro de %zu bytes...\n", sizeof(tabuleiro[40][40]) * 40 * 40);
    
    // Calcula o tamanho total da matriz na memória (1600 bytes se for uint8_t/char)
    size_t tamanho_total = 40 * 40 * sizeof(tabuleiro[0][0]);

    // Faz o cast da matriz para (const unsigned char*) para o send_buffer aceitar.
    // Usamos MSG_MAPA_TYPE no primeiro pacote para o cliente saber o que está entrando.
    int status = send_buffer(socket, (const unsigned char *)tabuleiro, tamanho_total, DATA_TYPE, DATA_TYPE);

    if (status == 1) {
        printf("[SERVER] Tabuleiro enviado e confirmado com sucesso!\n");
        return 1;
    } else {
        fprintf(stderr, "[SERVER] ERRO ao enviar o tabuleiro (timeouts/limite de retransmissões).\n");
        return -1;
    }
}

int receber_tabuleiro_jogo(int socket, uint8_t tabuleiro_destino[40][40]) {
    printf("[CLIENT] Aguardando o envio do tabuleiro pelo servidor...\n");

    size_t capacidade_maxima = 40 * 40 * sizeof(tabuleiro_destino[0][0]);
    size_t total_bytes_recebidos = 0;

    // Faz o cast da matriz de destino para (unsigned char*) para que a função escreva direto nela
    int status = receive_buffer(socket, (unsigned char *)tabuleiro_destino, capacidade_maxima, &total_bytes_recebidos);

    if (status == 1) {
        // Validação extra de segurança: conferir se o tamanho recebido bate com a matriz
        if (total_bytes_recebidos == capacidade_maxima) {
            printf("[CLIENT] Tabuleiro recebido e remontado com sucesso! (%zu bytes)\n", total_bytes_recebidos);
            return 1;
        } else {
            fprintf(stderr, "[CLIENT] ERRO: Tamanho recebido (%zu) difere do esperado (%zu).\n", 
                    total_bytes_recebidos, capacidade_maxima);
            return -1;
        }
    } else {
        fprintf(stderr, "[CLIENT] ERRO na recepção do tabuleiro via receive_buffer.\n");
        return -1;
    }
}

void imprimir_tabuleiro_jogo(uint8_t tabuleiro[40][40]) {
    printf("\n--- TABULEIRO DO JOGO (40x40) ---\n");
    
    for (int i = 0; i < 40; i++) {
        for (int j = 0; j < 40; j++) {
            // "%3d" garante que cada elemento ocupe exatamente 3 espaços,
            // mantendo as colunas perfeitamente alinhadas no terminal.
            printf("%c", tabuleiro[i][j]);
        }
        // Quebra de linha ao fim de cada linha da matriz
        printf("\n");
    }
    
    printf("---------------------------------\n\n");
}

/**
 * Envia uma mensagem de movimento do cliente para o servidor.
 * Parâmetro 'tipo_movimento' deve ser: MOVE_UP_TYPE, MOVE_DOWN_TYPE, MOVE_LEFT_TYPE ou MOVE_RIGHT_TYPE.
 * * Retorna 1 em caso de sucesso (ACK recebido), ou -1 em caso de erro/timeout.
 */
// int cliente_enviar_movimento(int socket, uint8_t tipo_movimento) {
//     printf("[CLIENT] Enviando comando de movimento (Tipo: %d, Seq: %d)...\n", tipo_movimento, *sequencia_atual);

//     // Enviamos um pacote com 0 bytes de dados, pois o próprio TYPE já indica a direção.
//     // O send_packet_with_retry cuidará de reenviar caso o pacote ou o ACK se perca.
//     int status = send_packet_with_retry(socket, 0, *sequencia_atual, tipo_movimento, NULL);

//     if (status == 1) {
//         printf("[CLIENT] Movimento confirmado pelo servidor!\n");
//         return 1;
//     } else {
//         fprintf(stderr, "[CLIENT] ERRO: Falha ao enviar movimento (timeout/retransmissões esgotadas).\n");
//         return -1;
//     }
// }