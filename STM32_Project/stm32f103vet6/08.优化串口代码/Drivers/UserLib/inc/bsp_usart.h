#ifndef BSP_USART_H
#define BSP_USART_H

#include "stm32f10x.h"
#include "bsp_led.h"
#include <stdio.h>
#include <string.h>

#define BAUDRATE_115200		115200U
#define BAUDRATE_9600		9600U
#define BAUDRATE_DEFAULT	BAUDRATE_115200

#define PREEMPT_PRIO	2
#define SUB_PRIO		2

#define RING_BUFFER_SIZE		256

typedef enum{
	BSP_USART1 = 0,
	BSP_USART2,
	BSP_USART3,
	BSP_UART4,
	BSP_UART5,
	BSP_USART_MAX
}bsp_usart_t;

typedef enum{
	BSP_BUS_APB1 = 0,
	BSP_BUS_APB2
}bsp_usart_bus_t;

extern void BSP_USART_Config(bsp_usart_t usart_id);
extern void BSP_USART_SendByte(bsp_usart_t usart_id, uint8_t data_byte);
extern void BSP_USART_SendArray(bsp_usart_t usart_id, uint8_t *arr, uint8_t size);
extern void BSP_USART_SendString(bsp_usart_t usart_id, char *str);
extern void BSP_USART_Init_RxBuffer(bsp_usart_t usart_id);
extern void BSP_USART_Stdio(bsp_usart_t usart_id);
extern uint8_t BSP_USART_ReceiveByte(bsp_usart_t usart_id, uint8_t *data);
extern uint8_t BSP_USART_GetRxCount(bsp_usart_t usart_id);
extern void BSP_USART_Clear_RxBuffer(bsp_usart_t usart_id);
extern void BSP_USART_IRQHandler(bsp_usart_t usart_id);
extern void BSP_NVIC_Priority_GroupConfig(void);
//extern void BSP_USART_ReceiveString(bsp_usart_t usart_id, uint8_t *buffer, uint8_t size);

#endif
