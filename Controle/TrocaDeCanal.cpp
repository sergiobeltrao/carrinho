#include "Arduino.h"
#include "TrocaDeCanal.h"
#include "Pinagem.h"
#include "Display.h"
#include "VariaveisGlobais.h"

void mudarCanalDeTrasmissao(short menuAtual, bool incremento, bool decremento) {
  if (menuAtual == CANAL) {
    static unsigned long ultimaTroca = 0;
    const unsigned long intervaloDebounce = 200;
    const short CANAL_MINIMO = 1;
    const short CANAL_MAXIMO = 12;

    if (millis() - ultimaTroca < intervaloDebounce) {
      return;
    }

    if (incremento == LOW && canalTemporario < CANAL_MAXIMO) {
      canalTemporario++;
      ultimaTroca = millis();
    } else if (decremento == LOW && canalTemporario > CANAL_MINIMO) {
      canalTemporario--;
      ultimaTroca = millis();
    }
  }

  if (ativarDebug) {
    Serial.println("Canal de Transmissão Atual: " + String(canalAtual));
    Serial.println("Canal de Transmissão Temporário: " + String(canalTemporario));
  }
}

void aplicarNovoCanal() {
  /*
  Verifica se o botão de salvar e reiniciar foi pressionado, e se o carrinho está se
  comunicando com o controle. O que significa que ele estando apto a receber a nova
  configuração de canal.
  */

  bool salvarEReiniciar = digitalRead(PIN_APLICAR_E_RESET) == LOW;

  if (salvarEReiniciar) {
    if (conexaoBemSucedida) {
      canalAtual = canalTemporario;
      reiniciar = true;
    } else {
      carroDesligado = true;
    }
  }
}

void salvarEReiniciar() {
  /*
  Caso a comunicação tenha sido bem-sucedida, envia o novo canal para o carrinho, salva
  o canal temporário como canal de transmissão, aguarda e reinicia o controle.
  */

  if (reiniciar) {
    ESP.restart();
  }
}