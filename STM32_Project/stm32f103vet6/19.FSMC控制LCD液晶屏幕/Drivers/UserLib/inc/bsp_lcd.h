#ifndef BSP_LCD_H
#define BSP_LCD_H

#include "stm32f10x.h"
#include "bsp_fsmc.h"
#include "bsp_usart.h"
#include "bsp_delay.h"

#define LCD_BASE_ADDR	0x60000000U
#define LCD_CMD_ADDR	((volatile uint16_t *)(LCD_BASE_ADDR + (0 << 17)))
#define LCD_DATA_ADDR	((volatile uint16_t *)(LCD_BASE_ADDR + (1 << 17)))

extern void BSP_LCD_Init(void);
extern uint16_t BSP_LCD_Test(void);

#endif
