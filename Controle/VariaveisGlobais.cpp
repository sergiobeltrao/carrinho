#include <Arduino.h>
#include "Pinagem.h"
#include "VariaveisGlobais.h"
#include "TrocaDeCanal.h"

bool carroDesligado = false;
bool ativarDebug = true;
bool reiniciar = false;
short canalAtual = 1;
short canalTemporario = canalAtual;
short menuAtual = 1;
unsigned short nivelDeVelocidadeDoCarro = 100;
unsigned int ultimoIncrementoDeVelocidade = 0;
unsigned int ultimoDecrementoDeVelocidade = 0;
bool conexaoBemSucedida = false;