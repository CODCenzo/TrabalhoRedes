#include <stdio.h> 
#include <stdlib.h> 
#include <ncurses.h>

#include "../../Headers/game.h"


int main () {


  initscr();            // Inicializa a tela do ncurses
  raw();                // Desativa o buffer de linha (pega as teclas na hora)
  keypad(stdscr, TRUE); // Permite capturar setas do teclado e F1, F2, etc.
  noecho();             // Não mostra na tela as letras que o usuário digita

  // Desenha o texto na coordenada Y=10, X=20
  mvprintw(10, 20, "Bem-vindo ao seu jogo em ncurses!");
  refresh();            // Atualiza a tela para mostrar a mensagem

  getch();              // Espera o usuário pressionar qualquer tecla
  endwin();             // Finaliza o modo ncurses e restaura o terminal

  return 0 ;
}
