#include "../../Headers/protocol.h"
#include "../../Headers/kermit.h"

int wait_response (int socket, uint8_t msgSequence) {

  unsigned char bufferDeCaptura[MAX_FRAME_SIZE];
  int tamanhoCapturado = recebe_mensagem(socket, 300, bufferDeCaptura, MAX_FRAME_SIZE);

  // TIMEOUT
  if (tamanhoCapturado == -1) {
    printf("WAITING RESPONSE, TIMEOUT MSG SEQUENCE %d\n", msgSequence);
    return 2;
  }

  struct kermit *parsedPacket = parsing_kermit(bufferDeCaptura, tamanhoCapturado);

  if (parsedPacket != NULL) {

    // Recebemos o retorno da sequencia errada
    if (parsedPacket->seq != msgSequence) {

      if (parsedPacket->type == NACK_TYPE) {
        printf("NACK SEQ %x, EXPECTED SEQ %x\n",parsedPacket->seq, msgSequence);
      }
      else if (parsedPacket->type == ACK_TYPE){
        printf("ACK SEQ: %d, EXPECTED ACK %d\n", parsedPacket->seq, msgSequence);
      }

      if (parsedPacket->dados != NULL) {free(parsedPacket->dados);}
      free(parsedPacket);

      return NACK_TYPE;
    }

    // Recebemos um ACK como esperado
    if (parsedPacket->type == ACK_TYPE) {
      printf("ACK SEQUENCE %d RECEIVED \n", parsedPacket->seq);

      if (parsedPacket->dados != NULL) {free(parsedPacket->dados);}
      free(parsedPacket);

      return ACK_TYPE;
    }    
  }
  
  if (parsedPacket != NULL) {
    if (parsedPacket->dados != NULL) { free(parsedPacket->dados); }
    free(parsedPacket);
  }

  // O pacote foi corrompido ou chegou errado
  printf("RESPOSTA NÃO CORRESPONDE AO ESPERADO\n");

  return -1;
}


int send_file (int socket, const char *filepath, int fileType) {

  if (socket < 0 || !filepath) {
    perror("ERRO DE PARAMETRO send_file\n");
    return -1;
  }

  unsigned char buffer[MAX_DADOS];

  int bytesLidos;
  uint8_t msgSequence, typeAux ;

  FILE *f = fopen(filepath, "rb");
  if (!f) {
    perror("ERRO AO ABRIR ARQUIVO PARA LEITURA send_file\n");
    return -1;
  }

  fseek(f, 0,SEEK_END);
  // Armazena o tamanho em bytes do arquivo
  unsigned long long fileSize = ftell(f);
  rewind(f);

  printf("----------------------------------------\n");
  printf("ENVIANDO ARQUIVO %s DE TAMANHO %llu BYTES\n", filepath, fileSize);
  printf("----------------------------------------\n");

  msgSequence = 0;

  // Lê pedaços de 31 bytes do arquivo e tenta enviar.
  do {
    bytesLidos = fread(buffer, 1, MAX_DADOS, f);

    typeAux = (bytesLidos < MAX_DADOS) ? FINAL_TYPE : fileType;

    if (send_packet_with_retry(socket, bytesLidos, msgSequence, typeAux, buffer) == -1) {
      printf("ERRO MAXIMO DE TENTATIVAS PARA O ENVIO DO PACOTE SEQ: %x\n send_file", msgSequence);
      fclose(f);
      return -1;
    }

    msgSequence++;
    // Reinicia a sequencia de envio
    if (msgSequence == SEQ_MODULO) {msgSequence = 0;}
    
  } while (typeAux != FINAL_TYPE);

  fclose(f);

  return 1;
}

// Falta enviar o nome(ou o tipo) do arquivo para o client 
int receive_file (int socket, const char *filepath) {

  if (socket < 0) {
    perror("PARAMETER ERROR send_file\n");
    return -1;
  }
  
  FILE *f = fopen(filepath, "wb");
  if (!f) {
    perror("FILE ERROR receive_file\n");
    return -1;
  }
  uint8_t counter = 0;
  uint8_t auxType = -1;
  
  struct kermit *parsedPacket ;
  unsigned char bufferDeCaptura[MAX_FRAME_SIZE];
  int tamanhoCapturado;
  int timeoutCounter = 0;

  do {
    parsedPacket = NULL;

    tamanhoCapturado = recebe_mensagem(socket, 300, bufferDeCaptura, MAX_FRAME_SIZE);

    // TIMEOUT
    if (tamanhoCapturado == -1) {
      printf("TIMEOUT RECEIVING SEQUENCE %d\n", counter);
      timeoutCounter++;
      if (timeoutCounter >= MAX_TIMEOUTS_SEGUIDOS) {
        perror("MÁXIMO DE TIMEOUTS\n");
        fclose(f);
        return -1;
      }

      continue;
    }
    timeoutCounter = 0;
    parsedPacket = parsing_kermit(bufferDeCaptura, tamanhoCapturado);

    if (!parsedPacket) {
      perror("ERRO DE CAPTURA/PARSING receive_file\n");
      fclose(f);
      return -1;
    }

    // O pacote chegou na ordem errada, enviar NACK
    if (counter != parsedPacket->seq) {
      // Se o pacote recebido é o imediatamente anterior (ajustado pelo módulo 64)
      if (parsedPacket->seq == (counter - 1 + SEQ_MODULO) % SEQ_MODULO) {
        printf("ACK PERDIDO DETECTADO. REENVIANDO ACK SEQ %d\n", parsedPacket->seq);
        sendMsg(socket, 0, parsedPacket->seq, ACK_TYPE, NULL);
    
      } else {
        printf("PACOTE FORA DE ORDEM, ENVIANDO NACK SEQ %d\n", counter);
        sendMsg(socket, 0, counter, NACK_TYPE, NULL);
      }
    } else {

      // Envia o ACK do pacote
      sendMsg(socket, 0, counter, ACK_TYPE, NULL);
      printf("PACOTE NUMERO %d CAPTURADO, SENDIGING ACK SEQ %d\n", parsedPacket->seq, counter);

      // Escreve o conteúdo no arquivo de saída
      fwrite(parsedPacket->dados, 1, parsedPacket->tamDados, f);

      auxType = parsedPacket->type;
      counter = (counter + 1) % SEQ_MODULO;
    }

    print_kermit(parsedPacket);

    if (parsedPacket != NULL) {
      if (parsedPacket->dados != NULL) {free(parsedPacket->dados);}
      free(parsedPacket);
    }

  } while (auxType != FINAL_TYPE);

  fclose(f);

  return 1;
}


// Retorna 1 em sucesso, -1 em falha ou após MAX_TENTATIVAS.
int send_packet_with_retry(int socket, int bytesLidos, uint8_t seq, uint8_t type, 
                            unsigned char *buf) {
  int tentativas = 1;
  do {
    if (sendMsg(socket, bytesLidos, seq, type, buf) == -1) {
      printf("ERRO AO ENVIAR PARTE DO ARQUIVO SEQ %d send_file\n", seq);
      return -1;
    }

    // Caso receba ACK sai do loop
    if (wait_response(socket, seq) == ACK_TYPE) {
      return 1;
    }
  tentativas++;

  } while (tentativas < MAX_TENTATIVAS_ENVIO);
  
  return -1;
}
