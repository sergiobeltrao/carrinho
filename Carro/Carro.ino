// Código do receptor. Testado com a versão 2.0.17 da biblioteca do ESP32 (a versão 3.0.0 ou superior irá resultar em erro de compilação).

#include <esp_now.h>
#include <WiFi.h>

const int PIN_ENA_L298N = 13;
const int PIN_IN1_L298N = 27;
const int PIN_IN2_L298N = 26;
const int PIN_IN3_L298N = 25;
const int PIN_IN4_L298N = 33;
const int PIN_ENB_L298N = 32;

// A estrutura de dados que será recebida
typedef struct mensagemEstruturada {
  int xJoystick;
  int yJoystick;
  int swJoystick;
  int codigoDeDirecao;
} mensagemEstruturada;

mensagemEstruturada meusDados;

void OnDataRecv(const uint8_t *mac, const uint8_t *dadosACaminho, int tamanho) {

  memcpy(&meusDados, dadosACaminho, sizeof(meusDados));
  Serial.println("Valor no X: " + String(meusDados.xJoystick));
  Serial.println("Valor no Y: " + String(meusDados.yJoystick));
  Serial.println("Valor no SW: " + String(meusDados.swJoystick));

  switch (meusDados.codigoDeDirecao) {
    case 1:
      carroParaFrente();
      Serial.println("Status: Para frente");
      break;
    case 2:
      carroParaTras();
      Serial.println("Status: Para trás");
      break;
    case 3:
      carroParaDireita();
      Serial.println("Status: Direita");
      break;
    case 4:
      carroParaEsquerda();
      Serial.println("Status: Esquerda");
      break;
    default:
      carroParado();
      Serial.println("Status: Parado");
  }
  Serial.println();
}

void setup() {

  Serial.begin(115200);

  pinMode(PIN_ENA_L298N, OUTPUT);
  pinMode(PIN_IN1_L298N, OUTPUT);
  pinMode(PIN_IN2_L298N, OUTPUT);
  pinMode(PIN_IN3_L298N, OUTPUT);
  pinMode(PIN_IN4_L298N, OUTPUT);
  pinMode(PIN_ENB_L298N, OUTPUT);

  // Velocidade no máximo
  // TODO: Criar controle de velocidade com PWM do ESP32
  digitalWrite(PIN_ENA_L298N, HIGH);
  digitalWrite(PIN_ENB_L298N, HIGH);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar o ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
}

void carroParaFrente() {
  digitalWrite(PIN_IN1_L298N, HIGH);
  digitalWrite(PIN_IN2_L298N, LOW);
  digitalWrite(PIN_IN3_L298N, HIGH);
  digitalWrite(PIN_IN4_L298N, LOW);
}

void carroParaTras() {
  digitalWrite(PIN_IN1_L298N, LOW);
  digitalWrite(PIN_IN2_L298N, HIGH);
  digitalWrite(PIN_IN3_L298N, LOW);
  digitalWrite(PIN_IN4_L298N, HIGH);
}

void carroParaDireita() {
  digitalWrite(PIN_IN1_L298N, LOW);
  digitalWrite(PIN_IN2_L298N, LOW);
  digitalWrite(PIN_IN3_L298N, HIGH);
  digitalWrite(PIN_IN4_L298N, LOW);
}

void carroParaEsquerda() {
  digitalWrite(PIN_IN1_L298N, HIGH);
  digitalWrite(PIN_IN2_L298N, LOW);
  digitalWrite(PIN_IN3_L298N, LOW);
  digitalWrite(PIN_IN4_L298N, LOW);
}

void carroParado() {
  digitalWrite(PIN_IN1_L298N, LOW);
  digitalWrite(PIN_IN2_L298N, LOW);
  digitalWrite(PIN_IN3_L298N, LOW);
  digitalWrite(PIN_IN4_L298N, LOW);
}