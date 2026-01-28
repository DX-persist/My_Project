#ifndef BSP_ESP8266_H
#define BSP_ESP8266_H

#include "stm32f10x.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_led.h"

#include <stdio.h>
#include <stdbool.h>

/* 定义 ESP8266 模块当前模式*/
#define FLASH_MODE	0
#define NORMAL_MODE	1

/* 定义 ESP8266 模块使用的串口 */
#define ESP8266_USART_ID				BSP_USART3
/* 定义 ESP8266 模块调试使用的串口 */
#define ESP8266_DEBUG_USART_ID			BSP_USART1

/* 定义 ESP8266 相关的 AT 指令 */
#define ESP8266_TEST_CMD				"AT"
#define ESP8266_CLOSE_ATE_CMD			"ATE0"
#define	ESP8266_SET_MIXED_MODE_CMD		"AT+CWMODE=3"
#define ESP8266_CONNECT_WIFI_CMD		"AT+CWJAP"
#define ESP8266_CONNECT_TCP_CMD			"AT+CIPSTART"
#define ESP8266_SET_TRAN_MODE_CMD		"AT+CIPMODE=1"
#define ESP8266_SEND_MSG_CMD			"AT+CIPSEND"
#define ESP8266_REBOOT_CMD				"AT+RST"

/* 定义 ESP8266 模块连接所需的信息 */
#define ESP8266_WIFI_NAME				"Ridiculous2.4g"
#define ESP8266_WIFI_PASSWORD			"persist011104"
#define ESP8266_SERVER_TYPE				"TCP"
#define ESP8266_SERVER_IP_ADDR			"192.168.10.21"
#define ESP8266_SERVER_PORT				"8080"

/* 判断指令是否执行成功 */
#define ESP8266_CMD_RESPONSE_SUCCESS	"OK"
#define ESP8266_CMD_RESPONSE_FAIL		"ERROR"

/* WIFI 状态类 */
#define ESP8266_WIFI_DISCONNECTED						"WIFI DISCONNECTED"
#define ESP8266_WIFI_CONNECTED							"WIFI CONNECTED"
#define ESP8266_WIFI_GOT_IP								"WIFI GOT IP"
#define ESP8266_WIFI_FAIL_PREFIX						"+CWJAP:"

/* TCP 状态类 */
#define ESP8266_TCP_CONNECTED							"CONNECT"
#define ESP8266_TCP_CLOSED								"CLOSED"

typedef enum{
	ESP8266_OK = 0,
    ESP8266_ERROR,
    ESP8266_TIMEOUT
}ESP8266_Status_t;

typedef enum{
	BSP_ESP8266_EN = 0,
	BSP_ESP8266_RST,
	BSP_ESP8266_GPIO0,
	BSP_ESP8266_GPIO2,
	BSP_ESP8266_MAX
}bsp_esp8266_t;

typedef enum{
	BSP_ESP8266_Flash_Mode = 0,
	BSP_ESP8266_Normal_Mode = 1,
}bsp_esp8266_mode_t;

extern void BSP_ESP8266_Init(void);

extern void BSP_ESP8266_SendString(uint8_t *str);
extern void BSP_ESP8266_SendCmd(char *cmd);
extern void BSP_ESP8266_ClearRxBuffer(void);

extern bool BSP_ESP8266_Test(void);
extern bool BSP_ESP8266_CloseEcho(void);
extern bool BSP_ESP8266_ConnectWiFI(void);
extern bool BSP_ESP8266_ConnectTCP(void);
extern bool BSP_ESP8266_SetTransMode(void);
extern bool BSP_ESP8266_Reboot(void);
extern void BSP_ESP8266_ClearRxBuffer(void);

#endif
