#include <stdio.h>

#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"

int main()
{
	//uint8_t array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	//uint8_t array[10] = {0x10, 0x20, 0x30, 0x40};
	//BSP_LED_Init();
	uint8_t data = 100;
	BSP_TimeBase_Init();
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);

	while(1){
		//BSP_USART_SendByte(BSP_USART1, 'a');
		//BSP_USART_SendArray(BSP_USART1, array, 10);
		printf("%d\r\n",data);
		BSP_Delay_s(1);	
	}
}
