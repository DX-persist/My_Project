/**
 * @file    bsp_hspi.h
 * @brief   硬件 SPI 总线底层驱动头文件
 * @version V1.0
 */

#ifndef BSP_HSPI_H
#define BSP_HSPI_H

#include "stm32f10x.h"
#include "bsp_bus.h"
#include <stdlib.h>

/** @brief 硬件 SPI 通信最大超时时间配置 */
#define BSP_HSPI_TIMEOUT_MAX     0x200

/** 
 * @brief 硬件 SPI 外设通道枚举定义 
 */
typedef enum{
    BSP_HSPI1 = 0,       /**< 使用 SPI1 外设 */
    BSP_HSPI2,           /**< 使用 SPI2 外设 */
    BSP_HSPI3,           /**< 使用 SPI3 外设 */
    BSP_HSPI_MAX         /**< SPI 外设数量上限，用于参数越界校验 */
} bsp_hspi_t;

/** 
 * @brief 硬件 SPI 通信操作状态枚举定义 
 */
typedef enum{
    HSPI_OK = 0,         /**< 通信成功，无错误 */
    HSPI_ERROR_PARA,     /**< 参数错误 (例如 ID 越界) */
    HSPI_ERROR_BUSY,     /**< SPI 总线忙碌超时 */
    HSPI_ERROR_TXE,      /**< 发送缓冲区空 (TXE) 等待超时 */
    HSPI_ERROR_RXNE,     /**< 接收缓冲区非空 (RXNE) 等待超时 */
} hspi_status_t;

/**
 * @brief  初始化指定的硬件 SPI 外设
 * @param  hspi_id SPI 通道号 (BSP_HSPI1 / BSP_HSPI2 / BSP_HSPI3)
 * @retval 无
 */
extern void BSP_HSPI_Init(bsp_hspi_t hspi_id);

/**
 * @brief  通过指定的硬件 SPI 总线发送并接收一个字节
 * @param  hspi_id    SPI 通道号
 * @param  write_byte 要发送的数据字节
 * @param  read_byte  用于接收返回数据的指针 (可传入 NULL 忽略接收)
 * @retval 操作状态枚举值 (HSPI_OK 表示成功，其他表示超时或错误)
 */
extern hspi_status_t BSP_HSPI_ReadWriteByte(bsp_hspi_t hspi_id, uint8_t write_byte, uint8_t *read_byte);

/**
 * @brief  等待指定的硬件 SPI 总线进入空闲状态
 * @param  hspi_id SPI 通道号
 * @retval 操作状态枚举值 (HSPI_OK 表示空闲，HSPI_ERROR_BUSY 表示等待超时)
 */
extern hspi_status_t BSP_HSPI_WaitIdle(bsp_hspi_t hspi_id);

#endif /* BSP_HSPI_H */
