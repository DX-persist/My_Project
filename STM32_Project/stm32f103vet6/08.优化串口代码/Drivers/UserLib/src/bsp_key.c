#include "bsp_key.h"

typedef struct{
	GPIO_TypeDef *port;
	uint16_t pin;
	uint32_t clk;
	GPIOMode_TypeDef mode;
	uint8_t active_level;
}bsp_key_hw_t;

static const bsp_key_hw_t bsp_key_hw[KEY_MAX] = {
	[KEY1] = {GPIOA, GPIO_Pin_0, RCC_APB2Periph_GPIOA, GPIO_Mode_IN_FLOATING, Bit_SET},
	[KEY2] = {GPIOC, GPIO_Pin_13, RCC_APB2Periph_GPIOC, GPIO_Mode_IN_FLOATING, Bit_SET}
};

void BSP_KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	uint32_t clk_mask = 0;
	
	for(int i = 0; i < KEY_MAX; i++){
		clk_mask |= bsp_key_hw[i].clk;
	}
	RCC_APB2PeriphClockCmd(clk_mask, ENABLE);

	for(int i = 0; i < KEY_MAX; i++){
		GPIO_InitStruct.GPIO_Pin = bsp_key_hw[i].pin;
		GPIO_InitStruct.GPIO_Mode = bsp_key_hw[i].mode;

		GPIO_Init(bsp_key_hw[i].port, &GPIO_InitStruct);
	}
}

uint8_t BSP_KEY_Scan(bsp_key_t key)
{
    static uint8_t last_state[KEY_MAX] = {0};

    uint8_t level = GPIO_ReadInputDataBit(
                        bsp_key_hw[key].port,
                        bsp_key_hw[key].pin);

    uint8_t now = (level == bsp_key_hw[key].active_level);

    if(now && !last_state[key])
    {
        last_state[key] = 1;
        return BSP_KEY_PRESSED;   // 只在上升沿返回
    }
    else if(!now)
    {
        last_state[key] = 0;
    }

    return BSP_KEY_RELEASED;
}

