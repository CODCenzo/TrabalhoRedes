#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "../../Headers/kermit.h"
#include "../../Headers/protocol.h"
#include "../../Headers/game_protocol.h"
#include "../../Headers/game.h"
#include "../../Headers/draw.h"

void gerar_arquivo_grande_teste(const char *nome, size_t tamanho) {
    FILE *f = fopen(nome, "wb");
    if (!f) return;
    for (size_t i = 0; i < tamanho; i++) {
        // Preenche com um padrão cíclico de caracteres de 'A' a 'Z'
        fputc('A' + (i % 26), f);
    }
    fclose(f);
    printf("[SENDER] Arquivo de teste '%s' criado com %zu bytes.\n", nome, tamanho);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <interface-de-rede>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s eth0\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *interface = argv[1];

    int sock = cria_raw_socket((char *)interface);
    if (sock < 0) {
        fprintf(stderr, "CLIENT: falha ao criar socket em %s\n", interface);
        return EXIT_FAILURE;
    }

    const char *arquivo_envio = "enviar_teste.txt";
    gerar_arquivo_grande_teste(arquivo_envio, 100);

    printf("[SENDER] Iniciando transmissão via protocol.c (send_file)...\n");
    // Passa 1 (ou um tipo qualquer mapeado para dados de arquivo)
    int resultado = send_file(sock, arquivo_envio, 1); 

    if (resultado == 1) {
        printf("[SENDER] SUCESSO: Todos os pacotes foram transmitidos e confirmados!\n");
    } else {
        fprintf(stderr, "[SENDER] FALHA: A transmissão falhou ou estourou o limite de tentativas.\n");
    }

    close(sock);
    unlink(arquivo_envio); // Remove o arquivo local temporário de teste
    return (resultado == 1) ? EXIT_SUCCESS : EXIT_FAILURE;
}
