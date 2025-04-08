#include <U8g2lib.h>
#include <Wire.h>
#include <Arduino.h>
#include "Display.h"
#include "VariaveisGlobais.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

const short ALTURA_DO_DISPLAY = 64;
const short LARGURA_DO_DISPLAY = 128;

short centralizarTxtHorizontal(char *texto) {
  short larguraDoTexto = u8g2.getStrWidth(texto);
  return (LARGURA_DO_DISPLAY - larguraDoTexto) / 2;
}

short centralizarTxtVertical() {
  short alturaDoTexto = u8g2.getAscent() - u8g2.getDescent();
  return (ALTURA_DO_DISPLAY + alturaDoTexto) / 2;
}

void itemDoMenu(char *titulo, char *valor) {
  const short POSICAO_VERTICAL_TITULO = 20;
  const short POSICAO_VERTICAL_VALOR = 50;

  // Espaçamento a esquerda, espaçamento superior, texto
  u8g2.drawStr(centralizarTxtHorizontal(titulo), POSICAO_VERTICAL_TITULO, titulo);
  u8g2.drawStr(centralizarTxtHorizontal(valor), POSICAO_VERTICAL_VALOR, valor);
}

void iconesDeNavegacao(bool anterior, bool posterior) {

  u8g2.setFont(u8g2_font_fub14_tr);

  if (posterior) {
    char proximoMenu[] = ">";
    int distanciaEsquerda = LARGURA_DO_DISPLAY - u8g2.getStrWidth(proximoMenu);
    u8g2.drawStr(distanciaEsquerda, centralizarTxtVertical(), proximoMenu);
  }

  if (anterior) {
    char menuAnterior[] = "<";
    u8g2.drawStr(0, centralizarTxtVertical(), menuAnterior);
  }
}

void mensagemDeErro() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub17_tr);
  char txtErro[] = "Erro!";
  u8g2.drawStr(centralizarTxtHorizontal(txtErro), centralizarTxtVertical(), txtErro);
  u8g2.sendBuffer();
  delay(1500);
}

void display(Menus menu, short valorAtual, short valorNovo) {

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub17_tr);

  if (menu == VELOCIDADE) {
    char textoDisplay[5];
    snprintf(textoDisplay, sizeof(textoDisplay), "%d%%", valorNovo);
    itemDoMenu("Velocidade", textoDisplay);
    iconesDeNavegacao(false, true);
  }

  if (menu == CANAL) {
    char textoCanal[4];
    snprintf(textoCanal, sizeof(textoCanal), "%d%", valorNovo);
    itemDoMenu("Canal", textoCanal);
    iconesDeNavegacao(true, false);

    if (valorNovo != valorAtual) {
      char txtNaoSalvo[] = "___";
      u8g2.drawStr(centralizarTxtHorizontal(txtNaoSalvo), 60, txtNaoSalvo);
    }
  }

  if (carroDesligado) {
    mensagemDeErro();
  }

  u8g2.sendBuffer();
}

void iniciarDisplay() {
  u8g2.begin();
}
