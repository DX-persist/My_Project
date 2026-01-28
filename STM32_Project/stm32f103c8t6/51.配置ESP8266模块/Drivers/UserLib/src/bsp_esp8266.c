#include "bsp_esp8266.h"

typedef struct{

	GPIO_TypeDef		*gpio_port;
	uint16_t			gpio_pin;
	uint32_t 			rcc_clk;

}bsp_esp8266_hw_t;

char test_cmd[8];
char set_mixed_mode_cmd[20];
char connect_wifi_cmd[104];
char connect_server_cmd[56];
char set_tran_mode_cmd[20];
char send_msg_cmd[16];
char reboot_cmd[20];
char close_ate_cmd[8];

uint8_t esp8266_recv_buf[RECV_BUF_MAX] = {0};
uint8_t esp8266_recv_buf_len = 0;
uint8_t esp8266_recv_done = 0;

static const bsp_esp8266_hw_t bsp_esp8266_hw[BSP_ESP8266_MAX] = {

	[BSP_ESP8266_EN] = {						/* EN 引脚用来决定 ESP8266 模块是否使能(0: 不使能 1: 使能)*/
		.gpio_port 	= GPIOB,					/* 配置 ESP8266 模块 EN 引脚所在的端口 */
		.gpio_pin 	= GPIO_Pin_12,				/* 配置 ESP8266 模块 EN 引脚 */
		.rcc_clk 	= RCC_APB2Periph_GPIOB,		/* 配置 ESP8266 模块 EN 引脚所在的端口的时钟 */
	},

	[BSP_ESP8266_RST] = {						/* RST 引脚用来决定 ESP8266 模块是否复位(0: 复位 1: 不复位)*/
		.gpio_port 	= GPIOB,					/* 配置 ESP8266 模块 RST 引脚所在的端口 */
		.gpio_pin 	= GPIO_Pin_13,				/* 配置 ESP8266 模块 RST 引脚 */
		.rcc_clk 	= RCC_APB2Periph_GPIOB,		/* 配置 ESP8266 模块 RST 引脚所在的端口的时钟 */
	},
	
	[BSP_ESP8266_GPIO0] = {						/* GPIO0 引脚用来决定 ESP8266 模块的启动方式(0: 进入Flash下载模式,用于烧录固件 1: 进入正常运行模式) */
		.gpio_port 	= GPIOB,					/* 配置 ESP8266 模块 GPIO0 引脚所在的端口 */
		.gpio_pin 	= GPIO_Pin_14,				/* 配置 ESP8266 模块 GPIO0 引脚 */
		.rcc_clk 	= RCC_APB2Periph_GPIOB,		/* 配置 ESP8266 模块 GPIO0 引脚所在的端口的时钟 */
	},

	[BSP_ESP8266_GPIO2] = {						/* GPIO2 引脚必须为高电平,否则可能无法启动或者异常 */
		.gpio_port 	= GPIOB,					/* 配置 ESP8266 模块 GPIO2 引脚所在的端口 */
		.gpio_pin 	= GPIO_Pin_15,				/* 配置 ESP8266 模块 GPIO2 引脚 */
		.rcc_clk 	= RCC_APB2Periph_GPIOB,		/* 配置 ESP8266 模块 GPIO2 引脚所在的端口的时钟 */
	}
};

static void BSP_ESP8266_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	uint32_t clk_mask = 0;

	/* 初始化 GPIO_InitStruct 结构体 */
	GPIO_StructInit(&GPIO_InitStruct);

	/* 开启 ESP8266 模块所用到 GPIO 端口时钟 */
	for(int i = 0; i < BSP_ESP8266_MAX; i++){
		clk_mask |= bsp_esp8266_hw[i].rcc_clk;
	}
	RCC_APB2PeriphClockCmd(clk_mask, ENABLE);
	
	/* 配置 ESP8266 模块配置引脚(EN,RST,GPIO0,GPIO2)的参数(pin,mode,speed) */
	for(int i = 0; i < BSP_ESP8266_MAX; i++){
		GPIO_InitStruct.GPIO_Pin = bsp_esp8266_hw[i].gpio_pin;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(bsp_esp8266_hw[i].gpio_port, &GPIO_InitStruct);
	}
}

static void BSP_ESP8266_WriteBit(bsp_esp8266_t id, BitAction Bitval)
{
	if(id >= BSP_ESP8266_MAX)	return;

	const bsp_esp8266_hw_t *hw = &bsp_esp8266_hw[id];

	GPIO_WriteBit(hw->gpio_port, hw->gpio_pin, Bitval);
}

static void BSP_ESP8266_ENABLE(void)
{
	/* 将 EN 引脚置为高电平表示使能模块 */
	BSP_ESP8266_WriteBit(BSP_ESP8266_EN, Bit_SET);
}

static void BSP_ESP8266_HardReset(void)
{
	/* 将 RST 引脚拉低 10ms 表示复位,复位后再拉高,否则模块不能正常使用 */
	BSP_ESP8266_WriteBit(BSP_ESP8266_RST, Bit_RESET);
	BSP_Delay_ms(20);
	BSP_ESP8266_WriteBit(BSP_ESP8266_RST, Bit_SET);
	BSP_Delay_ms(500);
}

static void BSP_ESP8266_Mode_Select(bsp_esp8266_mode_t mode)
{
	/* 由用户确定当前模块的模式 
	 * 模式0: Flash模式(用于下载固件)
	 * 模式1: 正常模式
	 */
	if(mode == BSP_ESP8266_Flash_Mode){
		BSP_ESP8266_WriteBit(BSP_ESP8266_GPIO0, Bit_RESET);
	}else{
		BSP_ESP8266_WriteBit(BSP_ESP8266_GPIO0, Bit_SET);
	}
}

static void BSP_ESP8266_GPIO2_ENABLE(void)
{
	/* 将 GPIO2 引脚配置为高电平 */
	BSP_ESP8266_WriteBit(BSP_ESP8266_GPIO2, Bit_SET);
}

static void BSP_ESP8266_GPIO_Config(void)
{
	BSP_ESP8266_GPIO_Init();										/* 配置 GPIO 的端口、引脚、模式、速度、启用 RCC 时钟 */
	BSP_ESP8266_ENABLE();											/* 使能 ESP8266 模块 */
	BSP_ESP8266_Mode_Select(BSP_ESP8266_Normal_Mode);				/* 配置 ESP8266 模块正常启动 */
	BSP_ESP8266_GPIO2_ENABLE();										/* 启动配置引脚 */
	BSP_ESP8266_HardReset();										/* 复位一下 ESP8266 模块 */
}

static void BSP_ESP8266_USART_Config(void)
{
	BSP_USART_Config(BSP_USART3);
}

void BSP_ESP8266_Init(void)
{
	BSP_ESP8266_GPIO_Config();
	BSP_ESP8266_USART_Config();

	/* 等待模块启动 */
	BSP_Delay_ms(500);
}

void BSP_ESP8266_Build_Cmd(void)
{
	/* 测试 ESP8266 模块能否正常工作 */
	snprintf(test_cmd, sizeof(test_cmd), "%s\r\n",BSP_ESP8266_TEST_CMD);
	/* 关闭 ESP8266模块的回显功能 */
	snprintf(close_ate_cmd, sizeof(close_ate_cmd), "%s\r\n", BSP_ESP8266_CLOSE_ATE_CMD);
	/* ESP8266 模块设置混合模式(AP + Station)指令*/
	snprintf(set_mixed_mode_cmd, sizeof(set_mixed_mode_cmd), "%s\r\n",BSP_ESP8266_SET_MIXED_MODE_CMD);
	/* ESP8266 模块连接 WIFI */
	snprintf(connect_wifi_cmd, sizeof(connect_wifi_cmd), "%s=\"%s\",\"%s\"\r\n",BSP_ESP8266_CONNECT_WIFI_CMD, 
				BSP_ESP8266_WIFI_NAME, BSP_ESP8266_WIFI_PASSWORD);
	/* ESP8266 模块连接 TCP 服务器(这里需要提前架设好服务器并启动监听) */
	snprintf(connect_server_cmd, sizeof(connect_server_cmd), "%s=\"%s\",\"%s\",%s\r\n",BSP_ESP8266_CONNECT_TCP_CMD, 
				BSP_ESP8266_SERVER_TYPE, BSP_ESP8266_SERVER_IP_ADDR, BSP_ESP8266_SERVER_PORT);
	/* ESP8266 模块设置透传模式 */
	snprintf(set_tran_mode_cmd, sizeof(set_tran_mode_cmd), "%s\r\n", BSP_ESP8266_SET_TRAN_MODE_CMD);
	/* ESP8266 模块开始发送数据 */
	snprintf(send_msg_cmd, sizeof(send_msg_cmd), "%s\r\n", BSP_ESP8266_SEND_MSG_CMD);
	/* ESP8266 模块重启 */
	snprintf(reboot_cmd, sizeof(reboot_cmd), "%s\r\n", BSP_ESP8266_REBOOT_CMD);
}

void BSP_ESP8266_SendString(uint8_t *str)
{
	BSP_USART_SendString(BSP_USART3, str);	
}

static void BSP_ESP8266_InputByte(uint8_t recv_byte)
{
	/* 前提条件：不能超过接收缓冲区最大接受字节数 */
	if(esp8266_recv_buf_len < RECV_BUF_MAX - 1){
		/* 将接受数据寄存器里边的数据存入接收数据缓冲区 */
		esp8266_recv_buf[esp8266_recv_buf_len++] = recv_byte;

		/* 这里需要注意的是接收回来的回复大概是这样的：OK\r\n，
		 * 所以这里就需要去判断它的最后一个字节数是否是'\n'，倒数第
		 * 二个字节数是否是'\r'，只有满足这两个条件的时候说明数据接
		 * 收完毕，此时将接收完毕标志位置为1，并在缓冲区最后一个字节
		 * 填入字符结束标志'\0'
		 *
		 * main函数中去判断字符是否接收完毕，接收完毕后做后续工作
		 * */

		/* 这里判断它接收到的字节数>= 2的目的是为了确保当前缓冲区中
		 * 至少有两个字节，要不然它后边的判断就是在越界访问！！！
		 * 只有满足这三个条件的前提下，才说明数据接收完毕
		 * 给最后一个字节填充字符结束标志'\0'
		 * */
		if(esp8266_recv_buf_len >= 2 && 
				esp8266_recv_buf[esp8266_recv_buf_len - 2] == '\r' && 
				esp8266_recv_buf[esp8266_recv_buf_len - 1] == '\n'){

			esp8266_recv_buf[esp8266_recv_buf_len] = '\0';
			esp8266_recv_done = 1;
		}
	}else{
		/* 最后一个字符放入字符结束标志'\0' */
		esp8266_recv_buf[RECV_BUF_MAX - 1] = '\0';
		esp8266_recv_done = 1;
	}
}

void BSP_ESP8266_CommandHandler(void)
{
	uint8_t recv_byte = 0;

	/* 检测接受数据寄存器(RDR)是否非空，若是则读取数据并清空中断标志位 
	 * 当 RDR 有数据时，SR 寄存器的 RXNE 标志位会自动置为1，对 RDR 寄存器
	 * 进行读取可以清空此标志位
	 * */
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET){
		/* 读取 RDR 寄存器里边的数据 */
		recv_byte = USART_ReceiveData(USART3);		
		/* 构建接收数据缓冲区 */
		BSP_ESP8266_InputByte(recv_byte);
		/* 清空 RXNE 中断标志位 */
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
}

void BSP_ESP8266_Clear_Recvbuf(void)
{

	/* 重置标志位并从头开始写入esp8266_recv_buf */
	esp8266_recv_buf_len = 0;
	esp8266_recv_done = 0;
	memset(esp8266_recv_buf, '\0', sizeof(esp8266_recv_buf));
}


uint8_t BSP_ESP8266_ConnectWiFi(char *cmd, const char *wifi_discon, const char *wifi_con, const char *wifi_gotip, 
								const char *wifi_prefix, const char *expect, const char *errmsg, uint32_t timeout_ms)
{
	uint32_t start = BSP_GetTick();
	bsp_esp8266_state_t state = WIFI_DISCONNECTED;

	/* 发送连接wifi的AT指令 */
	BSP_ESP8266_SendString((uint8_t *)cmd);

	while((BSP_GetTick() - start) < timeout_ms){
		if(esp8266_recv_done == 1){
			/* 将接收缓冲区的数据拿来判断 */	
			BSP_Set_USARTIT_RXNE_State(BSP_USART3, DISABLE);
			
			/* 优先判断是否接收到了"OK"/"ERROR"的字样，若接收到直接可以确定连接
			 * WiFi的结果，若没有接收到，则判断其是否接收到"WIFI GOT IP"/"+CWJAP:x"
			 * 的字样，若有则根据相应的情况做对应的处理
			 *
			 * 收到"+CWJAP:x"就说明连接失败了，具体的x的值有不同的意义，例如
			 * 1：连接超时 2：密码错误 3：找不到目标AP 4：连接失败
			 * 无论这里的x是多少，它都表明连接WiFi失败
			 *
			 * 注意：这里收到"WIFI CONNECTED"并不意味着已经完成，很可能路由器并没有给
			 * 模块分配IP地址，所以一定要检测到获取IP地址才说明连接WiFi成功
			 * */

			if(!strncmp((char *)esp8266_recv_buf, expect, strlen(expect))){
				state = WIFI_GOT_IP;
				break;
			}else if(!strncmp((char *)esp8266_recv_buf, errmsg, strlen(errmsg))){
				break;
			}else if(!strncmp((char *)esp8266_recv_buf, wifi_discon, strlen(wifi_discon))){
				state = WIFI_DISCONNECTED;
			}else if(!strncmp((char *)esp8266_recv_buf, wifi_con, strlen(wifi_con))){
				state = WIFI_CONNECTED;
			}else if(!strncmp((char *)esp8266_recv_buf, wifi_gotip, strlen(wifi_gotip))){
				state = WIFI_GOT_IP;
			}else if(!strncmp((char *)esp8266_recv_buf, wifi_prefix, strlen(wifi_prefix))){
				state = WIFI_CONNECT_FAILED;
			}
		}
	}

	BSP_Set_USARTIT_RXNE_State(BSP_USART3, ENABLE);
	/* 重置标志位并从头开始写入esp8266_recv_buf */
	esp8266_recv_buf_len = 0;
	esp8266_recv_done = 0;
	memset(esp8266_recv_buf, '\0', sizeof(esp8266_recv_buf));

	if(state == WIFI_GOT_IP){
		return SUCCESS;
	}else if(state == WIFI_CONNECT_FAILED){
		return FAILURE;
	}
	return TIMEOUT;
}

uint8_t BSP_ESP8266_ConnectServer(char *cmd, const char *server_con, const char *server_discon, 
									const char *expect, const char *errmsg, uint32_t timeout_ms)
{
	uint32_t start = BSP_GetTick();
	bsp_esp8266_state_t state = TCP_DISCONNECTED;


	BSP_ESP8266_SendString((uint8_t *)cmd);

	while((BSP_GetTick() - start) < timeout_ms){
		if(esp8266_recv_done == 1){
			BSP_Set_USARTIT_RXNE_State(BSP_USART3, DISABLE);

			/* 优先判断是否收到了"OK"/"ERROR"的字样，若收到不用执行
			 * 下边的逻辑关系直接退出循环；若没有收到就检测是否有收到
			 * "CONNECT"/"CLOSED"的字样，如果收到就能够判断是否连接到
			 * 服务器
			 * */

			if(strstr((char *)esp8266_recv_buf, server_con)){
				state = TCP_CONNECTED;
			}else if(strstr((char *)esp8266_recv_buf, server_discon)){
				state = TCP_DISCONNECTED;
			}else if(strstr((char *)esp8266_recv_buf, expect)){
				if(state == TCP_CONNECTED){
					break;			
				}
			}else if(strstr((char *)esp8266_recv_buf, errmsg)){
				state = TCP_DISCONNECTED;
				break;
			}
			/* 使能 RXNE 中断 */
			BSP_Set_USARTIT_RXNE_State(BSP_USART3, ENABLE);
			/* 清空标志位和接收数据缓冲区 */
			esp8266_recv_done = 0;
			esp8266_recv_buf_len = 0;
			memset(esp8266_recv_buf, '\0', sizeof(esp8266_recv_buf));
		}
	}

	/* 使能 RXNE 中断 */
	BSP_Set_USARTIT_RXNE_State(BSP_USART3, ENABLE);
	/* 清空标志位和接收数据缓冲区 */
	esp8266_recv_done = 0;
	esp8266_recv_buf_len = 0;
	memset(esp8266_recv_buf, '\0', sizeof(esp8266_recv_buf));
	

	/* 最后判断 state 的状态 
	 * state = TCP_CONNECTED 连接成功;
	 * state = TCP_DISCONNECTED 连接失败
	 * */
	if(state == TCP_CONNECTED){
		BSP_LED_On(LED_BLUE);
		return SUCCESS;
	}else if(state == TCP_DISCONNECTED){
		BSP_LED_On(LED_ORANGE);
		return FAILURE;
	}
	/* 否则返回超时 */
	return TIMEOUT;
}

/* 此函数仅用于检测模块的指令应答
 * 例如发送AT+CWMODE=3成功执行返回OK\r\n,若这里的指令写错
 * 假如说写成AT+CWMOD=3,那么指令执行失败，模块会返回ERROR
 * */

uint8_t BSP_ESP8266_WaitFor_Cmd_Response(const char *expect, const char *errmsg, uint32_t timeout_ms)
{
	/* 根据系统定时器引入超时机制 */
	uint32_t start = BSP_GetTick();

	/* BSP_GetTick()函数每ms增加1，当它们的差值还小于超时时间时就一直判断*/
	while((BSP_GetTick() - start) < timeout_ms){
		/* 检测到数据接收完毕 */
		if(esp8266_recv_done == 1){
			/* 临时关闭 RXNE 中断，防止后续有数据进来*/
			BSP_Set_USARTIT_RXNE_State(BSP_USART3, DISABLE);
			printf("%s|%s|%d 收到数据:[%s]\r\n",__FILE__, __func__,__LINE__,esp8266_recv_buf);

			/* 如果接收数据缓冲区里边的数据和所期望的数据一样则说明AT指令执行成功
			 * 反之若接收数据缓冲区里边的数据和出错信息一样则说明AT指令执行失败
			 * */
			
			if(strstr((char *)esp8266_recv_buf, expect)){
				/* 清除缓冲区 */
				BSP_ESP8266_Clear_Recvbuf();
				return SUCCESS;
			}else if(strstr((char *)esp8266_recv_buf, errmsg)){
				/* 清除缓冲区*/
				BSP_ESP8266_Clear_Recvbuf();
				return FAILURE;
			}else{
				/* 清除缓冲区*/
				BSP_ESP8266_Clear_Recvbuf();
			}
			/*打开中断接收数据*/
			BSP_Set_USARTIT_RXNE_State(BSP_USART3, ENABLE);
		}
	}
	return TIMEOUT;
}

/* 用于发送检测/设置 ESP8266 模块指令并回收模块 */
uint8_t BSP_ESP8266_SendCmdAnd_WaitFor(char *cmd, const char *expect, const char *errmsg, uint32_t timeout_ms)
{
	uint8_t retval = 0;
	
	/*打开中断接收数据*/
	BSP_Set_USARTIT_RXNE_State(BSP_USART3, ENABLE);

	//printf("%s|%s|%d esp_buf:%s\r\n",__FILE__, __func__,__LINE__, esp8266_recv_buf);
	/* 清空接收数据缓冲区 */
	BSP_ESP8266_Clear_Recvbuf();
	//printf("%s|%s|%d esp_buf:%s\r\n",__FILE__, __func__,__LINE__, esp8266_recv_buf);
	/* 发送 AT 指令 */
	BSP_ESP8266_SendString((uint8_t *)cmd);	
	printf("%s|%s|%d esp_buf:%s\r\n",__FILE__, __func__,__LINE__, esp8266_recv_buf);
	/* 将AT指令执行结果返回给main函数处理 */
	retval = BSP_ESP8266_WaitFor_Cmd_Response(expect, errmsg, timeout_ms);	

	return retval;
}

void BSP_ESP8266_Send_Testcmd(void)
{
	uint8_t retval = 0;
	/* 测试模块是否正常工作: 发送AT指令*/
    printf("%s|%s|%d step1：发送AT测试指令 [%s]\r\n",__FILE__, __func__, __LINE__, esp8266_recv_buf);
    retval = BSP_ESP8266_SendCmdAnd_WaitFor(test_cmd, BSP_ESP8266_CMD_RESPONSE_SUCCESS, BSP_ESP8266_CMD_RESPONSE_FAIL, 4000);
    printf("%s|%s|%d step1：发送AT测试指令 [%s]\r\n",__FILE__, __func__, __LINE__, esp8266_recv_buf);
	if(retval == SUCCESS){
        printf("%s|%s|%d 模块初始化成功\r\n",__FILE__, __func__, __LINE__);
    }else if(retval == FAILURE){
        printf("%s|%s|%d 模块初始化失败\r\n", __FILE__, __func__, __LINE__);
    }else if(retval == TIMEOUT){
        printf("%s|%s|%d 超时......\r\n", __FILE__, __func__, __LINE__);
    }
}

void BSP_ESP8266_Send_Closecmd(void)
{
	uint8_t retval = 0;
	/* (很重要)关闭模块的回显功能 */
    printf("%s|%s|%d step2: 发送关闭回显功能指令 [%s]\r\n", __FILE__, __func__, __LINE__, esp8266_recv_buf);
    retval = BSP_ESP8266_SendCmdAnd_WaitFor(close_ate_cmd, BSP_ESP8266_CMD_RESPONSE_SUCCESS, BSP_ESP8266_CMD_RESPONSE_FAIL, 4000);
    printf("%s|%s|%d step2: 发送关闭回显功能指令 [%s]\r\n", __FILE__, __func__, __LINE__, esp8266_recv_buf);
    if(retval == SUCCESS){
        printf("%s|%s|%d 关闭回显功能成功\r\n",__FILE__, __func__, __LINE__);
    }else if(retval == FAILURE){
        printf("%s|%s|%d 关闭回显功能失败\r\n", __FILE__, __func__, __LINE__);
    }else if(retval == TIMEOUT){
        printf("%s|%s|%d 超时......\r\n", __FILE__, __func__, __LINE__);
    }
}
