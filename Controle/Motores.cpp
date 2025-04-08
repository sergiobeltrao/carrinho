#include <Arduino.h>
#include "Motores.h"
#include "VariaveisGlobais.h"
#include "Display.h"
#include "Pinagem.h"

short direcao(short xDireita, short yDireita, short xEsquerda, short yEsquerda) {

  const short MINIMO = 0;
  const short MAXIMO = 4096;
  const short PROX_AO_MAXIMO = MAXIMO - 128;
  const short VAL_CENTRAL = 1800;
  const short ACIMA_DO_VAL_CENTRAL = VAL_CENTRAL + 64;
  const short ABAIXO_DO_VAL_CENTRAL = VAL_CENTRAL - 64;

  bool joyDireitoParado = (xDireita >= ABAIXO_DO_VAL_CENTRAL && xDireita <= ACIMA_DO_VAL_CENTRAL) && (yDireita >= ABAIXO_DO_VAL_CENTRAL && yDireita <= ACIMA_DO_VAL_CENTRAL);
  bool joyEsquerdoParado = (xEsquerda >= ABAIXO_DO_VAL_CENTRAL && xEsquerda <= ACIMA_DO_VAL_CENTRAL) && (yEsquerda >= ABAIXO_DO_VAL_CENTRAL && yEsquerda <= ACIMA_DO_VAL_CENTRAL);

  bool joyDireitoParaFrente = xDireita >= PROX_AO_MAXIMO && yDireita <= MAXIMO;
  bool joyEsquerdoParaFrente = xEsquerda >= PROX_AO_MAXIMO && yEsquerda <= MAXIMO;

  bool joyDireitoParaTras = xDireita == MINIMO && yDireita >= MINIMO;
  bool joyEsquerdoParaTras = xEsquerda == MINIMO && yEsquerda >= MINIMO;

  bool joyDireitoParaDireita = xDireita >= MINIMO && yDireita > PROX_AO_MAXIMO;
  bool joyEsquerdoParaDireita = xEsquerda >= MINIMO && yEsquerda > PROX_AO_MAXIMO;

  bool joyDireitoParaEsquerda = xDireita >= MINIMO && yDireita == MINIMO;
  bool joyEsquerdoParaEsquerda = xEsquerda >= MINIMO && yEsquerda == MINIMO;

  bool paraFrente = joyDireitoParaFrente && joyEsquerdoParaFrente || joyDireitoParaFrente && joyEsquerdoParado || joyDireitoParado && joyEsquerdoParaFrente;
  bool paraTras = joyDireitoParaTras && joyEsquerdoParaTras || joyDireitoParaTras && joyEsquerdoParado || joyDireitoParado && joyEsquerdoParaTras;
  bool direita = joyDireitoParaDireita && joyEsquerdoParaDireita || joyDireitoParado && joyEsquerdoParaDireita || joyDireitoParaDireita && joyEsquerdoParado;
  bool esquerda = joyDireitoParaEsquerda && joyEsquerdoParaEsquerda || joyDireitoParaEsquerda && joyEsquerdoParado || joyDireitoParado && joyEsquerdoParaEsquerda;

  if (paraFrente) {
    return 1;
  } else if (paraTras) {
    return 2;
  } else if (direita) {
    return 3;
  } else if (esquerda) {
    return 4;
  } else {
    return 0;
  }
}

short controleDeVelocidade() {
  short tempoDeAtrasoDeDebounce = 50;

  if (menuAtual != VELOCIDADE) {
    // Retorna o valor antigo quando o menu de velocidade não está ativo
    return nivelDeVelocidadeDoCarro;
  }

  unsigned int agora = millis();
  if (digitalRead(PIN_INCREMENTO) == LOW && nivelDeVelocidadeDoCarro < 100) {
    if (agora - ultimoIncrementoDeVelocidade > tempoDeAtrasoDeDebounce) {
      nivelDeVelocidadeDoCarro++;
      ultimoIncrementoDeVelocidade = agora;
    }
  }

  if (digitalRead(PIN_DECREMENTO) == LOW && nivelDeVelocidadeDoCarro > 0) {
    if (agora - ultimoDecrementoDeVelocidade > tempoDeAtrasoDeDebounce) {
      nivelDeVelocidadeDoCarro--;
      ultimoDecrementoDeVelocidade = agora;
    }
  }

  return nivelDeVelocidadeDoCarro;
}
