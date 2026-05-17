#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

#include "../../Headers/game.h" 

#define MAZE_SIZE 40

/*imprime o tabuleiro com o estado atual do jogo
 *INCOMPLETO: está imprimindo apenas o tabuleiro 
  sem o estado atual do jogo
 * */
void print_maze(char **m, const char *filepath ) {

  FILE *f ;
  char buffer[3] ;
  int i ;

  if (!filepath) {
    f = fopen("../../Files/ufpr.csv", "rb") ;
  }
  if (!f) {
    perror("erro na abertura do arquivo\n") ;
    exit(1) ;
  }

  for (i = 0; i < MAZE_SIZE * MAZE_SIZE; i++) {

    fread(buffer, 1, 2, f) ;

    if (buffer[0] == '0') {
      printf("%c ", ' ') ;
    }
    else {
      printf("%c ", buffer[0]) ;
    }

    // Limpa o buffer
    // se printar 7 deu erro 
    memset(buffer, 7, 3) ;

    if (i % 40 == 39) 
      printf("\n") ;
  }

  fclose(f) ;
}

/*int main() {

  print_maze(NULL, NULL) ;

  return 0;
}*/
