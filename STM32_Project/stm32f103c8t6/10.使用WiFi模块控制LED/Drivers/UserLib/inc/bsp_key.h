#ifndef BSP_KEY_H
#define BSP_KEY_H

#include "stm32f10x.h"

typedef enum{
	KEY1 = 0,
	KEY2,
	KEY_MAX
}bsp_key_t;

typedef enum{
	BSP_KEY_RELEASED = 0,
	BSP_KEY_PRESSED
}bsp_key_state_t;

extern void BSP_KEY_Init(void);
extern uint8_t BSP_KEY_Scan(bsp_key_t key);

#endif
