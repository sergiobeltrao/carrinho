#include "WiFi.h"

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin();
  Serial.println("O seu endereço MAC é: " + WiFi.macAddress());

  // Ao fim do upload, entre no terminal e pressione o botão EN do seu ESP32.
}

void loop() {
}
