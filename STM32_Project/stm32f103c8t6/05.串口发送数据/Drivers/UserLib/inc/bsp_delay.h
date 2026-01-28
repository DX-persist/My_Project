#ifndef BSP_DELAY_H
#define BSP_DELAY_H

#include "stm32f10x.h"

extern void BSP_TimeBase_Init(void);
extern uint32_t BSP_GetTick(void);
extern void BSP_Delay_us(uint32_t us);
extern void BSP_Delay_ms(uint32_t ms);
extern void BSP_Delay_s(uint32_t s);

#endif
