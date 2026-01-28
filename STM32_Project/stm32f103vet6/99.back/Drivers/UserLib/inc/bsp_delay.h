#ifndef BSP_DELAY_H
#define BSP_DELAY_H

#include "stm32f10x.h"

/* 使用宏定义提高可读性 */
#define SYSTICK_ENABLE      (1ul << 0)
#define SYSTICK_TICKINT     (1ul << 1)
#define SYSTICK_CLKSOURCE   (1ul << 2)
#define SYSTICK_COUNTFLAG   (1ul << 16)
#define SYSTICK_DISABLE     (0ul << 0)

extern void BSP_Delay_us(uint32_t us);
extern void BSP_Delay_ms(uint32_t ms);
extern void BSP_Delay_s(uint32_t s);

#endif
