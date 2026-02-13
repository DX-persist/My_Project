#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "stm32f10x_it.h"

#include <stdio.h>

int main(void)
{
    BSP_LED_Init();
	BSP_NVIC_Priority_GroupConfig();
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Init_RxBuffer(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);
	BSP_TimeBase_Init();
	char data[32];


	while(1){
		printf("请输入数据\r\n");
		scanf("%s",data);
		printf("data = %s\r\n",data);
	}
}
