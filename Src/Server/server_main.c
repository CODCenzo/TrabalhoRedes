#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "../../Headers/kermit.h"
#include "../../Headers/protocol.h"
#include "../../Headers/game_protocol.h"
#include "../../Headers/game.h"
#include "../../Headers/draw.h"

int main(int argc, char *argv[]) {

    srand((unsigned int)time(NULL));

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <interface> [labirinto.csv]\n", argv[0]);
        fprintf(stderr, "Exemplo: %s eth0 labirinto.csv\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *interface = argv[1];
    // const char *arq_mapa  = (argc >= 3) ? argv[2] : NULL;

    /* ── Cria o raw socket ── */
    int sock = cria_raw_socket((char *)interface);
    if (sock < 0) {
        fprintf(stderr, "SERVER: falha ao criar socket em %s\n", interface);
        return EXIT_FAILURE;
    }
    printf("SERVER: socket criado (fd=%d)\n", sock);

    
    unsigned char buffer_saida[512];
    size_t tamanho_reconstruido = 0;
    
    printf("[RECEPTOR] Iniciando a máquina de estados do receive_buffer...\n");
    
    // Aguarda e reconstrói todos os fragmentos até encontrar o FINAL_TYPE
    int status = receive_buffer(sock, buffer_saida, sizeof(buffer_saida), &tamanho_reconstruido);
    
    if (status == 1) {
        printf("\n==============================================\n");
        printf("[RECEPTOR] SUCESSO! Buffer remontado com sucesso.\n");
        printf("[RECEPTOR] Total de bytes recebidos: %zu\n", tamanho_reconstruido);
        
        // Finaliza a string para impressão segura
        buffer_saida[tamanho_reconstruido] = '\0';
        printf("[RECEPTOR] Conteúdo final:\n\"%s\"\n", buffer_saida);
        printf("==============================================\n");
    } else {
        fprintf(stderr, "[RECEPTOR] ERRO: Falha ao remontar o buffer ou estourou timeouts.\n");
    }

    close(sock);
    return EXIT_SUCCESS;
}
