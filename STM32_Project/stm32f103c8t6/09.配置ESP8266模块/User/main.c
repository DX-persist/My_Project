#include "bsp_usart.h"
#include "bsp_delay.h"
#include "bsp_led.h"
#include "bsp_esp8266.h"

#include <string.h>
#include <stdio.h>

int main()
{
	uint8_t step = 0;
	/* 初始化串口1并将printf重定向到串口1用作调试串口 */
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);

	/* 初始化系统时基：1ms */
	BSP_TimeBase_Init();
	
	printf("%s|%s|%d 09.WIFI模块测试\r\n", __FILE__, __func__,__LINE__);
	/* 初始化ESP8266模块 */
	BSP_ESP8266_Init();

	BSP_LED_Init();
	//BSP_Delay_ms(5000);
	// if(BSP_ESP8266_Test()){
	// 	printf("AT指令测试成功\r\n");
	// }
	// printf("hahahah\r\n");
#if 0	
	while(1){
		switch(step){
			case 0:		/* 测试 ESP8266 模块是否正常工作 */
				if(BSP_ESP8266_Test()){
					printf("%s|%s|%d AT指令测试成功\r\n",__FILE__, __func__,__LINE__);
					step++;
				}
				break;
			case 1:		/* 关闭模块的回显功能 */
				if(BSP_ESP8266_CloseEcho()){
					printf("%s|%s|%d 回显功能关闭成功\r\n",__FILE__, __func__,__LINE__);
					step++;
				}
				break;
			case 2:		/* 连接WiFi */
				 if(BSP_ESP8266_ConnectWiFI()){
                    printf("%s|%s|%d WiFi连接成功\r\n",__FILE__, __func__,__LINE__);
                    step++;
                }
				break;
			case 3:		/* 连接服务器 */
				if(BSP_ESP8266_ConnectTCP()){
                    printf("%s|%s|%d 服务器连接成功\r\n",__FILE__, __func__,__LINE__);
                    step++;
                }
				break;
			case 4:		/* 设置透传模式 */
				if(BSP_ESP8266_SetTransMode()){
                    printf("%s|%s|%d 透传模式开启成功\r\n",__FILE__, __func__,__LINE__);
                    step++;
                }
				break;
			case 5:
				printf("模块初始化成功\r\n");
				break;
		}
	}
#endif
}

