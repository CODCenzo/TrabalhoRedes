#include "../../Headers/timeout.h" 

// Retorna o tempo do sistema milissegundos
long long timestamp() {
	struct timeval tp;
	gettimeofday(&tp, NULL);

	return tp.tv_sec*1000 + tp.tv_usec/1000;
}
 
//Retorna 0 caso não encontre o marcador inícial do nosso frame
int protocolo_e_valido(char* buffer, int tamanho_buffer) {
	if (tamanho_buffer <= 0) { return 0; }
	// insira a sua validação de protocolo aqui
	return buffer[0] == 0x7e;
}
 
// retorna -1 se deu timeout, ou quantidade de bytes lidos
int recebe_mensagem(int soquete, int timeoutMillis, char* buffer, int tamanho_buffer) {
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