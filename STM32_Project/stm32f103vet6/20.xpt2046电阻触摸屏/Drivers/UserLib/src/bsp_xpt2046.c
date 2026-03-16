/**
 * @file bsp_xpt2046.c
 * @brief XPT2046触摸屏驱动实现
 */

#include "bsp_xpt2046.h"

/**
 * @brief 触摸屏校准数据结构
 *
 * 保存ADC坐标范围与LCD坐标范围之间的映射关系。
 */
typedef struct
{
    uint16_t x_min; /**< ADC最小X值 */
    uint16_t x_max; /**< ADC最大X值 */

    uint16_t y_min; /**< ADC最小Y值 */
    uint16_t y_max; /**< ADC最大Y值 */

    uint16_t lcd_x_min; /**< LCD最小X */
    uint16_t lcd_x_max; /**< LCD最大X */

    uint16_t lcd_y_min; /**< LCD最小Y */
    uint16_t lcd_y_max; /**< LCD最大Y */

} touch_calib_t;


/** 触摸校准参数 */
touch_calib_t touch_calib;


/**
 * @brief 初始化触摸屏
 */
void BSP_XPT2046_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	RCC_APB2PeriphClockCmd(XPT2046_PENIRQ_GPIO_CLK, ENABLE);

	GPIO_InitStruct.GPIO_Pin = XPT2046_PENIRQ_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;

	GPIO_Init(XPT2046_PENIRQ_GPIO_PORT, &GPIO_InitStruct);

	BSP_SSPI_Init(BSP_XPT2046_SSPI_ID);
}


/**
 * @brief SPI读取ADC数据
 *
 * XPT2046返回数据格式：
 *
 *  D11 D10 D9 ... D0
 *
 *  实际返回为16bit，需要右移3位。
 */
void BSP_XPT2046_ReadWrite(uint8_t writebyte, uint16_t *readbyte)
{
	uint8_t rx_high = 0;
	uint8_t rx_low = 0;
	uint16_t result = 0;

	BSP_SSPI_CtlChip(BSP_XPT2046_SSPI_ID, CHIP_SELECTED);
	
	BSP_SSPI_ReadWriteByte(BSP_XPT2046_SSPI_ID, writebyte, NULL);
	BSP_SSPI_ReadWriteByte(BSP_XPT2046_SSPI_ID, DUMMY_BYTE, &rx_high);
	BSP_SSPI_ReadWriteByte(BSP_XPT2046_SSPI_ID, DUMMY_BYTE, &rx_low);
	
	BSP_SSPI_CtlChip(BSP_XPT2046_SSPI_ID, CHIP_NON_SELECTED);

	result = ((uint16_t)rx_high << 8) | rx_low;
	result >>= 3;

	*readbyte = result;
}


/**
 * @brief 判断触摸是否按下
 *
 * PENIRQ 为低电平表示触摸发生。
 */
uint8_t BSP_XPT2046_IsPressed(void)
{
	uint8_t pressed = 0;

	if(GPIO_ReadInputDataBit(XPT2046_PENIRQ_GPIO_PORT, XPT2046_PENIRQ_GPIO_PIN)){
		pressed = 0;
	}else{
		pressed = 1;
	}

	return pressed;
}


/**
 * @brief 中值滤波求平均
 *
 * 对采样数据排序，
 * 去除最大值与最小值后求平均。
 *
 * @param arr 数据数组
 * @param len 数组长度
 *
 * @return 中间值平均
 */
static uint32_t BSP_XPT2046_SumMiddle(uint16_t *arr, uint8_t len)
{
    uint16_t temp;
    uint32_t sum = 0;

    for(int i = 0; i < len - 1; i++){
        for(int j = 0; j < len - 1 - i; j++){
            if(arr[j] > arr[j + 1]){
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }

    for(int i = 1; i < len - 1; i++){
        sum += arr[i];
    }

    return sum;
}


/**
 * @brief 获取触摸ADC值
 *
 * 进行多次采样并滤波，
 * 提高触摸数据稳定性。
 */
void BSP_XPT2046_GetADC(uint16_t *x_adc_v, uint16_t *y_adc_v)
{
    uint16_t x_adc[7] = {0};
    uint16_t y_adc[7] = {0};

    int len = sizeof(x_adc) / sizeof(x_adc[0]);

    for(int i = 0; i < len; i++){

        BSP_XPT2046_ReadWrite(XPT2046_GetX_ADC, &x_adc[i]);

        BSP_XPT2046_ReadWrite(XPT2046_GetY_ADC, &y_adc[i]);
    }

    *x_adc_v = BSP_XPT2046_SumMiddle(x_adc, len) / (len - 2);

    *y_adc_v = BSP_XPT2046_SumMiddle(y_adc, len) / (len - 2);
}


/**
 * @brief 等待触摸并读取坐标
 */
void Touch_Wait(uint16_t *x,uint16_t *y)
{
    while(!BSP_XPT2046_IsPressed());

    BSP_Delay_ms(20);

    BSP_XPT2046_GetADC(x,y);

    while(BSP_XPT2046_IsPressed());
}


/**
 * @brief 触摸屏校准
 *
 * 依次点击四个校准点，
 * 计算ADC与LCD坐标映射关系。
 */
void Touch_Calibrate(void)
{
    uint16_t x[4];
    uint16_t y[4];

    uint16_t x_pad = LCD_WIDTH  / 10;
    uint16_t y_pad = LCD_HEIGHT / 10;

    BSP_LCD_Clear(WHITE);

    printf("Touch calibration start\r\n");

    /* 左上 */
    BSP_LCD_DrawCircle(x_pad, y_pad, 15, RED, 1);
    Touch_Wait(&x[0], &y[0]);
    BSP_LCD_Clear(WHITE);

    /* 右上 */
    BSP_LCD_DrawCircle(LCD_WIDTH - x_pad, y_pad, 15, RED, 1);
    Touch_Wait(&x[1], &y[1]);
    BSP_LCD_Clear(WHITE);

    /* 左下 */
    BSP_LCD_DrawCircle(x_pad, LCD_HEIGHT - y_pad, 15, RED, 1);
    Touch_Wait(&x[2], &y[2]);
    BSP_LCD_Clear(WHITE);

    /* 右下 */
    BSP_LCD_DrawCircle(LCD_WIDTH - x_pad, LCD_HEIGHT - y_pad, 15, RED, 1);
    Touch_Wait(&x[3], &y[3]);
    BSP_LCD_Clear(WHITE);

    touch_calib.x_min = (y[0] + y[2]) / 2;
    touch_calib.x_max = (y[1] + y[3]) / 2;

    touch_calib.y_min = (x[0] + x[1]) / 2;
    touch_calib.y_max = (x[2] + x[3]) / 2;

    touch_calib.lcd_x_min = x_pad;
    touch_calib.lcd_x_max = LCD_WIDTH - x_pad;

    touch_calib.lcd_y_min = y_pad;
    touch_calib.lcd_y_max = LCD_HEIGHT - y_pad;

    printf("Calibration done\r\n");
}