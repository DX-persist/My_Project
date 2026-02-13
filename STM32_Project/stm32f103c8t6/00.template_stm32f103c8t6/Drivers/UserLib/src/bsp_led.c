#include "bsp_led.h"

typedef struct{
	GPIO_TypeDef *port;
	uint16_t pin;
	uint32_t clk;
}bsp_led_hw_t;

static const bsp_led_hw_t bsp_led_hw[LED_MAX] = {
	[LED_BLUE] = {GPIOB, GPIO_Pin_8, RCC_APB2Periph_GPIOB},
	[LED_ORANGE] = {GPIOB, GPIO_Pin_9, RCC_APB2Periph_GPIOB}
};

void BSP_LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	uint32_t clk_mask = 0;

	for(int i = 0; i < LED_MAX; i++){
		clk_mask |= bsp_led_hw[i].clk;
	}
	RCC_APB2PeriphClockCmd(clk_mask, ENABLE);

	for(int i = 0; i < LED_MAX; i++){
		GPIO_InitStruct.GPIO_Pin = bsp_led_hw[i].pin;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(bsp_led_hw[i].port, &GPIO_InitStruct);
	}

	//关闭所有的LED灯
	BSP_LED_Off(LED_BLUE);
	BSP_LED_Off(LED_ORANGE);
}

void BSP_LED_On(bsp_led_t led)
{
	if(led > LED_MAX)	return;

	GPIO_WriteBit(bsp_led_hw[led].port,
					bsp_led_hw[led].pin,
					Bit_RESET);
}

void BSP_LED_Off(bsp_led_t led)
{
	if(led > LED_MAX)	return;

	GPIO_WriteBit(bsp_led_hw[led].port,
					bsp_led_hw[led].pin,
					Bit_SET);
}

void BSP_LED_Toggle(bsp_led_t led)
{
	if(led > LED_MAX)	return;
	
	if(GPIO_ReadOutputDataBit(bsp_led_hw[led].port, bsp_led_hw[led].pin) == Bit_RESET){
		BSP_LED_Off(led);
	}else{
		BSP_LED_On(led);
	}
}
