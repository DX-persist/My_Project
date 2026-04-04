#ifndef BSP_GENERAL_TIMER_H
#define BSP_GENERAL_TIMER_H

#include "stm32f10x.h"

#define PREEMPT_PRIO	2
#define SUB_PRIO		2

typedef enum{
	BSP_GENERAL_TIMER2 = 0,
	BSP_GENERAL_TIMER3,
	BSP_GENERAL_TIMER4,
	BSP_GENERAL_TIMER5,
	BSP_GENERAL_TIMER_MAX	
}bsp_generaltimer_t;

typedef enum{
    BSP_TIM_COUNTER_MODE_UP = 0,    
    BSP_TIM_COUNTER_MODE_DOWN,
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED1,
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED2,
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED3
}bsp_generaltimer_counter_mode_t;

typedef enum{
    BSP_TIM_CLOCK_DIV_1 = 0,
    BSP_TIM_CLOCK_DIV_2,
    BSP_TIM_CLOCK_DIV_4
}bsp_generaltimer_clock_div_t;

typedef struct{
    uint16_t prescaler;
    bsp_generaltimer_counter_mode_t counter_mode;
    uint16_t period;
    bsp_generaltimer_clock_div_t clock_div;
    uint8_t repetition_cnt;
}bsp_generaltimer_config_t;

extern volatile uint16_t generaltimer_cnt[BSP_GENERAL_TIMER_MAX];

extern void BSP_BaseTIM_Init(bsp_generaltimer_t timer_id, bsp_generaltimer_config_t *config);
extern void BSP_TIM_IRQHandler(bsp_generaltimer_t timer_id);

#endif
