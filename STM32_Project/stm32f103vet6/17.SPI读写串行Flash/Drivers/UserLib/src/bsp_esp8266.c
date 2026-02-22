#include "bsp_esp8266.h"

/*
 * ESP8266 硬件引脚配置结构体
 * 用于管理 EN（使能）和 RST（复位）引脚
 */
typedef struct{
	GPIO_TypeDef *gpio_port;	// GPIO 端口
	uint16_t	gpio_pin;		// GPIO 引脚
	uint32_t 	rcc_clk;		// RCC 时钟
}bsp_esp8266_hw_t;

/*
 * ESP8266 硬件配置表
 * [BSP_ESP8266_EN]  - 使能引脚：PB8
 * [BSP_ESP8266_RST] - 复位引脚：PB9
 */
static const bsp_esp8266_hw_t bsp_esp8266_hw[BSP_ESP8266_MAX] = {
	[BSP_ESP8266_EN] = {
		.gpio_port 	= GPIOB,
		.gpio_pin 	= GPIO_Pin_8,	
		.rcc_clk	= RCC_APB2Periph_GPIOB,
	},

	[BSP_ESP8266_RST] = {
		.gpio_port 	= GPIOB,
		.gpio_pin 	= GPIO_Pin_9,
		.rcc_clk	= RCC_APB2Periph_GPIOB,
	}
};

/*
 * @brief  配置 ESP8266 控制引脚的 GPIO
 * @note   统一使能时钟，批量配置引脚为推挽输出
 */
static void BSP_ESP8266_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	uint32_t clk_mask = 0;

	GPIO_StructInit(&GPIO_InitStruct);
	
	/* 收集所有需要使能的 GPIO 时钟（避免重复使能）*/
	for(int i = 0; i < BSP_ESP8266_MAX; i++){
		clk_mask |= bsp_esp8266_hw[i].rcc_clk;	
	}
	RCC_APB2PeriphClockCmd(clk_mask, ENABLE);

	/* 配置所有控制引脚为推挽输出，50MHz */
	for(int i = 0; i < BSP_ESP8266_MAX; i++){
		GPIO_InitStruct.GPIO_Pin = bsp_esp8266_hw[i].gpio_pin;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(bsp_esp8266_hw[i].gpio_port, &GPIO_InitStruct);
	}
}

/*
 * @brief  使能 ESP8266 模块
 * @note   将 EN 引脚拉高
 */
static void BSP_ESP8266_Enable(void)
{
	GPIO_WriteBit(bsp_esp8266_hw[BSP_ESP8266_EN].gpio_port, 
			bsp_esp8266_hw[BSP_ESP8266_EN].gpio_pin, Bit_SET);
}

/*
 * @brief  硬件复位 ESP8266 模块
 * @note   
 *  - RST 引脚拉低 50ms 后拉高
 *  - 用于重启模块或从异常状态恢复
 */
static void BSP_ESP8266_HardReset(void)
{
	/* 拉低复位引脚 */
	GPIO_WriteBit(bsp_esp8266_hw[BSP_ESP8266_RST].gpio_port, 
			bsp_esp8266_hw[BSP_ESP8266_RST].gpio_pin, Bit_RESET);
	BSP_Delay_ms(50);
	
	/* 拉高复位引脚，模块开始运行 */
	GPIO_WriteBit(bsp_esp8266_hw[BSP_ESP8266_RST].gpio_port, 
			bsp_esp8266_hw[BSP_ESP8266_RST].gpio_pin, Bit_SET);
}

/*
 * @brief  初始化 ESP8266 GPIO（内部调用）
 * @note   依次执行：配置引脚 -> 使能模块 -> 硬件复位
 */
static void BSP_ESP8266_GPIO_Init(void)
{
	BSP_ESP8266_GPIO_Config();
	BSP_ESP8266_Enable();
	BSP_ESP8266_HardReset();
}

/*
 * @brief  配置与 ESP8266 通信的串口
 * @note   
 *  - ESP8266_USART_ID：与模块通信的串口（如 USART2）
 *  - ESP8266_DEBUG_USART_ID：调试输出串口（如 USART1，用于 printf）
 */
static void BSP_ESP8266_USART_Config(void)
{
	/* 配置 ESP8266 通信串口并初始化接收缓冲区 */
	BSP_USART_Config(ESP8266_USART_ID);
	BSP_USART_Init_RxBuffer(ESP8266_USART_ID);
	
	/* 配置调试串口并重定向 printf */
	BSP_USART_Config(ESP8266_DEBUG_USART_ID);
	BSP_USART_Stdio(ESP8266_DEBUG_USART_ID);
}

/*
 * @brief  ESP8266 初始化总入口（外部调用接口）
 * @note   依次初始化 GPIO 和串口
 */
void BSP_ESP8266_Init(void)
{
	BSP_ESP8266_GPIO_Init();
	BSP_ESP8266_USART_Config();
}

/*
 * @brief  发送 AT 命令到 ESP8266
 * @param  cmd  AT 命令字符串（不含 \r\n）
 * @note   自动在命令后添加 \r\n
 */
static void BSP_ESP8266_SendCmd(char *cmd)
{
	BSP_USART_SendString(ESP8266_USART_ID, cmd);
	BSP_USART_SendString(ESP8266_USART_ID, "\r\n");
}

/*
 * @brief  在透传模式下发送数据
 * @param  msg  要发送的数据字符串
 * @note   
 *  - 需先进入透传模式（AT+CIPSEND）
 *  - 会自动添加 \r\n
 */
void BSP_ESP8266_SendData(char *msg)
{
	BSP_USART_SendString(ESP8266_USART_ID, msg);
	BSP_USART_SendString(ESP8266_USART_ID, "\r\n");
}

/*
 * @brief  从 ESP8266 读取一行响应（以 \n 结尾）
 * @param  out         存放读取内容的缓冲区
 * @param  len         缓冲区大小
 * @param  timeout_ms  超时时间（毫秒）
 * @retval 1  成功读取一行
 * @retval 0  超时未读到完整行
 * @note   
 *  - 遇到 \r 忽略，遇到 \n 结束并返回
 *  - 自动在字符串末尾添加 '\0'
 */
static uint8_t BSP_ESP8266_ReadLine(char *out, uint16_t len, uint32_t timeout_ms)
{
	uint32_t start = BSP_GetTick();
	uint8_t data = 0;
	uint8_t esp8266_line_buf[ESP8266_LINE_BUF_SIZE];
	uint8_t esp8266_line_len = 0;

	while((BSP_GetTick() - start) < timeout_ms){
		if(BSP_USART_ReceiveByte(ESP8266_USART_ID, &data)){
			if(data == '\r'){
				/* 忽略回车符 */
				continue;
			}else if(data == '\n'){
				/* 遇到换行符，结束本行 */
				esp8266_line_buf[esp8266_line_len] = '\0';
				strncpy(out, (char *)esp8266_line_buf, len);
				return 1;
			}else{
				/* 累积字符到行缓冲区 */
				if(esp8266_line_len < (ESP8266_LINE_BUF_SIZE - 1)){
					esp8266_line_buf[esp8266_line_len++] = data;
				}
			}
		}
	}

	return 0;  /* 超时 */
}

/*
 * @brief  等待 ESP8266 响应特定字符串
 * @param  fail_msg     失败标志字符串（如 "ERROR"）
 * @param  success_msg  成功标志字符串（如 "OK"）
 * @param  timeout_ms   超时时间（毫秒）
 * @retval ESP8266_OK      成功
 * @retval ESP8266_ERROR   失败
 * @retval ESP8266_TIMEOUT 超时
 * @note   
 *  - 逐行读取响应，匹配成功/失败字符串
 *  - 会打印所有接收到的响应行（调试用）
 */
static ESP8266_Status_t BSP_ESP8266_WaitForResponse(
		char *fail_msg, 
		char *success_msg, 
		uint32_t timeout_ms)
{
	uint32_t start = BSP_GetTick();
	char resp_buf[ESP8266_LINE_BUF_SIZE];

	while((BSP_GetTick() - start) < timeout_ms){
		if(BSP_ESP8266_ReadLine(resp_buf, ESP8266_LINE_BUF_SIZE, timeout_ms)){
			
			/* 打印接收到的每一行（调试信息）*/
			printf("[ESP8266] %s\r\n", resp_buf);
			
			/* 检查是否包含成功标志 */
			if(strstr(resp_buf, success_msg)){
				return ESP8266_OK;
			}
			/* 检查是否包含失败标志 */
			else if(strstr(resp_buf, fail_msg)){
				return ESP8266_ERROR;
			}
		}
	}
	return ESP8266_TIMEOUT;
}

/*
 * @brief  发送 AT 命令并等待响应（外部接口）
 * @param  cmd          AT 命令字符串
 * @param  success_msg  成功标志（如 "OK"）
 * @param  fail_msg     失败标志（如 "ERROR"）
 * @param  timeout_ms   超时时间（毫秒）
 * @retval ESP8266_Status_t 执行结果
 * @note   
 *  - 先清空接收缓冲区（避免残留数据干扰）
 *  - 发送命令后等待响应
 */
ESP8266_Status_t BSP_ESP8266_SendCmdAndWait(
		char *cmd, 
		char *success_msg, 
		char *fail_msg, 
		uint32_t timeout_ms)
{
	BSP_USART_Clear_RxBuffer(ESP8266_USART_ID);
	BSP_ESP8266_SendCmd(cmd);
	return (BSP_ESP8266_WaitForResponse(fail_msg, success_msg, timeout_ms));
}

/*
 * @brief  测试 ESP8266 是否响应（发送 AT）
 * @retval ESP8266_Status_t
 */
ESP8266_Status_t BSP_ESP8266_TestAT(void)
{
    return BSP_ESP8266_SendCmdAndWait(
        "AT",
        "OK",
        "ERROR",
        1000
    );
}

/*
 * @brief  关闭回显（ATE0）
 * @note   避免模块回显命令，简化响应解析
 */
ESP8266_Status_t BSP_ESP8266_EchoOff(void)
{
    return BSP_ESP8266_SendCmdAndWait(
        "ATE0",
        "OK",
        "ERROR",
        1000
    );
}

/*
 * @brief  设置 WiFi 模式为 Station+AP 模式（模式3）
 * @note   
 *  - 模式 1：Station（客户端）
 *  - 模式 2：AP（接入点）
 *  - 模式 3：Station + AP
 */
ESP8266_Status_t BSP_ESP8266_SetWiFiMode(void)
{
    return BSP_ESP8266_SendCmdAndWait(
        "AT+CWMODE=3",
        "OK",
        "ERROR",
        2000
    );
}

/*
 * @brief  连接到指定 WiFi 网络
 * @note   
 *  - SSID: "Ridiculous2.4g"
 *  - 密码: "persist011104"
 *  - 超时 15 秒（连接 WiFi 可能较慢）
 */
ESP8266_Status_t BSP_ESP8266_ConnectWiFi(void)
{
    return BSP_ESP8266_SendCmdAndWait(
        "AT+CWJAP=\"Ridiculous2.4g\",\"persist011104\"",
        "OK",
        "FAIL",
        15000
    );
}

/*
 * @brief  连接到 TCP 服务器
 * @note   
 *  - 服务器 IP: 192.168.10.20
 *  - 端口: 8080
 *  - 需先连接 WiFi
 */
ESP8266_Status_t BSP_ESP8266_ConnectTCP(void)
{
    return BSP_ESP8266_SendCmdAndWait(
        "AT+CIPSTART=\"TCP\",\"192.168.10.20\",8080",
        "OK",
        "ERROR",
        5000
    );
}

/*
 * @brief  设置为透传模式
 * @note   
 *  - AT+CIPMODE=1：开启透传
 *  - AT+CIPMODE=0：关闭透传（普通模式）
 */
ESP8266_Status_t BSP_ESP8266_SetTransparentMode(void)
{
    return BSP_ESP8266_SendCmdAndWait(
        "AT+CIPMODE=1",
        "OK",
        "ERROR",
        2000
    );
}

/*
 * @brief  进入数据发送模式
 * @note   
 *  - 需先设置透传模式并建立 TCP 连接
 *  - 收到 ">" 后可直接发送数据
 *  - 发送 "+++" 可退出透传模式
 */
ESP8266_Status_t BSP_ESP8266_EnterSendMode(void)
{
    return BSP_ESP8266_SendCmdAndWait(
        "AT+CIPSEND",
        ">",
        "ERROR",
        3000
    );
}

void BSP_ESP8266_Ctl_Cmd(void)
{
	char cmd_buffer[128];
	BSP_ESP8266_ReadLine(cmd_buffer, 128, 5000);
	
	BSP_USART_ControlCmd(cmd_buffer);
}