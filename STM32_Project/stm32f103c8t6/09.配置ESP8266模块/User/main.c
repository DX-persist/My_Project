#include "bsp_usart.h"
#include "bsp_delay.h"
#include "bsp_led.h"
#include "bsp_esp8266.h"

#include <string.h>
#include <stdio.h>

int main()
{
	/* 初始化串口1并将printf重定向到串口1用作调试串口 */
	BSP_USART_Config(ESP8266_DEBUG_USART_ID);
	BSP_USART_Stdio(ESP8266_DEBUG_USART_ID);

	/* 初始化系统时基：1ms */
	BSP_TimeBase_Init();
	
	printf("%s|%s|%d 09.WIFI模块测试\r\n", __FILE__, __func__,__LINE__);
	/* 初始化ESP8266模块 */
	BSP_ESP8266_Init();

	BSP_LED_Init();

	if(BSP_ESP8266_Test()){
		printf("%s|%s|%d AT指令测试成功\r\n",__FILE__, __func__,__LINE__);
	}
	if(BSP_ESP8266_CloseEcho()){
		printf("%s|%s|%d 回显功能关闭成功\r\n",__FILE__, __func__,__LINE__);
	}
	if(BSP_ESP8266_ConnectWiFi()){
		printf("%s|%s|%d WiFi连接成功\r\n",__FILE__, __func__,__LINE__);
	}
	if(BSP_ESP8266_ConnectTCP()){
		printf("%s|%s|%d 服务器连接成功\r\n",__FILE__, __func__,__LINE__);
	}
	if(BSP_ESP8266_SetTransMode()){
		printf("%s|%s|%d 透传模式开启成功\r\n",__FILE__, __func__,__LINE__);
	}
	//printf("%s|%s|%d 模块初始化成功\r\n", __FILE__, __func__,__LINE__);
}

