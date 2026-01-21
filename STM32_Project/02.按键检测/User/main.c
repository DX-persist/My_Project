#include "bsp_led.h"
#include "bsp_key.h"
#include "delay.h"


int main(void)
{
	BSP_LED_Init();
	BSP_KEY_Init();

	while(1){
		if(BSP_KEY_Scan(KEY1) == BSP_KEY_PRESSED){
			BSP_LED_Toggle(LED_GREEN);
			BSP_LED_Off(LED_BLUE);
			BSP_LED_Off(LED_RED);
		}
		if(BSP_KEY_Scan(KEY2) == BSP_KEY_PRESSED){
			BSP_LED_Toggle(LED_BLUE);
			BSP_LED_Off(LED_GREEN);
			BSP_LED_Off(LED_RED);
		}
	}	
}
