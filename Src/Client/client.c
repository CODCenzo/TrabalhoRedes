#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../Headers/kermit.h"
#include "../../Headers/protocol.h"

void criar_arquivo_teste(const char *nome, int tamanho) {
    FILE *f = fopen(nome, "wb");
    if (!f) return;
    for (int i = 0; i < tamanho; i++) {
        // Preenche com caracteres visíveis sequenciais
        fputc('A' + (i % 26), f);
    }
    fclose(f);
}

int main() {
    const char *nome_arquivo = "entrada_teste.txt";
    // Criando um arquivo de 100 bytes (testará pacotes cheios e fracionados)
    printf("[CLIENTE] Criando arquivo de teste '%s' (100 bytes)...\n", nome_arquivo);
    criar_arquivo_teste(nome_arquivo, 100);

    printf("[CLIENTE] Inicializando Raw Socket na interface 'lo'...\n");
    int socket_c = cria_raw_socket("lo");
    if (socket_c < 0) {
        fprintf(stderr, "[CLIENTE] Falha ao criar socket. Garantiu privilégios de root?\n");
        return -1;
    }

    printf("[CLIENTE] Iniciando o envio do arquivo...\n");
    // TEXT_TYPE ou um ID numérico simulando o tipo de arquivo do protocolo
    int resultado = send_file(socket_c, nome_arquivo, 1); 

    if (resultado > 0) {
        printf("[CLIENTE] Arquivo enviado com sucesso!\n");
    } else {
        fprintf(stderr, "[CLIENTE] Falha ao enviar o arquivo.\n");
    }

    // Limpa o arquivo de teste local do cliente
    unlink(nome_arquivo);
    close(socket_c);
    return 0;
}