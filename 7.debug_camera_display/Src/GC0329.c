#include "GC0329.h"

CAMERA_DrvTypeDef *camera_drv;
DCMI_HandleTypeDef *phdcmi;
uint16_t *camera_buff = NULL;
static void(*pCapFunc)() = 0;

CAMERA_DrvTypeDef   gc0329_drv =
{
    gc0329_Init,
    gc0329_ReadID,
    gc0329_Config,
};



/* Initialization sequence for VGA resolution (640x480)*/
const unsigned char GC0329_VGA[][2] =
{
    {0xfe, 0x80},
    {0xfc, 0x16},
    {0xfc, 0x16},
    {0xfe, 0x00},


    {0xfa, 0x00},

    {0x70, 0x48},
    {0x73, 0x90},
    {0x74, 0x80},
    {0x75, 0x80},
    {0x76, 0x94},  //80 jambo
    {0x77, 0x62},
    {0x78, 0x47},
    {0x79, 0x40},

    {0x03, 0x02},
    {0x04, 0x40},


    ////////////////////analog////////////////////
    {0xfc, 0x16},
    {0x09, 0x00},
    {0x0a, 0x02},
    {0x0b, 0x00},
    {0x0c, 0x02},
    #ifndef CAMERA_GRAY_MIRROR
    {0x17, 0x15},  //0x14
    #endif
    {0x19, 0x05},
    {0x1b, 0x24},
    {0x1c, 0x04},
    {0x1e, 0x08},
    {0x1f, 0x08},  //C8
    {0x20, 0x01},
    {0x21, 0x48},
    {0x22, 0xba},
    {0x23, 0x22},
    {0x24, 0x16},


    ////////////////////blk////////////////////
    ////added for MID
    {0x26, 0xf4},  //BLK
    {0x2a, 0x2c},
    {0x2b, 0x2c},
    {0x2c, 0x2c},
    {0x2d, 0x2c},


    {0x26, 0xf7},  //BLK
    {0x28, 0x7f},  //BLK limit
    {0x29, 0x00},
    {0x32, 0x00},  //04 darkc
    {0x33, 0x20},  //blk ratio
    {0x34, 0x20},
    {0x35, 0x20},
    {0x36, 0x20},

    {0x3b, 0x04},  //manual offset
    {0x3c, 0x04},
    {0x3d, 0x04},
    {0x3e, 0x04},

    ////////////////////ISP BLOCK ENABLE////////////////////
    {0x40, 0xff},
    {0x41, 0x24}, //[5]skin detection
    {0x42, 0xfa}, //disable ABS
    {0x46, 0x03},
    {0x4b, 0xca},
    {0x4d, 0x01},
    {0x4f, 0x01},
    {0x70, 0x48},

    ////////////////////DNDD////////////////////
    {0x80, 0x07},  // 0xe7 20140915
    {0x81, 0xc2},  // 0x22 20140915
    {0x82, 0x90},  //DN auto DNDD DEC DNDD //0e //55 jambo
    {0x83, 0x05},
    {0x87, 0x40},  // 0x4a  20140915

    ////////////////////INTPEE////////////////////
    {0x90, 0x8c},  //ac
    {0x92, 0x05},
    {0x94, 0x05},
    {0x95, 0x45},  //0x44
    {0x96, 0x88},

    ////////////////////ASDE////////////////////
    {0xfe, 0x01},
    {0x18, 0x22},
    {0xfe, 0x00},
    {0x9c, 0x0a},
    {0xa0, 0xaf},
    {0xa2, 0xff},
    {0xa4, 0x30},  //50 jambo
    {0xa5, 0x31},
    {0xa7, 0x35},

    ////////////////////RGB gamma////////////////////
    {0xfe, 0x00},
    {0xbf, 0x0b},
    {0xc0, 0x1d},
    {0xc1, 0x33},
    {0xc2, 0x49},
    {0xc3, 0x5d},
    {0xc4, 0x6e},
    {0xc5, 0x7c},
    {0xc6, 0x99},
    {0xc7, 0xaf},
    {0xc8, 0xc2},
    {0xc9, 0xd0},
    {0xca, 0xda},
    {0xcb, 0xe2},
    {0xcc, 0xe7},
    {0xcd, 0xf0},
    {0xce, 0xf7},
    {0xcf, 0xff},



    ////////////////////Y gamma////////////////////
    {0xfe, 0x00},
    {0x63, 0x00},
    {0x64, 0x06},
    {0x65, 0x0d},
    {0x66, 0x1b},
    {0x67, 0x2b},
    {0x68, 0x3d},
    {0x69, 0x50},
    {0x6a, 0x60},
    {0x6b, 0x80},
    {0x6c, 0xa0},
    {0x6d, 0xc0},
    {0x6e, 0xe0},
    {0x6f, 0xff},


    //////////////////CC///////////////////
    #if 1 //main
    {0xfe, 0x00},
    {0xb3, 0x44},
    {0xb4, 0xfd},
    {0xb5, 0x02},
    {0xb6, 0xfa},
    {0xb7, 0x48},
    {0xb8, 0xf0},
    #else //sub
    {0xfe, 0x00},
    {0xb3, 0x42}, //40
    {0xb4, 0xff}, //00
    {0xb5, 0x06}, //06
    {0xb6, 0xf0}, //00
    {0xb7, 0x44}, //40
    {0xb8, 0xf0}, //00
    #endif

    // crop
    {0x50, 0x01},


    ////////////////////YCP////////////////////
    {0xfe, 0x00},

    {0xd0, 0x40},
    {0xd1, 0x28},
    {0xd2, 0x28},

    {0xd3, 0x40},  //cont 0x40
    {0xd5, 0x00},

    {0xdd, 0x14},
    {0xde, 0x34},

    ////////////////////AEC////////////////////
    {0xfe, 0x01},
    {0x10, 0x40},  // before Gamma
    {0x11, 0x21},  //
    {0x12, 0x13},	// center weight *2
    {0x13, 0x50},  //4 //4}, // AE Target
    #ifdef CAMERA_GRAY_MIRROR
    {0x17, 0xa9},	//88, 08, c8, a8
    #else
    {0x17, 0xa8},	//88, 08, c8, a8
    #endif
    {0x1a, 0x21},
    {0x20, 0x31},	//AEC stop margin
    {0x21, 0xc0},
    {0x22, 0x60},
    {0x3c, 0x50},
    {0x3d, 0x40},
    {0x3e, 0x45},  //read 3f for status

//main
    {0xfe, 0x01},
    {0x06, 0x12},
    {0x07, 0x06},
    {0x08, 0x9c},
    {0x09, 0xee},
    {0x50, 0xfc},
    {0x51, 0x28},
    {0x52, 0x10},
    {0x53, 0x20},
    {0x54, 0x12},
    {0x55, 0x16},
    {0x56, 0x30},
    {0x58, 0x60},
    {0x59, 0x08},
    {0x5a, 0x02},
    {0x5b, 0x63},
    {0x5c, 0x35},
    {0x5d, 0x72},
    {0x5e, 0x11},
    {0x5f, 0x40},
    {0x60, 0x40},
    {0x61, 0xc8},
    {0x62, 0xa0},
    {0x63, 0x40},
    {0x64, 0x50},
    {0x65, 0x98},
    {0x66, 0xfa},
    {0x67, 0x80},
    {0x68, 0x60},
    {0x69, 0x90},
    {0x6a, 0x40},
    {0x6b, 0x39},
    {0x6c, 0x30},
    {0x6d, 0x60},
    {0x6e, 0x41},
    {0x70, 0x10},
    {0x71, 0x00},
    {0x72, 0x10},
    {0x73, 0x40},
    {0x80, 0x60},
    {0x81, 0x50},
    {0x82, 0x42},
    {0x83, 0x40},
    {0x84, 0x40},
    {0x85, 0x40},
    {0x74, 0x40},
    {0x75, 0x58},
    {0x76, 0x24},
    {0x77, 0x40},
    {0x78, 0x20},
    {0x79, 0x60},
    {0x7a, 0x58},
    {0x7b, 0x20},
    {0x7c, 0x30},
    {0x7d, 0x35},
    {0x7e, 0x10},
    {0x7f, 0x08},

    ///////////////////ABS///////////////////////
    {0x9c, 0x00},
    {0x9e, 0xc0},
    {0x9f, 0x40},

    ////////////////////CC-AWB////////////////////
    {0xd0, 0x00},
    {0xd2, 0x2c},
    {0xd3, 0x80},

    ///////////////////LSC //////////////////////

    {0xfe, 0x01},
    {0xc0, 0x0b},
    {0xc1, 0x07},
    {0xc2, 0x05},
    {0xc6, 0x0b},
    {0xc7, 0x07},
    {0xc8, 0x05},
    {0xba, 0x39},
    {0xbb, 0x24},
    {0xbc, 0x23},
    {0xb4, 0x39},
    {0xb5, 0x24},
    {0xb6, 0x23},
    {0xc3, 0x00},
    {0xc4, 0x00},
    {0xc5, 0x00},
    {0xc9, 0x00},
    {0xca, 0x00},
    {0xcb, 0x00},
    {0xbd, 0x2b},
    {0xbe, 0x00},
    {0xbf, 0x00},
    {0xb7, 0x09},
    {0xb8, 0x00},
    {0xb9, 0x00},
    {0xa8, 0x31},
    {0xa9, 0x23},
    {0xaa, 0x20},
    {0xab, 0x31},
    {0xac, 0x23},
    {0xad, 0x20},
    {0xae, 0x31},
    {0xaf, 0x23},
    {0xb0, 0x20},
    {0xb1, 0x31},
    {0xb2, 0x23},
    {0xb3, 0x20},
    {0xa4, 0x00},
    {0xa5, 0x00},
    {0xa6, 0x00},
    {0xa7, 0x00},
    {0xa1, 0x3c},
    {0xa2, 0x50},
    {0xfe, 0x00},

    //////////////////// flicker ///////////////////

    {0x05, 0x02},
    {0x06, 0x2c},
    {0x07, 0x00},
    {0x08, 0xb8},

    {0xfe, 0x01},
    {0x29, 0x00},	//anti-flicker step [11:8]
    {0x2a, 0x60},	//anti-flicker step [7:0]

    {0x2b, 0x02},	//exp level 0  14.28fps
    {0x2c, 0xa0},
    {0x2d, 0x03},	//exp level 1  12.50fps
    {0x2e, 0x00},
    {0x2f, 0x03},	//exp level 2  10.00fps
    {0x30, 0xc0},
    {0x31, 0x05},	//exp level 3  7.14fps
    {0x32, 0x40},
    {0x33, 0x20},
    {0xfe, 0x00},

////////////////////out ///////////////////
    #ifdef CAMERA_GRAY_MIRROR
    {0x44, 0xB1},
    #else
    {0x44, 0xA6},
    #endif
    {0xf0, 0x07},
    {0xf1, 0x01},
};

/* Initialization sequence for QVGA resolution (320x240) */
const unsigned char GC0329_QVGA[][2] =
{
    {0xff, 0xff},
};

/* Initialization sequence for QQVGA resolution (160x120) */
const char GC0329_QQVGA[][2] =
{
    {0xff, 0xff},
};

void gc0329_mclk_onoff(int on)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /*xiehj add RCC_OSCILLATORTYPE_HSI48*/
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;

    /*xiehj add*/
    if(on)
        RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    else
        RCC_OscInitStruct.HSI48State = RCC_HSI48_OFF;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {

    }
}

void gc0329_power_onoff(int on)
{
    if(1 == on)
    {
        HAL_GPIO_WritePin(CAM_PD_GPIO_Port, CAM_PD_Pin, GPIO_PIN_RESET);
        HAL_Delay(10);
		HAL_GPIO_WritePin(CAM_RST_GPIO_Port, CAM_RST_Pin, GPIO_PIN_RESET);
		HAL_Delay(50);
	  	HAL_GPIO_WritePin(CAM_RST_GPIO_Port, CAM_RST_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(CAM_RST_GPIO_Port, CAM_RST_Pin, GPIO_PIN_RESET);
		HAL_Delay(50);
		HAL_GPIO_WritePin(CAM_PD_GPIO_Port, CAM_PD_Pin, GPIO_PIN_SET);
    }
}

uint8_t CAMERA_IO_Read(uint8_t DevAddr, uint8_t Reg)
{
    uint8_t read_value = 0;
    uint8_t status ;
    status = HAL_I2C_Mem_Read(&hi2c3, DevAddr, Reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&read_value, 1, 1000);
    if(HAL_OK != status)
    {
        return read_value ;
    }
    return read_value;
}

uint8_t CAMERA_IO_Write(uint8_t DevAddr, uint8_t Reg, uint8_t Value)
{
    uint8_t status ;
    status = HAL_I2C_Mem_Write(&hi2c3, DevAddr, Reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&Value, 1, 1000) ;
    if(HAL_OK != status)
    {
        return  HAL_ERROR;
    }
    return HAL_OK ;
}

uint16_t gc0329_ReadID(uint16_t dummy)
{  
  /* Get the camera ID */
 // CAMERA_IO_Write(0xfe, 0x00);
  CAMERA_IO_Write(GC0329_I2CADDR,0xfc, 0x16);
  return (CAMERA_IO_Read(GC0329_I2CADDR, GC0329_CHIPID));
}

void gc0329_Init(uint16_t DeviceAddr, uint32_t resolution)
{
    uint32_t index;

    /* Initialize GC0329 */
    switch (resolution)
    {
        case CAMERA_R160x120:
        {
            for(index = 0; index < (sizeof(GC0329_QQVGA) / 2); index++)
            {
                CAMERA_IO_Write(DeviceAddr, GC0329_QQVGA[index][0], GC0329_QQVGA[index][1]);
                //CAMERA_Delay(2);
                //krhino_task_sleep(krhino_ms_to_ticks(2));
                HAL_Delay(2);
            }

            break;
        }

        case CAMERA_R320x240:
        {
            for(index = 0; index < (sizeof(GC0329_QVGA) / 2); index++)
            {
                CAMERA_IO_Write(DeviceAddr, GC0329_QVGA[index][0], GC0329_QVGA[index][1]);
                //CAMERA_Delay(2);
                //krhino_task_sleep(krhino_ms_to_ticks(2));
                HAL_Delay(2);
            }

            break;
        }

        case CAMERA_R480x272:
        {
            /* Not supported resolution */
            break;
        }

        case CAMERA_R640x480:
        {
            for(index = 0; index < (sizeof(GC0329_VGA) / 2); index++)
            {
                CAMERA_IO_Write(DeviceAddr, GC0329_VGA[index][0], GC0329_VGA[index][1]);
                //CAMERA_Delay(2);
                //krhino_task_sleep(krhino_ms_to_ticks(2));
                HAL_Delay(2);
            }

            break;
        }

        default:
        {
            break;
        }
    }
}

void gc0329_Config(uint16_t DeviceAddr, uint32_t feature, uint32_t value, uint32_t brightness_value)
{
    /* Convert the input value into gc0329 parameters */
    gc0329_ConvertValue(feature, value);
    gc0329_ConvertValue(CAMERA_CONTRAST_BRIGHTNESS, brightness_value);
}

uint64_t gc0329_ConvertValue(uint32_t feature, uint32_t value)
{
//    uint64_t ret = 0;
    return 0;
		#if 0
    switch(feature)
    {
        case CAMERA_BLACK_WHITE:
        {
            switch(value)
            {
                case CAMERA_BLACK_WHITE_BW:
                {
                    ret =  GC0329_BLACK_WHITE_BW;
                    break;
                }

                case CAMERA_BLACK_WHITE_NEGATIVE:
                {
                    ret =  GC0329_BLACK_WHITE_NEGATIVE;
                    break;
                }

                case CAMERA_BLACK_WHITE_BW_NEGATIVE:
                {
                    ret =  GC0329_BLACK_WHITE_BW_NEGATIVE;
                    break;
                }

                case CAMERA_BLACK_WHITE_NORMAL:
                {
                    ret =  GC0329_BLACK_WHITE_NORMAL;
                    break;
                }

                default:
                {
                    ret =  GC0329_BLACK_WHITE_NORMAL;
                    break;
                }
            }

            break;
        }

        case CAMERA_CONTRAST_BRIGHTNESS:
        {
            switch(value)
            {
                case CAMERA_BRIGHTNESS_LEVEL0:
                {
                    ret =  GC0329_BRIGHTNESS_LEVEL0;
                    break;
                }

                case CAMERA_BRIGHTNESS_LEVEL1:
                {
                    ret =  GC0329_BRIGHTNESS_LEVEL1;
                    break;
                }

                case CAMERA_BRIGHTNESS_LEVEL2:
                {
                    ret =  GC0329_BRIGHTNESS_LEVEL2;
                    break;
                }

                case CAMERA_BRIGHTNESS_LEVEL3:
                {
                    ret =  GC0329_BRIGHTNESS_LEVEL3;
                    break;
                }

                case CAMERA_BRIGHTNESS_LEVEL4:
                {
                    ret =  GC0329_BRIGHTNESS_LEVEL4;
                    break;
                }

                case CAMERA_CONTRAST_LEVEL0:
                {
                    ret =  GC0329_CONTRAST_LEVEL0;
                    break;
                }

                case CAMERA_CONTRAST_LEVEL1:
                {
                    ret =  GC0329_CONTRAST_LEVEL1;
                    break;
                }

                case CAMERA_CONTRAST_LEVEL2:
                {
                    ret =  GC0329_CONTRAST_LEVEL2;
                    break;
                }

                case CAMERA_CONTRAST_LEVEL3:
                {
                    ret =  GC0329_CONTRAST_LEVEL3;
                    break;
                }

                case CAMERA_CONTRAST_LEVEL4:
                {
                    ret =  GC0329_CONTRAST_LEVEL4;
                    break;
                }

                default:
                {
                    ret =  GC0329_CONTRAST_LEVEL0;
                    break;
                }
            }

            break;
        }

        case CAMERA_COLOR_EFFECT:
        {
            switch(value)
            {
                case CAMERA_COLOR_EFFECT_ANTIQUE:
                {
                    ret =  GC0329_COLOR_EFFECT_ANTIQUE;
                    break;
                }

                case CAMERA_COLOR_EFFECT_BLUE:
                {
                    ret =  GC0329_COLOR_EFFECT_BLUE;
                    break;
                }

                case CAMERA_COLOR_EFFECT_GREEN:
                {
                    ret =  GC0329_COLOR_EFFECT_GREEN;
                    break;
                }

                case CAMERA_COLOR_EFFECT_RED:
                {
                    ret =  GC0329_COLOR_EFFECT_RED;
                    break;
                }

                case CAMERA_COLOR_EFFECT_NONE:
                default:
                {
                    ret =  GC0329_COLOR_EFFECT_NONE;
                    break;
                }
            }

            break;

            default:
            {
                ret = 0;
                break;
            }
        }
    }
		#endif
//    return ret;
}



uint8_t camera_dis_on = 0;

void enable_camera_display(uint8_t on)
{
    camera_dis_on = on;
}

void CAMERA_Init(uint32_t Resolution)
{
    camera_drv = &gc0329_drv;
    phdcmi = &hdcmi;

    /* Camera Module Initialization via I2C to the wanted 'Resolution' */
    if (Resolution == CAMERA_R640x480)
    {
        /* For 240x240 resolution, the OV9655 sensor is set to QVGA resolution
         * as OV9655 doesn't supports 240x240  resolution,
         * then DCMI is configured to output a 240x240 cropped window */
        camera_drv->Init(GC0329_I2CADDR, CAMERA_R640x480);


        HAL_DCMI_ConfigCROP(phdcmi,
                            150,                 /* Crop in the middle of the VGA picture */
                            120,                 /* Same height (same number of lines: no need to crop vertically) */
                            (ST7789_WIDTH * 2) - 1,     /* 2 pixels clock needed to capture one pixel */
                            (ST7789_HEIGHT * 1) - 1);    /* All 240 lines are captured */
        HAL_DCMI_EnableCROP(phdcmi);
    }
}

void camera_dispaly(uint16_t *data, uint32_t pixel_num)
{
    int i;
    int count, remain;
    uint16_t *pdata = data;

    st7789_write(1, ST7789_RAMWR);

    count = pixel_num / LCD_MAX_MEM16_BLOCK;
    remain = pixel_num % LCD_MAX_MEM16_BLOCK;
    HAL_GPIO_WritePin(LCD_DCX_GPIO_Port, LCD_DCX_Pin, GPIO_PIN_SET);

    for (i = 0; i < count; ++i)
    {
        st7789_write_fb(pdata, LCD_MAX_MEM16_BLOCK << 1);
        pdata += LCD_MAX_MEM16_BLOCK;
    }

    st7789_write_fb(pdata, remain << 1);
}

void CameraDEMO_Init(uint16_t *buff, uint32_t size)
{
    uint8_t  sensor_id;
    if (!buff || size < 2 * ST7789_WIDTH* ST7789_HEIGHT)
    {
          return;
    }
    camera_buff = buff;
    memset(camera_buff, 0x0, (ST7789_WIDTH * ST7789_HEIGHT) * 2);
    camera_dispaly(camera_buff, (ST7789_WIDTH * ST7789_HEIGHT));
    gc0329_power_onoff(1);
    sensor_id = gc0329_ReadID(0);
    printf("camera sensor_id = 0x%x\n", sensor_id);
    /* gc0329_power_onoff(0); */
    if (sensor_id == GC0329_ID)
    {
        CAMERA_Init(CAMERA_R640x480);
        /* Wait 1s to let auto-loops in the camera module converge and lead to correct exposure */
        /* aos_msleep(1000); */
        HAL_Delay(200);
        /*##-4- Camera Continuous capture start in QVGA resolution ############################*/
        /* Disable unwanted HSYNC (IT_LINE)/VSYNC interrupts */
        __HAL_DCMI_DISABLE_IT(phdcmi, DCMI_IT_LINE | DCMI_IT_VSYNC);

        /* LCD size is 240 x 240 and format is RGB565 i.e. 16 bpp or 2 bytes/pixel.
           The LCD frame size is therefore 240 * 240 half-words of (240*240)/2 32-bit long words .
           Since the DMA associated to DCMI IP is configured in  BSP_CAMERA_MspInit() of stm32l496g_discovery_camera.c file
           with words alignment, the last parameter of HAL_DCMI_Start_DMA is set to:
            (ST7789H2_LCD_PIXEL_WIDTH*ST7789H2_LCD_PIXEL_HEIGHT)/2, that is 240 * 240 / 2
         */
        HAL_DCMI_Start_DMA(phdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)camera_buff, (ST7789_WIDTH * ST7789_HEIGHT) / 2);
        /* OnError_Handler(hal_status != HAL_OK); */
    }
}

void GC0329_CAMERA_FrameEventCallback(void)
{
    if (pCapFunc)
    {
        pCapFunc();
        return;
    }

    if (camera_dis_on)
    {
        HAL_DCMI_Suspend(phdcmi);
        camera_dispaly(camera_buff, (ST7789_WIDTH * ST7789_HEIGHT));
        HAL_DCMI_Start_DMA(phdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)camera_buff, (ST7789_WIDTH * ST7789_HEIGHT) / 2);
        HAL_DCMI_Resume(phdcmi);
    }
}


/**
  * @brief  Frame event callback
  * @param  hdcmi: pointer to the DCMI handle
  * @retval None
  */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    GC0329_CAMERA_FrameEventCallback();
}

int CameraHAL_Capture_Config(uint16_t w, uint16_t h)
{
    HAL_StatusTypeDef hal_status = HAL_OK;

    if (w > 640 || h > 480) return 0;

    camera_drv = &gc0329_drv;
    phdcmi = &hdcmi;

    gc0329_power_onoff(1);
    camera_drv->Init(GC0329_I2CADDR, CAMERA_R640x480);

    HAL_DCMI_ConfigCROP(phdcmi,
                        (640 - w) >> 1,                 /* Crop in the middle of the VGA picture */
                        (480 - h) >> 1,                 /* Same height (same number of lines: no need to crop vertically) */
                        (w * 1) - 1,     /* 2 pixels clock needed to capture one pixel */
                        (w * 1) - 1);    /* All 240 lines are captured */
    HAL_DCMI_EnableCROP(phdcmi);



    //krhino_task_sleep(krhino_ms_to_ticks(1000));
    HAL_Delay(1000);

    __HAL_DCMI_DISABLE_IT(phdcmi, DCMI_IT_LINE | DCMI_IT_VSYNC);

    return hal_status == HAL_OK;
}


int CameraHAL_Capture_Start(uint8_t * buf, uint32_t len, void(*notify)())
{
    HAL_StatusTypeDef hal_status = HAL_OK;

    pCapFunc = notify;

    if (pCapFunc)
    {
        hal_status = HAL_DCMI_Start_DMA(phdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)buf, len);
    }

    return hal_status == HAL_OK;
}




