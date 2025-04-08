#include <Arduino.h>
#include "Motores.h"

const short PIN_ENA_L298N = 13;
const short PIN_IN1_L298N = 27;
const short PIN_IN2_L298N = 26;
const short PIN_IN3_L298N = 25;
const short PIN_IN4_L298N = 33;
const short PIN_ENB_L298N = 32;

void configuracaoMotores() {
  pinMode(PIN_ENA_L298N, OUTPUT);
  pinMode(PIN_IN1_L298N, OUTPUT);
  pinMode(PIN_IN2_L298N, OUTPUT);
  pinMode(PIN_IN3_L298N, OUTPUT);
  pinMode(PIN_IN4_L298N, OUTPUT);
  pinMode(PIN_ENB_L298N, OUTPUT);
}

void direcao(short direcao) {
  if (direcao == 1) {
    // Para frente
    digitalWrite(PIN_IN1_L298N, HIGH);
    digitalWrite(PIN_IN2_L298N, LOW);
    digitalWrite(PIN_IN3_L298N, HIGH);
    digitalWrite(PIN_IN4_L298N, LOW);
  } else if (direcao == 2) {
    // Para trás
    digitalWrite(PIN_IN1_L298N, LOW);
    digitalWrite(PIN_IN2_L298N, HIGH);
    digitalWrite(PIN_IN3_L298N, LOW);
    digitalWrite(PIN_IN4_L298N, HIGH);
  } else if (direcao == 3) {
    // Para a direita
    digitalWrite(PIN_IN1_L298N, LOW);
    digitalWrite(PIN_IN2_L298N, HIGH);
    digitalWrite(PIN_IN3_L298N, HIGH);
    digitalWrite(PIN_IN4_L298N, LOW);
  } else if (direcao == 4) {
    // Para a esquerda
    digitalWrite(PIN_IN1_L298N, HIGH);
    digitalWrite(PIN_IN2_L298N, LOW);
    digitalWrite(PIN_IN3_L298N, LOW);
    digitalWrite(PIN_IN4_L298N, HIGH);
  } else {
    // Parado
    digitalWrite(PIN_IN1_L298N, LOW);
    digitalWrite(PIN_IN2_L298N, LOW);
    digitalWrite(PIN_IN3_L298N, LOW);
    digitalWrite(PIN_IN4_L298N, LOW);
  }
}

void controleDeVelocidade(short codigoDeDirecao, short velocidade) {

  /* O valor enviado ao PWM  deve ficar entre 0 e 255. Como a variável de nível de velocidade
  varia de 0 a 100, o valor enviado será o resultado inteiro da sua multiplicação por 2,55. */

  bool virando = codigoDeDirecao == 3 || codigoDeDirecao == 4;
  int valorPWM;

  // Quando vira a direita ou a esquerda a velocidade é 15% menor
  if (virando) {
    int valorRecebido = round(velocidade * 2.55);
    int reducao = valorRecebido * 0.15;
    valorPWM = round(valorRecebido - reducao);
  } else {
    valorPWM = round(velocidade * 2.55);
  }

  // Pino, frequência em Hz e resolução em bits
  ledcAttach(PIN_ENA_L298N, 5000, 8);
  ledcAttach(PIN_ENB_L298N, 5000, 8);

  ledcWrite(PIN_ENA_L298N, valorPWM);
  ledcWrite(PIN_ENB_L298N, valorPWM);
}
