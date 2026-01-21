#include "bsp_led.h"

/* ================= 硬件抽象层 ================= */

/* 每个 LED 的物理连接描述 */
typedef struct{
	GPIO_TypeDef *port;
	uint16_t	 pin;
	uint32_t	 clk;
}bsp_led_hw_t;

/* 板级 LED 映射表（只在 BSP 内部使用） */
static const bsp_led_hw_t bsp_led_hw[LED_MAX] = {
	[LED_GREEN] = {GPIOB, GPIO_Pin_0, RCC_APB2Periph_GPIOB},
	[LED_BLUE] = {GPIOB, GPIO_Pin_1, RCC_APB2Periph_GPIOB},
	[LED_RED] = {GPIOB, GPIO_Pin_5, RCC_APB2Periph_GPIOB}
};

void BSP_LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	uint32_t clk_mask = 0;
	
	 /* 收集所有用到的 GPIO 时钟 */
	for(int i = 0; i < LED_MAX; i++){
		clk_mask |= bsp_led_hw[i].clk;
	}

	/* 使能所有用到的 GPIO 时钟 */
	RCC_APB2PeriphClockCmd(clk_mask, ENABLE);


	/* 配置所有 LED 的 GPIO 的引脚、模式、速度 */
	for(int i = 0; i < LED_MAX; i++){
		GPIO_InitStruct.GPIO_Pin = bsp_led_hw[i].pin;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(bsp_led_hw[i].port, &GPIO_InitStruct);
	}

	/* 默认全部关闭 */
	for(int i = 0; i < LED_MAX; i++){
		BSP_LED_Off((bsp_led_t)i);
	}
}

void BSP_LED_On(bsp_led_t led)
{
	/* 越界访问，直接返回 */
	if(led >= LED_MAX)	return;

	/* 低电平点亮 */
	GPIO_WriteBit(bsp_led_hw[led].port,
					bsp_led_hw[led].pin,
					Bit_RESET);
}

void BSP_LED_Off(bsp_led_t led)
{
	/* 越界访问，直接返回 */
	if(led >= LED_MAX)	return;
	
	/* 高电平熄灭 */
	GPIO_WriteBit(bsp_led_hw[led].port,
					bsp_led_hw[led].pin,
					Bit_SET);
}

void BSP_LED_Toggle(bsp_led_t led)
{
	/* 越界访问，直接返回 */
	if(led >= LED_MAX)	return;

	/* 先读取当前输出数据寄存器(ODR)里边的值 */
	BitAction state = GPIO_ReadOutputDataBit(
						bsp_led_hw[led].port, 
						bsp_led_hw[led].pin);

	/* 然后写入与当前电平相反的状态 */
	GPIO_WriteBit(bsp_led_hw[led].port,
					bsp_led_hw[led].pin,
					(state == Bit_RESET) ? Bit_SET : Bit_RESET);
}
