#ifndef BSP_USART_H
#define BSP_USART_H

#include "stm32f10x.h"
#include <string.h>

/* 常用波特率 */
#define BAUDRATE_9600       9600U      // 最常用低速率，蓝牙、GPS、GSM模块
#define BAUDRATE_115200     115200U    // ★最常用调试波特率，USB转串口标准速率
#define BAUDRATE_DEFAULT    BAUDRATE_115200

/* 串口的抢占优先级和响应优先级 */
#define USART_PREEMPT_PRIO	1
#define USART_SUB_PRIO		2

#define RECV_BUF_MAX	256

typedef enum{
	BSP_USART1 = 0,
	BSP_USART2,
	BSP_USART3,
	BSP_USART_MAX
}bsp_usart_t;

typedef enum{
	USART_APB1_BUS = 0,
	USART_APB2_BUS
}bsp_usart_bus_t;

extern volatile uint8_t recv_buf[RECV_BUF_MAX];
extern volatile uint8_t recv_done;
extern volatile uint16_t recv_buf_len;

extern void BSP_USART_Config(bsp_usart_t id);
extern void BSP_USART_Stdio(bsp_usart_t id);

extern void BSP_USART_SendByte(bsp_usart_t id, uint8_t byte);
extern void BSP_USART_SendString(bsp_usart_t id, char *str);
extern void BSP_USART_SendArray(bsp_usart_t id, uint8_t *array, uint8_t size);

extern void BSP_USART_CommandHandler(bsp_usart_t id);
extern void BSP_Set_USARTIT_RXNE_State(bsp_usart_t id, FunctionalState NewState);

#endif
