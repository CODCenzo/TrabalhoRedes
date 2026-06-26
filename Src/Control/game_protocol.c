#include "../../Headers/game_protocol.h"

#define printf(...) ((void)0)
#define fprintf(...) ((void)0)
#define perror(...) ((void)0)

void build_client_matrix_(Game *g, char **out) {
  int x, y, i;

  if (!g) {
    perror("erro build_client_matrix\n");
    exit(1);
  }

  for (y = 0; y < MAZE_SIZE; y++) {
    for (x = 0; x < MAZE_SIZE; x++) {
      if (!visible_to_pacman(g, x, y)) {
        out[y][x] = ' ';
      } else if (g->maze[y][x] == 'X') {
        out[y][x] = 'X';
      } else if (g->maze[y][x] >= '1' && g->maze[y][x] <= '6') {
        out[y][x] = g->maze[y][x];
      } else {
        out[y][x] = '.';
      }
    }
  }

  for (i = 0; i < GHOSTS; i++) {
    int gx = g->ghosts[i].body.x;
    int gy = g->ghosts[i].body.y;

    if (is_inside(gx, gy) && visible_to_pacman(g, gx, gy)) {
      out[gy][gx] = g->ghosts[i].symbol;
    }
  }

  if (is_inside(g->pacman.x, g->pacman.y)) {
    out[g->pacman.y][g->pacman.x] = 'P';
  }
}

int enviar_tabuleiro_jogo(int socket, char **tabuleiro) {
    int status;

    printf("[SERVER] Preparando envio do tabuleiro de %zu bytes...\n", sizeof(tabuleiro[40][40]) * 40 * 40);
    
    // Calcula o tamanho total da matriz na memória (1600 bytes se for uint8_t/char)
    size_t tamanho = 40 * sizeof(char);

    // Faz o cast da matriz para (const unsigned char*) para o send_buffer aceitar.
    // Usamos MSG_MAPA_TYPE no primeiro pacote para o cliente saber o que está entrando.
    for (int i = 0; i < 40; i++) {
        status = send_buffer(socket, (const unsigned char *)tabuleiro[i], tamanho, DATA_TYPE, DATA_TYPE);
        if (status != 1) {
            fprintf(stderr, "[SERVER] ERRO ao enviar a linha %d do tabuleiro.\n", i);
            return -1;
        }
    }

    return 1 ;
}

int receber_tabuleiro_jogo(int socket, uint8_t tabuleiro_destino[40][40]) {
    int status;

    printf("[CLIENT] Aguardando o envio do tabuleiro pelo servidor...\n");

    size_t capacidade_maxima = 40 * sizeof(tabuleiro_destino[0][0]);
    size_t total_bytes_recebidos = 0;

    for (int i = 0; i < 40; i++) {
        // Faz o cast da matriz de destino para (unsigned char*) para que a função escreva direto nela
        status = receive_buffer(socket, (unsigned char *)tabuleiro_destino[i], capacidade_maxima, &total_bytes_recebidos);

        if (status == 1) {
        if (total_bytes_recebidos == capacidade_maxima) {

            } else {
                fprintf(stderr, "[CLIENT] ERRO: Tamanho recebido (%zu) difere do esperado (%zu).\n", 
                        total_bytes_recebidos, capacidade_maxima);
                return -1;
            }

        }
    }
    printf("[CLIENT] Tabuleiro recebido com sucesso.\n");

    return 1 ;  
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
// Envia um pacote de 0 bytes onde o próprio TYPE indica a direção desejada
    int status = send_packet_with_retry(socket, 0, 0, tipo_movimento, NULL);
    return status;
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

    // Proteção contra ACKs perdidos: Se o cliente reenviou o pacote anterior cujo ACK sumiu na rede
    /*if (p->seq == 0) {
        printf("[SERVER] Pacote duplicado detectado (Seq: %d). Reenviando ACK...\n", p->seq);
        sendMsg(socket, 0, p->seq, ACK_TYPE, NULL);
        kermit_free(p);
        return 0;
    }

    // Se o pacote estiver completamente fora de ordem sequencial
    if (p->seq != 0) {
        printf("[SERVER] Sequência incorreta. Esperada: 0, Recebida: %d\n", p->seq);
        sendMsg(socket, 0, p->seq, NACK_TYPE, NULL);
        kermit_free(p);
        return -1;
    }*/

    // Verifica se o pacote é de fato um comando de movimento do personagem
    if (p->type == MOVE_UP_TYPE   || p->type == MOVE_DOWN_TYPE || 
        p->type == MOVE_LEFT_TYPE || p->type == MOVE_RIGHT_TYPE ) {
        
        printf("[SERVER] Comando de movimento recebido com sucesso (Tipo: %d, Seq: %d)!\n", p->type, p->seq);
        
        // Guarda o tipo do movimento na variável de saída para o jogo processar
        *tipo_movimento_recebido = p->type;

        // Envia o ACK confirmando para o cliente que a jogada foi aceita
        sendMsg(socket, 0, p->seq, ACK_TYPE, NULL);

        kermit_free(p);
        return 1; // Sucesso
    }

    // Caso receba outro tipo de pacote inesperado
    kermit_free(p);
    return -1;
}

int server_send_prize_collected(int socket, int prize_type) {
    
    int status ;
    unsigned char *buf = malloc(sizeof(int));

    memset(buf, prize_type, sizeof(int));

    status = send_packet_with_retry(socket, sizeof(int), 0, prize_type, buf);
    if (status != 1) {
        fprintf(stderr, "[SERVER] ERRO ao enviar mensagem de prêmio coletado.\n");
        free(buf);
        return -1;
    }

    free(buf);
    return 0;
}

int client_receive_prize_collected(int socket, int *prize_type, int *number) {

    unsigned char buffer[MAX_FRAME_SIZE];
    
    // Aguarda uma mensagem da rede com o timeout padrão (ex: 300ms)
    int bytes_lidos = recebe_mensagem(socket, DEFAULT_TIMEOUT_MS, buffer, MAX_FRAME_SIZE);

    if (bytes_lidos == -1) {
        // Timeout normal da rede, nenhum comando foi enviado nessa janela de tempo
        return 0; 
    }

    // Transforma o buffer capturado na estrutura Kermit
    struct kermit *p = parsing_kermit(buffer, bytes_lidos);
    if (p == NULL) {
        fprintf(stderr, "[SERVER] Falha no parsing do pacote de movimento recebido.\n");
        return -1;
    }

    // Verifica se o pacote é de fato um comando de movimento do personagem
    if (p->type == TXT_TYPE || p->type == MP4_TYPE || p->type == JPG_TYPE
        || p->type == DATA_TYPE) {
        
        printf("[SERVER] Comando de arquivo (Tipo: %d, Seq: %d)!\n", p->type, p->seq);
        
        *prize_type = p->type;
        memcpy(number, p->dados, sizeof(int)); 

        // ack
        sendMsg(socket, 0, p->seq, ACK_TYPE, NULL);

        kermit_free(p);
        return 1; // Sucesso
    }

    //caso que nao tem premio 
    if (p->type == ERROR_TYPE) {
        printf("[SERVER] Comando de nao-premio (Tipo: %d, Seq: %d)!\n", p->type, p->seq);

        *prize_type = -1 ;
        *number = -1 ;

        // ACK 
        sendMsg(socket, 0, p->seq, ACK_TYPE, NULL);

        kermit_free(p);
        return 2; // Sucesso
    }

    // Caso receba outro tipo de pacote inesperado
    kermit_free(p);
    return -1;
}