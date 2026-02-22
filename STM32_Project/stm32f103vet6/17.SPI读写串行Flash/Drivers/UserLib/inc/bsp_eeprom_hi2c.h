/**
 * @file bsp_eeprom_hi2c.h
 * @brief EEPROM驱动头文件
 * @note 基于硬件I2C接口的EEPROM读写驱动
 */
#ifndef BSP_EEPROM_HI2C_H
#define BSP_EEPROM_HI2C_H

#include "bsp_hi2c.h"
#include "bsp_usart.h"
#include "bsp_led.h"

/* EEPROM使用的硬件I2C接口编号 */
#define BSP_EEPROM_I2C_ID			BSP_I2C1

/* EEPROM页写入最大字节数(通常为8字节) */
#define BSP_EEPROM_WRITE_PAGE_MAX	8

/* EEPROM写操作的设备地址(7位地址0x50左移1位+写位0) */
#define BSP_EEPROM_WRITE_ADDR		0xA0

/* EEPROM读操作的设备地址(7位地址0x50左移1位+读位1) */
#define BSP_EEPROM_READ_ADDR		0xA1

/**
 * @brief EEPROM初始化函数
 * @note 初始化硬件I2C接口,为EEPROM通信做准备
 */
extern void BSP_EEPROM_Init(void);

/**
 * @brief 连续向EEPROM写入多字节数据(支持跨页自动处理)
 * @param write_addr EEPROM内部起始写入地址(0-255)
 * @param buffer 要写入的数据缓冲区指针
 * @param size 要写入的数据总大小(不限长度)
 * @note 自动处理EEPROM跨页写入的限制,可以连续写入任意长度数据
 */
extern void BSP_EEPROM_WriteBuffer(uint8_t write_addr, uint8_t *buffer, uint16_t size);

/**
 * @brief 从EEPROM读取任意长度数据 (对外提供的高层统一接口)
 * @param read_addr EEPROM内部起始地址
 * @param buffer 读取数据的缓冲区指针
 * @param size 要读取的数据大小
 * @note 内部调用了连续读取函数来实现
 */
extern void BSP_EEPROM_ReadBuffer(uint8_t read_addr, uint8_t *buffer, uint16_t size);

/**
 * @brief EEPROM功能测试函数
 * @note 测试EEPROM的读写功能是否正常
 */
extern void BSP_EEPROM_Test(void);

#endif
