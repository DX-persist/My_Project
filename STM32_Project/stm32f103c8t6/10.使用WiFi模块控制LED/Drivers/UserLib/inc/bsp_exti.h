#ifndef BSP_EXTI_H
#define BSP_EXTI_H

#include "stm32f10x.h"

#define BSP_EXTI_PREEMPT_PRIO	2
#define BSP_EXTI_SUB_PRIO		2

typedef enum{
	BSP_KEY1_EXTI = 0,
	BSP_KEY2_EXTI,
	BSP_KEY_MAX
}bsp_exti_t;

extern volatile uint8_t key_exti_flag;

extern void BSP_EXTI_Config(void);
extern void BSP_EXTI_CommandHandler(uint32_t EXTI_Line);

#endif
