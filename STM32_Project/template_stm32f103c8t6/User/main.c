#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include "bsp_led.h"
#include "bsp_key.h"

int main(void)
{
	BSP_LED_Init();	
	BSP_KEY_Init();

	while(1)
	{
		if(BSP_KEY_Scan(KEY1) == BSP_KEY_PRESSED){
			BSP_LED_Toggle(LED_BLUE);
		}
		if(BSP_KEY_Scan(KEY2) == BSP_KEY_PRESSED){
			BSP_LED_Toggle(LED_ORANGE);
		}
	}
}
