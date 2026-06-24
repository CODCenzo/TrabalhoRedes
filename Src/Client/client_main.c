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

    // uint8_t m[40][40]; 
    // memset(m, 0, sizeof(m));

    // // Agora m passa o endereço do bloco contínuo perfeitamente
    // if (receber_tabuleiro_jogo(sock, m) == 1) {
    //     printf("CLIENT: Tabuleiro recebido e pronto para uso!\n");
    // }

    // imprimir_tabuleiro_jogo(m);

    if (servidor_envia_game_show(sock, SHOW_TYPE) == 1) {
        printf("MOVIMENTO ENVIADO COM SUCESSO\n");
    }

    close(sock);
    return EXIT_SUCCESS;
}
