/**
 * @file    bsp_xpt2046.c
 * @brief   XPT2046 电阻触摸屏驱动
 * @details
 * 本文件实现 XPT2046 触摸芯片的驱动功能，包括：
 * - 初始化
 * - 触摸检测
 * - ADC采样（带滤波）
 * - 四点校准
 * - 坐标转换
 */

#include "bsp_xpt2046.h"

/**
 * @brief  触摸屏校准参数结构体
 * @details
 * 保存 ADC 原始值与 LCD 坐标之间的映射关系
 */
typedef struct
{
    uint16_t x_min;      /**< ADC最小X值（对应LCD左） */
    uint16_t x_max;      /**< ADC最大X值（对应LCD右） */
    uint16_t y_min;      /**< ADC最小Y值（对应LCD上） */
    uint16_t y_max;      /**< ADC最大Y值（对应LCD下） */

    uint16_t lcd_x_min;  /**< 校准点LCD最小X */
    uint16_t lcd_x_max;  /**< 校准点LCD最大X */
    uint16_t lcd_y_min;  /**< 校准点LCD最小Y */
    uint16_t lcd_y_max;  /**< 校准点LCD最大Y */
} touch_calib_t;

/** 全局触摸校准参数 */
touch_calib_t touch_calib;


/**
 * @brief  初始化 XPT2046 触摸芯片
 * @details
 * 初始化 PENIRQ 引脚以及软件 SPI 接口
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
 * @brief  向 XPT2046 发送命令并读取 ADC 数据
 * @param  writebyte 发送命令字节
 * @param  readbyte  读取结果存储地址
 * @details
 * 通过 SPI 发送控制字并读取 12 位 ADC 数据
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
 * @brief  判断触摸是否被按下
 * @retval 1 触摸按下
 * @retval 0 未按下
 * @details
 * PENIRQ 为低电平表示触摸发生
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
 * @brief  排序并去除极值后求和
 * @param  arr 数据数组
 * @param  len 数组长度
 * @retval 去除最大最小值后的累加和
 * @details
 * 使用冒泡排序，对采样数据进行排序后，
 * 去掉最大值和最小值，提高抗干扰能力
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
 * @brief  获取触摸 ADC 值（带滤波）
 * @param  x_adc_v X方向ADC输出
 * @param  y_adc_v Y方向ADC输出
 * @details
 * 连续采样7次，去掉最大值和最小值后求平均
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
 * @brief  等待触摸并读取坐标
 * @param  x 输出X坐标
 * @param  y 输出Y坐标
 * @details
 * 阻塞等待用户按下屏幕，读取数据后等待释放
 */
void Touch_Wait(uint16_t *x,uint16_t *y)
{
    while(!BSP_XPT2046_IsPressed());

    BSP_Delay_ms(20);

    BSP_XPT2046_GetADC(x,y);

    while(BSP_XPT2046_IsPressed());
}


/**
 * @brief  触摸屏四点校准
 * @details
 * 在屏幕四角绘制校准点，获取 ADC 值并建立映射关系
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
    touch_calib.lcd_x_max = LCD_WIDTH  - x_pad;
    touch_calib.lcd_y_min = y_pad;
    touch_calib.lcd_y_max = LCD_HEIGHT - y_pad;

    printf("Calibration done\r\n");
    printf("Xmin=%d Xmax=%d\r\n", touch_calib.x_min, touch_calib.x_max);
    printf("Ymin=%d Ymax=%d\r\n", touch_calib.y_min, touch_calib.y_max);
}


/**
 * @brief  将 ADC 坐标转换为 LCD 坐标
 * @param  adc_x 原始ADC X值
 * @param  adc_y 原始ADC Y值
 * @param  lcd_x 转换后的LCD X坐标
 * @param  lcd_y 转换后的LCD Y坐标
 * @details
 * 使用线性映射将触摸坐标转换为屏幕坐标
 */
void Touch_Convert(uint16_t adc_x, uint16_t adc_y, uint16_t *lcd_x, uint16_t *lcd_y)
{
    int32_t x, y;
    uint16_t raw_x = adc_y;
    uint16_t raw_y = adc_x;

    if(touch_calib.x_max > touch_calib.x_min){
        x = (int32_t)(raw_x - touch_calib.x_min) 
            * (touch_calib.lcd_x_max - touch_calib.lcd_x_min)
            / (touch_calib.x_max - touch_calib.x_min)
            + touch_calib.lcd_x_min;
    } else {
        x = (int32_t)(touch_calib.x_min - raw_x)
            * (touch_calib.lcd_x_max - touch_calib.lcd_x_min)
            / (touch_calib.x_min - touch_calib.x_max)
            + touch_calib.lcd_x_min;
    }

    if(touch_calib.y_max > touch_calib.y_min){
        y = (int32_t)(raw_y - touch_calib.y_min)
            * (touch_calib.lcd_y_max - touch_calib.lcd_y_min)
            / (touch_calib.y_max - touch_calib.y_min)
            + touch_calib.lcd_y_min;
    } else {
        y = (int32_t)(touch_calib.y_min - raw_y)
            * (touch_calib.lcd_y_max - touch_calib.lcd_y_min)
            / (touch_calib.y_min - touch_calib.y_max)
            + touch_calib.lcd_y_min;
    }

    if(x < 0) x = 0;
    if(x >= LCD_WIDTH)  x = LCD_WIDTH  - 1;
    if(y < 0) y = 0;
    if(y >= LCD_HEIGHT) y = LCD_HEIGHT - 1;

    *lcd_x = (uint16_t)x;
    *lcd_y = (uint16_t)y;
}