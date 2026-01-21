#include "bsp_led.h"
#include "bsp_key.h"
#include "delay.h"
#include "bsp_rccclk.h"

int main(void)
{
	BSP_LED_Init();

	/*
	 *	这里可以采用库函数void RCC_GetClocksFreq(RCC_ClocksTypeDef* RCC_Clocks)
	 *	来获取系统时钟频率，HCLK总线频率，PCLK1总线的频率，PCLK2总线的频率，然后
	 *	使用串口或者调试工具打印出来
	 * */

	//HSI_SetSysClock(RCC_PLLMul_16);
	HSE_SetSysClock(RCC_PLLMul_9);
	MCO_GPIO_Config();
	while(1){
		BSP_LED_On(LED_BLUE);
		delay_ms(1000);
		BSP_LED_Off(LED_BLUE);
		delay_ms(1000);
	}	
}
