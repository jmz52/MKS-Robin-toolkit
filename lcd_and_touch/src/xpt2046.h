#include <stdint.h>

#define XPT2046_DFR_MODE        0x00
#define XPT2046_SER_MODE        0x04
#define XPT2046_CONTROL         0x80

#define XPT2046_X               0x10
#define XPT2046_Z1              0x30
#define XPT2046_Z2              0x40
#define XPT2046_Y               0x50

#define XPT2046_SPI_CLOCK       SPI_CLOCK_DIV2

#define XPT2046_Z1_TRESHHOLD    10
#define XPT2046_X_CALIBRATION   11993
#define XPT2046_X_OFFSET        -32
#define XPT2046_Y_CALIBRATION   -8786
#define XPT2046_Y_OFFSET        256

void initTouch(void);
uint16_t getTouchCoordinate(uint8_t coordinate);
bool getTouchPoint(uint16_t *x, uint16_t *y);

inline bool isTouched() { return getTouchCoordinate(XPT2046_Z1) >= XPT2046_Z1_TRESHHOLD; }
inline void waitForRelease(void) { while (isTouched()) {}; }
inline void waitForTouch(uint16_t *x, uint16_t *y) { while(!getTouchPoint(x, y)) {}; }
