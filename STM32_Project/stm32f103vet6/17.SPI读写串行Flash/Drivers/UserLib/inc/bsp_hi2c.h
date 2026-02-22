/**
 * @file bsp_hi2c.h
 * @brief 硬件I2C驱动头文件
 * @note STM32F103的I2C外设驱动封装
 */
#ifndef BSP_HI2C_H
#define BSP_HI2C_H

#include "stm32f10x.h"
#include "bsp_delay.h"
#include "bsp_usart.h"

/* ============ 用于调试打印 I2C 寄存器的各个位 ============ */

/* I2C_CR1控制寄存器1的位定义 */
#define I2C_CR1_ACK_BIT			(1 << 10)	// 应答使能位
#define I2C_CR1_STOP_BIT		(1 << 9)	// 停止位产生
#define I2C_CR1_START_BIT		(1 << 8)	// 起始位产生
#define I2C_CR1_PE_BIT			(1 << 0)	// 外设使能位

/* I2C_SR1状态寄存器1的位定义 */
#define I2C_SR1_SB_BIT			(1 << 0)	// 起始位已发送(主模式)
#define I2C_SR1_ADDR_BIT		(1 << 1)	// 地址已发送(主模式)或地址匹配(从模式)
#define I2C_SR1_BTF_BIT			(1 << 2)	// 字节传输完成
#define I2C_SR1_STOPF_BIT		(1 << 4)	// 停止条件检测(从模式)
#define I2C_SR1_RXNE_BIT		(1 << 6)	// 接收数据寄存器非空
#define I2C_SR1_TXE_BIT			(1 << 7)	// 发送数据寄存器空
#define I2C_SR1_AF_BIT			(1 << 10)	// 应答失败

/* I2C_SR2状态寄存器2的位定义 */
#define I2C_SR2_MSL_BIT			(1 << 0)	// 主/从模式标志
#define I2C_SR2_BUSY_BIT		(1 << 1)	// 总线忙标志
#define I2C_SR2_TRA_BIT			(1 << 2)	// 发送器/接收器标志

/* I2C本机地址(作为从机时使用,7位地址0x30左移1位) */
#define I2C_OWNADDR 	(0x30 << 1)

/* I2C通信时钟频率(300kHz) */
#define I2C_CLK_SPEED	300000U

/**
 * @brief I2C外设编号枚举
 */
typedef enum{
	BSP_I2C1 = 0,		// I2C1外设
	BSP_I2C2,			// I2C2外设
	BSP_I2C_MAX			// I2C外设数量(用于数组边界检查)
}bsp_hi2c_t;

/**
 * @brief I2C传输方向枚举
 */
typedef enum{
	BSP_I2C_Dir_Transmitt = 0,	// 发送模式(主机->从机)
	BSP_I2C_Dir_Receive			// 接收模式(从机->主机)
}i2c_dir_t;

/**
 * @brief I2C操作状态枚举
 */
typedef enum{
	I2C_OK = 0,		// 操作成功
	I2C_ERROR,		// 操作错误
	I2C_TIMEOUT		// 操作超时
}i2c_status_t;

/**
 * @brief I2C初始化函数
 * @param i2c_id I2C外设编号
 * @note 配置GPIO引脚、I2C参数并启用I2C外设
 */
extern void BSP_HI2C_Init(bsp_hi2c_t i2c_id);

/**
 * @brief 产生I2C起始信号
 * @param i2c_id I2C外设编号
 * @note 起始信号:SCL为高电平时,SDA由高变低
 */
extern void BSP_HI2C_Start(bsp_hi2c_t i2c_id);

/**
 * @brief 产生I2C停止信号
 * @param i2c_id I2C外设编号
 * @note 停止信号:SCL为高电平时,SDA由低变高
 */
extern void BSP_HI2C_Stop(bsp_hi2c_t i2c_id);

/**
 * @brief 配置I2C应答功能
 * @param i2c_id I2C外设编号
 * @param NewState 使能状态(ENABLE/DISABLE)
 * @note 使能后,I2C会在接收字节后自动发送ACK信号
 */
extern void BSP_HI2C_AcknowledgeConfig(bsp_hi2c_t i2c_id, FunctionalState NewState);

/**
 * @brief 通过I2C发送一个字节
 * @param i2c_id I2C外设编号
 * @param Data 要发送的数据
 */
extern void BSP_HI2C_SendData(bsp_hi2c_t i2c_id, uint8_t Data);

/**
 * @brief 从I2C接收一个字节
 * @param i2c_id I2C外设编号
 * @return 接收到的数据
 */
extern uint8_t BSP_HI2C_ReceiveData(bsp_hi2c_t i2c_id);

/**
 * @brief 发送7位从机地址
 * @param i2c_id I2C外设编号
 * @param addr 从机设备地址(7位)
 * @param dir 传输方向(发送/接收)
 * @note 地址的最低位会自动设置为读/写位
 */
extern void BSP_HI2C_Send7bitAddress(bsp_hi2c_t i2c_id, uint8_t addr, i2c_dir_t dir);

/**
 * @brief 获取I2C标志位状态
 * @param i2c_id I2C外设编号
 * @param flag 标志位(如I2C_FLAG_BUSY、I2C_FLAG_TXE等)
 * @return 标志位状态(SET/RESET)
 */
extern FlagStatus BSP_HI2C_GetFlagStatus(bsp_hi2c_t i2c_id, uint32_t flag);

/**
 * @brief 清除I2C标志位
 * @param i2c_id I2C外设编号
 * @param flag 要清除的标志位
 */
extern void BSP_HI2C_ClearFlag(bsp_hi2c_t i2c_id, uint32_t flag);

/**
 * @brief 清除I2C地址发送标志位(ADDR)
 * @param i2c_id I2C外设编号
 * @note 通过读SR1和SR2寄存器来清除ADDR标志
 */
void BSP_HI2C_ClearFlag_Addr(bsp_hi2c_t i2c_id);

/**
 * @brief 检查I2C事件是否发生
 * @param i2c_id I2C外设编号
 * @param event 要检查的事件
 * @return SUCCESS:事件已发生 ERROR:事件未发生
 */
extern ErrorStatus BSP_HI2C_CheckEvent(bsp_hi2c_t i2c_id, uint32_t event);

/**
 * @brief 等待I2C事件发生(带超时保护)
 * @param i2c_id I2C外设编号
 * @param event 要等待的事件
 * @param timeout_ms 超时时间(毫秒)
 * @return I2C_OK:成功 I2C_TIMEOUT:超时 I2C_ERROR:错误
 * @note 使用超时机制避免程序死锁
 */
extern i2c_status_t BSP_HI2C_WaitEvent(bsp_hi2c_t i2c_id, uint32_t event, uint32_t timeout_ms);

/**
 * @brief 读取I2C寄存器的值
 * @param i2c_id I2C外设编号
 * @param reg 寄存器编号
 * @return 寄存器的值
 * @note 用于调试,读取I2C内部寄存器状态
 */
extern uint16_t BSP_HI2C_ReadRegister(bsp_hi2c_t i2c_id, uint8_t reg);

/**
 * @brief 打印I2C寄存器信息(调试用)
 * @param i2c_id I2C外设编号
 * @note 通过串口打印I2C各寄存器的值和关键位状态,用于故障诊断
 */
extern void BSP_HI2C_Echo_RegMsg(bsp_hi2c_t i2c_id);

#endif