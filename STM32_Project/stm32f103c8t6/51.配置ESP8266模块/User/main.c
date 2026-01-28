#include <string.h>
#include <stdio.h>
#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_esp8266.h"

int main()
{
	BSP_LED_Init();
	BSP_TimeBase_Init();

	/* 初始化 ESP8266 模块所在的串口以及引脚 */
	BSP_ESP8266_Init();
	/* 拼接AT指令*/
	BSP_ESP8266_Build_Cmd();
	/* 初始化串口1和重定向printf函数到串口1*/
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);
	
	/* 这里没有用到串口1的接收中断，所以不使能*/
	BSP_Set_USARTIT_RXNE_State(BSP_USART1, DISABLE);
	/* 关闭串口3的RXNE中断，防止垃圾数据进入到接收数据缓冲区中 */
	BSP_Set_USARTIT_RXNE_State(BSP_USART3, DISABLE);


	printf("------08.配置 ESP8266 模块程序------\r\n");	

	/* 延时2s等 ESP8266 模块启动(启动时会打印信息)*/
	printf("等待ESP8266模块启动......\r\n");
	//BSP_Delay_ms(8000);

	
	printf("%s|%s|%d 拼接指令内容测试\r\n", __FILE__, __func__, __LINE__);
	printf("%s|%s|%d  %s",__FILE__, __func__, __LINE__, test_cmd);
	printf("%s|%s|%d  %s",__FILE__, __func__, __LINE__, close_ate_cmd);
	printf("%s|%s|%d  %s",__FILE__, __func__, __LINE__, set_mixed_mode_cmd);
	printf("%s|%s|%d  %s",__FILE__, __func__, __LINE__, connect_wifi_cmd);
	printf("%s|%s|%d  %s",__FILE__, __func__, __LINE__, connect_server_cmd);
	printf("%s|%s|%d  %s",__FILE__, __func__, __LINE__, set_tran_mode_cmd);
	printf("%s|%s|%d  %s",__FILE__, __func__, __LINE__, send_msg_cmd);
	printf("%s|%s|%d  %s",__FILE__, __func__, __LINE__, reboot_cmd);

	/* 测试模块是否正常工作: 发送AT指令*/	
	BSP_ESP8266_Send_Testcmd();
	/* (很重要)关闭模块的回显功能 */
	BSP_ESP8266_Send_Closecmd();
	
#if 0
	int retval = 0;
	printf("%s|%s|%d 发送设置混合模式指令 step3: %s\r\n",__FILE__, __func__, __LINE__, esp8266_recv_buf);
	retval = BSP_ESP8266_SendCmdAnd_WaitFor(set_mixed_mode_cmd, BSP_ESP8266_CMD_RESPONSE_SUCCESS, BSP_ESP8266_CMD_RESPONSE_FAIL, 4000);
	if(retval == SUCCESS){
		printf("%s|%s|%d 混合模式设置成功\r\n", __FILE__, __func__, __LINE__); 
	}else if(retval == FAILURE){
		printf("%s|%s|%d 混合模式设置失败\r\n", __FILE__, __func__, __LINE__);
	}else if(retval == TIMEOUT){
		printf("%s|%s|%d 超时......\r\n", __FILE__, __func__, __LINE__);
	}


	retval = 0;
	printf("数据缓冲区step3: %s\r\n",esp8266_recv_buf);
	retval = BSP_ESP8266_ConnectWiFi(connect_wifi_cmd, BSP_ESP8266_WIFI_DISCONNECTED, BSP_ESP8266_WIFI_CONNECTED, 
			BSP_ESP8266_WIFI_GOT_IP, BSP_ESP8266_WIFI_FAIL_PREFIX, BSP_ESP8266_CMD_RESPONSE_SUCCESS, BSP_ESP8266_CMD_RESPONSE_FAIL, 15000);
	printf("数据缓冲区step3: %s\r\n",esp8266_recv_buf);
	if(retval == SUCCESS){
		printf("连接WIFI成功\r\n");
	}else if(retval == FAILURE){
		printf("连接WiFi失败\r\n");
	}else if(retval == TIMEOUT){
		printf("超时......\r\n");
	}

	retval = 0;
	retval = BSP_ESP8266_ConnectServer(connect_server_cmd, BSP_ESP8266_TCP_CONNECTED, BSP_ESP8266_TCP_CLOSED,
			BSP_ESP8266_CMD_RESPONSE_SUCCESS,BSP_ESP8266_CMD_RESPONSE_FAIL, 5000);
	if(retval == SUCCESS){
		printf("连接服务器成功\r\n");
	}else if(retval == FAILURE){
		printf("连接服务器失败\r\n");
	}else if(retval == TIMEOUT){
		printf("超时......\r\n");
	}
#endif
	while(1){
#if 0
		/* 检测到接收数据完毕 */
		if(esp8266_recv_done){
			BSP_LED_On(LED_BLUE);	
			/* 临时关闭中断 */
			BSP_Set_USARTIT_RXNE_State(BSP_USART3, DISABLE);
			
			printf("Received message: \r\n");	
			/* 将数据发送到串口 */
			BSP_ESP8266_SendString(esp8266_recv_buf);
			//BSP_USART_SendString(BSP_USART3, (uint8_t *)recv_buf);

			/* 对接收缓冲区操作完以后打开(接受数据寄存器非空)中断 */
			BSP_Set_USARTIT_RXNE_State(BSP_USART3, ENABLE);

			/* 启动下一轮的接收 */
			esp8266_recv_done = 0;
			esp8266_recv_buf_len = 0;
			memset(esp8266_recv_buf, '\0', sizeof(esp8266_recv_buf));
		}
#endif		
	}
}
