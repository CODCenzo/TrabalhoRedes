#include "game_protocol.h"

// Envia um buffer arbitrário já em memória, em múltiplos pacotes se necessário
// Útil para enviar o mapa e mensagens de controle sem escrever em arquivo temporário
int send_buffer(int socket, const unsigned char *data, size_t size, uint8_t msgType) {

  unsigned char buffer[MAX_DADOS];

  int bytesLidos;
  uint8_t msgSequence = 0;
  uint8_t typeAux = -1;

  do {
    bytesLidos = memcpy(buffer, data, MAX_DADOS);

    typeAux = (bytesLidos < MAX_DADOS) ? FINAL_TYPE : msgType;

    if (send_packet_with_retry(socket, bytesLidos, msgSequence, typeAux, buffer) == -1) {
      printf("ERRO MAXIMO DE TENTATIVAS PARA O ENVIO DO PACOTE SEQ: %x\n send_file", msgSequence);
      return -1;
    }

    msgSequence++;
    // Reinicia a sequencia de envio
    if (msgSequence == SEQ_MODULO) {msgSequence = 0;}
    
  } while (typeAux != FINAL_TYPE);




}