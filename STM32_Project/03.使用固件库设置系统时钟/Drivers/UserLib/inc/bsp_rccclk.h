#ifndef BSP_RCCCLK_H
#define BSP_RCCCLK_H

#include "stm32f10x.h"

typedef enum{
	BSP_HSE_16MHz = 2,
	BSP_HSE_24MHz,
	BSP_HSE_32MHz,
	BSP_HSE_40MHz,
	BSP_HSE_48MHz,
	BSP_HSE_56MHz,
	BSP_HSE_64MHz,
	BSP_HSE_72MHz,
	BSP_HSE_80MHz,
	BSP_HSE_88MHz,
	BSP_HSE_96MHz,
	BSP_HSE_104MHz,
	BSP_HSE_112MHz,
	BSP_HSE_120MHz,
	BSP_HSE_128MHz,
	BSP_HSE_MAX
}bsp_hse_freq_t;

extern void HSE_SetSysClock(uint32_t RCC_PLLMul);
extern void MCO_GPIO_Config(void);
extern void HSI_SetSysClock(uint32_t RCC_PLLMul);
#endif
