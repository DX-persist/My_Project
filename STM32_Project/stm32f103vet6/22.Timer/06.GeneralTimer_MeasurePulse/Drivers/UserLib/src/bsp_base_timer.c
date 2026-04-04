#include "bsp_base_timer.h"

typedef struct{
	TIM_TypeDef *timer;
	uint32_t rcc_clk;

	uint8_t irq_update_channel;
	uint8_t irq_pre_prio;
	uint8_t irq_sub_prio;
}bsp_basetimer_hw_t;

volatile uint16_t basetimer_cnt[BSP_BASE_TIMER_MAX] = {0};

static const bsp_basetimer_hw_t bsp_basetimer_hw[BSP_BASE_TIMER_MAX] = {
	[BSP_BASE_TIMER6] = {
		.timer = TIM6,
		.rcc_clk = RCC_APB1Periph_TIM6,

		.irq_update_channel = TIM6_IRQn,
		.irq_pre_prio 		= PREEMPT_PRIO,
		.irq_sub_prio		= SUB_PRIO,
	},

	[BSP_BASE_TIMER7] = {
		.timer = TIM7,
		.rcc_clk = RCC_APB1Periph_TIM7,

		.irq_update_channel	= TIM7_IRQn,
		.irq_pre_prio 		= PREEMPT_PRIO,
		.irq_sub_prio		= SUB_PRIO,
	}
};

static uint16_t BSP_TIM_ConvertCounterMode(bsp_basetimer_counter_mode_t counter_mode)
{
	switch(counter_mode){
		case BSP_TIM_COUNTER_MODE_UP: return TIM_CounterMode_Up;
		case BSP_TIM_COUNTER_MODE_DOWN:	return TIM_CounterMode_Down;
		case BSP_TIM_COUNTER_MODE_CENTER_ALIGNED1:	return TIM_CounterMode_CenterAligned1;
		case BSP_TIM_COUNTER_MODE_CENTER_ALIGNED2:	return TIM_CounterMode_CenterAligned2;
		case BSP_TIM_COUNTER_MODE_CENTER_ALIGNED3:	return TIM_CounterMode_CenterAligned3;
		default:	return 0;
	}
}

static uint16_t BSP_TIM_ConvertClockDivision(bsp_basetimer_clock_div_t clock_div)
{
	switch(clock_div){
		case BSP_TIM_CLOCK_DIV_1: 	return TIM_CKD_DIV1;
		case BSP_TIM_CLOCK_DIV_2:	return TIM_CKD_DIV2;
		case BSP_TIM_CLOCK_DIV_4:	return TIM_CKD_DIV4;
		default:	return 0;
	}
}

static void BSP_TIM_NVIC_Config(bsp_basetimer_t timer_id)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	const bsp_basetimer_hw_t *hw = &bsp_basetimer_hw[timer_id];

	NVIC_InitStruct.NVIC_IRQChannel = hw->irq_update_channel;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = hw->irq_pre_prio;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = hw->irq_sub_prio;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

void BSP_BaseTIM_Init(bsp_basetimer_t timer_id, bsp_basetimer_config_t *config)
{
	if(timer_id >= BSP_BASE_TIMER_MAX)	return;

	const bsp_basetimer_hw_t *hw = &bsp_basetimer_hw[timer_id];
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	
	RCC_APB1PeriphClockCmd(hw->rcc_clk, ENABLE);	/**< 开启TIMx的时钟 */
	/* 初始化 TIM_BaseInitTypeDef 结构体 */
	TIM_TimeBaseInitStruct.TIM_Prescaler = config->prescaler;
	TIM_TimeBaseInitStruct.TIM_CounterMode = BSP_TIM_ConvertCounterMode(config->counter_mode);
	TIM_TimeBaseInitStruct.TIM_Period = config->period;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = BSP_TIM_ConvertClockDivision(config->clock_div);
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = config->repetition_cnt;
	TIM_TimeBaseInit(hw->timer, &TIM_TimeBaseInitStruct);
	
	/* 启用Prescaler预装载功能 */
	TIM_ARRPreloadConfig(hw->timer, ENABLE);

	/* 这里需要清空TIM_IT_Update标志位,因为 ARR 和 Prescaler
	 * 都具有预装载功能，所以硬件会自动发生一次更新事件将数据
	 * 写入到寄存器中
	 **/
	TIM_ClearFlag(hw->timer, TIM_IT_Update);		/**< 清空TIM_IT_Update标志位 */

	/* 配置中断触发源和设置中断优先级 */
	TIM_ITConfig(hw->timer, TIM_IT_Update, ENABLE);
	BSP_TIM_NVIC_Config(timer_id);

	TIM_Cmd(hw->timer, ENABLE);
}

void BSP_TIM_IRQHandler(bsp_basetimer_t timer_id)
{
	if(timer_id >= BSP_BASE_TIMER_MAX)	return;
	
	const bsp_basetimer_hw_t *hw = &bsp_basetimer_hw[timer_id];

	if(TIM_GetITStatus(hw->timer, TIM_IT_Update) != RESET){
		basetimer_cnt[timer_id]++;
		TIM_ClearITPendingBit(hw->timer, TIM_IT_Update);
	}
}
