#include "bsp_led.h"
#include "delay.h"
#include "bsp_exti.h"

int main(void)
{
	BSP_LED_Init();
	
	BSP_NVIC_Config();
	BSP_KEY_Config();
	BSP_EXTI_Config();
	

	while(1){

	}	
}
