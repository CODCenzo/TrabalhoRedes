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

    // Recebemos um ACK coomo esperado
    if (parsedPacket->type == ACK_TYPE) {
      printf("ACK SEQUENCE %d RECEIVED \n", parsedPacket->seq);

      if (parsedPacket->dados != NULL) {free(parsedPacket->dados);}
      free(parsedPacket);

      return ACK_TYPE;
    }

    if (parsedPacket->dados != NULL) {free(parsedPacket->dados);}
    free(parsedPacket);
  }

  printf("NAO RECEBEMOS NENHUMA RESPOSTA\n");

  return -1;
}

int send_file (int socket, const char *filepath, int fileType) {

  if (socket < 0 || !filepath) {
    perror("PARAMETER ERROR send_file\n");
    return -1;
  }

  unsigned char buffer[MAX_DADOS];

  int bytesLidos;
  uint8_t msgSequence, typeAux ;
  int tentativas;

  FILE *f = fopen(filepath, "rb");
  if (!f) {
    perror("FILE ERROR send_file\n");
    return -1;
  }

  fseek(f, 0,SEEK_END);
  // Armazena o tamanho em bytes do arquivo
  unsigned long long fileSize = ftell(f);
  rewind(f);

  printf("----------------------------------------\n");
  printf("ENVIANDO ARQUIVO %s DE TAMANHO %llu BYTES\n", filepath, fileSize);
  printf("----------------------------------------\n");

  typeAux = fileType;
  msgSequence = 0;

  do {
    bytesLidos = fread(buffer, 1, MAX_DADOS, f);

    if (bytesLidos < MAX_DADOS) {
      typeAux = FINAL_TYPE;
    }

    tentativas = 0;
    do {
      if (sendMsg(socket, bytesLidos, msgSequence, typeAux, buffer) == -1) {
        perror("ERRO send_file\n");
        return -1;
      }

      // Caso receba ACK sai do loop
      if (wait_response(socket, msgSequence) == ACK_TYPE) {break;}
      tentativas++;
    } while (tentativas < MAX_TENTATIVAS_ENVIO);

    if (tentativas >= MAX_TENTATIVAS_ENVIO) {
      printf("ERRO MAXIMO DE TENTATIVAS PARA O ENVIO DO PACOTE SEQ: %x\n", msgSequence);
      fclose(f);
      return -1;
    }

    msgSequence++;
    // Reinicia a sequencia de envio
    if (msgSequence == 64) {msgSequence = 0;}
    
  } while (typeAux != FINAL_TYPE);

  fclose(f);

  return 1;
}

int receive_file (int socket) {

  if (socket < 0) {
    perror("PARAMETER ERROR send_file\n");
    return -1;
  }

  uint8_t counter, auxType;

  const char *filepath = "saida1.txt";
  //unsigned char *frameAck ;

  FILE *f = fopen(filepath, "wb");
  if (!f) {
    perror("FILE ERROR receive_file\n");
    return -1;
  }

  struct kermit *parsedPacket ;
  unsigned char bufferDeCaptura[MAX_FRAME_SIZE];
  int tamanhoCapturado;
  int timeoutCounter = 0;

  counter = 0;
  do {
    parsedPacket = NULL;

    tamanhoCapturado = recebe_mensagem(socket, 300, bufferDeCaptura, MAX_FRAME_SIZE);
    // TIMEOUT
    if (tamanhoCapturado == -1) {
      printf("TIMEOUT RECEIVING SEQUENCE %d\n", counter);
      timeoutCounter++;
      if (timeoutCounter >= MAX_TIMEOUTS_SEGUIDOS) {
        perror("MÁXIMO DE TIMEOUTS\n");
        return -1;
      }

      continue;
    }

    parsedPacket = parsing_kermit(bufferDeCaptura, tamanhoCapturado);

    if (!parsedPacket) {
      perror("CAPTURE ERROR receive_file\n");
      return -1;
    }

    // O pacote chegou na ordem errada, enviar NACK
    if (counter != parsedPacket->seq) {
      // Se o pacote recebido é o imediatamente anterior (ajustado pelo módulo 64)
      if (parsedPacket->seq == (counter - 1 + 64) % 64) {
        printf("ACK PERDIDO DETECTADO. REENVIANDO ACK SEQ %d\n", parsedPacket->seq);
        sendMsg(socket, 0, parsedPacket->seq, ACK_TYPE, NULL);
      } else {
        printf("PACOTE FORA DE ORDEM, ENVIANDO NACK SEQ %d\n", counter);
        sendMsg(socket, 0, counter, NACK_TYPE, NULL);
      }
    } else {

      // Envia o ACK do pacote
      sendMsg(socket, 0, counter, ACK_TYPE, NULL);
      printf("PACOTE NUMERO %d CAPTURADO\n", parsedPacket->seq);
      printf("SENDIGING ACK SEQ %d\n", counter);

      // Escreve o conteúdo no arquivo de saída
      fwrite(parsedPacket->dados, 1, parsedPacket->tamDados, f);

      auxType = parsedPacket->type;
      counter = (counter + 1) % 64;
    }

    print_kermit(parsedPacket);

    if (parsedPacket != NULL) {
      free(parsedPacket->dados);
      free(parsedPacket);
    }

  } while (auxType != FINAL_TYPE);

  return 1;
}
