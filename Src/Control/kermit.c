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
	printf("IMPRIMINDO FRAME ENVIADO: \n");
	for (int i = 0; i < tamFrameCompleto; i ++) {
		printf("%02x ", bufferFrame[i]);
	}
	printf("\n");
}

// Aloca e preenche uma estrutura kermit com os dados do buffer 
// Melhorar lógica usando máscaras
struct kermit *parsing_kermit(unsigned char *bufferCapturado, int tamCaptura) {

  if (tamCaptura < 4) {
    perror("ERRO PARSING KERMIT\n");
    return NULL;
  }

  struct kermit *k = malloc(sizeof(struct kermit));
	if (!k) {return NULL;}

	uint8_t aux_seq ;

	// Separa o tamanhoMsg
  k->tamDados = bufferCapturado[1] ;
	k->tamDados = k->tamDados >> 3 ;

	// Separa os MSB do seq
  aux_seq = bufferCapturado[1];
	//zera os 5 bits mais significativos
  for (int i = 3; i < 8; i++)
	  aux_seq &= ~(1 << i) ; 
  aux_seq = aux_seq << 3 ;

	// Separa os LSB do seq
  k->seq = bufferCapturado[2] ;
  k->seq = k->seq >> 5 ;
	// Separa ambas as partes do campo sequência
	k->seq += aux_seq ;

	// Separada o campo type
	k->type = bufferCapturado[2] ;
  for (int i = 5; i < 8; i++)
	  k->type &= ~(1 << i) ; 
	

  if (tamCaptura > 4) {
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

// Constrói e preenche o frame. O tamanhoFrame min é 4 sempre e o máximo é 35
// Falta implementar CRC
unsigned char* buildFrame(unsigned char *bufferDados, uint8_t tamDados, uint8_t seq,
                 uint8_t type, uint8_t crc) {

	//Verifica os parâmetros da função
	if (tamDados < 0 ) {
		perror("Erro buildFrame, tamanho do buffer negativo\n");
		return NULL;
	} 
	else if (tamDados > 31) {
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

	unsigned char *bufferPaddingTotal = NULL;
	// PADDING TOTAL
	if (tamDados == 0) {
		tamDados = 10;
		bufferPaddingTotal = malloc (tamDados);
		if (!bufferPaddingTotal) {
			perror("Erro buildFrame, erro ao alocar buffer de padding\n");
			return NULL;
		}
		memset(bufferPaddingTotal, 0, tamDados);
	}

	// PADDING PARCIAL
	// tem que implementar

	unsigned char *bufferFrame = malloc (tamDados + 4);
	if (!bufferFrame) {
		perror("Erro buildFrame, erro ao alocar frame\n");
		return NULL;
	}

	// Byte 0: Marcador de inicio(8 bits), 01111110 ou 7e
	memset(bufferFrame, 0x7e, 1) ;

	// Byte 1: TAM(5 bits MSB) | SEQ Alta(3 bits LSB)
  bufferFrame[1] = ((tamDados & 0x1F) << 3) | ((seq >> 3) & 0x07);

  // Byte 2: SEQ Baixa(3 bits MSB) | TYPE(5 bits LSB)
  bufferFrame[2] = ((seq & 0x07) << 5) | (type & 0x1F);

	// Campo Dados
	if (bufferDados != NULL && bufferPaddingTotal == NULL) {
		memcpy(bufferFrame + 3, bufferDados ,tamDados);
	} else {
		memcpy(bufferFrame + 3, bufferPaddingTotal ,tamDados);
	}

	// Campo CRC (8bits)
	bufferFrame[3 + tamDados] = crc;

  return bufferFrame;
}

// Contrói o frame e envia pelo socket. Não trata de ACK ou NACK
int sendMsg (int socket, uint8_t tamDados, uint8_t sequencia, uint8_t tipo, unsigned char *dadosMsg, uint8_t crc) {
	
	// Constrói o frame para o envio
	unsigned char *frameCompleto = buildFrame(dadosMsg, tamDados, sequencia, tipo, crc);
	if (!frameCompleto) {
		perror("ERRO AO CRIAR FRAME\n");
		return -1;
	}

	unsigned int tamFrameCompleto = tamDados + 4; 
	// printf("FRAME CONSTRUÍDO COM SUCESSO\n");

	// imprimeFrame(frameCompleto, tamFrameCompleto);

	if (send(socket, frameCompleto, tamFrameCompleto, 0) == -1) {
		perror("ERRO AO ENVIAR FRAME\n");
		return -1;
	}
	// printf("FRAME ENVIADO\n");
	free(frameCompleto);

  return 1;
}

// Retorna o tempo do sistema milissegundos
long long timestamp() {
	struct timeval tp;
	gettimeofday(&tp, NULL);

	return tp.tv_sec*1000 + tp.tv_usec/1000;
}
 
//Retorna 0 caso não encontre o marcador inícial do nosso frame
int protocolo_e_valido(unsigned char* buffer, int tamanho_buffer) {
	
	if (tamanho_buffer < MIN_FRAME_SIZE) { 
		return 0; }
	// insira a sua validação de protocolo aqui
	// Colocar função que calcula CRC aqui
	if (buffer[0] == 0x7e) {
		return 1;
	}

	return 0;
}
 
// retorna -1 se deu timeout, ou quantidade de bytes lidos
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
		if (protocolo_e_valido(buffer, bytes_lidos)) { return bytes_lidos; }
	} while (timestamp() - comeco <= timeoutMillis);

	return -1;
}
