#include <stdlib.h> 
#include <string.h> 
#include <stdint.h>
#include "../receiver/socket.h"

// Representa os 14 bytes de header ethernet (AF_PACKET)
struct eth_header {
	//Força o compilador a usar inteiros de 8 ou 16 bits
	uint8_t  dest_mac[6];
	uint8_t  src_mac[6];
	uint16_t ethertype;
} __attribute__((packed));
//__attribute__((packed)) força o compilador a usar o tamanho definido e evitar padding

int main(int argc, char *argv[]) {

	// 14 bytes eth header e 1500 bytes para mensagem
	unsigned char buffer[1514];

	//------------------------------------------
	//HEADER ETHERNET

	// Aloca 14 bytes para o header da ethernet.
	// 6 bytes MAC addres / 6 bytes MAC destiny / 2 Bytes ethernet type / 1500 mensagem
	struct eth_header *eth_header_msg1 = (struct eth_header*) buffer;

	// Utiliza a função host to network para atribuir valor ao campo
	// 0x88B5 é um tipo experimental usado para testes
	eth_header_msg1->ethertype = htons(0x88B5);

	//Atribui os valores FFFFFFFFFFFFFFF ; significa BROADCAST(envia para todo mundo conectado)
	memset(eth_header_msg1->dest_mac, 0xFF, 6);

	//Estou usando o MAC do meu endereço HARDWIRED
	//MAC: 	00:e0:4c:88:00:fc ; nome da interface enx00e04c8800fc
	/*eth_header_msg1->src_mac[0] = 0x00;
	eth_header_msg1->src_mac[1] = 0xe0;
	eth_header_msg1->src_mac[2] = 0x4c;
	eth_header_msg1->src_mac[3] = 0x88;
	eth_header_msg1->src_mac[4]	= 0x00;
	eth_header_msg1->src_mac[5] = 0xfc;*/

	//Estou usando o MAC do meu endereço HARDWIRED
	//MAC: dc:0e:a1:c5:dc:f6	; nome da interface 
	eth_header_msg1->src_mac[0] = 0xdc;
	eth_header_msg1->src_mac[1] = 0x0e;
	eth_header_msg1->src_mac[2] = 0xa1;
	eth_header_msg1->src_mac[3] = 0xc5;
	eth_header_msg1->src_mac[4]	= 0xdc;
	eth_header_msg1->src_mac[5] = 0xf6;


	//------------------------------------------
	//SOCKET

	if (argc < 2) {
		printf("Uso: sudo %s <nome_da_interface>\n", argv[0]);
		printf("Exemplo: sudo %s eth0\n", argv[0]);
		return 1;
	}

	// Cria um socket a partir do nome da interface
	int sock = cria_raw_socket(argv[1]);

	//------------------------------------------
	//MESSAGE (nosso kermit)

  // Enche o campo mensagem com o valor 1 (0x01)
	memset(buffer + 14, 1, sizeof(buffer) - 14);
	
	//------------------------------------------
	//SEND BUFFER
	send(sock, buffer, sizeof(unsigned char) *1514, 0);

	//------------------------------------------

  close(sock);

  return 0;
}


