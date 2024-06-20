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
          playSample(chihat2_DATA, chihat2_NUM_CELLS);
        } else if (sound == HI_HAT_TABLE) {
          playSample(HIHATTABLE_DATA, HIHATTABLE_NUM_CELLS);
        } else if (sound == WATER_SOUND) {
          playSample(water1_DATA, water1_NUM_CELLS);
        }

        // Verificar si el secuenciador se detuvo mientras se reproducía el sample
        if (!isRunning) {
          i2s_zero_dma_buffer(I2S_NUM_0); // Limpia el buffer I2S si se detuvo
          break; // Salir del bucle si se detuvo
        }
      }
    }

    currentStep = (currentStep + 1) % numSteps;
    delay(125); // 125 ms delay for 128 BPM (60,000 ms / 128 beats per minute / 4 steps per beat)
    Serial.printf("Current Step: %d\n", currentStep);
    printMatrix();
  } else {
    i2s_zero_dma_buffer(I2S_NUM_0); // Asegurar que el buffer I2S esté limpio cuando no esté ejecutándose
  }
}

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
