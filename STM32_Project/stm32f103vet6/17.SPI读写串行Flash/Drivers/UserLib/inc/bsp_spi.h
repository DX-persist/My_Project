/**
 * @file    bsp_hspi.h
 * @brief   硬件 SPI 总线底层驱动头文件
 * @version V1.0
 */

#ifndef BSP_SPI_H
#define BSP_SPI_H

#include "stm32f10x.h"
#include "bsp_bus.h"
#include <stdlib.h>

/** @brief SPI 通信最大超时时间配置 */
#define BSP_SPI_TIMEOUT_MAX     0x200

/** 
 * @brief SPI 外设通道枚举定义 
 */
typedef enum{
    BSP_SPI1 = 0,       /**< 使用 SPI1 外设 */
    BSP_SPI2,           /**< 使用 SPI2 外设 */
    BSP_SPI3,           /**< 使用 SPI3 外设 */
    BSP_SPI_MAX         /**< SPI 外设数量上限，用于参数越界校验 */
} bsp_spi_t;

/** 
 * @brief SPI 通信错误码枚举定义 
 */
typedef enum{
    SPI_ERROR_OK = 0,   /**< 通信成功，无错误 */
    SPI_ERROR_PARA,     /**< 参数错误 (例如 ID 越界) */
    SPI_ERROR_BUSY,     /**< SPI 总线忙碌超时 */
    SPI_ERROR_TXE,      /**< 发送缓冲区空 (TXE) 等待超时 */
    SPI_ERROR_RXNE,     /**< 接收缓冲区非空 (RXNE) 等待超时 */
} spi_error_t;

/**
 * @brief  初始化指定的 SPI 外设硬件
 * @param  spi_id SPI 通道号 (BSP_SPI1 / BSP_SPI2 / BSP_SPI3)
 * @retval 无
 */
extern void BSP_SPI_Init(bsp_spi_t spi_id);

/**
 * @brief  通过指定的 SPI 总线发送并接收一个字节
 * @param  spi_id     SPI 通道号
 * @param  write_byte 要发送的数据字节
 * @param  read_byte  用于接收返回数据的指针 (可传入 NULL 忽略接收)
 * @retval 错误码枚举值 (SPI_ERROR_OK 表示成功，其他表示超时或错误)
 */
extern spi_error_t BSP_SPI_ReadWriteByte(bsp_spi_t spi_id, uint8_t write_byte, uint8_t *read_byte);

/**
 * @brief  等待指定的 SPI 总线进入空闲状态
 * @param  spi_id SPI 通道号
 * @retval 错误码枚举值 (SPI_ERROR_OK 表示空闲，SPI_ERROR_BUSY 表示等待超时)
 */
extern spi_error_t BSP_SPI_WaitIdle(bsp_spi_t spi_id);

#endif /* BSP_SPI_H */
