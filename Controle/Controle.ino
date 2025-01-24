// Código do transmissor. Testado com a versão 3.1.1 da biblioteca do ESP32.

#include <esp_now.h>
#include <WiFi.h>
#include <U8g2lib.h>
#include <Wire.h>

// Não se esqueça de trocar pelo endereço do seu receptor.
const uint8_t ENDERECO_MAC_DO_RECEPTOR[] = { 0x3C, 0x8A, 0x1F, 0x50, 0x65, 0x7C };

// Configuração do display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

const short PIN_EIXO_X_JOYSTICK_DIREITA = 34;
const short PIN_EIXO_Y_JOYSTICK_DIREITA = 35;
const short PIN_EIXO_X_JOYSTICK_ESQUERDA = 36;
const short PIN_EIXO_Y_JOYSTICK_ESQUERDA = 39;
const short PIN_AUMENTAR_VELOCIDADE = 33;
const short PIN_DIMINUIR_VELOCIDADE = 25;

unsigned short nivelDeVelocidadeDoCarro = 100;
unsigned int ultimoIncrementoDeVelocidade = 0;
unsigned int ultimoDecrementoDeVelocidade = 0;
const short TEMPO_DE_ATRASO_DE_DEBOUNCE = 50;

// A estrutura de dados que será enviada
typedef struct mensagemEstruturada {
  short xJoystickDireita;
  short yJoystickDireita;
  short xJoystickEsquerda;
  short yJoystickEsquerda;
  short codigoDeDirecao;
  short velocidade;
} mensagemEstruturada;

mensagemEstruturada meusDados;

esp_now_peer_info_t informacoesDoPar;

// A função que é chamada quando o pacote é enviado
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nStatus de envio: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "bem-sucedido" : "falhou");
}

void setup() {

  u8g2.begin();

  Serial.begin(115200);
  pinMode(PIN_AUMENTAR_VELOCIDADE, INPUT_PULLUP);
  pinMode(PIN_DIMINUIR_VELOCIDADE, INPUT_PULLUP);
  pinMode(PIN_EIXO_X_JOYSTICK_ESQUERDA, INPUT_PULLUP);
  pinMode(PIN_EIXO_Y_JOYSTICK_ESQUERDA, INPUT_PULLUP);
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

short direcao(short xDireita, short yDireita, short xEsquerda, short yEsquerda) {

  bool joystickDaDireitoParado = (xDireita >= 1600 && xDireita <= 1900) && (yDireita >= 1600 && yDireita <= 1900);
  bool joystickDaEsquerdaParado = (xEsquerda >= 1600 && xEsquerda <= 1900) && (yEsquerda >= 1600 && yEsquerda <= 1900);

  bool joystickDaDireitoParaFrente = xDireita >= 0 && yDireita == 0;
  bool joystickDaEsquerdaParaFrente = xEsquerda >= 0 && yEsquerda == 0;

  bool joystickDaDireitoParaTras = xDireita >= 0 && yDireita > 3072;
  bool joystickDaEsquerdaParaTras = xEsquerda >= 0 && yEsquerda > 3072;

  bool joystickDaDireitoParaDireita = xDireita >= 3072 && yDireita <= 4096;
  bool joystickDaEsquerdaParaDireita = xEsquerda >= 3072 && yEsquerda <= 4096;

  bool joystickDaDireitoParaEsquerda = xDireita == 0 && yDireita >= 0;
  bool joystickDaEsquerdaParaEsquerda = xEsquerda == 0 && yEsquerda >= 0;

  bool paraFrente = joystickDaDireitoParaFrente && joystickDaEsquerdaParaFrente || joystickDaDireitoParaFrente && joystickDaEsquerdaParado || joystickDaDireitoParado && joystickDaEsquerdaParaFrente;
  bool paraTras = joystickDaDireitoParaTras && joystickDaEsquerdaParaTras || joystickDaDireitoParaTras && joystickDaEsquerdaParado || joystickDaDireitoParado && joystickDaEsquerdaParaTras;
  bool direita = joystickDaDireitoParaDireita && joystickDaEsquerdaParaDireita || joystickDaDireitoParado && joystickDaEsquerdaParaDireita || joystickDaDireitoParaDireita && joystickDaEsquerdaParado;
  bool esquerda = joystickDaDireitoParaEsquerda && joystickDaEsquerdaParaEsquerda || joystickDaDireitoParaEsquerda && joystickDaEsquerdaParado || joystickDaDireitoParado && joystickDaEsquerdaParaEsquerda;

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
  unsigned int agora = millis();
  if (digitalRead(PIN_AUMENTAR_VELOCIDADE) == LOW && nivelDeVelocidadeDoCarro < 100) {
    if (agora - ultimoIncrementoDeVelocidade > TEMPO_DE_ATRASO_DE_DEBOUNCE) {
      nivelDeVelocidadeDoCarro++;
      ultimoIncrementoDeVelocidade = agora;
    }
  }

  if (digitalRead(PIN_DIMINUIR_VELOCIDADE) == LOW && nivelDeVelocidadeDoCarro > 0) {
    if (agora - ultimoDecrementoDeVelocidade > TEMPO_DE_ATRASO_DE_DEBOUNCE) {
      nivelDeVelocidadeDoCarro--;
      ultimoDecrementoDeVelocidade = agora;
    }
  }
  return nivelDeVelocidadeDoCarro;
}

void display() {
  char textoDisplay[5];
  snprintf(textoDisplay, sizeof(textoDisplay), "%d%%", meusDados.velocidade);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub30_tr);

  if (meusDados.velocidade < 100) {
    // Espaçamento a esquerda, espaçamento superior, texto
    u8g2.drawStr(25, 50, textoDisplay);
  } else {
    u8g2.drawStr(10, 50, textoDisplay);
  }
  u8g2.sendBuffer();
}

void mensagensDeDebug(bool ativado) {
  Serial.println("Valor no X da Direita: " + String(meusDados.xJoystickDireita));
  Serial.println("Valor no Y da Direita: " + String(meusDados.yJoystickDireita));
  Serial.println("Valor no X da Esquerda: " + String(meusDados.xJoystickEsquerda));
  Serial.println("Valor no Y da Esquerda: " + String(meusDados.yJoystickEsquerda));
  Serial.println("Nível de velocidade: " + String(meusDados.velocidade) + "%");

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

void loop() {
  meusDados.xJoystickDireita = analogRead(PIN_EIXO_X_JOYSTICK_DIREITA);
  meusDados.yJoystickDireita = analogRead(PIN_EIXO_Y_JOYSTICK_DIREITA);
  meusDados.xJoystickEsquerda = analogRead(PIN_EIXO_X_JOYSTICK_ESQUERDA);
  meusDados.yJoystickEsquerda = analogRead(PIN_EIXO_Y_JOYSTICK_ESQUERDA);

  meusDados.codigoDeDirecao = direcao(meusDados.xJoystickDireita, meusDados.yJoystickDireita, meusDados.xJoystickEsquerda, meusDados.yJoystickEsquerda);
  meusDados.velocidade = controleDeVelocidade();
  display();

  esp_err_t resultadoDoEnvio = esp_now_send(ENDERECO_MAC_DO_RECEPTOR, (uint8_t *)&meusDados, sizeof(meusDados));
  if (resultadoDoEnvio == ESP_OK) {
    Serial.println("Pacote enviado");
    mensagensDeDebug(true);
  } else {
    Serial.println("Erro no envio do pacote");
  }
  delay(100);
}