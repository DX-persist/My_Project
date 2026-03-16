#ifndef BSP_XPT2046_H
#define BSP_XPT2046_H

#include "bsp_sspi.h"
#include "bsp_lcd.h"

/**
 * @file bsp_xpt2046.h
 * @brief XPT2046 电阻式触摸屏驱动
 *
 * 该模块实现 XPT2046 触摸控制器的驱动，包括：
 * - 触摸检测
 * - ADC数据读取
 * - 触摸校准
 * - 坐标转换
 *
 * 通信方式：软件 SPI
 */

#define BSP_XPT2046_SSPI_ID BSP_SSPI1 /**< 使用的软件SPI编号 */


/** @brief X坐标ADC读取指令 */
#define XPT2046_GetX_ADC 0x90

/** @brief Y坐标ADC读取指令 */
#define XPT2046_GetY_ADC 0xD0


/** @brief PENIRQ 引脚端口 */
#define XPT2046_PENIRQ_GPIO_PORT GPIOE

/** @brief PENIRQ 引脚 */
#define XPT2046_PENIRQ_GPIO_PIN GPIO_Pin_4

/** @brief PENIRQ GPIO 时钟 */
#define XPT2046_PENIRQ_GPIO_CLK RCC_APB2Periph_GPIOE


/** @brief LCD宽度 */
#define LCD_WIDTH 240

/** @brief LCD高度 */
#define LCD_HEIGHT 320


/** @brief ADC触摸范围 */
#define TOUCH_X_MIN 200
#define TOUCH_X_MAX 3900

#define TOUCH_Y_MIN 200
#define TOUCH_Y_MAX 3800


/**
 * @brief 坐标修正选项
 */
#define TOUCH_SWAP_XY   0   /**< 是否交换XY轴 */
#define TOUCH_INVERT_X  0   /**< X轴是否反向 */
#define TOUCH_INVERT_Y  0   /**< Y轴是否反向 */


/**
 * @brief 初始化XPT2046触摸屏
 *
 * 初始化内容：
 * - PENIRQ引脚
 * - 软件SPI接口
 */
extern void BSP_XPT2046_Init(void);


/**
 * @brief 发送命令并读取ADC数据
 *
 * 向XPT2046发送读取命令，并获取返回的12位ADC数据。
 *
 * @param writebyte 发送命令
 * @param readbyte  读取到的ADC值
 */
extern void BSP_XPT2046_ReadWrite(uint8_t writebyte, uint16_t *readbyte);


/**
 * @brief 判断触摸屏是否被按下
 *
 * 通过检测 PENIRQ 引脚状态判断触摸状态。
 *
 * @retval 1 触摸屏被按下
 * @retval 0 未触摸
 */
extern uint8_t BSP_XPT2046_IsPressed(void);


/**
 * @brief 读取触摸屏ADC值
 *
 * 多次采样并进行滤波处理，得到稳定的触摸ADC值。
 *
 * @param x_adc_v X轴ADC值
 * @param y_adc_v Y轴ADC值
 */
extern void BSP_XPT2046_GetADC(uint16_t *x_adc_v, uint16_t *y_adc_v);


/**
 * @brief 触摸屏校准
 *
 * 用户依次点击四个校准点，
 * 计算ADC值与LCD坐标之间的映射关系。
 */
extern void Touch_Calibrate(void);


/**
 * @brief ADC坐标转换为LCD坐标
 *
 * 根据校准数据，将ADC值转换为LCD像素坐标。
 *
 * @param adc_x 触摸ADC X
 * @param adc_y 触摸ADC Y
 * @param lcd_x 转换后的LCD X坐标
 * @param lcd_y 转换后的LCD Y坐标
 */
extern void Touch_Convert(uint16_t adc_x,uint16_t adc_y, uint16_t *lcd_x,uint16_t *lcd_y);

#endif