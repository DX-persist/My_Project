#ifndef BSP_LED_H
#define BSP_LED_H

#include "stm32f10x.h"

typedef enum{
	LED_BLUE = 0,
	LED_ORANGE,
	LED_MAX
}bsp_led_t;

extern void BSP_LED_Init(void);
extern void BSP_LED_On(bsp_led_t led);
extern void BSP_LED_Off(bsp_led_t led);
extern void BSP_LED_Toggle(bsp_led_t led);

#endif
