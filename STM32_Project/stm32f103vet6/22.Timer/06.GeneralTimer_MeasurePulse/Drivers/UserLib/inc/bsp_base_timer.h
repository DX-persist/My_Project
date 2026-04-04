#ifndef BSP_BASE_TIMER_H
#define BSP_BASE_TIMER_H

#include "stm32f10x.h"

#define PREEMPT_PRIO	2
#define SUB_PRIO		2

typedef enum{
	BSP_BASE_TIMER6 = 0,
	BSP_BASE_TIMER7,
	BSP_BASE_TIMER_MAX
}bsp_basetimer_t;

typedef enum{
	BSP_TIM_COUNTER_MODE_UP = 0,	
    BSP_TIM_COUNTER_MODE_DOWN,
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED1,
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED2,
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED3
}bsp_basetimer_counter_mode_t;

typedef enum{
	BSP_TIM_CLOCK_DIV_1 = 0,
    BSP_TIM_CLOCK_DIV_2,
    BSP_TIM_CLOCK_DIV_4
}bsp_basetimer_clock_div_t;

typedef struct{
	uint16_t prescaler;
	bsp_basetimer_counter_mode_t counter_mode;
	uint16_t period;
	bsp_basetimer_clock_div_t clock_div;
	uint8_t repetition_cnt;
}bsp_basetimer_config_t;

extern volatile uint16_t basetimer_cnt[BSP_BASE_TIMER_MAX]; 

extern void BSP_BaseTIM_Init(bsp_basetimer_t timer_id, bsp_basetimer_config_t *config);
extern void BSP_TIM_IRQHandler(bsp_basetimer_t timer_id); 

#endif
