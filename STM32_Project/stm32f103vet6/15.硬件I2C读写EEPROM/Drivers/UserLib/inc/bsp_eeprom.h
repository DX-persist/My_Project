/**
 * @file bsp_eeprom.h
 * @brief EEPROM驱动头文件
 * @note 基于I2C接口的EEPROM读写驱动
 */
#ifndef BSP_EEPROM_H
#define BSP_EEPROM_H

#include "bsp_hi2c.h"
#include "bsp_usart.h"
#include "bsp_led.h"

/* EEPROM使用的I2C接口编号 */
#define BSP_EEPROM_I2C_ID			BSP_I2C1

/* EEPROM页写入最大字节数(通常为8字节) */
#define BSP_EEPROM_WRITE_PAGE_MAX	8

/* EEPROM写操作的设备地址(7位地址0x50左移1位+写位0) */
#define BSP_EEPROM_WRITE_ADDR		0xA0

/* EEPROM读操作的设备地址(7位地址0x50左移1位+读位1) */
#define BSP_EEPROM_READ_ADDR		0xA1

/**
 * @brief EEPROM初始化函数
 * @note 初始化I2C接口,为EEPROM通信做准备
 */
extern void BSP_EEPROM_Init(void);

/**
 * @brief 向EEPROM写入单个字节
 * @param write_addr EEPROM内部存储地址(0-255)
 * @param write_data 要写入的数据
 */
extern void BSP_EEPROM_WriteByte(uint8_t write_addr, uint8_t write_data);

/**
 * @brief 向EEPROM写入一页数据
 * @param write_addr EEPROM内部起始地址
 * @param buffer 要写入的数据缓冲区指针
 * @param size 要写入的数据大小(不超过BSP_EEPROM_WRITE_PAGE_MAX)
 * @note 页写入可以一次性写入多个字节,提高效率
 */
extern void BSP_EEPROM_WritePage(uint8_t write_addr, uint8_t *buffer, uint8_t size);

/**
 * @brief 从EEPROM随机读取单个字节
 * @param read_addr EEPROM内部存储地址(0-255)
 * @param read_data 读取数据的存储指针
 * @note 随机读取:可以从任意地址开始读取
 */
extern void BSP_EEPROM_ReadRandom(uint8_t read_addr, uint8_t *read_data);

/**
 * @brief 从EEPROM顺序读取多个字节
 * @param read_addr EEPROM内部起始地址
 * @param buffer 读取数据的缓冲区指针
 * @param size 要读取的数据大小
 * @note 顺序读取:从指定地址开始连续读取多个字节
 */
extern void BSP_EEPROM_ReadSequential(uint8_t read_addr, uint8_t *buffer, uint8_t size);

/**
 * @brief EEPROM功能测试函数
 * @note 测试EEPROM的读写功能是否正常
 */
extern void BSP_EEPROM_Test(void);

#endif
