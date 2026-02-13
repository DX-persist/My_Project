#ifndef BSP_LED_H
#define BSP_LED_H

#include "stm32f10x.h"

/* ==== 逻辑层：给上层用 ==== */
typedef enum{
	LED_GREEN = 0,
	LED_BLUE, 
	LED_RED,
	LED_MAX			/* 必须：用于数组边界和安全 */
}bsp_led_t;

extern void BSP_LED_Init(void);
extern void BSP_LED_On(bsp_led_t led);
extern void BSP_LED_Off(bsp_led_t led);
extern void BSP_LED_Toggle(bsp_led_t led);

#endif
