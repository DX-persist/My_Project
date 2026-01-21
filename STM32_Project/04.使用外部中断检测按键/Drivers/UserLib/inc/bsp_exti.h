#ifndef BSP_EXTI_H
#define BSP_EXTI_H

#include "stm32f10x.h"

#define BSP_EXTI_PREEMPT_PRIO	3
#define BSP_EXTI_SUB_PRIO		0

typedef enum{
	BSP_KEY1_EXTI = 0,
	BSP_KEY2_EXTI,
	BSP_KEY_EXTI_MAX
}bsp_key_exti_t;

extern volatile int key_flag;

extern void BSP_NVICGroup_Config(void);
extern void BSP_KEY_EXTI_Config(void);
extern void BSP_EXTI_NVIC_Config(void);
extern void BSP_EXTI_Callback(uint32_t EXTI_Line);

#endif
