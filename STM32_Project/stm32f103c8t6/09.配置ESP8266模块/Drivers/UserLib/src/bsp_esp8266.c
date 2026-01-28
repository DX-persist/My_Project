#include "bsp_esp8266.h"

typedef struct{

	GPIO_TypeDef		*gpio_port;
	uint16_t			gpio_pin;
	uint32_t 			rcc_clk;

}bsp_esp8266_hw_t;

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

static void BSP_ESP8266_HardReset(void)
{
	/* 将 RST 引脚拉低 50ms 表示复位,复位后再拉高,否则模块不能正常使用 */
	BSP_ESP8266_WriteBit(BSP_ESP8266_RST, Bit_RESET);
	BSP_Delay_ms(50);
	BSP_ESP8266_WriteBit(BSP_ESP8266_RST, Bit_SET);

	/* 给 ESP8266 模块上电时间以及连接WIFI的时间
	 * (若这里已经连接过WIFI,它上电后会自动连接)
	 * */
	BSP_Delay_ms(5000);

	/* 将缓冲区里边的垃圾数据(上电时接收到的数据)清理干净 */
	BSP_ESP8266_ClearRxBuffer();
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
	BSP_ESP8266_USART_Config();
	BSP_ESP8266_GPIO_Config();
}

void BSP_ESP8266_ClearRxBuffer(void)
{
	/* 清空串口3所在的接收数据缓冲区 */
	BSP_USART_ClearRxBuffer(ESP8266_USART_ID);
}

void BSP_ESP8266_SendCmd(char *cmd)
{
	BSP_ESP8266_ClearRxBuffer();
	BSP_USART_SendString(ESP8266_USART_ID, cmd);
	BSP_USART_SendString(ESP8266_USART_ID, "\r\n");
}


static ESP8266_Status_t BSP_ESP8266_WaitForResponse(const char *success_msg, const char *fail_msg, uint32_t timeout_ms)
{
	uint32_t start = BSP_GetTick();
	uint8_t resp_buffer[512];
	uint16_t resp_len = 0;
	int data;
	
	memset(resp_buffer, '\0', sizeof(resp_buffer));
	/*
	 * 在超时时间内将数据从环形缓冲区取出来存放到resp_buffer中
	 * 并判断它里边的内容是成功标志还是失败标志返回对应的状态
	 *
	 * */
	while((BSP_GetTick() - start) < timeout_ms){
		data = BSP_USART_ReadByte(ESP8266_USART_ID);
		if(data >= 0 && resp_len < sizeof(resp_buffer) - 1){
			 /* 【调试大法】把收到的每一个字节都转发给串口1，看看是什么 */
        	BSP_USART_SendByte(BSP_USART1, (uint8_t)data); 
			resp_buffer[resp_len++] = data;
			resp_buffer[resp_len] = '\0';

			if(strstr((char *)resp_buffer, success_msg)){
				return ESP8266_OK;
			}else if(strstr((char *)resp_buffer, fail_msg)){
				return ESP8266_ERROR;
			}
		}else{
			//BSP_Delay_us(100); 
		}	
	}
	return ESP8266_TIMEOUT;
}

bool BSP_ESP8266_Test(void)
{
	/* 发送AT测试指令检测模块是否准备好了 */
	/* 清空环形缓冲区 */
	BSP_ESP8266_ClearRxBuffer();
	BSP_ESP8266_SendCmd(ESP8266_TEST_CMD);
	if(BSP_ESP8266_WaitForResponse(ESP8266_CMD_RESPONSE_SUCCESS, ESP8266_CMD_RESPONSE_FAIL, 2000) == ESP8266_OK){
		return true;
	}else{
		return false;
	}
}

bool BSP_ESP8266_CloseEcho(void)
{
	/* 关闭模块的回显功能,防止后续对接收数据的判断造成影响 */
	BSP_ESP8266_ClearRxBuffer();
	BSP_ESP8266_SendCmd(ESP8266_CLOSE_ATE_CMD);
	if(BSP_ESP8266_WaitForResponse(ESP8266_CMD_RESPONSE_SUCCESS, ESP8266_CMD_RESPONSE_FAIL, 2000) == ESP8266_OK){
        return true;
	}else{
        return false;
	}
}

bool BSP_ESP8266_ConnectWiFI(void)
{
	char cmd[100];

	memset(cmd, '\0', sizeof(cmd));

	/* 设置混合模式(AP+Station) */
	BSP_ESP8266_ClearRxBuffer();
	BSP_ESP8266_SendCmd(ESP8266_SET_MIXED_MODE_CMD);
	if(BSP_ESP8266_WaitForResponse(ESP8266_CMD_RESPONSE_SUCCESS, ESP8266_CMD_RESPONSE_FAIL, 2000) == ESP8266_ERROR){
        return false; 
	}
	
	/* 连接WiFi */
	snprintf(cmd, sizeof(cmd), "%s=\"%s\",\"%s\"",ESP8266_CONNECT_WIFI_CMD, ESP8266_WIFI_NAME, ESP8266_WIFI_PASSWORD);
	BSP_ESP8266_ClearRxBuffer();
    BSP_ESP8266_SendCmd(cmd);
    if(BSP_ESP8266_WaitForResponse(ESP8266_WIFI_GOT_IP, ESP8266_WIFI_FAIL_PREFIX, 12000) == ESP8266_OK){
        return true;
	}else{
        return false;
	}
}

bool BSP_ESP8266_ConnectTCP(void)
{
	char cmd[100];
    memset(cmd, '\0', sizeof(cmd));
	
	/* 连接TCP服务器 */
	snprintf(cmd, sizeof(cmd), "%s=\"%s\",\"%s\",%s",ESP8266_CONNECT_TCP_CMD, ESP8266_SERVER_TYPE, 
				ESP8266_SERVER_IP_ADDR, ESP8266_SERVER_PORT);

    BSP_ESP8266_ClearRxBuffer();
    BSP_ESP8266_SendCmd(cmd);
    if(BSP_ESP8266_WaitForResponse("CONNECT", "ERROR", 3000) == ESP8266_OK){
        return true;
	}else{
        return false;
	}

}

bool BSP_ESP8266_SetTransMode(void)
{
	/* 设置透传模式 */
	BSP_ESP8266_ClearRxBuffer();
    BSP_ESP8266_SendCmd(ESP8266_SET_TRAN_MODE_CMD);
    if(BSP_ESP8266_WaitForResponse(ESP8266_CMD_RESPONSE_SUCCESS, NULL, 2000) == ESP8266_ERROR){
        return false;
	}

	/* 发送消息,收到">"表示此时双方可以互发消息了 */
	BSP_ESP8266_ClearRxBuffer();
    BSP_ESP8266_SendCmd(ESP8266_SEND_MSG_CMD);
    if(BSP_ESP8266_WaitForResponse(">", NULL, 2000) == ESP8266_OK){
        return true;
	}else{
        return false;
	}
}

bool BSP_ESP8266_Reboot(void)
{
	/* 重启模块 */
	BSP_ESP8266_ClearRxBuffer();
    BSP_ESP8266_SendCmd(ESP8266_REBOOT_CMD);
    if(BSP_ESP8266_WaitForResponse(ESP8266_CMD_RESPONSE_SUCCESS, NULL, 2000) == ESP8266_OK){
        return true;
	}else{
        return false;
	}
}
