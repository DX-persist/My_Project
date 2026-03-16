#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_lcd.h"
#include "bsp_xpt2046.h"

#include <stdio.h>
#include <string.h>               

int main(void)
{
	uint16_t x_adc = 0;
	uint16_t y_adc = 0;
	uint16_t x_lcd = 0;
	uint16_t y_lcd = 0;

	BSP_LED_Init();
	BSP_TimeBase_Init();
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);
	
	
	printf("21.xpt2046触摸屏\r\n");
	//BSP_LCD_Test_Demo();
	BSP_LCD_Init();
	BSP_XPT2046_Init();
	BSP_LCD_Clear(WHITE);
	Touch_Calibrate();

	while(1){	
    if(BSP_XPT2046_IsPressed())
    {
        BSP_Delay_ms(20);

        if(BSP_XPT2046_IsPressed())
        {
            BSP_XPT2046_GetADC(&x_adc, &y_adc);
            Touch_Convert(x_adc, y_adc, &x_lcd, &y_lcd);
			BSP_LCD_DrawCircle(x_lcd, y_lcd, 5, RED, 1);
            printf("ADC:%d %d  LCD:%d %d\r\n", x_adc, y_adc, x_lcd, y_lcd);

            while(BSP_XPT2046_IsPressed());
        }
    }
}
}
