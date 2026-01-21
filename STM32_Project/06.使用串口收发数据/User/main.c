//#include "bsp_led.h"
#include "stm32f10x_it.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include <stdio.h>

#define ARRAY_SIZE 10

int main(void)
{
    //uint8_t array2[ARRAY_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    BSP_LED_Init();
    BSP_USART_Init(BSP_USART1);
    BSP_USART_Setdio(BSP_USART1);
    
    while(1){
		BSP_LED_Off(LED_BLUE | LED_GREEN);
        printf("哈喽\r\n");
		BSP_Delay_ms(1000);
    }
}
