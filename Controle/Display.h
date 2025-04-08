#ifndef DISPLAY_H
#define DISPLAY_H

// Mantenha o item "ULTIMO_INDICE" sempre como o último da enum, pois ele funciona como um contador.
enum Menus {
  VELOCIDADE = 1,
  CANAL = 2,
  ULTIMO_INDICE
};

void display(Menus menu, short valorAtual, short valorNovo);
bool faixaDeMenus(short valorMenu);
void iniciarDisplay();

#endif