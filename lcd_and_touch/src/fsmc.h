#include "Arduino.h"
#include "libmaple/fsmc.h"
#include "libmaple/gpio.h"
#include "libmaple/dma.h"
#include "stdint.h"

#define __IO volatile
#define __ASM __asm
#define __STATIC_INLINE static inline

#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */

#define FSMC_DMA_DEV        DMA2
#define FSMC_DMA_CHANNEL    DMA_CH4

static inline uint32_t dma_get_count32(dma_dev *dev, dma_tube tube) { return dma_channel_regs(dev, tube)->CNDTR; }
static inline void dma_set_num_transfers32(dma_dev *dev, dma_channel channel, uint32_t num_transfers) { dma_channel_regs(dev, channel)->CNDTR = num_transfers; }

#define FSMC_D0   PD14
#define FSMC_D1   PD15
#define FSMC_D2   PD0
#define FSMC_D3   PD1
#define FSMC_D4   PE7
#define FSMC_D5   PE8
#define FSMC_D6   PE9
#define FSMC_D7   PE10
#define FSMC_D8   PE11
#define FSMC_D9   PE12
#define FSMC_D10  PE13
#define FSMC_D11  PE14
#define FSMC_D12  PE15
#define FSMC_D13  PD8
#define FSMC_D14  PD9
#define FSMC_D15  PD10
#define FSMC_NOE  PD4
#define FSMC_NWE  PD5

#define FSMC_NE4  PG12
#define FSMC_A0   PF0

#define LCD_BACKLIGHT_PIN   PG11
#define LCD_RESET_PIN       PF6

#define LCD       ((LCD_CONTROLLER_TypeDef *) (0x6C000000 | 0x00000000)) // FSMC_NE4 FSMC_A0

typedef struct {
  __IO uint16_t REG;
  __IO uint16_t RAM;
} LCD_CONTROLLER_TypeDef;

/* Timing configuration */
#define FSMC_ADDRESS_SETUP_TIME   15  // AddressSetupTime
#define FSMC_DATA_SETUP_TIME      15  // DataSetupTime

__attribute__((always_inline)) __STATIC_INLINE void __DSB(void) {
  __ASM volatile ("dsb 0xF":::"memory");
}

void LCD_IO_Init(void);
void LCD_IO_WriteData(uint16_t RegValue);
void LCD_IO_WriteReg(uint16_t Reg);
uint32_t LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize);
uint16_t LCD_IO_ReadData(uint16_t RegValue);
uint16_t LCD_IO_ReadData(void);
void LCD_Delay (uint32_t milliseconds);

void LCD_BacklightOn(void);
void LCD_BacklightOff(void);
