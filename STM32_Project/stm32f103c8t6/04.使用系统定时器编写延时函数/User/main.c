#include "bsp_led.h"
#include "bsp_delay.h"

int main()
{
	BSP_LED_Init();	
	BSP_TimeBase_Init();

	while(1){
		BSP_LED_On(LED_BLUE);
		BSP_LED_Off(LED_ORANGE);
		BSP_Delay_s(1);	
		BSP_LED_On(LED_ORANGE);
		BSP_LED_Off(LED_BLUE);
		BSP_Delay_s(1);	
	}
}
