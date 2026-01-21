//#include "bsp_led.h"
#include "stm32f10x_it.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include <stdio.h>

#define ARRAY_SIZE 10

int main(void)
{
	//uint8_t array1[ARRAY_SIZE] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
	uint8_t array2[ARRAY_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    //BSP_LED_Init();
	BSP_NVICGroup_Config();
	BSP_USART_Init(BSP_USART1);
	BSP_USART_Setdio(BSP_USART1);
	
	BSP_USART_SendString(BSP_USART1, (uint8_t*)"我最帅\n");
	while(1){
		// BSP_LED_Toggle(LED_RED);
		//BSP_USART_SendByte(BSP_USART1, 'S');
		//BSP_USART_SendHalfWord(BSP_USART1, 0x1234);
		//BSP_USART_SendArray(BSP_USART1, array2, ARRAY_SIZE);
		
		//printf("我最帅！！！\r\n");
		//BSP_Delay_ms(500);
	}
}
