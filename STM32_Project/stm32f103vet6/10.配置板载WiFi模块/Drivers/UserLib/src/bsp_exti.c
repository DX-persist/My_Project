#include "bsp_exti.h"

volatile int key_flag = 0;

typedef struct{
	/* GPIO相关配置 */
	GPIO_TypeDef *port;
	uint16_t pin;
	uint32_t clk;
	GPIOMode_TypeDef mode;

	/* 外部中断配置端口和引脚配置 */
	uint8_t GPIO_PortSource;
	uint8_t GPIO_PinSource;

	/* 外部中断配置线、模式、触发方式、是否启用配置 */
	uint32_t EXTI_LINE;
	EXTIMode_TypeDef EXTI_Mode;
	EXTITrigger_TypeDef EXTI_Trigger;
	FunctionalState EXTI_LineCmd;

	/* 配置嵌套向量中断控制器 */
	uint8_t NVIC_IRQChannel;
	uint8_t NVIC_IRQChannelPreemptionPriority;
	uint8_t NVIC_IRQChannelSubPriority; 
	FunctionalState NVIC_IRQChannelCmd;   
}bsp_key_exti_hw_t;

static const bsp_key_exti_hw_t bsp_key_exti_hw[BSP_KEY_EXTI_MAX] = {
	[BSP_KEY1_EXTI] = {GPIOA, GPIO_Pin_0, RCC_APB2Periph_GPIOA, GPIO_Mode_IN_FLOATING, 
		GPIO_PortSourceGPIOA, GPIO_PinSource0, 
		EXTI_Line0, EXTI_Mode_Interrupt, EXTI_Trigger_Rising, ENABLE, 
		EXTI0_IRQn, BSP_EXTI_PREEMPT_PRIO, BSP_EXTI_SUB_PRIO, ENABLE},
	
	[BSP_KEY2_EXTI] = {GPIOC, GPIO_Pin_13, RCC_APB2Periph_GPIOC, GPIO_Mode_IN_FLOATING, 
		GPIO_PortSourceGPIOC, GPIO_PinSource13, 
		EXTI_Line13, EXTI_Mode_Interrupt, EXTI_Trigger_Falling, ENABLE, 
		EXTI15_10_IRQn, BSP_EXTI_PREEMPT_PRIO, BSP_EXTI_SUB_PRIO, ENABLE}	
};

void BSP_NVICGroup_Config(void)
{
	/* 配置优先级组：抢占优先级和响应优先级 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

void BSP_KEY_EXTI_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	
	GPIO_StructInit(&GPIO_InitStruct);
	EXTI_StructInit(&EXTI_InitStruct);

	uint32_t clk_mask = 0;

	for(int i = 0; i < BSP_KEY_EXTI_MAX; i++){
		clk_mask |= bsp_key_exti_hw[i].clk;	
	}
	RCC_APB2PeriphClockCmd(clk_mask, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	for(int i = 0; i < BSP_KEY_EXTI_MAX; i++){
		GPIO_InitStruct.GPIO_Pin = bsp_key_exti_hw[i].pin;
		GPIO_InitStruct.GPIO_Mode = bsp_key_exti_hw[i].mode;

		GPIO_Init(bsp_key_exti_hw[i].port, &GPIO_InitStruct);
		GPIO_EXTILineConfig(bsp_key_exti_hw[i].GPIO_PortSource, bsp_key_exti_hw[i].GPIO_PinSource);

		EXTI_InitStruct.EXTI_Line = bsp_key_exti_hw[i].EXTI_LINE;
		EXTI_InitStruct.EXTI_Mode = bsp_key_exti_hw[i].EXTI_Mode;
		EXTI_InitStruct.EXTI_Trigger = bsp_key_exti_hw[i].EXTI_Trigger;
		EXTI_InitStruct.EXTI_LineCmd = bsp_key_exti_hw[i].EXTI_LineCmd;

		EXTI_Init(&EXTI_InitStruct);
		/* 防止在EXTI Line在配置前已经有电平变化导致EXTI_PR标志位已经置位 */
		EXTI_ClearITPendingBit(bsp_key_exti_hw[i].EXTI_LINE);
	}
}

void BSP_EXTI_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;

	for(int i = 0; i < BSP_KEY_EXTI_MAX; i++){
		NVIC_InitStruct.NVIC_IRQChannel = bsp_key_exti_hw[i].NVIC_IRQChannel;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority =  bsp_key_exti_hw[i].NVIC_IRQChannelPreemptionPriority;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = bsp_key_exti_hw[i].NVIC_IRQChannelSubPriority;
		NVIC_InitStruct.NVIC_IRQChannelCmd = bsp_key_exti_hw[i].NVIC_IRQChannelCmd;

		NVIC_Init(&NVIC_InitStruct);
	}
}

void BSP_EXTI_Callback(uint32_t EXTI_Line)
{
	switch(EXTI_Line){
		case EXTI_Line0:
			key_flag = 1;
			break;
		case EXTI_Line13:
			key_flag = 2;
			break;

		default:
			break;
	}
}
