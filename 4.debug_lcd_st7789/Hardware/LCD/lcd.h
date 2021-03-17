#ifndef __LCD_H
#define __LCD_H
#include "main.h"
#include "spi.h"
#include <string.h>
#include <stdlib.h>

#define WIDTH		240
#define HEIGHT		240
#define BPP		16
#define LCD_MAX_MEM16_BLOCK             (1 << 6)
#define LCD_PIXEL_PER_BLOCK             (LCD_MAX_MEM16_BLOCK >> 1)

/* Init script function */
struct st7789_function {
  uint8_t cmd;
  uint16_t data;
};

/* Init script commands */
enum st7789_cmd {
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
//打开LCD电源
void st7789_power_on(void);
//写寄存器
void LcdWriteReg_St7789(uint8_t Data);
//写数据
void LcdWriteData_St7789(uint8_t Data);
//将多个值写入显示寄存器
void LcdWriteDataMultiple_St7789(uint8_t * pData, int NumItems);
//写一个像素到指定位置
void ST7789H2_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGBCode);
//写一行数据	
void ST7789H2_WriteLine(uint16_t Xpos, uint16_t Ypos, uint16_t *RGBCode, uint16_t pointNum);
//显示图片
void st7789_display_picture(void);
#endif //__ST7789_H

