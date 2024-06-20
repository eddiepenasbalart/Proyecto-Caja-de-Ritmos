# Proyecto de Secuenciador de Sonidos con ESP32 y Pantalla OLED

## Descripción

Este proyecto implementa un secuenciador de sonidos utilizando una ESP32, una pantalla OLED y varios botones. El secuenciador permite reproducir diferentes sonidos en una matriz de 7x16, donde cada punto puede activarse o desactivarse mediante los botones de navegación. Además, muestra en la pantalla OLED la matriz de sonidos y la posición actual del cursor, así como el paso actual del secuenciador cuando está en funcionamiento.

## Hardware Utilizado

- ESP32
- Pantalla OLED (SSD1306)
- Botones de navegación (arriba, abajo, izquierda, derecha, select, start/stop, reset)
- Módulo I2S para reproducción de sonidos
- Conexiones de I2C para la pantalla OLED

## Software Utilizado

- Platformio IDE
- Biblioteca Adafruit GFX
- Biblioteca Adafruit SSD1306
- Biblioteca I2S

## Configuración de Pines

- I2S:
  - `I2S_BCK_PIN`: 26
  - `I2S_WS_PIN`: 25
  - `I2S_DATA_PIN`: 27
- I2C:
  - `I2C_SCL_PIN`: 22
  - `I2C_SDA_PIN`: 21
- Botones:
  - `buttonUpPin`: 15
  - `buttonDownPin`: 4
  - `buttonLeftPin`: 16
  - `buttonRightPin`: 17
  - `buttonSelectPin`: 18
  - `buttonStartStopPin`: 19
  - `buttonResetPin`: 5

## Funcionalidades

- Navegación en la matriz de sonidos usando botones de dirección.
- Activación y desactivación de puntos en la matriz usando el botón select.
- Inicio y detención del secuenciador usando el botón start/stop.
- Reseteo de la matriz usando el botón reset.
- Reproducción de sonidos en cada paso del secuenciador.
- Visualización de la matriz y el estado del secuenciador en la pantalla OLED.

## Código

### Configuración Inicial

El código comienza configurando el hardware, incluyendo la pantalla OLED y el módulo I2S:

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
  "S2",
  "Cl",
  "Sn",
  "CH",
  "HH",
  "Wt"
};

void playSample(const int8_t* sampleData, int sampleLength);
void printMatrix();
void setup() {
  Serial.begin(115200);

  // Configuración de I2C
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Inicialización de la pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Dirección I2C predeterminada 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  // Configuración de I2S
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK_PIN,
    .ws_io_num = I2S_WS_PIN,
    .data_out_num = I2S_DATA_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);

  pinMode(buttonUpPin, INPUT_PULLUP);
  pinMode(buttonDownPin, INPUT_PULLUP);
  pinMode(buttonLeftPin, INPUT_PULLUP);
  pinMode(buttonRightPin, INPUT_PULLUP);
  pinMode(buttonSelectPin, INPUT_PULLUP);
  pinMode(buttonStartStopPin, INPUT_PULLUP);
  pinMode(buttonResetPin, INPUT_PULLUP);

  printMatrix();
}

void loop() {
  if (digitalRead(buttonUpPin) == LOW && cursorY > 0) {
    cursorY--;
    Serial.println("Moved Up");
    printMatrix();
    delay(200);
  }
  if (digitalRead(buttonDownPin) == LOW && cursorY < numSounds - 1) {
    cursorY++;
    Serial.println("Moved Down");
    printMatrix();
    delay(200);
  }
  if (digitalRead(buttonLeftPin) == LOW && cursorX > 0) {
    cursorX--;
    Serial.println("Moved Left");
    printMatrix();
    delay(200);
  }
  if (digitalRead(buttonRightPin) == LOW && cursorX < numSteps - 1) {
    cursorX++;
    Serial.println("Moved Right");
    printMatrix();
    delay(200);
  }
  if (digitalRead(buttonSelectPin) == LOW) {
    soundSequences[cursorY][cursorX] = !soundSequences[cursorY][cursorX];
    Serial.printf("Toggled Point at Sound %d, Step %d to %s\n", cursorY, cursorX, soundSequences[cursorY][cursorX] ? "ON" : "OFF");
    printMatrix();
    delay(200);
  }
  if (digitalRead(buttonStartStopPin) == LOW) {
    isRunning = !isRunning;
    Serial.printf("Sequencer %s\n", isRunning ? "Started" : "Stopped");
    delay(200);
  }
  if (digitalRead(buttonResetPin) == LOW) {
    for (int i = 0; i < numSounds; i++) {
      for (int j = 0; j < numSteps; j++) {
        soundSequences[i][j] = false;
      }
    }
    Serial.println("Sequencer Reset");
    printMatrix();
    delay(200);
  }

  if (isRunning) {
    for (int sound = 0; sound < numSounds; sound++) {
      if (soundSequences[sound][currentStep]) {
        Serial.printf("Playing sound %d at step %d\n", sound, currentStep);
        if (sound == KICK_SOUND) {
          playSample(kick9_DATA, kick9_NUM_CELLS);
        } else if (sound == SNARE1_SOUND) {
          playSample(snare3_DATA, snare3_NUM_CELLS);
        } else if (sound == SNARE2_SOUND) {
          playSample(snare3_DATA, snare4_NUM_CELLS);
        } else if (sound == CLAP_SOUND) {
          playSample(clap2_DATA, clap2_NUM_CELLS);
        } else if (sound == SNAP_SOUND) {
          playSample(snap1_DATA, snap1_NUM_CELLS);
        } else if (sound == CLOSED_HAT_SOUND) {
          playSample(chihat2_DATA, chihat2
