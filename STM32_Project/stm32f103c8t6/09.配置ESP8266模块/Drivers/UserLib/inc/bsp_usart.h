#ifndef BSP_USART_H
#define BSP_USART_H

#include "stm32f10x.h"
#include "bsp_led.h"

#include <string.h>
#include <stdio.h>

/* 常用波特率 */
#define BAUDRATE_9600       9600U      // 最常用低速率，蓝牙、GPS、GSM模块
#define BAUDRATE_115200     115200U    // ★最常用调试波特率，USB转串口标准速率
#define BAUDRATE_DEFAULT    BAUDRATE_115200

/* 串口的抢占优先级和响应优先级 */
#define USART_PREEMPT_PRIO	1
#define USART_SUB_PRIO		2

#define RING_BUF_SIZE	256

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

typedef struct{
	uint8_t buffer[RING_BUF_SIZE];		/* 用来存放接收到的数据 */
	volatile uint16_t head;				/* 给中断写入数据用 */
	volatile uint16_t tail;				/* 给主函数读取数据用 */
}RingBuffer_t;


extern void BSP_USART_Config(bsp_usart_t id);

extern void BSP_USART_Stdio(bsp_usart_t id);
extern void BSP_USART_SendByte(bsp_usart_t id, uint8_t byte);
extern void BSP_USART_SendString(bsp_usart_t id, char *str);
extern void BSP_USART_SendArray(bsp_usart_t id, uint8_t *array, uint8_t size);

extern uint8_t BSP_USART_ReadByte(bsp_usart_t id);

extern void BSP_USART_ClearRxBuffer(bsp_usart_t id);
extern uint8_t BSP_USART_GetRxCount(bsp_usart_t id);

extern void BSP_USART_Set_ITRXNE_State(bsp_usart_t id, FunctionalState NewState);

extern void BSP_USART_CommandHandler(bsp_usart_t id);

extern void BSP_RecvCommand_Analysis(char *cmd_buf);

#endif
