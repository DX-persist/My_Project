#ifndef BSP_LCD_H
#define BSP_LCD_H

#include "stm32f10x.h"
#include "bsp_fsmc.h"
#include "bsp_usart.h"
#include "bsp_delay.h"

#define ILI9341_LCD_ID 0x009341

#define LCD_BASE_ADDR	0x60000000U
#define LCD_CMD_ADDR	((volatile uint16_t *)(LCD_BASE_ADDR + (0 << 17)))
#define LCD_DATA_ADDR	((volatile uint16_t *)(LCD_BASE_ADDR + (1 << 17)))

/* 获取 RGB565 格式的颜色宏 (R,G,B 取值均为 0~255) */
#define RGB888_TO_RGB565(r, g, b)  ((((uint16_t)(r) >> 3) << 11) | (((uint16_t)(g) >> 2) << 5) | ((uint16_t)(b) >> 3))
#define WHITE   RGB888_TO_RGB565(255, 255, 255)  // 白色
#define BLACK   RGB888_TO_RGB565(0,   0,   0)    // 黑色
#define RED     RGB888_TO_RGB565(255, 0,   0)    // 红色
#define GREEN   RGB888_TO_RGB565(0,   255, 0)    // 绿色
#define BLUE    RGB888_TO_RGB565(0,   0,   255)  // 蓝色
#define YELLOW  RGB888_TO_RGB565(255, 255, 0)    // 黄色

#define LCD_WIDTH      240
#define LCD_HEIGHT     320

#define LCD_SET_COL_CMD		0x2A
#define LCD_SET_PAGE_CMD	0x2B
#define LCD_SET_MEM_CMD		0x2C

extern void BSP_LCD_Init(void);
extern void BSP_LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
extern void BSP_LCD_Clear(uint16_t color);
extern void BSP_LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
extern void BSP_LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
extern void BSP_LCD_DrawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);
extern void BSP_LCD_Test_Demo(void);

#endif
