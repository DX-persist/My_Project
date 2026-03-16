#ifndef BSP_SSPI_H
#define BSP_SSPI_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include <stdlib.h>

/**
 * @file bsp_sspi.h
 * @brief 软件SPI驱动接口（Software SPI）
 *
 * 该模块使用GPIO模拟SPI时序，实现SPI通信。
 * 适用于没有硬件SPI或者需要自定义SPI引脚的场景。
 */

/** @brief 片选有效状态 */
#define CHIP_SELECTED		0   /**< 片选有效（CS低电平） */

/** @brief 片选无效状态 */
#define CHIP_NON_SELECTED	1   /**< 片选无效（CS高电平） */

/** @brief SPI通信时的空字节 */
#define DUMMY_BYTE			0x00

/**
 * @brief 软件SPI编号
 *
 * 用于区分不同的软件SPI设备
 */
typedef enum{
	BSP_SSPI1 = 0,   /**< 软件SPI1 */
	BSP_SSPI_MAX     /**< 软件SPI数量 */
}bsp_sspi_t;


/**
 * @brief 初始化软件SPI
 *
 * 初始化软件SPI相关GPIO，包括：
 * - CS
 * - MOSI
 * - MISO
 * - CLK
 *
 * @param sspi_id 软件SPI编号
 */
extern void BSP_SSPI_Init(bsp_sspi_t sspi_id);


/**
 * @brief 控制SPI片选信号
 *
 * 用于控制SPI从设备的片选信号
 *
 * @param sspi_id 软件SPI编号
 * @param option  片选状态
 *                - CHIP_SELECTED      选中设备
 *                - CHIP_NON_SELECTED  取消选中
 */
extern void BSP_SSPI_CtlChip(bsp_sspi_t sspi_id, uint8_t option);


/**
 * @brief SPI读写一个字节
 *
 * SPI为全双工通信，该函数在发送一个字节的同时接收一个字节。
 *
 * @param sspi_id   软件SPI编号
 * @param writebyte 需要发送的数据
 * @param readbyte  接收数据指针
 *                  - 若为NULL则忽略读取数据
 *
 * @note SPI Mode0：
 *       - CPOL = 0
 *       - CPHA = 0
 *       - 上升沿采样
 */
extern void BSP_SSPI_ReadWriteByte(bsp_sspi_t sspi_id, uint8_t writebyte, uint8_t *readbyte);

#endif