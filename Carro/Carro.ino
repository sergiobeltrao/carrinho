// Código do receptor. Testado com a versão 3.1.1 da biblioteca do ESP32.

#include <esp_now.h>
#include <WiFi.h>

const int PIN_ENA_L298N = 13;
const int PIN_IN1_L298N = 27;
const int PIN_IN2_L298N = 26;
const int PIN_IN3_L298N = 25;
const int PIN_IN4_L298N = 33;
const int PIN_ENB_L298N = 32;

// Não se esqueça de trocar pelo endereço do seu transmissor.
const uint8_t ENDERECO_MAC_DO_TRANSMISSOR[] = { 0x3C, 0x8A, 0x1F, 0x55, 0xBD, 0xA0 };

// A estrutura de dados que será recebida
typedef struct mensagemEstruturada {
  int xJoystick;
  int yJoystick;
  int swJoystick;
  int codigoDeDirecao;
  int velocidade;
} mensagemEstruturada;

mensagemEstruturada meusDados;

esp_now_peer_info_t informacoesDoPar;

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *dadosACaminho, int tamanho) {

  // Valida se os pacotes recebidos são do transmissor correto
  bool macCorreto = true;
  for (int i = 0; i < 6; i++) {
    if (info->src_addr[i] != ENDERECO_MAC_DO_TRANSMISSOR[i]) {
      macCorreto = false;
      break;
    }
  }

  if (!macCorreto) {
    return;
  }

  memcpy(&meusDados, dadosACaminho, sizeof(meusDados));

  Serial.println("Valor no X: " + String(meusDados.xJoystick));
  Serial.println("Valor no Y: " + String(meusDados.yJoystick));
  Serial.println("Valor no SW: " + String(meusDados.swJoystick));
  Serial.println("Nível de velocidade: " + String(meusDados.velocidade) + "%");

  direcao(meusDados.codigoDeDirecao);
}

void setup() {

  Serial.begin(115200);

  pinMode(PIN_ENA_L298N, OUTPUT);
  pinMode(PIN_IN1_L298N, OUTPUT);
  pinMode(PIN_IN2_L298N, OUTPUT);
  pinMode(PIN_IN3_L298N, OUTPUT);
  pinMode(PIN_IN4_L298N, OUTPUT);
  pinMode(PIN_ENB_L298N, OUTPUT);

  // Para a velocidade dos motores ficar sempre no máximo, descomente as próximas
  // linhas e comente a função controleDeVelocidade() dentro do loop
  // digitalWrite(PIN_ENA_L298N, HIGH);
  // digitalWrite(PIN_ENB_L298N, HIGH);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar o ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  memcpy(informacoesDoPar.peer_addr, ENDERECO_MAC_DO_TRANSMISSOR, 6);
  informacoesDoPar.channel = 0;
  informacoesDoPar.encrypt = false;

  if (esp_now_add_peer(&informacoesDoPar) != ESP_OK) {
    Serial.println("Falha ao adicionar o dispositivo par");
    return;
  }
}

void loop() {
  controleDeVelocidade();
}

void controleDeVelocidade() {
  /*
  O valor enviado ao PWM  deve ficar entre 0 e 255. Como a variável de nível de velocidade
  variade 0 a 100, o valor enviado será o resultado inteiro da sua multiplicação por 2,55.
  */
  int valorPWM = round(meusDados.velocidade * 2.55);

  Serial.println("Valor de clock: " + String(valorPWM));

  // Pino, frequência em Hz e resolução em bits
  ledcAttach(PIN_ENA_L298N, 5000, 8);
  ledcAttach(PIN_ENB_L298N, 5000, 8);

  ledcWrite(PIN_ENA_L298N, valorPWM);
  ledcWrite(PIN_ENB_L298N, valorPWM);
}

void direcao(int codigoDeDirecao) {

  if (codigoDeDirecao == 1) {
    digitalWrite(PIN_IN1_L298N, HIGH);
    digitalWrite(PIN_IN2_L298N, LOW);
    digitalWrite(PIN_IN3_L298N, HIGH);
    digitalWrite(PIN_IN4_L298N, LOW);
    Serial.println("Status: Para frente");
  } else if (codigoDeDirecao == 2) {
    digitalWrite(PIN_IN1_L298N, LOW);
    digitalWrite(PIN_IN2_L298N, HIGH);
    digitalWrite(PIN_IN3_L298N, LOW);
    digitalWrite(PIN_IN4_L298N, HIGH);
    Serial.println("Status: Para trás");
  } else if (codigoDeDirecao == 3) {
    digitalWrite(PIN_IN1_L298N, LOW);
    digitalWrite(PIN_IN2_L298N, LOW);
    digitalWrite(PIN_IN3_L298N, HIGH);
    digitalWrite(PIN_IN4_L298N, LOW);
    Serial.println("Status: Direita");
  } else if (codigoDeDirecao == 4) {
    digitalWrite(PIN_IN1_L298N, HIGH);
    digitalWrite(PIN_IN2_L298N, LOW);
    digitalWrite(PIN_IN3_L298N, LOW);
    digitalWrite(PIN_IN4_L298N, LOW);
    Serial.println("Status: Esquerda");
  } else {
    digitalWrite(PIN_IN1_L298N, LOW);
    digitalWrite(PIN_IN2_L298N, LOW);
    digitalWrite(PIN_IN3_L298N, LOW);
    digitalWrite(PIN_IN4_L298N, LOW);
    Serial.println("Status: Parado");
  }
  Serial.println();
}