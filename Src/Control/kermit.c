#include "../../Headers/kermit.h" 

void print_kermit(struct kermit *k) {
  printf("===============================\n") ;
  printf("tamDados: %u\n", (unsigned int) k->tamDados) ;
  printf("seq %u\n", (unsigned int) k->seq) ;
  printf("type %u\n", (unsigned int) k->type) ;
  if (k->tamDados > 0) 
    for (int i = 0; i < k->tamDados; i++)
      printf("%02x ", k->dados[i] ) ;
  printf("\n") ;
  printf("crc %u\n", (unsigned int) k->crc) ;
  printf("===============================\n") ;
}

// Imprime o frame byte a byte
void imprimeFrame (unsigned char *bufferFrame, int tamFrameCompleto) {
	printf("IMPRIMINDO FRAME COMPLETO: \n");
	for (int i = 0; i < tamFrameCompleto; i ++) {
		printf("%02x ", bufferFrame[i]);
	}
	printf("\n");
}

void kermit_free(struct kermit *k) {
	if (!k) return;
	if (k->dados != NULL) {
		free(k->dados);
		k->dados = NULL;
	}
	free(k);
	k = NULL;
}

// Calcula o CRC dos campos TAM/SEQ/TIPO/DADOS
// Utiliza PG=0x07
uint8_t calculaCRC8(const unsigned char *data, int tamData) {

	uint8_t crc = 0x00;

	for (int i = 0; i < tamData; i++) {
		crc ^= data[i]; // Alinha o byte para iniciar a divisão

		for (int j = 0; j < 8; j++) {
			if (crc & 0x80) { // 0x80 = 1000 0000. Isola o bit mais a esquerda fazendo AND bit a bit
				crc = (crc << 1) ^ 0x07; // Aplica o polinômio se o MSB for 1
			} else {
				crc <<= 1;
			}
		}
	}

	return crc;
}


// Aloca e preenche o frame com seguindo o protocolo.
// Caso o tamDados seja < 10 colocamos padding no final do frame para que o tamanho mínimo
// do frame seja 14 e máximo seja 35.
// Retorna um ponteiro para um buffer contendo o frame.
// Em caso de erro retorna NULL.
unsigned char* buildFrame(unsigned char *bufferDados, uint8_t tamDados, uint8_t seq,
													uint8_t type) {
		
	if (tamDados > 31) {
		perror("Erro buildFrame, o tamanho da mensagem é maior que 31\n");
		return NULL;
	}
	else if (seq > 63) {
		perror("Erro buildFrame, o campo sequência é maior que 63\n");
		return NULL;
	}
	else if (type > 31) {
		perror("Erro buildFrame, o campo type é maior que 31\n");
		return NULL;
	}
  
  uint8_t tam_padding = 0;
	if (tamDados < 10) {
    tam_padding = 10;
	}

	unsigned char *bufferFrame = malloc (tamDados + 4 + tam_padding);
	if (!bufferFrame) {
		perror("Erro buildFrame, erro ao alocar frame\n");
		return NULL;
	}
	
	// Byte 0: Marcador de inicio(8 bits), 01111110 ou 7e
	memset(bufferFrame, MI, 1) ;
	
	// Byte 1: TAM(5 bits MSB) | SEQ Alta(3 bits LSB)
  bufferFrame[1] = ((tamDados & 0x1F) << 3) | ((seq >> 3) & 0x07);
	
  // Byte 2: SEQ Baixa(3 bits MSB) | TYPE(5 bits LSB)
  bufferFrame[2] = ((seq & 0x07) << 5) | (type & 0x1F);
	
	if (bufferDados != NULL && tamDados > 0) {
		memcpy(bufferFrame + 3, bufferDados ,tamDados);
	} 
  
	//Adiciona padding de zeros ao final do frame
  memset(bufferFrame + 4 + tamDados, 0, tam_padding);

	// Calcula o CRC dos campos: TAM/SEQ/TIPO/DADOS
	uint8_t crc = calculaCRC8(bufferFrame + 1, 2 + tamDados);
	
	// Campo CRC (8bits)
	bufferFrame[3 + tamDados] = crc;
	
  return bufferFrame;
}

// Aloca e preenche uma estrutura kermit com os dados do buffer capturado 
// Ignora o padding.
// Retorna uma estrutura kermit e NULL em caso de erro
struct kermit *parsing_kermit(unsigned char *bufferCapturado, int tamCaptura) {

	if (tamCaptura < MIN_FRAME_SIZE) {
		perror("Erro parsing_kermit, o tamanho captura é menor que 14\n");
		return NULL;
	}

	struct kermit *k = malloc(sizeof(struct kermit));
	if (!k) {return NULL;}

	// Separa o tamanhoMsg
	k->tamDados = (bufferCapturado[1] >> 3) & 0x1F;

	k->seq = ((bufferCapturado[1] & 0x07) << 3) | (bufferCapturado[2] >> 5);

	// Separa o campo type
	k->type = bufferCapturado[2] & 0x1F;
	
	if (k->tamDados > 0) {
		// Copia os dados do buffer para a struct
		k->dados = malloc(k->tamDados);
		if (!k->dados) {
			free(k);
			return NULL;
		}
		memcpy(k->dados, &bufferCapturado[3], k->tamDados);
	} else {
		k->dados = NULL;
	}

	// Separa o CRC
	k->crc = bufferCapturado[3 + k->tamDados] ;

	return k;
}

// Contrói o frame e envia o frame pelo socket. Não trata de ACK ou NACK
// Retorna 1 e em caso de erro -1
int sendMsg (int socket, uint8_t tamDados, uint8_t sequencia, uint8_t tipo, unsigned char *dadosMsg) {
	
	// Constrói o frame para o envio
	unsigned char *frameCompleto = buildFrame(dadosMsg, tamDados, sequencia, tipo);
	if (!frameCompleto) {
		perror("ERRO AO CRIAR FRAME\n");
		return -1;
	}

	// Verifica se houve padding na contrução do frame
	int padding = 0;
	if (tamDados < 10) { padding = 10;}
	unsigned int tamFrameCompleto = tamDados + 4 + padding;

	printf("-------------------------------\n");
	printf("ENVIANDO FRAME TAMDADOS: %x TAMFRAME: %d SEQ: %x TIPO: %x\n", tamDados, 
				tamFrameCompleto, sequencia, tipo);

	imprimeFrame(frameCompleto, tamFrameCompleto);

	if (send(socket, frameCompleto, tamFrameCompleto, 0) == -1) {
		perror("ERRO AO ENVIAR FRAME\n");
		return -1;
	}

	printf("FRAME ENVIADO COM SUCESSO\n");
	printf("-------------------------------\n");
	
	free(frameCompleto);

  return 1;
}

// Retorna o tempo do sistema milissegundos
long long timestamp() {
	struct timeval tp;
	gettimeofday(&tp, NULL);

	return tp.tv_sec*1000 + tp.tv_usec/1000;
}
 
// Verifica a validade do pacote usando marcador inicial e CRC.
// Retorna 1 e 0 em caso de pacote inválido
int protocolo_e_valido(unsigned char* buffer, int tamanho_buffer) {
	
	if (tamanho_buffer < MIN_FRAME_SIZE) { 
		return 0; 
	}

	if (buffer[0] == MI) { // Verifica marcador inícial

		uint8_t tamDados = buffer[1] >> 3;
		if (tamanho_buffer < (tamDados + 4)) {return 0;}

		uint8_t crcRecebido = buffer[tamDados + 3];
		uint8_t crcCalculado = calculaCRC8(buffer + 1, tamDados + 2);

		if (crcRecebido != crcCalculado) { 
			printf("PACOTE CORROMPIDO | CRC RECEBIDO: %x | CRC CALCULADO %x", crcRecebido, crcCalculado);
			return 0;
		}

		return 1;
	}

	return 0;
}
 
// Loop de espera de mensagem.
// Retorna -1 se deu timeout (pacote inválido ou não recebeu), ou quantidade de bytes lidos.
int recebe_mensagem(int soquete, int timeoutMillis, unsigned char* buffer, int tamanho_buffer) {
	long long comeco = timestamp();

	struct timeval timeout = { 
		.tv_sec = timeoutMillis/1000, 
		.tv_usec = (timeoutMillis%1000) * 1000 
	};

	setsockopt(soquete, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));

	int bytes_lidos;
	do {
		bytes_lidos = recv(soquete, buffer, tamanho_buffer, 0);
		if (bytes_lidos > 0 && protocolo_e_valido(buffer, bytes_lidos)) { return bytes_lidos; }
	} while (timestamp() - comeco <= timeoutMillis);

	return -1;
}
