#ifndef BSP_USART_H
#define BSP_USART_H

#include "stm32f10x.h"
#include <stdio.h>
#include "bsp_led.h"
#include "bsp_delay.h"
/* ================= STM32F1系列常用波特率定义 ================= */

/* 低速波特率 (适用于长距离、工业控制、抗干扰场景) */
#define BAUDRATE_1200       1200U      // 工业仪表、远程抄表
#define BAUDRATE_2400       2400U      // Modbus RTU低速通信
#define BAUDRATE_4800       4800U      // GPS模块(部分型号)
#define BAUDRATE_9600       9600U      // 最常用低速率，蓝牙、GPS、GSM模块

/* 中速波特率 (STM32F1常用调试和通用通信) */
#define BAUDRATE_19200      19200U     // 工业通信、PLC
#define BAUDRATE_38400      38400U     // 高速Modbus、部分传感器
#define BAUDRATE_57600      57600U     // 快速调试、数据采集

/* 高速波特率 (STM32F1推荐使用范围) */
#define BAUDRATE_115200     115200U    // ★最常用调试波特率，USB转串口标准速率
#define BAUDRATE_230400     230400U    // 高速数据传输
#define BAUDRATE_460800     460800U    // USART2/3在APB1=36MHz下推荐最高速率
#define BAUDRATE_921600     921600U    // USART1在APB2=72MHz下推荐最高速率

/* 超高速波特率 (STM32F1理论支持，需谨慎使用) */
#define BAUDRATE_1000000    1000000U   // 1Mbps，需要稳定的时钟和短线缆
#define BAUDRATE_1152000    1152000U   // 1.152Mbps
#define BAUDRATE_2000000    2000000U   // 2Mbps，仅USART1支持，需严格测试

/* ================= STM32F1系列波特率限制说明 ================= */
/*
 * STM32F1系列USART波特率限制：
 * - 最大波特率取决于系统时钟频率（APB1/APB2时钟）
 * - APB2总线（USART1）: 最高72MHz，理论最大波特率 4.5Mbps
 * - APB1总线（USART2/3/4/5）: 最高36MHz，理论最大波特率 2.25Mbps
 * - 实际应用中建议不超过1-2Mbps以保证稳定性
 * 
 * 常见配置（72MHz系统时钟）：
 * - USART1 (APB2=72MHz): 推荐最高115200或921600
 * - USART2/3 (APB1=36MHz): 推荐最高115200或460800
 */

/* ================= 默认波特率 ================= */
#define BAUDRATE_DEFAULT    BAUDRATE_115200

/* 定义抢占优先级和响应优先级 */
#define BSP_USART_PREEMPT_PRIO      2
#define BSP_USART_SUB_PRIO          2

typedef enum{
    BSP_USART1 = 0,
    BSP_USART2,
    BSP_USART3,
    BSP_UART4,
    BSP_UART5,
    BSP_USART_MAX
}bsp_usart_t;

typedef enum{
    USART_BUS_APB1 = 0,
    USART_BUS_APB2
}bsp_usart_bus_t;


extern void BSP_USART_Init(bsp_usart_t id);
extern void BSP_USART_SendByte(bsp_usart_t id, uint8_t byte);
extern void BSP_USART_SendHalfWord(bsp_usart_t id, uint16_t data);
extern void BSP_USART_SendArray(bsp_usart_t id, uint8_t *array, uint16_t size);
//extern uint8_t BSP_USART_ReceiveData(bsp_usart_t id);
extern void BSP_USART_SendString(bsp_usart_t id, uint8_t *str);
extern void BSP_USART_Setdio(bsp_usart_t id);

#endif