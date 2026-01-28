#include "bsp_led.h"
#include "stm32f10x_it.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include <stdio.h>

#define ARRAY_SIZE 10

int main(void)
{
  BSP_LED_Init();
  BSP_NVICGroup_Config();
  BSP_USART_Init(BSP_USART1);
  BSP_USART_Setdio(BSP_USART1);

  printf("07.使用串口接收数据\r\n");
  while(1){
    if(g_rx_flag == 1){
      //while()
      BSP_USART_SendHalfWord(BSP_USART1, recv_data);
      g_rx_flag = 0;
    }
  }
}
