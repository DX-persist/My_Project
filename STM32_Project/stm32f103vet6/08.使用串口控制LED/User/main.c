#include "bsp_led.h"
#include "stm32f10x_it.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include <stdio.h>

int main(void)
{
  BSP_LED_Init();
  BSP_NVICGroup_Config();
  BSP_USART_Init(BSP_USART1);
  BSP_USART_Setdio(BSP_USART1);
  printf("08.使用串口控制LED\r\n");
  while(1)
  { 
    if(g_rx_flag == 1){
      BSP_USART_SendByte(BSP_USART1, recv_data);
      g_rx_flag = 0;
    }
    switch(recv_data){
      case '1':
        BSP_LED_On(LED_GREEN);
        BSP_LED_Off(LED_BLUE);
        BSP_LED_Off(LED_RED);
        break;
      case '2':
        BSP_LED_On(LED_BLUE);
        BSP_LED_Off(LED_GREEN);
        BSP_LED_Off(LED_RED);
        break;
      case '3':
        BSP_LED_On(LED_RED);
        BSP_LED_Off(LED_GREEN);
        BSP_LED_Off(LED_BLUE);
        break;
      default:
        BSP_LED_Off(LED_GREEN);
        BSP_LED_Off(LED_RED);
        BSP_LED_Off(LED_BLUE);
        break;
    }
  }
}