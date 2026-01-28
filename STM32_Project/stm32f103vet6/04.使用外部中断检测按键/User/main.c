#include "bsp_led.h"
#include "bsp_exti.h"
#include "stm32f10x_it.h"

extern volatile int key_flag;

int main(void)
{
    BSP_LED_Init();
    BSP_NVICGroup_Config();
	BSP_KEY_EXTI_Config();
	BSP_EXTI_NVIC_Config();
	
    while (1) {
		if(key_flag){
			switch(key_flag){
			case 1:
				BSP_LED_Toggle(LED_GREEN);
				key_flag = 0;
				break;
			case 2:
				BSP_LED_Toggle(LED_BLUE);
				key_flag = 0;
				break;
			}
		}
    }
}
