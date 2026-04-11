#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Função que explicamos anteriormente
int cria_raw_socket(char* nome_interface_rede) {
    int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (soquete == -1) {
        perror("Erro ao criar socket");
        exit(-1);
    }

    int ifindex = if_nametoindex(nome_interface_rede);
    struct sockaddr_ll endereco = {0};
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ifindex;

    if (bind(soquete, (struct sockaddr*) &endereco, sizeof(endereco)) == -1) {
        perror("Erro ao fazer bind");
        exit(-1);
    }

    return soquete;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: sudo %s <nome_da_interface>\n", argv[0]);
        printf("Exemplo: sudo %s eth0\n", argv[0]);
        return 1;
    }

    int sock = cria_raw_socket(argv[1]);
    unsigned char buffer[1600]; // Buffer para capturar o quadro Ethernet
    
    printf("Ouvindo a interface %s... Pressione Ctrl+C para parar.\n", argv[1]);

    while (1) {
        // recv retorna o tamanho do pacote recebido
        ssize_t tam = recv(sock, buffer, sizeof(buffer), 0);
        
        if (tam < 0) {
            perror("Erro ao receber");
            break;
        }

        printf("Pacote capturado! Tamanho: %ld bytes\n", tam);
        
        // Exibe os primeiros 14 bytes (Cabeçalho Ethernet: MAC Dest, MAC Orig, EtherType)
        printf("  Cabeçalho L2: ");
        for(int i = 0; i < 14 && i < tam; i++) {
            printf("%02x ", buffer[i]);
        }
        printf("\n----------------------------------\n");
    }

    close(sock);
    return 0;
}