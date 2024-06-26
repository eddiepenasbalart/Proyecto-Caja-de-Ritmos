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

### Parte 1: Librerías i asignación de pines 

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

// Configuración del I2C para la pantalla OLED
#define I2C_SCL_PIN 22
#define I2C_SDA_PIN 21

// Definiciones para la reproducción de muestras de sonido
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
  "Kick",
  "Snare 1",
  "Snare 2",
  "Clap",
  "Snap",
  "Closed Hat",
  "Hi Hat",
  "Water"
};

```


### Parte 2: Setup() - Configuración Inicial
En la función setup(), configuramos los pines I2C para la comunicación con la pantalla OLED, inicializamos la pantalla y configuramos el I2S para la reproducción de audio. Además, se configuran los pines de los botones y se llama a la función printMatrix() para mostrar la matriz en la pantalla OLED al inicio.

```cpp

void setup() {
  Serial.begin(115200);

  // Configuración de I2C para la pantalla OLED
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Inicialización de la pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Dirección I2C predeterminada 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000); // Espera inicial para la pantalla OLED
  display.clearDisplay();

  // Configuración de I2S para la reproducción de audio
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

  // Instalación y configuración del driver I2S
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);

  // Configuración de los pines de los botones
  pinMode(buttonUpPin, INPUT_PULLUP);
  pinMode(buttonDownPin, INPUT_PULLUP);
  pinMode(buttonLeftPin, INPUT_PULLUP);
  pinMode(buttonRightPin, INPUT_PULLUP);
  pinMode(buttonSelectPin, INPUT_PULLUP);
  pinMode(buttonStartStopPin, INPUT_PULLUP);
  pinMode(buttonResetPin, INPUT_PULLUP);

  printMatrix(); // Mostrar la matriz en la pantalla OLED inicialmente
}
```

### Parte 3:Loop() - Funcionalidad Principal
El loop principal se encarga de manejar la lógica del secuenciador y actualizar la visualización en la pantalla OLED. Aquí se implementa la lógica para mover el cursor por la matriz con los botones, activar/desactivar puntos en la matriz, iniciar/parar el secuenciador y reproducir las muestras de sonido según la configuración actual de la matriz.
```cpp
void loop() {
  // Lógica para mover el cursor en la matriz con los botones
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

  // Lógica para activar/desactivar puntos en la matriz con el botón Select
  if (digitalRead(buttonSelectPin) == LOW) {
    soundSequences[cursorY][cursorX] = !soundSequences[cursorY][cursorX];
    Serial.printf("Toggled Point at Sound %d, Step %d to %s\n", cursorY, cursorX, soundSequences[cursorY][cursorX] ? "ON" : "OFF");
    printMatrix();
    delay(200);
  }

  // Lógica para iniciar/parar el secuenciador con el botón Start/Stop
  if (digitalRead(buttonStartStopPin) == LOW) {
    isRunning = !isRunning;
    Serial.printf("Sequencer %s\n", isRunning ? "Started" : "Stopped");
    delay(200);
  }

  // Lógica para reiniciar la matriz con el botón Reset
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

  // Lógica para reproducir las muestras de sonido según el estado actual del secuenciador
  if (isRunning) {
    for (int sound = 0; sound < numSounds; sound++) {
      if (soundSequences[sound][currentStep]) {
        Serial.printf("Playing sound %d at step %d\n", sound, currentStep);
        // Lógica para reproducir las muestras de sonido según el tipo de sonido
        switch (sound) {
          case KICK_SOUND:
            playSample(kick9_DATA, kick9_NUM_CELLS);
            break;
          case SNARE1_SOUND:
            playSample(snare3_DATA, snare3_NUM_CELLS);
            break;
          case SNARE2_SOUND:
            playSample(snare3_DATA, snare4_NUM_CELLS);
            break;
          case CLAP_SOUND:
            playSample(clap2_DATA, clap2_NUM_CELLS);
            break;
          case SNAP_SOUND:
            playSample(snap1_DATA, snap1_NUM_CELLS);
            break;
          case CLOSED_HAT_SOUND:
            playSample(chihat2_DATA, chihat2_NUM_CELLS);
            break;
          case HI_HAT_TABLE:
            playSample(HIHATTABLE_DATA, HIHATTABLE_NUM_CELLS);
            break;
          case WATER_SOUND:
            playSample(water1_DATA, water1_NUM_CELLS);
            break;
          default:
            break;
        }

        // Verificar si el secuenciador se detuvo mientras se repro
  currentStep = (currentStep + 1) % numSteps;
    delay(125); // 125 ms delay for 128 BPM (60,000 ms / 128 beats per minute / 4 steps per beat)
    Serial.printf("Current Step: %d\n", currentStep);
    printMatrix();
  } else {
    i2s_zero_dma_buffer(I2S_NUM_0); // Asegurar que el buffer I2S esté limpio cuando no esté ejecutándose
  }
}

```
### Parte 4:Funciones playsample() i printMatrix()
La función printMatrix se encarga de mostrar la matriz de sonidos en una pantalla OLED i la función playSample se encarga de reproducir una muestra de audio utilizando el periférico I2S.
```cpp
void printMatrix() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Iterar sobre cada sonido en la matriz
  for (int sound = 0; sound < numSounds; sound++) {
    display.setCursor(0, sound * 8); // Ajustar la posición vertical del texto según el sonido
    display.printf("%s: ", soundNames[sound]);

    // Iterar sobre cada paso en la matriz
    for (int step = 0; step < numSteps; step++) {
      // Verificar si el cursor está sobre este punto y reemplazar el carácter
      if (sound == cursorY && step == cursorX) {
        display.print("X"); // Dibujar 'X' en lugar del carácter actual
      } else if (soundSequences[sound][step]) {
        display.print("O"); // Punto activo en la matriz
      } else {
        display.print("-"); // Punto inactivo en la matriz
      }
    }
    display.println(); // Nueva línea para el siguiente sonido
  }

  display.display(); // Mostrar en la pantalla OLED
}



void playSample(const int8_t* sampleData, int sampleLength) {
  // Usa memoria dinámica para evitar stack overflow
  int16_t* i2sData = (int16_t*)malloc(sampleLength * sizeof(int16_t));

  if (i2sData == nullptr) {
    Serial.println("Error allocating memory");
    return;
  }

  // Convierte los datos del sample a 16 bits
  for (int i = 0; i < sampleLength; i++) {
    i2sData[i] = sampleData[i] << 8;
  }

  size_t bytesWritten;
  i2s_write(I2S_NUM_0, i2sData, sampleLength * sizeof(int16_t), &bytesWritten, portMAX_DELAY);

  free(i2sData); // Libera la memoria después de usarla
}

```
