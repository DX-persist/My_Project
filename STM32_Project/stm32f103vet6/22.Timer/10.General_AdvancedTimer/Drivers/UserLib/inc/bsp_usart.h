#ifndef BSP_USART_H
#define BSP_USART_H

#include "stm32f10x.h"    // STM32F10x 系列寄存器定义
#include "bsp_led.h"       // LED 控制（调试用）
#include "bsp_dma.h"       // DMA 配置接口
#include "bsp_bus.h"

#include <stdio.h>
#include <string.h>

/* ===================== 波特率定义 ===================== */
#define BAUDRATE_115200		115200U     // 常用高速波特率
#define BAUDRATE_9600		9600U       // 常用低速波特率
#define BAUDRATE_DEFAULT	BAUDRATE_115200  // 默认波特率

/* ===================== 中断优先级 ===================== */
#define PREEMPT_PRIO	2   // 抢占优先级
#define SUB_PRIO		2   // 响应优先级（子优先级）

/* ===================== 环形缓冲区大小 ===================== */
#define RING_BUFFER_SIZE		256     // 接收缓冲区大小（字节数）

/* ===================== 串口枚举 ===================== */
typedef enum{
	BSP_USART1 = 0,   // USART1
	BSP_USART2,       // USART2
	BSP_USART3,       // USART3
	BSP_UART4,        // UART4
	BSP_UART5,        // UART5
	BSP_USART_MAX     // 串口数量上限
} bsp_usart_t;

/* ===================== 函数接口 ===================== */

/**
 * @brief 初始化指定串口
 * @param usart_id 串口编号
 */
extern void BSP_USART_Config(bsp_usart_t usart_id);

/**
 * @brief 发送单个字节
 * @param usart_id 串口编号
 * @param data_byte 要发送的字节
 */
extern void BSP_USART_SendByte(bsp_usart_t usart_id, uint8_t data_byte);

/**
 * @brief 发送数组数据
 * @param usart_id 串口编号
 * @param arr 数据数组
 * @param size 数组长度
 */
extern void BSP_USART_SendArray(bsp_usart_t usart_id, uint8_t *arr, uint8_t size);

/**
 * @brief 发送字符串
 * @param usart_id 串口编号
 * @param str 字符串指针（以 '\0' 结尾）
 */
extern void BSP_USART_SendString(bsp_usart_t usart_id, char *str);

/**
 * @brief 初始化接收缓冲区
 * @param usart_id 串口编号
 */
extern void BSP_USART_Init_RxBuffer(bsp_usart_t usart_id);

/**
 * @brief 将指定串口绑定为标准输入输出 (printf/scanf)
 * @param usart_id 串口编号
 */
extern void BSP_USART_Stdio(bsp_usart_t usart_id);

/**
 * @brief 接收单个字节（非阻塞）
 * @param usart_id 串口编号
 * @param data 接收数据指针
 * @return 1:成功接收 0:无数据
 */
extern uint8_t BSP_USART_ReceiveByte(bsp_usart_t usart_id, uint8_t *data);

/**
 * @brief 获取接收缓冲区内的数据字节数
 * @param usart_id 串口编号
 * @return 缓冲区内有效数据长度
 */
extern uint8_t BSP_USART_GetRxCount(bsp_usart_t usart_id);

/**
 * @brief 清空接收缓冲区
 * @param usart_id 串口编号
 */
extern void BSP_USART_Clear_RxBuffer(bsp_usart_t usart_id);

/**
 * @brief 串口中断服务函数
 * @param usart_id 串口编号
 */
extern void BSP_USART_IRQHandler(bsp_usart_t usart_id);

/**
 * @brief 配置 NVIC 中断优先级分组
 */
extern void BSP_NVIC_Priority_GroupConfig(void);

/**
 * @brief 使用 DMA 发送数据
 * @param usart_id 串口编号
 * @param buffer 数据缓冲区
 * @param size 发送数据长度
 */
extern void BSP_USART_DMA_Tx_Config(bsp_usart_t usart_id, uint8_t *buffer, uint16_t size);

/**
 * @brief 串口命令控制接口（示例函数，可根据需求解析命令）
 * @param cmd 命令字符串
 */
extern void BSP_USART_ControlCmd(char *cmd);

#endif
