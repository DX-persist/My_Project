/**
 * @file    bsp_si2c.h
 * @author  Antigravity
 * @brief   软件模拟I2C驱动头文件
 * @version V1.0
 * @date    2026-02-15
 * @note    本驱动文件实现了基于GPIO模拟的I2C通信协议，支持标准I2C时序。
 *          适用于STM32系列微控制器，通过宏定义适配不同型号。
 *          
 *          功能列表：
 *          - I2C GPIO初始化
 *          - 起始/停止信号生成
 *          - 字节发送/接收
 *          - ACK/NACK信号处理
 *          - 寄存器读写封装
 *          - 多字节Buffer读写封装
 */

#ifndef __BSP_SI2C_H
#define __BSP_SI2C_H

#include "stm32f10x.h"
#include "bsp_delay.h"
#include <stdbool.h>

/**
 * @brief I2C通道枚举定义
 * @note  用于区分不同的软件I2C总线实例
 */
typedef enum{
    BSP_SI2C1 = 0,      /*!< 软件I2C通道1 (PB6/PB7) */
    BSP_SI2C2,          /*!< 软件I2C通道2 (PC6/PC7) */
    BSP_SI2C_MAX        /*!< I2C通道最大数量，用于参数检查 */
}bsp_si2c_t;

/**
 * @brief I2C应答状态枚举
 */
typedef enum{
    ACK = 0,            /*!< 应答成功 (SDA被拉低) */
    NACK,               /*!< 无应答 (SDA保持高电平) */
}si2c_ack_status;

/* ================= 底层控制函数 ================= */

/**
 * @brief  初始化软件I2C引脚
 * @param  i2c_id: I2C通道ID (BSP_SI2C1 或 BSP_SI2C2)
 * @note   配置SCL和SDA引脚为开漏输出模式，并开启对应时钟
 */
void BSP_SI2C_Init(bsp_si2c_t i2c_id);

/**
 * @brief  产生I2C起始信号 (Start Condition)
 * @param  i2c_id: I2C通道ID
 * @note   SCL高电平期间，SDA由高变低
 */
void BSP_SI2C_Start(bsp_si2c_t i2c_id);

/**
 * @brief  产生I2C停止信号 (Stop Condition)
 * @param  i2c_id: I2C通道ID
 * @note   SCL高电平期间，SDA由低变高
 */
void BSP_SI2C_Stop(bsp_si2c_t i2c_id);

/* ================= 字节收发函数 ================= */

/**
 * @brief  发送一个字节数据
 * @param  i2c_id: I2C通道ID
 * @param  byte:   要发送的8位数据
 * @note   MSB先行，发送完8位后不包含ACK检测（需单独调用WaitAck）
 */
void BSP_SI2C_SendByte(bsp_si2c_t i2c_id, uint8_t byte);

/**
 * @brief  读取一个字节数据
 * @param  i2c_id: I2C通道ID
 * @retval 读取到的8位数据
 * @note   MSB先行，读取完后不包含ACK发送（需单独调用Ack/NAck）
 */
uint8_t BSP_SI2C_ReadByte(bsp_si2c_t i2c_id);

/**
 * @brief  等待从机应答信号
 * @param  i2c_id: I2C通道ID
 * @retval ACK: 接收到应答
 * @retval NACK: 未接收到应答
 * @note   函数内部会产生第9个时钟脉冲
 */
si2c_ack_status BSP_SI2C_WaitAck(bsp_si2c_t i2c_id);

/**
 * @brief  主机发送应答信号 (ACK)
 * @param  i2c_id: I2C通道ID
 * @note   SCL低电平期间拉低SDA，产生第9个时钟
 */
void BSP_SI2C_Ack(bsp_si2c_t i2c_id);

/**
 * @brief  主机发送非应答信号 (NACK)
 * @param  i2c_id: I2C通道ID
 * @note   SCL低电平期间释放SDA(高电平)，产生第9个时钟
 */
void BSP_SI2C_NAck(bsp_si2c_t i2c_id);

/* ================= 高层封装函数 ================= */

/**
 * @brief  向指定寄存器写入单个字节
 * @param  i2c_id:      I2C通道ID
 * @param  device_addr: 从机设备地址 (8位地址，如0xA0)
 * @param  reg_addr:    目标寄存器地址
 * @param  data:        要写入的数据
 * @retval 0: 成功
 * @retval 1: 失败 (无应答)
 */
uint8_t BSP_SI2C_WriteReg(bsp_si2c_t i2c_id, uint8_t device_addr, uint8_t reg_addr, uint8_t data);

/**
 * @brief  从指定寄存器读取单个字节
 * @param  i2c_id:      I2C通道ID
 * @param  device_addr: 从机设备地址 (8位地址，如0xA0)
 * @param  reg_addr:    目标寄存器地址
 * @param  data:        接收数据指针
 * @retval 0: 成功
 * @retval 1: 失败
 */
uint8_t BSP_SI2C_ReadReg(bsp_si2c_t i2c_id, uint8_t device_addr, uint8_t reg_addr, uint8_t *data);

/**
 * @brief  连续写入多个字节 (Buffer Write)
 * @param  i2c_id:      I2C通道ID
 * @param  device_addr: 从机设备地址
 * @param  reg_addr:    起始寄存器地址
 * @param  data:        数据缓冲区指针
 * @param  size:        写入数据长度
 * @retval 0: 成功
 * @retval 1: 失败
 */
uint8_t BSP_SI2C_WriteBuffer(bsp_si2c_t i2c_id, uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint16_t size);

/**
 * @brief  连续读取多个字节 (Buffer Read)
 * @param  i2c_id:      I2C通道ID
 * @param  device_addr: 从机设备地址
 * @param  reg_addr:    起始寄存器地址
 * @param  data:        接收数据缓冲区指针
 * @param  size:        读取数据长度
 * @retval 0: 成功
 * @retval 1: 失败
 */
uint8_t BSP_SI2C_ReadBuffer(bsp_si2c_t i2c_id, uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint16_t size);

#endif /* __BSP_SI2C_H */

