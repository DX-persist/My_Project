#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_lcd.h"

#include <stdio.h>
#include <string.h>               

int main(void)
{
	BSP_LED_Init();
	BSP_TimeBase_Init();
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);
	
	printf("FSMC 控制LCD液晶屏\r\n");
	BSP_LCD_Test_Demo();
	
	while(1){
	
	}
}
