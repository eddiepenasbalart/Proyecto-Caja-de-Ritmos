# Proyecto de Secuenciador de Sonidos con ESP32 y Pantalla OLED

Este proyecto implementa un secuenciador de sonidos utilizando un ESP32, una pantalla OLED y varios botones para la interacción del usuario. El proyecto permite reproducir diferentes muestras de audio en un patrón de matriz, y la pantalla OLED muestra el estado actual del secuenciador.

## Descripción

El secuenciador de sonidos permite a los usuarios crear patrones de sonido en una matriz de 8 sonidos y 16 pasos. Los sonidos disponibles incluyen golpes de bombo, caja, aplausos y otros efectos. Los usuarios pueden navegar y seleccionar puntos en la matriz para activar o desactivar sonidos en pasos específicos. 

## Componentes Utilizados

- **ESP32**
- **Pantalla OLED (SSD1306)**
- **Botones de entrada**
- **Altavoces o auriculares para salida de audio**
- **Bibliotecas de Arduino**: Wire, Adafruit_GFX, Adafruit_SSD1306, driver/i2s

## Esquema de Conexiones

- **Pantalla OLED**
  - GND: GND
  - VCC: 3.3V
  - SCL: GPIO 22
  - SDA: GPIO 21

- **Botones de entrada**
  - Button Up: GPIO 15
  - Button Down: GPIO 4
  - Button Left: GPIO 16
  - Button Right: GPIO 17
  - Button Select: GPIO 18
  - Button Start/Stop: GPIO 19
  - Button Reset: GPIO 5

- **I2S (Audio)**
  - BCK: GPIO 26
  - WS: GPIO 25
  - DATA: GPIO 27

## Código Fuente

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <driver/i2s.h>
#include "d_kit.h" // Incluye el archivo de cabecera con las muestras de audio

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configuración del I2S
#define I2S_BCK_PIN  26
#define I2S_WS_PIN   25
#define I2S_DATA_PIN 27

// Configuración del I2C
#define I2C_SCL_PIN 22
#define I2C_SDA_PIN 21

const int SAMPLE_RATE = 16384;
const int KICK_SOUND = 0;
const int SNARE1_SOUND = 1;
const int SNARE2_SOUND = 2;
const int CLAP_SOUND = 3;
const int SNAP_SOUND = 4;
const int CLOSED_HAT_SOUND = 5;
const int HI_HAT_TABLE = 6;
const int WATER_SOUND = 7;

const int numSounds = 8;
const int numSteps = 16;
bool soundSequences[numSounds][numSteps] = {false};
int cursorX = 0;
int cursorY = 0;
bool isRunning = false;
int currentStep = 0;

const int buttonUpPin = 15;
const int buttonDownPin = 4;
const int buttonLeftPin = 16;
const int buttonRightPin = 17;
const int buttonSelectPin = 18;
const int buttonStartStopPin = 19;
const int buttonResetPin = 5;

const char* soundNames[numSounds] = {
  "Kc",
  "S1",
  "
