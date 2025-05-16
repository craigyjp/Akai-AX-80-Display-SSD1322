#include <SPI.h>
#include <U8g2lib.h>
#include <RoxMux.h>

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 64

// SSD1322 OLED pins
#define OLED_CS 5
#define OLED_RST 4
#define OLED_DC 2
#define OLED_SCK 18
#define OLED_MOSI 23

// Setup SSD1322 display using hardware SPI
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RST);

// Interrupt pins
const uint8_t INTERRUPT_PINS[8] = {13, 34, 14, 27, 26, 25, 21, 35};

#define LABEL_SET 0

uint8_t DISPLAY_RANGE0[2] = {1, 7};
uint8_t DISPLAY_RANGE1[2] = {0, 7};
uint8_t DISPLAY_RANGE2[2] = {1, 7};
uint8_t DISPLAY_RANGE3[2] = {1, 6};
uint8_t DISPLAY_RANGE4[2] = {0, 8};

// Input multiplexer
int yValues[8][13];
int lowerRange = 0;
int upperRange = 8;

volatile bool updateDisplay = false;
volatile bool interruptTriggered[8] = {false};

#define MUX_TOTAL 2
Rox74HC165<MUX_TOTAL> mux;

#define PIN_DATA 22
#define PIN_LOAD 33
#define PIN_CLK 32

TaskHandle_t TaskDisplay;

void IRAM_ATTR handleInterrupt0() { interruptTriggered[0] = true; }
void IRAM_ATTR handleInterrupt1() { interruptTriggered[1] = true; }
void IRAM_ATTR handleInterrupt2() { interruptTriggered[2] = true; }
void IRAM_ATTR handleInterrupt3() { interruptTriggered[3] = true; }
void IRAM_ATTR handleInterrupt4() { interruptTriggered[4] = true; }
void IRAM_ATTR handleInterrupt5() { interruptTriggered[5] = true; }
void IRAM_ATTR handleInterrupt6() { interruptTriggered[6] = true; }
void IRAM_ATTR handleInterrupt7() { interruptTriggered[7] = true; }

void setup() {
  Serial.begin(115200);

  mux.begin(PIN_DATA, PIN_LOAD, PIN_CLK);

  void (*interruptHandlers[8])() = {
    handleInterrupt0, handleInterrupt1, handleInterrupt2, handleInterrupt3,
    handleInterrupt4, handleInterrupt5, handleInterrupt6, handleInterrupt7};

  for (int i = 0; i < 8; i++) {
    pinMode(INTERRUPT_PINS[i], INPUT_PULLUP);
    attachInterrupt(INTERRUPT_PINS[i], interruptHandlers[i], FALLING);
  }

  switch (LABEL_SET) {
    case 0: lowerRange = DISPLAY_RANGE0[0]; upperRange = DISPLAY_RANGE0[1]; break;
    case 1: lowerRange = DISPLAY_RANGE1[0]; upperRange = DISPLAY_RANGE1[1]; break;
    case 2: lowerRange = DISPLAY_RANGE2[0]; upperRange = DISPLAY_RANGE2[1]; break;
    case 3: lowerRange = DISPLAY_RANGE3[0]; upperRange = DISPLAY_RANGE3[1]; break;
    case 4: lowerRange = DISPLAY_RANGE4[0]; upperRange = DISPLAY_RANGE4[1]; break;
    default: lowerRange = DISPLAY_RANGE1[0]; upperRange = DISPLAY_RANGE1[1]; break;
  }

  u8g2.begin();

  xTaskCreatePinnedToCore(
    displayTask, "DisplayTask", 10000, NULL, 1, &TaskDisplay, 0);
}

void loop() {
  for (int i = 0; i < 8; i++) {
    if (interruptTriggered[i]) {
      interruptTriggered[i] = false;
      delayMicroseconds(70);
      mux.update();
      for (uint8_t j = 0; j < 13; j++) {
        yValues[i][j] = mux.read(j);
      }
      updateDisplay = true;
    }
  }
}

void displayTask(void* parameter) {
  while (true) {
    if (updateDisplay) {
      updateDisplay = false;

      u8g2.clearBuffer();

      int row0Height = (SCREEN_HEIGHT / 13) * 2;               // Double-height for row 0
      int remainingHeight = SCREEN_HEIGHT - row0Height;        // Remaining height for rows 1-12
      int blockHeight = remainingHeight / 12;                  // Even height for rows 1-12
      int blockWidth = 19;                                     // Narrow column width
      int spacing = blockWidth + 13;                           // Adjusted spacing

      // Draw row 0 at the bottom with double height
      for (int x = lowerRange; x < upperRange; x++) {
        int xPos = x * spacing;
        int yPos = SCREEN_HEIGHT - row0Height;

        if (yValues[x][0] == 0) {
          u8g2.drawBox(xPos + 1, yPos + 1, blockWidth - 2, row0Height - 2);
        } else {
          u8g2.drawFrame(xPos + 1, yPos + 1, blockWidth - 2, row0Height - 2);
        }
      }

      // Draw rows 1-12 above row 0
      for (int y = 1; y < 13; y++) {
        for (int x = lowerRange; x < upperRange; x++) {
          int xPos = x * spacing;
          int yPos = SCREEN_HEIGHT - row0Height - (y * blockHeight);

          if (yValues[x][y] == 0) {
            u8g2.drawBox(xPos + 1, yPos + 1, blockWidth - 2, blockHeight - 2);
          }
        }
      }

      u8g2.sendBuffer();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

