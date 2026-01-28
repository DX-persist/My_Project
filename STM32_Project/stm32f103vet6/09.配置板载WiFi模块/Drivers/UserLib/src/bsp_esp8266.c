#include "bsp_esp8266.h"

typedef struct{
	GPIO_TypeDef 		*port;
	uint16_t			pin;
	uint32_t			clk;
}bsp_esp8266_hw_t;

static const bsp_esp8266_hw_t bsp_esp8266_hw[BSP_ESP8266_MAX] = {
	[BSP_ESP8266_CHPD] = {
		.port = GPIOB,
		.pin  = GPIO_Pin_8,
		.clk  = RCC_APB2Periph_GPIOB,
	},
	[BSP_ESP8266_RST] = {
		.port = GPIOB,
		.pin  = GPIO_Pin_9,
		.clk  = RCC_APB2Periph_GPIOB,
	}
};

volatile uint8_t bsp_esp8266_rx_buf[BSP_ESP8266_RX_BUF_LEN] = {0};
volatile uint16_t bsp_esp8266_rx_len = 0;
volatile uint16_t bsp_esp8266_rx_done = 0;

static void BSP_ESP8266_WriteBit(bsp_esp8266_t id, BitAction BitVal)
{
	if(id >= BSP_ESP8266_MAX)	return;
	
	const bsp_esp8266_hw_t *hw = &bsp_esp8266_hw[id];

	GPIO_WriteBit(hw->port, hw->pin, BitVal);
}

static void BSP_ESP8266_HardReset(void)
{
	BSP_ESP8266_WriteBit(BSP_ESP8266_RST, Bit_RESET);
	BSP_Delay_ms(20);
	BSP_ESP8266_WriteBit(BSP_ESP8266_RST, Bit_SET);
	BSP_Delay_ms(500);
}

static void BSP_ESP8266_ENABLE(void)
{
	BSP_ESP8266_WriteBit(BSP_ESP8266_CHPD, Bit_SET);
}

static void BSP_ESP8266_GPIO_Init(void)
{
	uint32_t clk_mask = 0;
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_StructInit(&GPIO_InitStruct);

	/* 使能与ESP8266相接的GPIO端口的时钟 */
	for(int i = 0; i < BSP_ESP8266_MAX; i++){
		clk_mask |= bsp_esp8266_hw[i].clk;
	}
	RCC_APB2PeriphClockCmd(clk_mask, ENABLE);

	/* 配置ESP8266使能和复位的引脚、模式、输出速度*/
	for(int i = 0; i < BSP_ESP8266_MAX; i++){
		GPIO_InitStruct.GPIO_Pin = bsp_esp8266_hw[i].pin;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(bsp_esp8266_hw[i].port, &GPIO_InitStruct);
	}
	
	/* 拉高PB9使能ESP8266模块 */
	BSP_ESP8266_ENABLE();

	/* 复位ESP8266 模块*/
	BSP_ESP8266_HardReset();
}

static void BSP_ESP8266_USART_Init(void)
{
	BSP_NVICGroup_Config();
	BSP_USART_Init(BSP_USART3);
	BSP_USART_Setdio(BSP_USART3);	
}

void BSP_ESP8266_Init(void)
{
	BSP_ESP8266_GPIO_Init();
	BSP_ESP8266_USART_Init();
}

void BSP_ESP8266_SendArray(volatile uint8_t *array, uint16_t size)
{
    for(uint16_t i = 0; i < size; i++)
    {
        BSP_USART_SendByte(BSP_USART3, array[i]);
    }
}

void BSP_ESP8266_SendCmd(uint8_t *str)
{
	BSP_USART_SendString(BSP_USART3, str);
}

void BSP_ESP8266_InputByte(uint8_t ch)
{
	if(bsp_esp8266_rx_len < BSP_ESP8266_RX_BUF_LEN - 1){
		bsp_esp8266_rx_buf[bsp_esp8266_rx_len++] = ch;

		/* 如果收到\n表示数据接收完毕，在main函数中判断 */
		if(ch == '\n'){
			bsp_esp8266_rx_buf[bsp_esp8266_rx_len] = '\0';
			bsp_esp8266_rx_done = 1;
		}
		
	}else{
		/*缓冲区满，考虑清零或者丢弃 */
		bsp_esp8266_rx_len = 0;
	}
}

static uint8_t BSP_ESP8266_WaitFor(const char *expect, uint32_t timeout_ms)
{
	uint32_t start = BSP_GetTick();

	while((BSP_GetTick() - start) < timeout_ms){
		if(bsp_esp8266_rx_done){
			if(strstr((const char *)bsp_esp8266_rx_buf, expect) != NULL){
				bsp_esp8266_rx_done = 0;
				bsp_esp8266_rx_len = 0;
			
				return 0;
			}else{
				bsp_esp8266_rx_done = 0;
				bsp_esp8266_rx_len = 0;
			}
		}
	}
	/* 超时 */
	return -1;
}

uint8_t BSP_ESP8266_SendCmdAndWait(const char *cmd, const char *expect, uint32_t timeout_ms)
{
	uint8_t retval = 0;

	bsp_esp8266_rx_done = 0;
	bsp_esp8266_rx_len = 0;

	BSP_ESP8266_SendCmd((uint8_t *)cmd);
	retval = BSP_ESP8266_WaitFor(expect, timeout_ms);

	return retval;
}
