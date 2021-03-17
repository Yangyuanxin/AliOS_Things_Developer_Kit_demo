#include "lcd.h"

static SPI_HandleTypeDef *hspi_lcd = NULL;
uint8_t black_gui[480] = {0};
uint8_t endian_buffer[480];

static struct st7789_function st7789_cfg_script[] = {
  {ST7789_START, ST7789_START},
  {ST7789_CMD, 0x11},
  {ST7789_DELAY, 120},
  {ST7789_CMD, 0x36},
  {ST7789_DATA, 0x00},
  {ST7789_CMD, 0x3a},
  {ST7789_DATA, 0x05},
  {ST7789_CMD, 0xb2},
  {ST7789_DATA, 0x0c},
  {ST7789_DATA, 0x0c},
  {ST7789_DATA, 0x00},
  {ST7789_DATA, 0x33},
  {ST7789_DATA, 0x33},
  {ST7789_CMD, 0xb7},
  {ST7789_DATA, 0x72},
  {ST7789_CMD, 0xbb},
  {ST7789_DATA, 0x3d},
  {ST7789_CMD, 0xc2},
  {ST7789_DATA, 0x01},
  {ST7789_CMD, 0xc3},
  {ST7789_DATA, 0x19},
  {ST7789_CMD, 0xc4},
  {ST7789_DATA, 0x20},
  {ST7789_CMD, 0xc6},
  {ST7789_DATA, 0x0f},
  {ST7789_CMD, 0xd0},
  {ST7789_DATA, 0xa4},
  {ST7789_DATA, 0xa1},
  {ST7789_CMD, 0xe0},
  {ST7789_DATA, 0x70},
  {ST7789_DATA, 0x04},
  {ST7789_DATA, 0x08},
  {ST7789_DATA, 0x09},
  {ST7789_DATA, 0x09},
  {ST7789_DATA, 0x05},
  {ST7789_DATA, 0x2a},
  {ST7789_DATA, 0x33},
  {ST7789_DATA, 0x41},
  {ST7789_DATA, 0x07},
  {ST7789_DATA, 0x13},
  {ST7789_DATA, 0x13},
  {ST7789_DATA, 0x29},
  {ST7789_DATA, 0x2f},
  {ST7789_CMD, 0xe1},
  {ST7789_DATA, 0x70},
  {ST7789_DATA, 0x03},
  {ST7789_DATA, 0x09},
  {ST7789_DATA, 0x0a},
  {ST7789_DATA, 0x09},
  {ST7789_DATA, 0x06},
  {ST7789_DATA, 0x2b},
  {ST7789_DATA, 0x34},
  {ST7789_DATA, 0x41},
  {ST7789_DATA, 0x07},
  {ST7789_DATA, 0x12},
  {ST7789_DATA, 0x14},
  {ST7789_DATA, 0x28},
  {ST7789_DATA, 0x2e},
  {ST7789_CMD, 0x21},
  {ST7789_CMD, 0x29},
  {ST7789_CMD, 0x2a},
  {ST7789_DATA, 0x00},
  {ST7789_DATA, 0x00},
  {ST7789_DATA, 0x00},
  {ST7789_DATA, 0xef},
  {ST7789_CMD, 0x2b},
  {ST7789_DATA, 0x00},
  {ST7789_DATA, 0x00},
  {ST7789_DATA, 0x00},
  {ST7789_DATA, 0xef},
  {ST7789_CMD, 0x2c},
  {ST7789_END, ST7789_END},
};

//ST7789复位
static void st7789_reset(void)
{    
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET); 
  HAL_Delay(1);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);  
  HAL_Delay(120);
}

//打开LCD电源
void st7789_power_on(void)
{
    HAL_GPIO_WritePin(LCD_POWER_GPIO_Port, LCD_POWER_Pin, GPIO_PIN_SET); 
}

//ST7789写函数
static HAL_StatusTypeDef st7789_write(int is_cmd, uint8_t data)
{
  uint8_t pData[2] = {0};
  assert_param(NULL != hspi_lcd);
  pData[0] = data;
	if (is_cmd)
        HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);  
	else
		HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);  

  return HAL_SPI_Transmit(hspi_lcd, pData, 1, HAL_MAX_DELAY);
}


static HAL_StatusTypeDef st7789_write_fb(uint16_t *data, uint16_t size)
{
  assert_param(NULL != hspi_lcd);
  return HAL_SPI_Transmit(hspi_lcd, (uint8_t *)data, size, HAL_MAX_DELAY);
}

/********************************************************************
*
*       LcdWriteReg
*
* Function description:
*   Sets display register
*/
void LcdWriteReg_St7789(uint8_t Data) 
{
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);  
  HAL_SPI_Transmit(&hspi1, &Data, 1, 10);
}

/********************************************************************
*
*       LcdWriteData
*
* Function description:
*   Writes a value to a display register
*/
void LcdWriteData_St7789(uint8_t Data) 
{
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);    
  HAL_SPI_Transmit(&hspi1, &Data, 1, 10);
}

/********************************************************************
*
*       LcdWriteDataMultiple
*
* Function description:
*   Writes multiple values to a display register.
*/
void LcdWriteDataMultiple(uint8_t * pData, int NumItems) 
{
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
  HAL_SPI_Transmit(&hspi1, pData, NumItems, 10);
}

//配置启动参数
static void st7789_run_cfg_script(void)
{
  uint8_t data[2] = {0};
  int i = 0;
  int end_script = 0;

  do {
    switch (st7789_cfg_script[i].cmd) {
    case ST7789_START:
      break;
    case ST7789_CMD:
      data[0] = st7789_cfg_script[i].data & 0xff;
      st7789_write(1, data[0]);
      break;
    case ST7789_DATA:
      data[0] = st7789_cfg_script[i].data & 0xff;
      st7789_write(0, data[0]);
      break;
    case ST7789_DELAY:
      HAL_Delay(120);
      break;
    case ST7789_END:
      end_script = 1;
    }
    i++;
  } while (!end_script);
}


/**
 * @brief	设置数据写入LCD缓存区域
 * @param   x1,y1	—— 起点坐标
 * @param   x2,y2	—— 终点坐标
 * @return  none
 */
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    LcdWriteReg_St7789(0x2a);
    LcdWriteData_St7789(x1 >> 8);
    LcdWriteData_St7789(x1);
    LcdWriteData_St7789(x2 >> 8);
    LcdWriteData_St7789(x2);

    LcdWriteReg_St7789(0x2b);
    LcdWriteData_St7789(y1 >> 8);
    LcdWriteData_St7789(y1);
    LcdWriteData_St7789(y2 >> 8);
    LcdWriteData_St7789(y2);

    LcdWriteReg_St7789(0x2C);
}

/**
* @brief		SPI2 发送一个字节
* @param   TxData	要发送的数据
* @param   size	发送数据的字节大小
 * @return  uint8_t		0:写入成功,其他:写入失败
 */
uint8_t SPI2_WriteByte(uint8_t *TxData, uint16_t size)
{
    return HAL_SPI_Transmit(&hspi1, TxData, size, 1000);
    //return HAL_SPI_Transmit_DMA(&hspi2, TxData, size);
}

/**
 * @brief	LCD底层SPI发送数据函数
 * @param   data —— 数据的起始地址
 * @param   size —— 发送数据字节数
 * @return  none
 */
static void LCD_SPI_Send(uint8_t *data, uint16_t size)
{
    SPI2_WriteByte(data, size);
}

/**
 * @brief	显示图片函数
 * @param   x,y	    —— 起点坐标
 * @param   width	—— 图片宽度
 * @param   height	—— 图片高度
 * @param   p       —— 图片缓存数据起始地址
 * @return  none
 * @note	Image2Lcd取模方式：C语言数据/水平扫描/16位真彩色(RGB565)/高位在前，其他的不选
 */
void LCD_Show_Image(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *p)
{
	uint32_t img_size = width * height * 2;		//图片所占字节数
	uint32_t remain_size = img_size;		    //图片每次发送后剩余的字节数
	uint8_t i = 0;
	
	/* 错误检测 */
    if(x + width > 240 || y + height > 240)
    {
        return;
    }
				
    LCD_Address_Set(x, y, x + width - 1, y + height - 1);

    //LCD_WR_RS(1);
		HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); 

    /* SPI每次最大发送2^16 = 65536个数据,图片最大大小为240*240*2 = 115200，会超过此大小，所以设计循环发送算法 */
    for(i = 0;i <= img_size / 65536; i++)
    {
        if(remain_size / 65536 >= 1)
        {
            LCD_SPI_Send((uint8_t *)p, 65535);
            p += 65535;
            remain_size -= 65535;
        }
        else
        {
            LCD_SPI_Send((uint8_t *)p, remain_size % 65535);
        }
            
    }  
}

//发送到显示区域
static void spec_send_fb(uint16_t color, uint16_t pixel_num)
{
  int i;
  int count, remain;
  uint16_t real_mem[LCD_MAX_MEM16_BLOCK] = {0};

  for (i = 0; i < LCD_MAX_MEM16_BLOCK; ++i) {
    real_mem[i] = color;
  }
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
  if (pixel_num <= LCD_MAX_MEM16_BLOCK) {
    st7789_write_fb(real_mem, pixel_num << 1);
  } else {
    count = pixel_num / LCD_MAX_MEM16_BLOCK;
    remain = pixel_num % LCD_MAX_MEM16_BLOCK;
    for (i = 0; i < count; ++i) {
      st7789_write_fb(real_mem, LCD_MAX_MEM16_BLOCK << 1);
    }
    st7789_write_fb(real_mem, remain << 1);
  }
}

//写寄存器
void ST7789H2_WriteReg(uint8_t Command, uint8_t *Parameters, uint8_t NbParameters)
{
  uint8_t   i;

  /* Send command */
  LcdWriteReg_St7789(Command);
  
  /* Send command's parameters if any */
  for (i=0; i<NbParameters; i++)
  {
    LcdWriteData_St7789(Parameters[i]);
  }
}

//在指定位置设置光标
void ST7789H2_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
  uint8_t   parameter[4];
  /* CASET: Comumn Addrses Set */
  parameter[0] = 0x00;     
  parameter[1] = 0x00 + Xpos;
  parameter[2] = 0x00;
  parameter[3] = 0xEF + Xpos;
  ST7789H2_WriteReg(0x2A, parameter, 4);
  /* RASET: Row Addrses Set */  
  parameter[0] = 0x00;
  parameter[1] = 0x00 + Ypos;
  parameter[2] = 0x00;
  parameter[3] = 0xEF + Ypos;
  ST7789H2_WriteReg(0x2B, parameter, 4);
}

//清屏
void BSP_LCD_Clear(void)
{
  uint32_t counter = 0;
	memset(black_gui, 0x00, sizeof(black_gui));
  for (counter = 0; counter < 240; counter++)
  {
		/* Set Cursor */
		ST7789H2_SetCursor(0, counter); 
		
		/* Prepare to write to LCD RAM */
		ST7789H2_WriteReg(0x2C, (uint8_t*)NULL, 0);   /* RAM write data command */
		
		LcdWriteDataMultiple(black_gui, 480);	 
  }
}

//往指定区域写一个像素
void ST7789H2_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t data)
{
  uint8_t dataB = 0;

  /* Set Cursor */
  ST7789H2_SetCursor(Xpos, Ypos);

  /* Prepare to write to LCD RAM */
  ST7789H2_WriteReg(0x2C, (uint8_t*)NULL, 0);   /* RAM write data command */

  /* Write RAM data */
  dataB = (uint8_t)(data >> 8);
  LcdWriteData_St7789(dataB);
  dataB = (uint8_t)data;
  LcdWriteData_St7789(dataB);
}

//往指定区域写一行数据
void ST7789H2_WriteLine(uint16_t Xpos, uint16_t Ypos, uint16_t *RGBCode, uint16_t pointNum)
{
  int i = 0;

  /* Set Cursor */
  ST7789H2_SetCursor(Xpos, Ypos);

  /* Prepare to write to LCD RAM */
  ST7789H2_WriteReg(0x2C, (uint8_t*)NULL, 0);   /* RAM write data command */

  for (i = 0; i < pointNum; i++) {
    endian_buffer[2*i] = (uint8_t)(RGBCode[i] >> 8);
    endian_buffer[2*i + 1] = (uint8_t)RGBCode[i];
  }

  /* Write RAM data */
  LcdWriteDataMultiple(endian_buffer, pointNum*2);
}




//ST7789初始化
int st7789_init(void)
{
  hspi_lcd = &hspi1;
  st7789_reset();
  st7789_run_cfg_script();
	st7789_power_on();
  return HAL_OK;
}
