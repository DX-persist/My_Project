#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_esp8266.h"

#include <stdio.h>

int main(void)
{
    BSP_LED_Init();
	BSP_TimeBase_Init();

	BSP_NVIC_Priority_GroupConfig();
	BSP_ESP8266_Init();
	printf("10.调试板载WIFI\r\n");
	
	BSP_Delay_ms(5000);
	
	printf("ESP8266 初始化测试\r\n");

	if (BSP_ESP8266_TestAT() != ESP8266_OK) {
		printf("AT 测试失败\r\n");
		while (1);
	}

	BSP_ESP8266_EchoOff();
	BSP_ESP8266_SetWiFiMode();

	if (BSP_ESP8266_ConnectWiFi() != ESP8266_OK) {
		printf("WiFi 连接失败\r\n");
		while (1);
	}

	if (BSP_ESP8266_ConnectTCP() != ESP8266_OK) {
		printf("TCP 连接失败\r\n");
		while (1);
	}

	BSP_ESP8266_SetTransparentMode();
	BSP_ESP8266_EnterSendMode();

	printf("ESP8266 已进入透传模式\r\n");
	BSP_ESP8266_SendData("hello,server");

	while(1){
		
	}

}
