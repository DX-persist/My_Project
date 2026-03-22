/**
 * @file    bsp_xpt2046.h
 * @brief   XPT2046 触摸屏驱动接口定义
 * @details
 * 本文件定义了 XPT2046 电阻触摸屏驱动的接口，包括：
 * - 硬件配置
 * - 命令定义
 * - 坐标参数
 * - API函数声明
 */

#ifndef BSP_XPT2046_H
#define BSP_XPT2046_H

#include "bsp_sspi.h"
#include "bsp_lcd.h"

/* ===================== 硬件配置 ===================== */

/** 使用的软件SPI编号 */
#define BSP_XPT2046_SSPI_ID		BSP_SSPI1

/** 读取X坐标ADC命令 */
#define XPT2046_GetX_ADC		0x90

/** 读取Y坐标ADC命令 */
#define XPT2046_GetY_ADC		0xD0

/** PENIRQ端口 */
#define XPT2046_PENIRQ_GPIO_PORT	GPIOE

/** PENIRQ引脚 */
#define XPT2046_PENIRQ_GPIO_PIN	GPIO_Pin_4

/** PENIRQ时钟 */
#define XPT2046_PENIRQ_GPIO_CLK	RCC_APB2Periph_GPIOE


/* ===================== 屏幕参数 ===================== */

/** LCD宽度（像素） */
#define LCD_WIDTH   240

/** LCD高度（像素） */
#define LCD_HEIGHT  320


/* ===================== 触摸原始范围（默认值） ===================== */

/** X轴最小ADC值（未校准默认值） */
#define TOUCH_X_MIN  200

/** X轴最大ADC值（未校准默认值） */
#define TOUCH_X_MAX  3900

/** Y轴最小ADC值（未校准默认值） */
#define TOUCH_Y_MIN  200

/** Y轴最大ADC值（未校准默认值） */
#define TOUCH_Y_MAX  3800


/* ===================== 坐标修正选项 ===================== */

/**
 * @brief 是否交换X/Y坐标
 * @note  1：交换，0：不交换
 */
#define TOUCH_SWAP_XY   0

/**
 * @brief X轴是否反向
 * @note  1：反向，0：正常
 */
#define TOUCH_INVERT_X  0

/**
 * @brief Y轴是否反向
 * @note  1：反向，0：正常
 */
#define TOUCH_INVERT_Y  0


/* ===================== API函数声明 ===================== */

/**
 * @brief  初始化XPT2046触摸芯片
 * @details
 * 初始化GPIO及SPI接口
 *
 * @param  None
 * @retval None
 */
void BSP_XPT2046_Init(void);


/**
 * @brief  发送命令并读取ADC值
 * @param  writebyte 发送的命令字节
 * @param  readbyte  接收ADC值的指针
 * @retval None
 */
void BSP_XPT2046_ReadWrite(uint8_t writebyte, uint16_t *readbyte);


/**
 * @brief  判断触摸是否被按下
 * @details
 * 通过检测PENIRQ引脚电平判断触摸状态
 *
 * @param  None
 * @retval uint8_t
 *         - 1：按下
 *         - 0：未按下
 */
uint8_t BSP_XPT2046_IsPressed(void);


/**
 * @brief  获取触摸ADC值（带滤波）
 * @details
 * 多次采样并去除极值，提高数据稳定性
 *
 * @param  x_adc_v 输出X方向ADC值
 * @param  y_adc_v 输出Y方向ADC值
 * @retval None
 */
void BSP_XPT2046_GetADC(uint16_t *x_adc_v, uint16_t *y_adc_v);


/**
 * @brief  触摸屏四点校准
 * @details
 * 用户点击四个校准点，建立ADC到LCD的映射关系
 *
 * @param  None
 * @retval None
 */
void Touch_Calibrate(void);


/**
 * @brief  将ADC坐标转换为LCD坐标
 * @details
 * 根据校准结果进行线性映射，并自动进行边界限制
 *
 * @param  adc_x 原始X ADC值
 * @param  adc_y 原始Y ADC值
 * @param  lcd_x 输出LCD X坐标
 * @param  lcd_y 输出LCD Y坐标
 *
 * @retval None
 */
void Touch_Convert(uint16_t adc_x,uint16_t adc_y, uint16_t *lcd_x,uint16_t *lcd_y);


#endif /* BSP_XPT2046_H */