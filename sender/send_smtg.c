#include <stdlib.h> 
#include <string.h> 
#include <stdint.h>
#include "../receiver/socket.h"

#define TAM_FRAME 1514

// Representa os 14 bytes de header ethernet (AF_PACKET)
struct eth_header {
	//Força o compilador a usar inteiros de 8 ou 16 bits
	uint8_t  dest_mac[6];
	uint8_t  src_mac[6];
	uint16_t ethertype;
} __attribute__((packed));
//__attribute__((packed)) força o compilador a usar o tamanho definido e evitar padding

/* funcao que recebe buffer como parametro
   e constroi o buffer de envio
   
	 tam = tamanho da msg
*/
int build_kermit(unsigned char buffer[TAM_FRAME], uint8_t tam, uint8_t seq,
                 uint8_t type, uint8_t crc) {

  if (tam > 255 || tam < 0) {
		
		perror("erro build_kermit, mensagem errada\n") ;
		exit(1) ;
	}

	struct eth_header *eth_header_msg1 = (struct eth_header*) buffer;

	// 0x88B5 é um tipo experimental usado para testes
	eth_header_msg1->ethertype = htons(0x88B5);

	//Atribui os valores FFFFFFFFFFFFFFF ; significa BROADCAST
	memset(eth_header_msg1->dest_mac, 0xFF, 6);

	//Estou usando o MAC do meu endereço HARDWIRED
	//MAC: dc:0e:a1:c5:dc:f6	; nome da interface 
	eth_header_msg1->src_mac[0] = 0xdc;
	eth_header_msg1->src_mac[1] = 0x0e;
	eth_header_msg1->src_mac[2] = 0xa1;
	eth_header_msg1->src_mac[3] = 0xc5;
	eth_header_msg1->src_mac[4]	= 0xdc;
	eth_header_msg1->src_mac[5] = 0xf6;

 	//Estou usando o MAC do meu endereço HARDWIRED
	//MAC: 	00:e0:4c:88:00:fc ; nome da interface enx00e04c8800fc
	/*eth_header_msg1->src_mac[0] = 0x00;
	eth_header_msg1->src_mac[1] = 0xe0;
	eth_header_msg1->src_mac[2] = 0x4c;
	eth_header_msg1->src_mac[3] = 0x88;
	eth_header_msg1->src_mac[4]	= 0x00;
	eth_header_msg1->src_mac[5] = 0xfc;*/

  // Enche o campo mensagem com o valor 0 (0x00)
	memset(buffer + 14, 0, TAM_FRAME - 14);
	
	// marcador de inicio
	memset(buffer + 14, 0x7e, 1) ;

  // tamanho dos dados
	uint8_t aux_tam = tam ;
  aux_tam = aux_tam << 3 ;

	//buffer[15] = aux_tam ;

	// sequencia 
  // deixa os 3 bits mais significativos
	uint8_t aux_seq1 = seq ;
  for (int i = 0; i < 3; i++)
		aux_seq1 &= ~(1 << i)  ;

	aux_seq1 = aux_seq1 >> 3 ;

  // completa esse byte 
	buffer[15] = aux_seq1 + aux_tam ;
	//printf("VALOR %X\n", buffer[15]) ; 

  // apaga os 3 bits mais significativos
	uint8_t aux_seq2 = seq ;
  for (int i = 3; i < 6; i++)
		aux_seq2 &= ~(1 << i)  ;
  
	aux_seq2 = aux_seq2 << 5 ;

	// tipo
	uint8_t aux_type = type ;
	
	buffer[16] = aux_seq2 + aux_type ;

	//printf("VALOR %X\n", buffer[16]) ; 

  return 0 ;
}

int main(int argc, char *argv[]) {

	// 14 bytes eth header e 1500 bytes para mensagem
	unsigned char buffer[TAM_FRAME];

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
	
  /*numeros arbitrarios de teste*/
  build_kermit(buffer, 2, 2, 2, 6) ;

	//------------------------------------------
	//SEND BUFFER
	send(sock, buffer, sizeof(unsigned char) * TAM_FRAME, 0);

	//------------------------------------------

  close(sock);

  return 0;
}


