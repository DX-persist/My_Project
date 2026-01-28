#include "bsp_usart.h"
#include "bsp_delay.h"
#include "bsp_led.h"

#include <string.h>
#include <stdio.h>

int main()
{
	/* 初始化串口 */
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);

	/* 初始化系统时基：1ms */
	BSP_TimeBase_Init();
	
	BSP_LED_Init();

	char buffer[24];

	for(int i = 0; i < 5; i++){
		printf("请输入数据\r\n");
		scanf("%s",buffer);
		printf("buffer: %s\r\n",buffer);
	}
	while(1){
	}
}


#if 0

使用示例：

uint8_t array[3] = {0x10, 0x20, 0x30};

BSP_USART_Stdio(BSP_USART1);		/* 在使用 printf(scanf) 函数时必须要先调用此函数以初始化 printf 函数所使用的串口 */
BSP_USART_SendByte(BSP_USART1, 'A');
BSP_USART_SendString(BSP_USART1, "hello world\r\n");
BSP_USART_SendArray(BSP_USART1, array, sizeof(array) / sizeof(array[0]));

#endif
