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

    const char *arquivo_saida = "recebido_resultado.txt";

    printf("[RECEIVER] Aguardando início de transmissão pelo protocolo...\n");
    
    // Fica em loop na máquina de estados aguardando os frames até o FINAL_TYPE
    int resultado = receive_file(sock, arquivo_saida);

    if (resultado == 1) {
        printf("[RECEIVER] SUCESSO: Arquivo totalmente reconstruído em '%s'!\n", arquivo_saida);
        
        // Exibe o conteúdo do arquivo reconstruído para conferência manual visual
        printf("[RECEIVER] Conteúdo do arquivo gravado:\n\"");
        FILE *f = fopen(arquivo_saida, "r");
        if (f) {
            int ch;
            while ((ch = fgetc(f)) != EOF) {
                putchar(ch);
            }
            fclose(f);
        }
        printf("\"\n");
    } else {
        fprintf(stderr, "[RECEIVER] FALHA: Erro no meio da recepção ou estourou timeouts.\n");
    }

    close(sock);
    return (resultado == 1) ? EXIT_SUCCESS : EXIT_FAILURE;

    return EXIT_SUCCESS;
}
