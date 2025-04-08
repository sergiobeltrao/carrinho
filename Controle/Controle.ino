// Código do transmissor. Testado com a versão 3.2.0 da biblioteca do ESP32.
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Preferences.h>
#include "Display.h"
#include "TrocaDeCanal.h"
#include "Pinagem.h"
#include "VariaveisGlobais.h"
#include "Motores.h"

// Não se esqueça de trocar pelo endereço do seu receptor.
const uint8_t ENDERECO_MAC_DO_RECEPTOR[] = { 0x3C, 0x8A, 0x1F, 0x50, 0x65, 0x7C };

Preferences preferences;

// A estrutura de dados que será enviada
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

// A função que é chamada quando o pacote é enviado
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

  Serial.print("\r\nStatus de envio: ");

  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("bem-sucedido");
    conexaoBemSucedida = true;
  } else {
    Serial.println("falhou");
    conexaoBemSucedida = false;
  }
}

void mensagensDeDebug(bool ativado) {
  if (ativado) {
    Serial.println("Valor no X da Direita: " + String(meusDados.xJoystickDireita));
    Serial.println("Valor no Y da Direita: " + String(meusDados.yJoystickDireita));
    Serial.println("Valor no X da Esquerda: " + String(meusDados.xJoystickEsquerda));
    Serial.println("Valor no Y da Esquerda: " + String(meusDados.yJoystickEsquerda));
    Serial.println("Nível de Velocidade: " + String(meusDados.velocidade) + "%");

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
  }
}

void setup() {

  Serial.begin(115200);
  preferences.begin("config", false);
  canalAtual = preferences.getInt("valor", 1);
  canalTemporario = preferences.getInt("valor", 1);

  iniciarDisplay();

  configurarPinos();

  WiFi.mode(WIFI_STA);

  esp_wifi_set_channel(canalAtual, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar o ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(informacoesDoPar.peer_addr, ENDERECO_MAC_DO_RECEPTOR, 6);
  informacoesDoPar.channel = canalAtual;
  informacoesDoPar.encrypt = false;

  if (esp_now_add_peer(&informacoesDoPar) != ESP_OK) {
    Serial.println("Falha ao adicionar o dispositivo par");
    return;
  }
}

void loop() {
  salvarEReiniciar();
  aplicarNovoCanal();
  meusDados.xJoystickDireita = analogRead(PIN_EIXO_X_JOYSTICK_DIREITA);
  meusDados.yJoystickDireita = analogRead(PIN_EIXO_Y_JOYSTICK_DIREITA);
  meusDados.xJoystickEsquerda = analogRead(PIN_EIXO_X_JOYSTICK_ESQUERDA);
  meusDados.yJoystickEsquerda = analogRead(PIN_EIXO_Y_JOYSTICK_ESQUERDA);

  meusDados.codigoDeDirecao = direcao(meusDados.xJoystickDireita, meusDados.yJoystickDireita, meusDados.xJoystickEsquerda, meusDados.yJoystickEsquerda);
  meusDados.velocidade = controleDeVelocidade();

  meusDados.canal = canalAtual;
  bool incremento = digitalRead(PIN_INCREMENTO);
  bool decremento = digitalRead(PIN_DECREMENTO);
  mudarCanalDeTrasmissao(menuAtual, incremento, decremento);

  if (reiniciar) {
    preferences.putInt("valor", canalTemporario);
  }

  if (digitalRead(PIN_SW_JOYSTICK_DIREITA) == LOW && menuAtual < ULTIMO_INDICE - 1) {
    menuAtual++;
  } else if (digitalRead(PIN_SW_JOYSTICK_ESQUERDA) == LOW && menuAtual > 1) {
    menuAtual--;
  }

  if (menuAtual == VELOCIDADE) {
    display(VELOCIDADE, meusDados.velocidade, meusDados.velocidade);
  } else if (menuAtual == CANAL) {
    // Menu, valor atual, valor anterior, se o carro está desligado
    display(CANAL, canalAtual, canalTemporario);
  }

  esp_err_t resultadoDoEnvio = esp_now_send(ENDERECO_MAC_DO_RECEPTOR, (uint8_t *)&meusDados, sizeof(meusDados));

  if (resultadoDoEnvio == ESP_OK) {
    Serial.println("Pacote enviado");
    mensagensDeDebug(ativarDebug);
  } else {
    Serial.println("Erro no envio do pacote");
  }

  if (conexaoBemSucedida) {
    digitalWrite(PIN_LED_CONEXAO_BEM_SUCEDIDA, HIGH);
    digitalWrite(PIN_LED_CONEXAO_FALHOU, LOW);
  } else {
    digitalWrite(PIN_LED_CONEXAO_BEM_SUCEDIDA, LOW);
    digitalWrite(PIN_LED_CONEXAO_FALHOU, HIGH);
  }

  carroDesligado = false;

  delay(100);
}
