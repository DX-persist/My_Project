/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	led.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2016-11-23
	*
	*	版本： 		V1.0
	*
	*	说明： 		LED初始化、控制
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//硬件驱动
#include "LED.h"


LED_INFO LED_info = {0};


/*
************************************************************
*	函数名称：	LED_Init
*
*	函数功能：	蜂鸣器初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
void LED_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//打开GPIOB的时钟
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;				//设置为输出
	GPIO_InitStruct.GPIO_Pin = LED_PIN;						//将初始化的Pin脚
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;				//可承载的最大频率
	
	GPIO_Init(LED_PORT, &GPIO_InitStruct);							//初始化GPIO
	
	LED_Set(LED_OFF);											//初始化完成后，关闭蜂鸣器

}

/*
************************************************************
*	函数名称：	LED_Set
*
*	函数功能：	蜂鸣器控制
*
*	入口参数：	status：开关蜂鸣器
*
*	返回参数：	无
*
*	说明：		开-LED_ON		关-LED_OFF
************************************************************
*/
void LED_Set(_Bool status)
{
	
	GPIO_WriteBit(LED_PORT, LED_PIN, status == LED_ON ? Bit_RESET : Bit_SET);		//如果status等于LED_ON，则返回Bit_RESET，否则返回Bit_SET
	
	LED_info.LED_Status = status;

}
