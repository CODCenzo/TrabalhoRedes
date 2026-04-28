#include "socket.h" 
 
int cria_raw_socket(char* nome_interface_rede) {
  // Cria arquivo para o socket sem qualquer protocolo
  // AF_PACKET = Acesso direto ao nível de pacote da interface de rede
  // SOCK_RAW = O kernel para de processar cabeçalhos
  // htons(host to network) = captura todos os protocolos e gatante endianess
  int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (soquete == -1) {
    fprintf(stderr, "Erro ao criar socket: Verifique se você é root!\n");
    exit(-1);
  }

  // Transforma o nome da interface de rede "wlp2s0" em índice
  int ifindex = if_nametoindex(nome_interface_rede);

  // Conecta o socket a interface física
  struct sockaddr_ll endereco = {0};
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ifindex;

  // Inicializa socket
  if (bind(soquete, (struct sockaddr*) &endereco, sizeof(endereco)) == -1) {
    fprintf(stderr, "Erro ao fazer bind no socket\n");
    exit(-1);
  }

  // Modo promíscuo "Aceite tudo o que passar pelo cabo, mesmo que não seja para o meu endereço MAC".
  struct packet_mreq mr = {0};
  mr.mr_ifindex = ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  // Não joga fora o que identifica como lixo: Modo promíscuo
  if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
    fprintf(stderr, "Erro ao fazer setsockopt: "
      "Verifique se a interface de rede foi especificada corretamente.\n");
    exit(-1);
  }
 
  return soquete;
}
