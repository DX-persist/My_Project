#ifndef BSP_RCCCLK_H
#define BSP_RCCCLK_H

#include "stm32f10x.h"

extern void HSE_SetSysClock(uint32_t RCC_PLLMul);
extern void MCO_GPIO_Config(void);
extern void HSI_SetSysClock(uint32_t RCC_PLLMul);

#endif

