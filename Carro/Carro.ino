// Código do receptor. Testado com a versão 3.1.2 da biblioteca do ESP32.
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Preferences.h>

// Não se esqueça de trocar pelo endereço do seu transmissor.
const uint8_t ENDERECO_MAC_DO_TRANSMISSOR[] = { 0x3C, 0x8A, 0x1F, 0x55, 0xBD, 0xA0 };

Preferences preferences;
int canalAtual;

const short PIN_ENA_L298N = 13;
const short PIN_IN1_L298N = 27;
const short PIN_IN2_L298N = 26;
const short PIN_IN3_L298N = 25;
const short PIN_IN4_L298N = 33;
const short PIN_ENB_L298N = 32;
const short PIN_LED_ONBOARD = 2;

// A estrutura de dados que será recebida
typedef struct mensagemEstruturada {
  short xJoystickDireita;
  short yJoystickDireita;
  short xJoystickEsquerda;
  short yJoystickEsquerda;
  short codigoDeDirecao;
  short velocidade;
  short canal;
} mensagemEstruturada;

mensagemEstruturada meusDados;

esp_now_peer_info_t informacoesDoPar;

// Função que será chamada ao receber os pacotes
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *dadosACaminho, int tamanho) {

  // Valida se os pacotes recebidos são do transmissor correto
  bool macCorreto = true;
  for (short i = 0; i < 6; i++) {
    if (info->src_addr[i] != ENDERECO_MAC_DO_TRANSMISSOR[i]) {
      macCorreto = false;
      break;
    }
  }

  if (!macCorreto) {
    return;
  }

  memcpy(&meusDados, dadosACaminho, sizeof(meusDados));
}

void controleDeVelocidade() {

  /* O valor enviado ao PWM  deve ficar entre 0 e 255. Como a variável de nível de velocidade
  varia de 0 a 100, o valor enviado será o resultado inteiro da sua multiplicação por 2,55. */

  bool virando = meusDados.codigoDeDirecao == 3 || meusDados.codigoDeDirecao == 4;
  int valorPWM;

  // Quando vira a direita ou a esquerda a velocidade é 15% menor
  if (virando) {
    int valorRecebido = round(meusDados.velocidade * 2.55);
    int reducao = valorRecebido * 0.15;
    valorPWM = round(valorRecebido - reducao);
  } else {
    valorPWM = round(meusDados.velocidade * 2.55);
  }

  // Pino, frequência em Hz e resolução em bits
  ledcAttach(PIN_ENA_L298N, 5000, 8);
  ledcAttach(PIN_ENB_L298N, 5000, 8);

  ledcWrite(PIN_ENA_L298N, valorPWM);
  ledcWrite(PIN_ENB_L298N, valorPWM);
}

void direcao(short codigoDeDirecao) {
  if (codigoDeDirecao == 1) {
    // Para frente
    digitalWrite(PIN_IN1_L298N, HIGH);
    digitalWrite(PIN_IN2_L298N, LOW);
    digitalWrite(PIN_IN3_L298N, HIGH);
    digitalWrite(PIN_IN4_L298N, LOW);
  } else if (codigoDeDirecao == 2) {
    // Para trás
    digitalWrite(PIN_IN1_L298N, LOW);
    digitalWrite(PIN_IN2_L298N, HIGH);
    digitalWrite(PIN_IN3_L298N, LOW);
    digitalWrite(PIN_IN4_L298N, HIGH);
  } else if (codigoDeDirecao == 3) {
    // Para a direita
    digitalWrite(PIN_IN1_L298N, LOW);
    digitalWrite(PIN_IN2_L298N, HIGH);
    digitalWrite(PIN_IN3_L298N, HIGH);
    digitalWrite(PIN_IN4_L298N, LOW);
  } else if (codigoDeDirecao == 4) {
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

void mensagensDeDebug(bool ativado) {
  if (ativado) {
    Serial.println("Valor no X da Direita: " + String(meusDados.xJoystickDireita));
    Serial.println("Valor no Y da Direita: " + String(meusDados.yJoystickDireita));
    Serial.println("Valor no X da Esquerda: " + String(meusDados.xJoystickEsquerda));
    Serial.println("Valor no Y da Esquerda: " + String(meusDados.yJoystickEsquerda));
    Serial.println("Nível de Velocidade: " + String(meusDados.velocidade) + "%");
    Serial.println("Canal de Transmissão: " + String(preferences.getInt("valor", 1)));
    // Serial.println("Valor de clock: " + String(valorPWM));
    if (meusDados.codigoDeDirecao == 0) {
      Serial.println("Direção: Parado");
    } else if (meusDados.codigoDeDirecao == 1) {
      Serial.println("Direção: Para Frente");
    } else if (meusDados.codigoDeDirecao == 2) {
      Serial.println("Direção: Para Trás");
    } else if (meusDados.codigoDeDirecao == 3) {
      Serial.println("Direção: Para a Direita");
    } else if (meusDados.codigoDeDirecao == 4) {
      Serial.println("Direção: Para a Esquerda");
    }
    Serial.println();
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(PIN_ENA_L298N, OUTPUT);
  pinMode(PIN_IN1_L298N, OUTPUT);
  pinMode(PIN_IN2_L298N, OUTPUT);
  pinMode(PIN_IN3_L298N, OUTPUT);
  pinMode(PIN_IN4_L298N, OUTPUT);
  pinMode(PIN_ENB_L298N, OUTPUT);
  pinMode(PIN_LED_ONBOARD, OUTPUT);

  preferences.begin("config", false);
  canalAtual = preferences.getInt("valor", 1);

  /* Para a velocidade dos motores ficar sempre no máximo, descomente as próximas
  linhas e comente a função controleDeVelocidade() dentro do loop
  digitalWrite(PIN_ENA_L298N, HIGH);
  digitalWrite(PIN_ENB_L298N, HIGH); */

  WiFi.mode(WIFI_STA);

  esp_wifi_set_channel(canalAtual, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar o ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  memcpy(informacoesDoPar.peer_addr, ENDERECO_MAC_DO_TRANSMISSOR, 6);
  informacoesDoPar.channel = canalAtual;
  informacoesDoPar.encrypt = false;

  if (esp_now_add_peer(&informacoesDoPar) != ESP_OK) {
    Serial.println("Falha ao adicionar o dispositivo par");
    return;
  }
}

void loop() {

  controleDeVelocidade();
  direcao(meusDados.codigoDeDirecao);
  mensagensDeDebug(true);

  // Evita reinicialização se ainda não recebeu um valor válido
  if (meusDados.canal != 0 && meusDados.canal != canalAtual) {
    preferences.putInt("valor", meusDados.canal);

    for (int i = 10; i >= 0; i--) {
      digitalWrite(PIN_LED_ONBOARD, HIGH);
      delay(50);
      digitalWrite(PIN_LED_ONBOARD, LOW);
      delay(50);
    }

    ESP.restart();
  }
}