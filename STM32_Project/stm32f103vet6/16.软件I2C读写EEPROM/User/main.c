#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_eeprom_si2c.h"

#include <stdio.h>

int main(void)
{
	BSP_LED_Init();
	BSP_TimeBase_Init();
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);
	BSP_EEPROM_Test();

	while(1){
	
	}
}
