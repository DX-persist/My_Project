#include "bsp_led.h"
#include "stm32f10x_it.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include <stdio.h>

#define ARRAY_SIZE 10

int main(void)
{
    //uint8_t array2[ARRAY_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    uint16_t recv_data = 0x00;
    uint8_t rx_buf[10] = {0};
    BSP_LED_Init();
    BSP_USART_Init(BSP_UART5);
    BSP_USART_Setdio(BSP_UART5);
    
    printf("07.使用串口接收数据\r\n");
    while(1){ 
      BSP_USART_ReceiveString(BSP_UART5, rx_buf, 4);
      BSP_USART_SendArray(BSP_UART5, rx_buf, 10);
      // recv_data = BSP_USART_ReceiveByte(BSP_UART5);
      // BSP_USART_SendByte(BSP_UART5, recv_data);
      //BSP_Delay_ms(500);
    }
}
