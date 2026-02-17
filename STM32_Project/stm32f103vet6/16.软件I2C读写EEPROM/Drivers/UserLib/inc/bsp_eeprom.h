/**
 * @file    bsp_eeprom.h
 * @brief   EEPROM驱动头文件
 * @note    基于软件模拟I2C接口的EEPROM读写驱动
 */
#ifndef BSP_EEPROM_H
#define BSP_EEPROM_H

#include "bsp_si2c.h"
#include "bsp_usart.h"
#include "bsp_led.h"
#include <math.h>

/* EEPROM使用的I2C接口编号 (对应bsp_si2c.h中的定义) */
#define BSP_EEPROM_I2C_ID           BSP_SI2C1

/* EEPROM页写入最大字节数(AT24C02为8字节) */
#define BSP_EEPROM_WRITE_PAGE_MAX   8

/* EEPROM设备地址 (7位地址0x50左移1位，低位用于R/W) */
#define EEPROM_DEV_ADDR     0xA0

/* 忙等待超时计数最大值 */
#define EEPROM_WAIT_TIMEOUT_MAX     0xFFFF

/**
 * @brief EEPROM初始化函数
 * @note 初始化I2C接口,为EEPROM通信做准备
 */
extern void BSP_EEPROM_Init(void);

/**
 * @brief 向EEPROM写入单个字节
 * @param reg_addr EEPROM内部存储地址(0-255)
 * @param data 要写入的数据
 * @return 0:成功, 1:失败
 */
extern uint8_t BSP_EEPROM_WriteByte(uint8_t reg_addr, uint8_t data);

/**
 * @brief 向EEPROM写入一页或多页数据
 * @param reg_addr EEPROM内部起始地址
 * @param data 要写入的数据缓冲区指针
 * @param size 要写入的数据大小
 * @note  函数内部会自动处理跨页逻辑，确保数据正确写入
 * @return 0:成功, 1:失败
 */
extern uint8_t BSP_EEPROM_WritePage(uint8_t reg_addr, uint8_t *data, uint16_t size);

/**
 * @brief 从EEPROM随机读取单个字节
 * @param reg_addr EEPROM内部存储地址(0-255)
 * @param data 读取数据的存储指针
 * @note 随机读取:可以从任意地址开始读取
 * @return 0:成功, 1:失败
 */
extern uint8_t BSP_EEPROM_ReadRandom(uint8_t reg_addr, uint8_t *data);

/**
 * @brief 从EEPROM顺序读取多个字节
 * @param reg_addr EEPROM内部起始地址
 * @param data 读取数据的缓冲区指针
 * @param size 要读取的数据大小
 * @note 顺序读取:从指定地址开始连续读取多个字节
 * @return 0:成功, 1:失败
 */
extern uint8_t BSP_EEPROM_ReadSequential(uint8_t reg_addr, uint8_t *data, uint16_t size);

/**
 * @brief EEPROM功能测试函数
 * @note 测试EEPROM的读写功能是否正常
 */
extern void BSP_EEPROM_Test(void);

#endif /* BSP_EEPROM_H */

