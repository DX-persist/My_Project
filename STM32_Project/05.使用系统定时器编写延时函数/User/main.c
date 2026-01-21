#include "bsp_led.h"
#include "bsp_exti.h"
#include "stm32f10x_it.h"
#include "bsp_delay.h"

extern volatile int key_flag;

int main(void)
{
    BSP_LED_Init();
    BSP_NVICGroup_Config();
	BSP_KEY_EXTI_Config();
	BSP_EXTI_NVIC_Config();
	
	while(1){
		BSP_LED_Toggle(LED_RED);
		BSP_Delay_ms(500);
	}
}
