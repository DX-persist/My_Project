#ifndef BSP_ESP8266_H
#define BSP_ESP8266_H

#include "stm32f10x.h"
#include "bsp_usart.h"
#include "bsp_delay.h"
#include <string.h>

#define BSP_ESP8266_RX_BUF_LEN	256

#define BSP_ESP8266_TESTAT_CMD			"AT\r\n"
#define BSP_ESP8266_SETMODE_CMD			"AT+CWMODE=3\r\n"
#define BSP_ESP8266_CONNECT_WIFI_CMD	"AT+CWJAP=\"Ridiculous2.4g\",\"persist011104\"\r\n"
//#define BSP_ESP8266_CONNECT_TCP_CMD		"AT+CIPSTART=\"TCP\",\"192.168.10.21\",8080\r\n"
#define BSP_ESP8266_CONNECT_TCP_CMD		"AT+CIPSTART=\"TCP\",\"192.168.10.41\",8080\r\n"
#define BSP_ESP8266_SET_TRANMODE_CMD	"AT+CIPMODE=1\r\n"
#define BSP_ESP8266_SENDDATA_CMD		"AT+CIPSEND\r\n"

#define BSP_ESP8266_TESTAT_REPLY		"OK"
#define BSP_ESP8266_SETMODE_REPLY		"OK"
#define BSP_ESP8266_CONNECT_WIFI_REPLY	"OK"
#define BSP_ESP8266_CONNECT_TCP_REPLY	"OK"
#define BSP_ESP8266_SET_TRANMODE_REPLY	"OK"
#define BSP_ESP8266_SENDDATA_REPLY		"OK"

#define BSP_ESP8266_LED_GREEN_ON		"green_on"
#define BSP_ESP8266_LED_BLUE_ON			"blue_on"
#define BSP_ESP8266_LED_RED_ON			"red_on"

#define BSP_ESP8266_LED_GREEN_OFF		"green_off"
#define BSP_ESP8266_LED_BLUE_OFF		"blue_off"
#define BSP_ESP8266_LED_RED_OFF			"red_off"
#define BSP_ESP8266_LED_ALL_OFF			"all_off"


typedef enum{
	BSP_ESP8266_CHPD = 0,			/* 配置ESP8266的使能引脚 */
	BSP_ESP8266_RST,				/* 配置ESP8266的复位引脚 */
	BSP_ESP8266_MAX
}bsp_esp8266_t;

extern volatile uint16_t bsp_esp8266_rx_len;
extern volatile uint16_t bsp_esp8266_rx_done;
extern volatile uint8_t bsp_esp8266_rx_buf[BSP_ESP8266_RX_BUF_LEN];

extern void BSP_ESP8266_Init(void);
extern void BSP_ESP8266_SendArray(volatile uint8_t *array, uint16_t size);
extern void BSP_ESP8266_SendCmd(uint8_t *str);
extern void BSP_ESP8266_InputByte(uint8_t ch);
extern uint8_t BSP_ESP8266_SendCmdAndWait(const char *cmd, const char *expect, uint32_t timeout_ms);

#endif
