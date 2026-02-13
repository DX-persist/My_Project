#include "bsp_exti.h"

typedef struct{
	uint32_t				rcc_clk;
	GPIO_TypeDef 			*gpio_port;
	uint16_t				gpio_pin;
	GPIOMode_TypeDef 		gpio_mode;

	uint8_t					gpio_portsource;
	uint8_t					gpio_pinsource;

	uint32_t				exti_line;
	EXTIMode_TypeDef		exti_mode;
	EXTITrigger_TypeDef		exti_trigger;

	uint8_t					irq_channel;
	uint8_t					irq_preempt_prio;
	uint8_t					irq_sub_prio;

}bsp_exti_hw_t;

volatile uint8_t key_exti_flag = 0;

/* 定义按键KEY所在的GPIO端口、引脚、模式、RCC时钟、 */
static const bsp_exti_hw_t bsp_exti_hw[BSP_KEY_MAX] = {
	[BSP_KEY1_EXTI] = {
		.rcc_clk   		  = RCC_APB2Periph_GPIOA,
		.gpio_port 		  = GPIOA,
		.gpio_pin  		  = GPIO_Pin_0,
		.gpio_mode 		  = GPIO_Mode_IPU,

		.gpio_portsource  = GPIO_PortSourceGPIOA,
		.gpio_pinsource	  = GPIO_PinSource0,

		.exti_line		  = EXTI_Line0,
		.exti_mode		  = EXTI_Mode_Interrupt,
		.exti_trigger	  = EXTI_Trigger_Falling,

		.irq_channel	  =	EXTI0_IRQn,
	    .irq_preempt_prio = BSP_EXTI_PREEMPT_PRIO,
		.irq_sub_prio	  = BSP_EXTI_SUB_PRIO,
	},
	[BSP_KEY2_EXTI] = {
		.rcc_clk   		  = RCC_APB2Periph_GPIOA,
		.gpio_port 		  = GPIOA,
		.gpio_pin  		  = GPIO_Pin_1,
		.gpio_mode 		  = GPIO_Mode_IPU,

		.gpio_portsource  = GPIO_PortSourceGPIOA,
		.gpio_pinsource	  = GPIO_PinSource1,

		.exti_line		  = EXTI_Line1,
		.exti_mode		  = EXTI_Mode_Interrupt,
		.exti_trigger	  = EXTI_Trigger_Falling,

		.irq_channel	  =	EXTI1_IRQn,
	    .irq_preempt_prio = BSP_EXTI_PREEMPT_PRIO,
		.irq_sub_prio	  = BSP_EXTI_SUB_PRIO,
	}
};

static void BSP_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	uint32_t clk_mask = 0;

	/* 初始化GPIO_InitStruct 结构体 */
	GPIO_StructInit(&GPIO_InitStruct);

	/* 使能按键所在端口的时钟 */
	for(int i = 0; i < BSP_KEY_MAX; i++){
		clk_mask |= bsp_exti_hw[i].rcc_clk;
	}
	RCC_APB2PeriphClockCmd(clk_mask, ENABLE);
	/* 使能AFIO的时钟，AFIO负责把某个GPIO引脚映射到某一条线路(必须使能) */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* 配置按键所在引脚的参数 */
	for(int i = 0; i < BSP_KEY_MAX; i++){
		GPIO_InitStruct.GPIO_Pin = bsp_exti_hw[i].gpio_pin;
		GPIO_InitStruct.GPIO_Mode = bsp_exti_hw[i].gpio_mode;

		GPIO_Init(bsp_exti_hw[i].gpio_port, &GPIO_InitStruct);
	}

	/* 配置用作外部中断线的GPIO端口和引脚 */
	for(int i = 0; i < BSP_KEY_MAX; i++){
		GPIO_EXTILineConfig(bsp_exti_hw[i].gpio_portsource, bsp_exti_hw[i].gpio_pinsource);
	}
}

static void BSP_EXTI_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStruct;

	/* 初始化EXTI_InitStruct结构体 */
	EXTI_StructInit(&EXTI_InitStruct);

	for(int i = 0; i < BSP_KEY_MAX; i++){
		EXTI_InitStruct.EXTI_Line 	 = bsp_exti_hw[i].exti_line;
		EXTI_InitStruct.EXTI_Mode 	 = bsp_exti_hw[i].exti_mode;
		EXTI_InitStruct.EXTI_Trigger = bsp_exti_hw[i].exti_trigger;
		EXTI_InitStruct.EXTI_LineCmd = ENABLE;

		EXTI_Init(&EXTI_InitStruct);
	}
}

static void BSP_EXTI_PriorityGroupConfig(void)
{
	/* 配置中断优先级组为组2 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

static void BSP_EXTI_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStruct;

	for(int i = 0; i < BSP_KEY_MAX; i++){
		NVIC_InitStruct.NVIC_IRQChannel 					= bsp_exti_hw[i].irq_channel;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority  	= bsp_exti_hw[i].irq_preempt_prio;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority 			= bsp_exti_hw[i].irq_sub_prio;
		NVIC_InitStruct.NVIC_IRQChannelCmd 					= ENABLE;

		NVIC_Init(&NVIC_InitStruct);
	}
}

void BSP_EXTI_Config(void)
{
	BSP_GPIO_Init();	
	BSP_EXTI_PriorityGroupConfig();
	BSP_EXTI_NVIC_Config();
	BSP_EXTI_Init();
}

void BSP_EXTI_CommandHandler(uint32_t EXTI_Line)
{
	/* 判断指定的中断线是否出发，若触发执行响应的逻辑关系 */
	if(EXTI_GetITStatus(EXTI_Line) == SET){
		switch(EXTI_Line){
			case EXTI_Line0:
				key_exti_flag = 1;
				break;
			case EXTI_Line1:
				key_exti_flag = 2;
				break;
		}
		/* 清空触发中断的中断线，防止下一次循环来的时候该标志位仍然置位导致再次进入中断 */
		EXTI_ClearITPendingBit(EXTI_Line);
	}
}
