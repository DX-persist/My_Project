#include "stm32f10x.h"                  // Device header
#include "bsp_led.h"

void delay_func(uint32_t cnt)
{
	for(; cnt > 0 ; cnt--);
}

int main(void)
{
	BSP_LED_Init();	
	
	while(1)
	{
		BSP_LED_On(LED_BLUE);
		BSP_LED_On(LED_ORANGE);
		delay_func(900000);
		BSP_LED_Off(LED_BLUE);
		BSP_LED_Off(LED_ORANGE);
		delay_func(900000);
	}
}
