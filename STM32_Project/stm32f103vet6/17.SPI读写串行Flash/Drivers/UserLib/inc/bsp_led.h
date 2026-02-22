#ifndef BSP_LED_H
#define BSP_LED_H

#include "stm32f10x.h"

#define BSP_LED_GREEN_ON	"green_on"
#define BSP_LED_GREEN_OFF	"green_off"

#define BSP_LED_BLUE_ON		"blue_on"
#define BSP_LED_BLUE_OFF	"blue_off"

#define BSP_LED_RED_ON		"red_on"
#define BSP_LED_RED_OFF		"red_off"

#define BSP_LED_ALL_ON		"all_on"
#define BSP_LED_ALL_OFF		"all_off"

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
