#ifndef __ST7789_H
#define __ST7789_H
#include "main.h"
#include "spi.h"

#define WIDTH		240
#define HEIGHT		240

#define ST7789_WIDTH ((uint16_t)240)
#define ST7789_HEIGHT ((uint16_t)240)

#define BPP		16
#define LCD_MAX_MEM16_BLOCK             (1 << 6)
#define LCD_PIXEL_PER_BLOCK             (LCD_MAX_MEM16_BLOCK >> 1)

/* Init script function */
struct st7789_function
{
    uint8_t cmd;
    uint16_t data;
};

/* Init script commands */
enum st7789_cmd
{
    ST7789_START,
    ST7789_END,
    ST7789_CMD,
    ST7789_DATA,
    ST7789_DELAY
};

/* ST7789 Commands */
#define ST7789_CASET	0x2A
#define ST7789_RASET	0x2B
#define ST7789_RAMWR	0x2C
#define ST7789_RAMRD	0x2E

//LCD初始化
int st7789_init(void);
//写寄存器
void LcdWriteReg(uint8_t Data);
//写数据
void LcdWriteData(uint8_t Data);
//将多个值写入显示寄存器
void LcdWriteDataMultiple(uint8_t * pData, int NumItems);
void st7789_display_picture(void);

void ST7789H2_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGBCode);
void ST7789H2_WriteLine(uint16_t Xpos, uint16_t Ypos, uint16_t *RGBCode, uint16_t pointNum);

HAL_StatusTypeDef st7789_write(int is_cmd, uint8_t data);
HAL_StatusTypeDef st7789_write_fb(uint16_t *data, uint16_t size);
#endif //__ST7789_H
