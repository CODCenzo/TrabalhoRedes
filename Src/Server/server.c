
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../Headers/kermit.h"
#include "../../Headers/protocol.h"

int main() {
    printf("[SERVIDOR] Inicializando Raw Socket na interface 'lo'...\n");
    int socket_s = cria_raw_socket("lo");
    if (socket_s < 0) {
        fprintf(stderr, "[SERVIDOR] Falha ao criar socket.\n");
        return -1;
    }

    printf("[SERVIDOR] Aguardando recepção do arquivo...\n");
    // O arquivo recebido será salvo como "saida_teste.txt"
    int resultado = receive_file(socket_s, "saida_teste.txt");

    if (resultado > 0) {
        printf("[SERVIDOR] Arquivo recebido com sucesso e salvo como 'saida_teste.txt'!\n");
    } else {
        fprintf(stderr, "[SERVIDOR] Falha na recepção do arquivo.\n");
    }

    close(socket_s);
    return 0;
}