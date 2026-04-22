#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define TAM_FRAME 1514

extern int cria_raw_socket(char* nome_interface_rede);

/*recebe o buffer e retorna dados na estrutura*/
int parsing_kermit(unsigned char buffer[TAM_FRAME]) ;

int loop_recv(int sock, unsigned char buffer[TAM_FRAME]) ;
