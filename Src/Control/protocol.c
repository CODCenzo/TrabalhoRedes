#include "../../Headers/protocol.h"
#include "../../Headers/kermit.h"

#define TAM_MAX_DADOS 31

// Tipo que sinaliza o fim da sequencia de envio
// Os campo dados de uma mensagem deste tipo contém o final do arquivo e pode ter um
// tamanho de 1 a 31
#define FINAL_TYPE 16
#define DEFAULT_CRC 1

int wait_response (int socket, uint8_t msgSequence) {

  unsigned char bufferDeCaptura[MAX_FRAME_SIZE];
  int tamanhoCapturado = recebe_mensagem(socket, 300, bufferDeCaptura, MAX_FRAME_SIZE);

  // TIMEOUT
  if (tamanhoCapturado == -1) {
    printf("TIMEOUT SEQUENCE %d\n", msgSequence);
    return 2;
  }

  struct kermit *parsedPacket = parsing_kermit(bufferDeCaptura, tamanhoCapturado);
  // printf("Tamanho capturado: %d\n");

  // Recebemos a mensagem na sequencia errada, enviar NACK do msgSequence
  if (parsedPacket->seq != msgSequence || parsedPacket->type == NACK_TYPE) {
    printf("NACK SEQUENCE %d\n", msgSequence);
    if (parsedPacket->dados != NULL) {free(parsedPacket->dados);}
    free(parsedPacket);
    return NACK_TYPE;
  }

  if (parsedPacket->type == ACK_TYPE) {
    printf("ACK SEQUENCE %d\n", msgSequence);
    if (parsedPacket->dados != NULL) {free(parsedPacket->dados);}
    free(parsedPacket);
    return ACK_TYPE;
  }

  if (parsedPacket->dados != NULL) {free(parsedPacket->dados);}
  free(parsedPacket);

  printf("WAITING RESPONSE ERROR\n");
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
    bytesLidos = fread(buffer, 1, TAM_MAX_DADOS, f);

    if (bytesLidos < TAM_MAX_DADOS) {
      typeAux = FINAL_TYPE;
    }

    tentativas = 0;
    do {
      if (sendMsg(socket, bytesLidos, msgSequence, typeAux, buffer, DEFAULT_CRC) == -1) {
        perror("ERRO send_file\n");
        return -1;
      }

      // Caso receba ACK sai do loop
      if (wait_response(socket, msgSequence) == ACK_TYPE) {break;}
      tentativas++;
    } while (tentativas < MAX_TENTATIVAS_ENVIO);

    if (tentativas >= MAX_TENTATIVAS_ENVIO) {
      perror("ERRO MAXIMO DE TENTATIVAS\n");
      return -1;
    }

    printf("PACOTE %d ENVIADO COM SUCESSO, ACK RECEBIDO _send_file\n", msgSequence);

    msgSequence++;
    // Reinicia a sequencia de envio
    if (msgSequence == 64) {msgSequence = 0;}
    printf("----------------------------------------\n");
    
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
      printf("TIMEOUT SEQUENCE %d\n", counter);
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
      printf("NACK COUNTER %d\n", counter);
      sendMsg(socket, 0, counter, NACK_TYPE, NULL, DEFAULT_CRC);
    } else {

      // Envia o ACK do pacote
      sendMsg(socket, 0, counter, ACK_TYPE, NULL, DEFAULT_CRC);
      printf("PACOTE NUMERO %d CAPTURADO\n", parsedPacket->seq);
      printf("ACK COUNTER %d\n", counter);

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
