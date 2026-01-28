#include "bsp_led.h"
#include "stm32f10x_it.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_esp8266.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
  BSP_TimeBase_Init();
  BSP_LED_Init();
  BSP_ESP8266_Init();
  // BSP_NVICGroup_Config();
  // BSP_USART_Init(BSP_USART1);
  // BSP_USART_Setdio(BSP_USART1);
  //printf("09.配置板载WIFI模块\r\n");
  BSP_ESP8266_SendCmd((uint8_t *)"09.配置板载WIFI模块\r\n");

  if(BSP_ESP8266_SendCmdAndWait(BSP_ESP8266_TESTAT_CMD, BSP_ESP8266_TESTAT_REPLY, 2000) == 0){
    BSP_LED_On(LED_GREEN);
  }else{
    BSP_LED_Off(LED_GREEN);
  }

  if(BSP_ESP8266_SendCmdAndWait(BSP_ESP8266_SETMODE_CMD, BSP_ESP8266_SETMODE_REPLY, 2000) == 0){
    BSP_LED_On(LED_BLUE);
    BSP_LED_Off(LED_GREEN);
  }else{
    BSP_LED_Off(LED_BLUE);
    BSP_LED_Off(LED_GREEN);
  }

  if(BSP_ESP8266_SendCmdAndWait(BSP_ESP8266_CONNECT_WIFI_CMD, BSP_ESP8266_CONNECT_WIFI_REPLY, 15000) == 0){
    BSP_LED_On(LED_RED);
    BSP_LED_Off(LED_GREEN);
    BSP_LED_Off(LED_BLUE);
  }else{
    BSP_LED_Off(LED_RED);
    BSP_LED_Off(LED_GREEN);
    BSP_LED_Off(LED_BLUE);
  }

  if(BSP_ESP8266_SendCmdAndWait(BSP_ESP8266_CONNECT_TCP_CMD, BSP_ESP8266_CONNECT_TCP_REPLY, 2000) == 0){
    BSP_LED_On(LED_RED);
    BSP_LED_On(LED_GREEN);
    BSP_LED_Off(LED_BLUE);
  }else{
    BSP_LED_Off(LED_RED);
    BSP_LED_Off(LED_GREEN);
    BSP_LED_Off(LED_BLUE);
  }

  if(BSP_ESP8266_SendCmdAndWait(BSP_ESP8266_SET_TRANMODE_CMD, BSP_ESP8266_SET_TRANMODE_REPLY, 2000) == 0){
    BSP_LED_On(LED_RED);
    BSP_LED_On(LED_BLUE);
    BSP_LED_Off(LED_GREEN);
  }else{
    BSP_LED_Off(LED_RED);
    BSP_LED_Off(LED_GREEN);
    BSP_LED_Off(LED_BLUE);
  }

  if(BSP_ESP8266_SendCmdAndWait(BSP_ESP8266_SENDDATA_CMD, BSP_ESP8266_SENDDATA_REPLY, 2000) == 0){
    BSP_LED_Off(LED_RED);
    BSP_LED_Off(LED_GREEN);
    BSP_LED_Off(LED_BLUE);
  }else{
    BSP_LED_On(LED_RED);
  }

  BSP_ESP8266_SendCmd((uint8_t *)"Successfully connect to the server\r\n");

  while(1){
    #if 0
    if(bsp_esp8266_rx_done){
      BSP_ESP8266_SendArray(bsp_esp8266_rx_buf, bsp_esp8266_rx_len);
      bsp_esp8266_rx_done = 0;
      bsp_esp8266_rx_len = 0;
    }
    #endif
  }
}