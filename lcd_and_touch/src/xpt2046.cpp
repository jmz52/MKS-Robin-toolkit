#include <Arduino.h>
#include <SPI.h>

#include "xpt2046.h"

static SPISettings spiConfig;

void initTouch(void) {
  pinMode(PB1, OUTPUT);
  digitalWrite(PB1, HIGH);

  SPI.setModule(2);
  spiConfig = SPISettings(XPT2046_SPI_CLOCK, MSBFIRST, SPI_MODE0);
}

uint16_t getTouchCoordinate(uint8_t coordinate) {
    coordinate |= XPT2046_CONTROL | XPT2046_DFR_MODE;

    digitalWrite(PB1, LOW);
    SPI.beginTransaction(spiConfig);

    uint16_t data[3], delta[3];

    SPI.transfer(coordinate);
    for (uint32_t i = 0; i < 3 ; i++) {
      data[i] = ((uint16_t) SPI.transfer(0x00)) << 4;
      data[i] |= ((uint16_t) SPI.transfer(coordinate)) >> 4;
    }
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x00);

    SPI.endTransaction();
    digitalWrite(PB1, HIGH);

    delta[0] = data[0] > data[1] ? data[0] - data [1] : data[1] - data [0];
    delta[1] = data[0] > data[2] ? data[0] - data [2] : data[2] - data [0];
    delta[2] = data[1] > data[2] ? data[1] - data [2] : data[2] - data [1];

    if (delta[0] <= delta[1] && delta[0] <= delta[2]) return (data[0] + data [1]) >> 1;
    if (delta[1] <= delta[2]) return (data[0] + data [2]) >> 1;
    return (data[1] + data [2]) >> 1;
}

bool getTouchPoint(uint16_t *x, uint16_t *y) {
  if (!isTouched()) return false;
  *x = getTouchCoordinate(XPT2046_X);
  *y = getTouchCoordinate(XPT2046_Y);
  if (!isTouched()) return false;
  return true;
}
