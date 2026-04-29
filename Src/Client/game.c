#include <stdio.h>

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Uso: sudo %s <nome_da_interface>\n", argv[0]);
		printf("Exemplo: sudo %s eth0\n", argv[0]);
		return 1;
	}  
  
  int input;

  while (1) {
    scanf("%d", &input);

    if (input == 1) {
      //Send input and wait for ACK

    }
  }


  return 0;
}