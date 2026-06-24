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
int cliente_enviar_movimento(int socket, uint8_t tipo_movimento) {
    printf("[CLIENT] Enviando comando de movimento (Tipo: %d)...\n", tipo_movimento);

    // Enviamos um pacote com 0 bytes de dados, pois o próprio TYPE já indica a direção.
    // O send_packet_with_retry cuidará de reenviar caso o pacote ou o ACK se perca.
    int status = send_packet_with_retry(socket, 0, 0, tipo_movimento, NULL);

    if (status == 1) {
        printf("[CLIENT] Movimento confirmado pelo servidor!\n");
        return 1;
    } else {
        fprintf(stderr, "[CLIENT] ERRO: Falha ao enviar movimento (timeout/retransmissões esgotadas).\n");
        return -1;
    }
}

/**
 * Aguarda uma mensagem de movimento vinda do cliente.
 * Se receber um movimento válido, preenche 'tipo_movimento_recebido' e envia o ACK correspondente.
 * * Retorna 1 se um movimento válido foi processado, 0 em caso de timeout, ou -1 em caso de erro crítico.
 */
int servidor_receber_movimento(int socket, uint8_t *tipo_movimento_recebido) {
    unsigned char buffer_captura[MAX_FRAME_SIZE];
    
    // Aguarda uma mensagem da rede com o timeout padrão (ex: 300ms)
    int bytes_lidos = recebe_mensagem(socket, DEFAULT_TIMEOUT_MS, buffer_captura, MAX_FRAME_SIZE);

    if (bytes_lidos == -1) {
        // Timeout normal da rede, nenhum comando foi enviado nessa janela de tempo
        return 0; 
    }

    // Transforma o buffer capturado na estrutura Kermit
    struct kermit *p = parsing_kermit(buffer_captura, bytes_lidos);
    if (p == NULL) {
        fprintf(stderr, "[SERVER] Falha no parsing do pacote de movimento recebido.\n");
        return -1;
    }

    // Verifica se o número de sequência bate com o esperado
    if (p->seq != 0) {
        printf("[SERVER] Pacote descartado: Sequência errada (Recebida: %d, Esperada: 0)\n", p->seq);
        // Envia um NACK ou ACK antigo dependendo da sua regra de controle de fluxo
        sendMsg(socket, 0, p->seq, NACK_TYPE, NULL);
        kermit_free(p);
        return -1;
    }

    // Verifica se o pacote é de fato um comando de movimento do personagem
    if (p->type == MOVE_UP_TYPE   || p->type == MOVE_DOWN_TYPE || 
        p->type == MOVE_LEFT_TYPE || p->type == MOVE_RIGHT_TYPE) {
        
        printf("[SERVER] Comando de movimento recebido com sucesso (Tipo: %d, Seq: %d)!\n", p->type, p->seq);
        
        // Guarda o tipo do movimento na variável de saída para o jogo processar
        *tipo_movimento_recebido = p->type;

        // Envia o ACK confirmando para o cliente que a jogada foi aceita
        sendMsg(socket, 0, 0, ACK_TYPE, NULL);

        kermit_free(p);
        return 1; // Sucesso
    }

    // Caso receba outro tipo de pacote inesperado
    kermit_free(p);
    return -1;
}

int servidor_envia_game_show (int socket, uint8_t tipo_msg) {
    printf("[SERVIDOR] Enviando comando de movimento (Tipo: %d)...\n", tipo_msg);

    // Enviamos um pacote com 0 bytes de dados, pois o próprio TYPE já indica a direção.
    // O send_packet_with_retry cuidará de reenviar caso o pacote ou o ACK se perca.
    int status = send_packet_with_retry(socket, 0, 0, tipo_msg, NULL);

    if (status == 1) {
        printf("[SERVIDOR] Mensagem enviada pelo servidor!\n");
        return 1;
    } else {
        fprintf(stderr, "[SERVIDOR] ERRO: Falha ao enviar game show (timeout/retransmissões esgotadas).\n");
        return -1;
    }
}

int client_receber_game_show (int socket, uint8_t *tipo_msg) {
    unsigned char buffer_captura[MAX_FRAME_SIZE];
    
    // Aguarda uma mensagem da rede com o timeout padrão (ex: 300ms)
    int bytes_lidos = recebe_mensagem(socket, DEFAULT_TIMEOUT_MS, buffer_captura, MAX_FRAME_SIZE);

    if (bytes_lidos == -1) {
        // Timeout normal da rede, nenhum comando foi enviado nessa janela de tempo
        return 0; 
    }

    // Transforma o buffer capturado na estrutura Kermit
    struct kermit *p = parsing_kermit(buffer_captura, bytes_lidos);
    if (p == NULL) {
        fprintf(stderr, "[CLIENT] Falha no parsing do pacote de show recebido.\n");
        return -1;
    }

    // Verifica se o número de sequência bate com o esperado
    if (p->seq != 0) {
        printf("[CLIENT] Pacote descartado: Sequência errada (Recebida: %d, Esperada: 0)\n", p->seq);
        // Envia um NACK ou ACK antigo dependendo da sua regra de controle de fluxo
        sendMsg(socket, 0, p->seq, NACK_TYPE, NULL);
        kermit_free(p);
        return -1;
    }

    // Verifica se o pacote é de fato um comando de movimento do personagem
    if (p->type == SHOW_TYPE) {
        
        printf("[CLIENT] Comando de show recebido com sucesso (Tipo: %d, Seq: %d)!\n", p->type, p->seq);
        
        // Guarda o tipo do movimento na variável de saída para o jogo processar
        *tipo_msg = p->type;

        // Envia o ACK confirmando para o cliente que a jogada foi aceita
        sendMsg(socket, 0, 0, ACK_TYPE, NULL);

        kermit_free(p);
        return 1; // Sucesso
    }

    // Caso receba outro tipo de pacote inesperado
    kermit_free(p);
    return -1;
}