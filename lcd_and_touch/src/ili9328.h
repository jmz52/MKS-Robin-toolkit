#include <stdint.h>

/*
0x0020  Horizontal GRAM Address Set (0-255)
0x0021  Vertical GRAM Address Set (0-511)
0x0022  Write Data to GRAM

0x0050  Horizontal Address Start (0-255)
0x0051  Horizontal Address End Position (0-255)
0x0052  Vertical Address Start Position (0-511)
0x0053  Vertical Address End Position (0-511)
*/

#define ILI9328_WRITE_RAM          0x0022

void ili9328_Init(void);
void ili9328_DisplayOn(void);
void ili9328_WriteRam(void);
void ili9328_SetCursor(uint16_t Xpos, uint16_t Ypos);
void ili9328_SetWindow(uint16_t Xmin, uint16_t Ymin, uint16_t XMax = 319, uint16_t Ymax = 239);

void LCD_IO_Init(void);
void LCD_IO_WriteData(uint16_t RegValue);
void LCD_IO_WriteReg(uint16_t Reg);
uint32_t LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize);
uint16_t LCD_IO_ReadData(uint16_t RegValue);
uint16_t LCD_IO_ReadData(void);
void LCD_Delay (uint32_t milliseconds);
