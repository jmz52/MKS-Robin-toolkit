#include <Arduino.h>
#include <HardwareSerial.h>

#include "stdio.h"

#include "lcd.h"
#include "xpt2046.h"

uint16_t reg00, lcdId;

int16_t xCalibration, yCalibration, xOffset, yOffset;

uint32_t backlightTimeout;
uint32_t ledTimeout = 0;
uint16_t color = WHITE, bgColor = BLACK;
char text[41];
char controller[8];

void drawCross(uint16_t x, uint16_t y, uint16_t color) {
  lcdSetWindow(x - 15, y, x + 15, y); lcdFill(color, 31);
  lcdSetWindow(x, y - 15, x, y + 15); lcdFill(color, 31);
}

void setup() {
  volatile uint32_t data;
  uint32_t i, j;

  uint16_t x[4] = {0,0,0,0}, y[4] = {0,0,0,0};
  uint16_t length;

  pinMode(PB2, OUTPUT); // initialize LED digital pin as an output.

  Serial3.begin(250000);
  Serial3.println("\nSTM32F103ZET6");
  Serial3.flush();

  dma_init(FSMC_DMA_DEV);
  dma_disable(FSMC_DMA_DEV, FSMC_DMA_CHANNEL);
  dma_set_priority(FSMC_DMA_DEV, FSMC_DMA_CHANNEL, DMA_PRIORITY_HIGH);

  LCD_BacklightOn();

  LCD_IO_Init();
  LCD_Delay(100);

  Serial3.print("DisplayID : ");
  reg00 = LCD_IO_ReadData(0x00);
  if (reg00 == 0) {
    data = LCD_IO_ReadData(0x04, 3);
    lcdId = (uint16_t)(data & 0xFFFF);
    Serial3.print((data >> 16) & 0xFF, HEX);
    Serial3.print(" ");
  } else {
    lcdId = reg00;
  }
  Serial3.print((data >> 8) & 0xFF, HEX);
  Serial3.print(" ");
  Serial3.println(data & 0xFF, HEX);
  Serial3.flush();

  switch(lcdId) {
    case 0x1505: Serial3.println("LCD Controller: R61505U"); sprintf(controller, "R61505U"); break;
    case 0x8552: Serial3.println("LCD Controller: ST7789V"); sprintf(controller, "ST7789V"); break;
    case 0x8989: Serial3.println("LCD Controller: SSD1289"); sprintf(controller, "SSD1289"); break;
    case 0x9325: Serial3.println("LCD Controller: ILI9325"); sprintf(controller, "ILI9325"); break;
    case 0x9328: Serial3.println("LCD Controller: ILI9328"); sprintf(controller, "ILI9328"); break;
    case 0x9341: Serial3.println("LCD Controller: ILI9341"); sprintf(controller, "ILI9341"); break;
    case 0x0404: Serial3.println("No LCD Controller detected"); break;
    default: Serial3.print("LCD Controller: Unknown (0x"); Serial3.print(data & 0xFFFF, HEX); Serial3.println(")"); sprintf(controller, "Unknown"); break;
  }

  Serial3.flush();
  if (lcdId == 0x0404) return;

  lcdInit();

  color = YELLOW;
  bgColor = BLACK;
  lcdClear(bgColor);

  /**
   *  Calibrate touch screen
   */
  initTouch();

  Serial3.println("\nRaw ADC data:");
  for (i = 0; i < 4;) {
    lcdClear(bgColor);

  /**
   * Test coordinates and colors inversion.
   * Draw RED and GREEN squares in top left area of the screen.
   */
    lcdSetWindow(40, 20, 79, 59);
    for (j = 0 ; j < 20; j++) {
      lcdFill(RED, 20);
      lcdFill(GREEN, 20);
    }

    length = sprintf(text, "ControllerID:  %04X", lcdId);
    lcdPrint(160 - length * 4, 48, text);
    length = sprintf(text, "Controller: %s", controller);
    lcdPrint(160 - length * 4, 66, text);

    length = sprintf(text, "Touch calibration");
    lcdPrint(92, 88, text);

    switch (i) {
      case 0:
        drawCross( 20 , 20, 0xFFFF);
        length = sprintf(text, "Top Left");
        break;
      case 1:
        drawCross( 20, 219, 0xFFFF);
        length = sprintf(text, "Bottom Left");
        break;
      case 2:
        drawCross(299,  20, 0xFFFF);
        length = sprintf(text, "Top Right");
        break;
      case 3:
        drawCross(299, 219, 0xFFFF);
        length = sprintf(text, "Bottom Right");
        break;
    }

    lcdPrint(160 - length * 4, 108, text);

    waitForTouch(x + i, y + i);

    Serial3.print("X: ");
    Serial3.print(x[i]);
    Serial3.print("   Y: ");
    Serial3.println(y[i]);
    Serial3.flush();

    if ((x[i] < 409 || x[i] > 1637) && (y[i] < 409 || y[i] > 1637)) {
      switch (i) {
        case 0: // Top Left
          i++;
          waitForRelease();
          delay(300);
          continue;
        case 1: // Bottom Left
          if (((x[0] < 409 && x[1] < 409) || (x[0] > 1637 && x[1] > 1637)) && ((y[0] < 409 && y[1] > 1637) || (y[0] > 1637 && y[1] < 409))) {
            i++;
            waitForRelease();
            delay(300);
            continue;
          }
          break;
        case 2: // Top Right
          if (((x[0] < 409 && x[2] > 1637) || (x[0] > 1637 && x[2] < 409)) && ((y[0] < 409 && y[2] < 409) || (y[0] > 1637 && y[2] > 1637))) {
            i++;
            waitForRelease();
            delay(300);
            continue;
          }
          break;
        case 3: // Bottom Right
          if (((x[0] < 409 && x[3] > 1637) || (x[0] > 1637 && x[3] < 409)) && ((y[0] < 409 && y[3] > 1637) || (y[0] > 1637 && y[3] < 409))) {
            i++;
            waitForRelease();
            delay(300);
            continue;
          }
          break;
        }
    }
    lcdClear(RED);
    waitForRelease();
    delay(500);
   }

  lcdClear(bgColor);

  lcdSetWindow(40, 20, 79, 59);
  for (j = 0 ; j < 20; j++) {
    lcdFill(RED, 20);
    lcdFill(GREEN, 20);
  }

  length = sprintf(text, "ControllerID:  %04X", lcdId);
  lcdPrint(160 - length * 4, 48, text);
  length = sprintf(text, "Controller: %s", controller);
  lcdPrint(160 - length * 4, 66, text);

  length = sprintf(text, "Touch calibration completed");
  lcdPrint(160 - length * 4, 88, text);

  // 36569088L == ((int32_t)(299 - 20)) << 17
  xCalibration = (int16_t)(36569088L / ((int32_t)x[3] + (int32_t)x[2] - (int32_t)x[1] - (int32_t)x[0]));
  // 26083328L == ((int32_t)(219 - 20)) << 17
  yCalibration = (int16_t)(26083328L / ((int32_t)y[3] - (int32_t)y[2] + (int32_t)y[1] - (int32_t)y[0]));

  xOffset = (int16_t)(20 - ((((int32_t)(x[0] + x[1])) * (int32_t)xCalibration) >> 17));
  yOffset = (int16_t)(20 - ((((int32_t)(y[0] + y[2])) * (int32_t)yCalibration) >> 17));

  sprintf(text, "X_CALIBRATION:");
  lcdPrint(76, 108, text);
  sprintf(text, "%6d", xCalibration);
  color = xCalibration >= 0 ? GREEN : RED;
  lcdPrint(196, 108, text);
  color = YELLOW;

  sprintf(text, "Y_CALIBRATION:");
  lcdPrint(76, 124, text);
  sprintf(text, "%6d", yCalibration);
  color = yCalibration >= 0 ? GREEN : RED;
  lcdPrint(196, 124, text);
  color = YELLOW;

  sprintf(text, "X_OFFSET:");
  lcdPrint(76, 140, text);
  sprintf(text, "%6d", xOffset);
  color = xOffset >= 0 ? GREEN : RED;
  lcdPrint(196, 140, text);
  color = YELLOW;

  sprintf(text, "Y_OFFSET:");
  lcdPrint(76, 156, text);
  sprintf(text, "%6d", yOffset);
  color = yOffset >= 0 ? GREEN : RED;
  lcdPrint(196, 156, text);
  color = WHITE;

  x[0] = (uint16_t)((((int32_t)x[0] * (int32_t)xCalibration) >> 16) + xOffset);
  x[1] = (uint16_t)((((int32_t)x[1] * (int32_t)xCalibration) >> 16) + xOffset);
  x[2] = (uint16_t)((((int32_t)x[2] * (int32_t)xCalibration) >> 16) + xOffset);
  x[3] = (uint16_t)((((int32_t)x[3] * (int32_t)xCalibration) >> 16) + xOffset);
  y[0] = (uint16_t)((((int32_t)y[0] * (int32_t)yCalibration) >> 16) + yOffset);
  y[1] = (uint16_t)((((int32_t)y[1] * (int32_t)yCalibration) >> 16) + yOffset);
  y[2] = (uint16_t)((((int32_t)y[2] * (int32_t)yCalibration) >> 16) + yOffset);
  y[3] = (uint16_t)((((int32_t)y[3] * (int32_t)yCalibration) >> 16) + yOffset);

  Serial3.println("\nCalibrated coordinates:");
  Serial3.print("X: "); Serial3.print(x[0]); Serial3.print("   Y: "); Serial3.println(y[0]);
  Serial3.print("X: "); Serial3.print(x[1]); Serial3.print("   Y: "); Serial3.println(y[1]);
  Serial3.print("X: "); Serial3.print(x[2]); Serial3.print("   Y: "); Serial3.println(y[2]);
  Serial3.print("X: "); Serial3.print(x[3]); Serial3.print("   Y: "); Serial3.println(y[3]);
  Serial3.flush();

  while (!isTouched()) {};

  backlightTimeout = millis() + 60000;
}

void loop() {
  uint16_t x, y;

  if (ledTimeout < millis()) {
    digitalWrite(PB2, LOW);
    ledTimeout = millis() + 2000;
  } else if (ledTimeout < millis() + 1000) {
    digitalWrite(PB2, HIGH);
  }

  if (backlightTimeout < millis()) {
    LCD_BacklightOff();
  }

  if (!getTouchPoint(&x, &y)) return;

  if (backlightTimeout < millis()) {
    LCD_BacklightOn();
    waitForRelease();
    delay(200);
    backlightTimeout = millis() + 30000;
    return;
  } else {
    backlightTimeout = millis() + 30000;
  }

  x = (uint16_t)((((int32_t)x * (int32_t)xCalibration) >> 16) + xOffset);
  y = (uint16_t)((((int32_t)y * (int32_t)yCalibration) >> 16) + yOffset);

  lcdSetWindow(x, y);
  LCD_IO_WriteData(color);
}
