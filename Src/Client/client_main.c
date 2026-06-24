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

    #define TAM_BUFFER_TESTE 150

    unsigned char dados_teste[TAM_BUFFER_TESTE + 1];
    
    // Preenche o buffer com um padrão textual conhecido ("ABCDE...")
    for (int i = 0; i < TAM_BUFFER_TESTE; i++) {
        dados_teste[i] = 'A' + (i % 26);
    }
    dados_teste[TAM_BUFFER_TESTE] = '\0';

    printf("[EMISSOR] Dados originais gerados (%d bytes).\n", TAM_BUFFER_TESTE);
    printf("[EMISSOR] Iniciando send_buffer fragmentado (Stop-and-Wait)...\n");

    // Tipos fictícios para emular cabeçalhos dinâmicos do seu jogo/sistema
    uint8_t primeiro_tipo = 5;  // Ex: Início de transmissão
    uint8_t tipo_meio = 1;      // Ex: DATA_TYPE padrão

    // Dispara a cadeia de retransmissões automáticas controlada por ACKs
    int status = send_buffer(sock, dados_teste, TAM_BUFFER_TESTE, primeiro_tipo, tipo_meio);

    if (status == 1) {
        printf("[EMISSOR] SUCESSO! Todos os fragmentos receberam ACK com sucesso.\n");
    } else {
        fprintf(stderr, "[EMISSOR] ERRO: Transmissão abortada (limite de tentativas excedido).\n");
    }

    close(sock);
    return EXIT_SUCCESS;
}
