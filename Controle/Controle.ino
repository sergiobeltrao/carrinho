// Código do transmissor. Testado com a versão 3.1.3 da biblioteca do ESP32.
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Preferences.h>
#include <U8g2lib.h>
#include <Wire.h>

// Não se esqueça de trocar pelo endereço do seu receptor.
const uint8_t ENDERECO_MAC_DO_RECEPTOR[] = { 0x3C, 0x8A, 0x1F, 0x50, 0x65, 0x7C };

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

const short ALTURA_DISPLAY = 64;
const short LARGURA_DISPLAY = 128;

Preferences preferences;

const short PIN_EIXO_X_JOYSTICK_DIREITA = 34;
const short PIN_EIXO_Y_JOYSTICK_DIREITA = 35;
const short PIN_SW_JOYSTICK_DIREITA = 17;
const short PIN_EIXO_X_JOYSTICK_ESQUERDA = 36;
const short PIN_EIXO_Y_JOYSTICK_ESQUERDA = 39;
const short PIN_SW_JOYSTICK_ESQUERDA = 16;
const short PIN_INCREMENTO = 33;
const short PIN_DECREMENTO = 25;
const short PIN_APLICAR_E_RESET = 32;
const short PIN_LED_CONEXAO_FALHOU = 26;
const short PIN_LED_CONEXAO_BEM_SUCEDIDA = 27;

bool conexaoBemSucedida = false;
bool reiniciar = false;
bool mudancaDeCanalComCarroDesligado = false;

unsigned short nivelDeVelocidadeDoCarro = 100;
unsigned int ultimoIncrementoDeVelocidade = 0;
unsigned int ultimoDecrementoDeVelocidade = 0;
const short TEMPO_DE_ATRASO_DE_DEBOUNCE = 50;

short canalDeTransmissao = 1;
bool menuVelocidade = true;
bool menuCanal = false;

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

void menuAtual() {
  if (digitalRead(PIN_SW_JOYSTICK_DIREITA) == LOW) {
    menuVelocidade = false;
    menuCanal = true;
  } else if (digitalRead(PIN_SW_JOYSTICK_ESQUERDA) == LOW) {
    menuVelocidade = true;
    menuCanal = false;
  }
}

short controleDeVelocidade() {
  if (!menuVelocidade) {
    // Retorna o valor antigo quando o menu de velocidade não está ativo
    return nivelDeVelocidadeDoCarro;
  }

  unsigned int agora = millis();
  if (digitalRead(PIN_INCREMENTO) == LOW && nivelDeVelocidadeDoCarro < 100) {
    if (agora - ultimoIncrementoDeVelocidade > TEMPO_DE_ATRASO_DE_DEBOUNCE) {
      nivelDeVelocidadeDoCarro++;
      ultimoIncrementoDeVelocidade = agora;
    }
  }

  if (digitalRead(PIN_DECREMENTO) == LOW && nivelDeVelocidadeDoCarro > 0) {
    if (agora - ultimoDecrementoDeVelocidade > TEMPO_DE_ATRASO_DE_DEBOUNCE) {
      nivelDeVelocidadeDoCarro--;
      ultimoDecrementoDeVelocidade = agora;
    }
  }

  return nivelDeVelocidadeDoCarro;
}

void trocarCanalEReiniciar(bool reiniciar) {
  if (reiniciar) {
    preferences.putInt("valor", canalDeTransmissao);
    ESP.restart();
  }
}

void enviarNovaConfiguracaoDeCanal() {
  if (digitalRead(PIN_APLICAR_E_RESET) == LOW && digitalRead(PIN_LED_CONEXAO_BEM_SUCEDIDA) == HIGH) {
    meusDados.canal = canalDeTransmissao;
    reiniciar = true;
  } else if (digitalRead(PIN_APLICAR_E_RESET) == LOW && digitalRead(PIN_LED_CONEXAO_FALHOU) == HIGH) {
    mudancaDeCanalComCarroDesligado = true;
  }
}

void mudarCanalDeTrasmissao() {
  if (menuCanal) {
    static bool lastStateUp = HIGH;
    static bool lastStateDown = HIGH;

    bool currentStateUp = digitalRead(PIN_INCREMENTO);
    bool currentStateDown = digitalRead(PIN_DECREMENTO);

    short menorCanal = 1;
    short maiorCanal = 12;

    if (lastStateUp == HIGH && currentStateUp == LOW && canalDeTransmissao < maiorCanal) {
      canalDeTransmissao++;
    }

    if (lastStateDown == HIGH && currentStateDown == LOW && canalDeTransmissao > menorCanal) {
      canalDeTransmissao--;
    }

    lastStateUp = currentStateUp;
    lastStateDown = currentStateDown;

    delay(50);
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

short displayCentralizarHorizontal(char *texto) {
  short larguraDoTexto = u8g2.getStrWidth(texto);
  return (LARGURA_DISPLAY - larguraDoTexto) / 2;
}

short displayCentralizarVertical() {
  short alturaDoTexto = u8g2.getAscent() - u8g2.getDescent();
  return (ALTURA_DISPLAY + alturaDoTexto) / 2;
}

void displayItemDoMenu(char *titulo, char *valor) {
  const short POSICAO_VERTICAL_TITULO = 20;
  const short POSICAO_VERTICAL_VALOR = 50;

  // Espaçamento a esquerda, espaçamento superior, texto
  u8g2.drawStr(displayCentralizarHorizontal(titulo), POSICAO_VERTICAL_TITULO, titulo);
  u8g2.drawStr(displayCentralizarHorizontal(valor), POSICAO_VERTICAL_VALOR, valor);
}

void displayIconesDeNavegacao(bool anterior, bool posterior) {

  u8g2.setFont(u8g2_font_fub14_tr);

  if (posterior) {
    char proximoMenu[] = ">";
    int distanciaEsquerda = LARGURA_DISPLAY - u8g2.getStrWidth(proximoMenu);
    u8g2.drawStr(distanciaEsquerda, displayCentralizarVertical(), proximoMenu);
  }

  if (anterior) {
    char menuAnterior[] = "<";
    u8g2.drawStr(0, displayCentralizarVertical(), menuAnterior);
  }
}

void displayErro() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub17_tr);
  char txtErro[] = "Erro!";
  u8g2.drawStr(displayCentralizarHorizontal(txtErro), displayCentralizarVertical(), txtErro);
  u8g2.sendBuffer();
  delay(1500);
}

void display() {

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub17_tr);

  if (menuVelocidade) {

    char textoDisplay[5];
    snprintf(textoDisplay, sizeof(textoDisplay), "%d%%", meusDados.velocidade);
    displayItemDoMenu("Velocidade", textoDisplay);

    displayIconesDeNavegacao(false, true);
  } else {
    char textoCanal[4];
    snprintf(textoCanal, sizeof(textoCanal), "%d%", canalDeTransmissao);
    displayItemDoMenu("Canal", textoCanal);

    displayIconesDeNavegacao(true, false);

    if (canalDeTransmissao != preferences.getInt("valor", 1)) {
      char txtNaoSalvo[] = "___";
      u8g2.drawStr(displayCentralizarHorizontal(txtNaoSalvo), 60, txtNaoSalvo);
    }
  }

  if (mudancaDeCanalComCarroDesligado == true) {
    displayErro();
    mudancaDeCanalComCarroDesligado = false;
  }
  u8g2.sendBuffer();
}

void setup() {

  Serial.begin(115200);
  u8g2.begin();

  pinMode(PIN_INCREMENTO, INPUT_PULLUP);
  pinMode(PIN_DECREMENTO, INPUT_PULLUP);
  pinMode(PIN_EIXO_X_JOYSTICK_ESQUERDA, INPUT_PULLUP);
  pinMode(PIN_EIXO_Y_JOYSTICK_ESQUERDA, INPUT_PULLUP);
  pinMode(PIN_APLICAR_E_RESET, INPUT_PULLUP);
  pinMode(PIN_SW_JOYSTICK_DIREITA, INPUT_PULLUP);
  pinMode(PIN_SW_JOYSTICK_ESQUERDA, INPUT_PULLUP);

  pinMode(PIN_LED_CONEXAO_FALHOU, OUTPUT);
  pinMode(PIN_LED_CONEXAO_BEM_SUCEDIDA, OUTPUT);

  WiFi.mode(WIFI_STA);

  preferences.begin("config", false);
  canalDeTransmissao = preferences.getInt("valor", 1);
  esp_wifi_set_channel(canalDeTransmissao, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar o ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(informacoesDoPar.peer_addr, ENDERECO_MAC_DO_RECEPTOR, 6);
  informacoesDoPar.channel = canalDeTransmissao;
  informacoesDoPar.encrypt = false;

  if (esp_now_add_peer(&informacoesDoPar) != ESP_OK) {
    Serial.println("Falha ao adicionar o dispositivo par");
    return;
  }
}

void loop() {
  trocarCanalEReiniciar(reiniciar);
  enviarNovaConfiguracaoDeCanal();
  meusDados.xJoystickDireita = analogRead(PIN_EIXO_X_JOYSTICK_DIREITA);
  meusDados.yJoystickDireita = analogRead(PIN_EIXO_Y_JOYSTICK_DIREITA);
  meusDados.xJoystickEsquerda = analogRead(PIN_EIXO_X_JOYSTICK_ESQUERDA);
  meusDados.yJoystickEsquerda = analogRead(PIN_EIXO_Y_JOYSTICK_ESQUERDA);

  meusDados.codigoDeDirecao = direcao(meusDados.xJoystickDireita, meusDados.yJoystickDireita, meusDados.xJoystickEsquerda, meusDados.yJoystickEsquerda);
  meusDados.velocidade = controleDeVelocidade();
  menuAtual();
  display();
  mudarCanalDeTrasmissao();

  esp_err_t resultadoDoEnvio = esp_now_send(ENDERECO_MAC_DO_RECEPTOR, (uint8_t *)&meusDados, sizeof(meusDados));

  if (resultadoDoEnvio == ESP_OK) {
    Serial.println("Pacote enviado");
    mensagensDeDebug(true);
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

  delay(100);
}