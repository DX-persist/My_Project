#include "bsp_usart.h"
#include "bsp_delay.h"
#include "bsp_led.h"
#include "bsp_esp8266.h"

#include <string.h>
#include <stdio.h>

void BSP_ESP8266_Connect(void)
{
	if(BSP_ESP8266_Test()){
		printf("%s|%s|%d AT指令测试成功\r\n",__FILE__, __func__,__LINE__);
	}
	if(BSP_ESP8266_CloseEcho()){
		printf("%s|%s|%d 回显功能关闭成功\r\n",__FILE__, __func__,__LINE__);
	}
	if(BSP_ESP8266_ConnectWiFi()){
		printf("%s|%s|%d WiFi连接成功\r\n",__FILE__, __func__,__LINE__);
		BSP_Delay_ms(2000);
	}
	if(BSP_ESP8266_ConnectTCP()){
		printf("%s|%s|%d 服务器连接成功\r\n",__FILE__, __func__,__LINE__);
	}
	if(BSP_ESP8266_SetTransMode()){
		printf("%s|%s|%d 透传模式开启成功\r\n",__FILE__, __func__,__LINE__);
	}
	printf("%s|%s|%d 模块初始化成功\r\n", __FILE__, __func__,__LINE__);
}

int main()
{
	char msg_buffer[256] = {'\0'};
	/* 初始化串口1并将printf重定向到串口1用作调试串口 */
	BSP_USART_Config(ESP8266_DEBUG_USART_ID);
	BSP_USART_Stdio(ESP8266_DEBUG_USART_ID);
	/* 初始化系统时基：1ms */
	BSP_TimeBase_Init();
	
	printf("%s|%s|%d 10.使用WiFi模块控制LED\r\n", __FILE__, __func__,__LINE__);
	/* 初始化ESP8266模块 */
	BSP_ESP8266_Init();

	BSP_LED_Init();

	BSP_ESP8266_Connect();


	BSP_ESP8266_SendData("successfully connect to the server\r\n");
	printf("已发送消息，等待接收数据...\r\n");
	 // 主循环：持续接收数据
    while(1) {
        memset(msg_buffer, 0, sizeof(msg_buffer));  // 清空缓冲区
        BSP_ESP8266_ReceiveData(msg_buffer, sizeof(msg_buffer));
        
        // 去掉首尾的空白字符，检查是否有有效内容
    // 简单判断：长度 > 0 且不全是换行符
		if(strlen(msg_buffer) > 0 && msg_buffer[0] != '\n' && msg_buffer[0] != '\r') {
			printf("收到: %s\n", msg_buffer);
			
			// 处理命令
			BSP_RecvCommand_Analysis(msg_buffer);
		}
	}
}

