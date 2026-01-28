#include "bsp_led.h"
#include "bsp_exti.h"

int main()
{
	BSP_LED_Init();	
	BSP_EXTI_Config();
	while(1){
		switch(key_exti_flag){
			case 1:
				BSP_LED_Toggle(LED_BLUE);
				key_exti_flag = 0;
				break;
			case 2:
				BSP_LED_Toggle(LED_ORANGE);
				key_exti_flag = 0;
				break;
		}
	}
}
