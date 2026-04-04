/**
 * @file    bsp_eeprom_si2c.h
 * @brief   EEPROM驱动头文件
 * @note    基于软件模拟I2C接口的EEPROM读写驱动
 */
#ifndef BSP_EEPROM_SI2C_H
#define BSP_EEPROM_SI2C_H

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
 * @brief 连续向EEPROM写入任意长度数据(支持跨页自动处理)
 * @param reg_addr EEPROM内部起始读取地址(0-255)
 * @param data 要写入的数据缓冲区指针
 * @param size 要写入的数据大小
 * @return 0:成功, 1:失败
 */
extern uint8_t BSP_EEPROM_WriteBuffer(uint8_t reg_addr, uint8_t *data, uint16_t size);

/**
 * @brief 从EEPROM读取任意长度数据
 * @param reg_addr EEPROM内部起始读取地址(0-255)
 * @param data 读取数据的缓冲区指针
 * @param size 要读取的数据大小
 * @return 0:成功, 1:失败
 */
extern uint8_t BSP_EEPROM_ReadBuffer(uint8_t reg_addr, uint8_t *data, uint16_t size);

/**
 * @brief EEPROM功能测试函数
 * @note 测试EEPROM的读写功能是否正常
 */
extern void BSP_EEPROM_Test(void);

#endif /* BSP_EEPROM_H */
