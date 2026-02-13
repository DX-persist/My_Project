#ifndef BSP_ESP8266_H
#define BSP_ESP8266_H

#include "bsp_usart.h"
#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_ringbuffer.h"

/*===================== 串口配置 =====================*/
#define ESP8266_DEBUG_USART_ID		BSP_USART1  /* 调试输出串口（printf）*/
#define ESP8266_USART_ID			BSP_USART3  /* 与 ESP8266 通信的串口 */

/*===================== 缓冲区配置 =====================*/
#define ESP8266_LINE_BUF_SIZE		256         /* 单行响应最大长度 */

/*===================== AT 命令定义（未使用，可删除）=====================*/
/* 注意：这些宏定义未被实际使用，建议删除或改为函数内部使用 */
#define BSP_ESP8266_TESTAT_CMD          "AT\r\n"
#define BSP_ESP8266_SETMODE_CMD         "AT+CWMODE=3\r\n"
#define BSP_ESP8266_CONNECT_WIFI_CMD    "AT+CWJAP=\"Ridiculous2.4g\",\"persist011104\"\r\n"
#define BSP_ESP8266_CONNECT_TCP_CMD     "AT+CIPSTART=\"TCP\",\"192.168.10.21\",8888\r\n"
#define BSP_ESP8266_SET_TRANMODE_CMD    "AT+CIPMODE=1\r\n"
#define BSP_ESP8266_SENDDATA_CMD        "AT+CIPSEND\r\n"

/*===================== 枚举类型定义 =====================*/

/**
 * @brief ESP8266 控制引脚枚举
 * @note  
 *  - BSP_ESP8266_EN:  使能引脚（PB8）
 *  - BSP_ESP8266_RST: 复位引脚（PB9）
 */
typedef enum{
	BSP_ESP8266_EN = 0,     /* 使能引脚 */
	BSP_ESP8266_RST,        /* 复位引脚 */
	BSP_ESP8266_MAX         /* 引脚数量（用于数组大小）*/
}bsp_esp8266_t;

/**
 * @brief ESP8266 命令执行状态
 * @note  所有 AT 命令函数的返回值类型
 */
typedef enum{
	ESP8266_OK = 0,         /* 命令执行成功 */
	ESP8266_ERROR,          /* 命令执行失败 */
	ESP8266_TIMEOUT         /* 超时未响应 */
}ESP8266_Status_t;

/*===================== 函数声明 =====================*/

/**
 * @brief  初始化 ESP8266 模块
 * @note   
 *  - 配置控制引脚（EN、RST）
 *  - 配置通信串口和调试串口
 *  - 硬件复位模块
 */
extern void BSP_ESP8266_Init(void);

/**
 * @brief  发送 AT 命令并等待响应
 * @param  cmd          AT 命令字符串
 * @param  success_msg  成功标志（如 "OK"）
 * @param  fail_msg     失败标志（如 "ERROR"）
 * @param  timeout_ms   超时时间（毫秒）
 * @retval ESP8266_Status_t 执行结果
 * @note   通用接口，可用于发送任意 AT 命令
 */
extern ESP8266_Status_t BSP_ESP8266_SendCmdAndWait(
		char *cmd,
        char *success_msg,
        char *fail_msg,
        uint32_t timeout_ms);

/**
 * @brief  在透传模式下发送数据
 * @param  msg  要发送的数据字符串
 * @note   需先调用 BSP_ESP8266_EnterSendMode() 进入透传模式
 */
extern void BSP_ESP8266_SendData(char *msg);

/*===================== AT 命令封装函数 =====================*/

/**
 * @brief  测试 ESP8266 通信（发送 AT）
 * @retval ESP8266_OK      通信正常
 * @retval ESP8266_ERROR   通信异常
 * @retval ESP8266_TIMEOUT 超时
 */
extern ESP8266_Status_t BSP_ESP8266_TestAT(void);

/**
 * @brief  关闭回显（ATE0）
 * @note   避免模块回显命令，简化响应解析
 */
extern ESP8266_Status_t BSP_ESP8266_EchoOff(void);

/**
 * @brief  设置 WiFi 模式为 Station+AP（模式3）
 * @note   
 *  - 模式1：Station（客户端）
 *  - 模式2：AP（热点）
 *  - 模式3：Station+AP（双模式）
 */
extern ESP8266_Status_t BSP_ESP8266_SetWiFiMode(void);

/**
 * @brief  连接到 WiFi 网络
 * @note   
 *  - SSID: "Ridiculous2.4g"
 *  - 密码: "persist011104"
 *  - 超时 15 秒
 */
extern ESP8266_Status_t BSP_ESP8266_ConnectWiFi(void);

/**
 * @brief  连接到 TCP 服务器
 * @note   
 *  - 服务器 IP: 192.168.10.20
 *  - 端口: 8080
 *  - 需先连接 WiFi
 */
extern ESP8266_Status_t BSP_ESP8266_ConnectTCP(void);

/**
 * @brief  设置为透传模式
 * @note   透传模式下无需 AT 指令，可直接发送数据
 */
extern ESP8266_Status_t BSP_ESP8266_SetTransparentMode(void);

/**
 * @brief  进入数据发送模式
 * @note   
 *  - 需先设置透传模式并建立 TCP 连接
 *  - 收到 ">" 后可调用 BSP_ESP8266_SendData() 发送数据
 *  - 发送 "+++" 可退出透传模式
 */
extern ESP8266_Status_t BSP_ESP8266_EnterSendMode(void);

#endif