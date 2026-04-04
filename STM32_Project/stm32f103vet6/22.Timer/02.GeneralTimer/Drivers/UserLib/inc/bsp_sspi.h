/**
 * @file    bsp_sspi.h
 * @brief   软件 SPI 驱动接口定义
 * @details
 * 本文件提供基于 GPIO 模拟实现的软件 SPI（Software SPI）接口。
 * 适用于以下场景：
 * - 芯片硬件 SPI 资源不足
 * - 需要灵活配置 SPI 引脚
 * - 某些外设时序不便直接使用硬件 SPI
 */

#ifndef BSP_SSPI_H
#define BSP_SSPI_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include <stdlib.h>

/* ===================== 宏定义 ===================== */

/**
 * @brief  片选有效状态
 * @details
 * 当片选信号为低电平时，从设备被选中。
 */
#define CHIP_SELECTED      0

/**
 * @brief  片选无效状态
 * @details
 * 当片选信号为高电平时，从设备未被选中。
 */
#define CHIP_NON_SELECTED  1

/**
 * @brief  SPI 通信中的空字节
 * @details
 * 在 SPI 读取操作中，主机通常需要发送一个空字节以产生时钟。
 */
#define DUMMY_BYTE         0x00


/* ===================== 类型定义 ===================== */

/**
 * @brief  软件 SPI 编号枚举
 * @details
 * 用于标识不同的软件 SPI 实例，便于扩展多个 SPI 设备。
 */
typedef enum
{
    BSP_SSPI1 = 0,   /**< 软件 SPI1 */
    BSP_SSPI_MAX     /**< 软件 SPI 实例数量上限 */
} bsp_sspi_t;


/* ===================== API 函数声明 ===================== */

/**
 * @brief  初始化软件 SPI
 * @details
 * 初始化指定软件 SPI 实例所使用的 GPIO，包括：
 * - CS（片选）
 * - MOSI（主机输出从机输入）
 * - MISO（主机输入从机输出）
 * - CLK（时钟）
 *
 * @param  sspi_id 软件 SPI 编号
 * @retval None
 */
void BSP_SSPI_Init(bsp_sspi_t sspi_id);


/**
 * @brief  控制 SPI 片选信号
 * @details
 * 用于控制指定软件 SPI 从设备的片选状态。
 *
 * @param  sspi_id 软件 SPI 编号
 * @param  option  片选控制选项
 *         - CHIP_SELECTED：选中从设备
 *         - CHIP_NON_SELECTED：取消选中从设备
 *
 * @retval None
 */
void BSP_SSPI_CtlChip(bsp_sspi_t sspi_id, uint8_t option);


/**
 * @brief  SPI 读写一个字节
 * @details
 * SPI 为全双工通信方式，该函数在发送一个字节数据的同时，
 * 也会从从设备接收一个字节数据。
 *
 * 若调用者只关心发送结果而不关心接收数据，可将 @p readbyte 设为 NULL。
 *
 * 默认通信模式为 SPI Mode 0：
 * - CPOL = 0
 * - CPHA = 0
 * - 数据在时钟上升沿采样
 *
 * @param  sspi_id   软件 SPI 编号
 * @param  writebyte 待发送的 1 字节数据
 * @param  readbyte  接收数据存储指针
 *         - 非 NULL：保存接收到的数据
 *         - NULL：忽略接收到的数据
 *
 * @retval None
 */
void BSP_SSPI_ReadWriteByte(bsp_sspi_t sspi_id, uint8_t writebyte, uint8_t *readbyte);

#endif /* BSP_SSPI_H */