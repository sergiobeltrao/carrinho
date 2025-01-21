// Código do transmissor. Testado com a versão 2.0.17 da biblioteca do ESP32 (a versão 3.0.0 ou superior irá resultar em erro de compilação).

#include <esp_now.h>
#include <WiFi.h>

const int PIN_SW_JOYSTICK = 32;
const int PIN_EIXO_X_JOYSTICK = 34;
const int PIN_EIXO_Y_JOYSTICK = 35;

// Não se esqueça de trocar pelo endereço do seu receptor.
const uint8_t ENDERECO_MAC_DO_RECEPTOR[] = { 0x3C, 0x8A, 0x1F, 0x50, 0x65, 0x7C };

// A estrutura de dados que será enviada
typedef struct mensagemEstruturada {
  int xJoystick;
  int yJoystick;
  int swJoystick;
  int codigoDeDirecao;
} mensagemEstruturada;

mensagemEstruturada meusDados;

esp_now_peer_info_t informacoesDoPar;

// A função que é chamada quando o pacote é enviado
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nStatus de envio: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "bem-sucedido" : "falhou");
}

void setup() {

  Serial.begin(115200);

  pinMode(PIN_SW_JOYSTICK, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar o ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(informacoesDoPar.peer_addr, ENDERECO_MAC_DO_RECEPTOR, 6);
  informacoesDoPar.channel = 0;
  informacoesDoPar.encrypt = false;

  if (esp_now_add_peer(&informacoesDoPar) != ESP_OK) {
    Serial.println("Falha ao adicionar o dispositivo par");
    return;
  }
}

int direcao(int x, int y) {
  if (x > 1000 && y < 1000) {
    // Para frente
    return 1;
  } else if (x > 1000 && y > 3000) {
    // Para trás
    return 2;
  } else if (x > 3000 && y > 1000) {
    // Virar à direita
    return 3;
  } else if (x < 1000 && y > 1000) {
    // Virar à esquerda
    return 4;
  } else {
    // Ficar parado
    return 0;
  }
}

void loop() {

  meusDados.xJoystick = analogRead(PIN_EIXO_X_JOYSTICK);
  meusDados.yJoystick = analogRead(PIN_EIXO_Y_JOYSTICK);
  meusDados.swJoystick = analogRead(PIN_SW_JOYSTICK);

  // Define o sentido em que o carro deverá se mover
  meusDados.codigoDeDirecao = direcao(meusDados.xJoystick, meusDados.yJoystick);

  esp_err_t resultadoDoEnvio = esp_now_send(ENDERECO_MAC_DO_RECEPTOR, (uint8_t *)&meusDados, sizeof(meusDados));

  if (resultadoDoEnvio == ESP_OK) {
    Serial.println("Pacote enviado");
  } else {
    Serial.println("Erro no envio do pacote");
  }
  delay(100);
}