#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_dma.h"

#include <stdio.h>

#define BUFFER_SIZE	8

int main(void)
{
	uint8_t sram_buffer[BUFFER_SIZE] = {'a','b','c','d','e','f','g','h'};

    BSP_LED_Init();
	BSP_TimeBase_Init();
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);

	BSP_USART_DMA_Tx_Config(BSP_USART1, sram_buffer, BUFFER_SIZE);
	while(DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);
	
	BSP_LED_On(LED_BLUE);

	while(1){
	
	}

}
