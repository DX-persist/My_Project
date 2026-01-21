#include "bsp_led.h"
#include "delay.h"

int main(void)
{
	BSP_LED_Init();

	while(1){
		BSP_LED_On(LED_GREEN);
		BSP_LED_Off(LED_BLUE);
		BSP_LED_Off(LED_RED);
		delay_ms(1000);
		BSP_LED_On(LED_BLUE);
		BSP_LED_Off(LED_GREEN);
		BSP_LED_Off(LED_RED);
		delay_ms(1000);
		BSP_LED_On(LED_RED);
		BSP_LED_Off(LED_BLUE);
		BSP_LED_Off(LED_GREEN);
		delay_ms(1000);
	}	
}
