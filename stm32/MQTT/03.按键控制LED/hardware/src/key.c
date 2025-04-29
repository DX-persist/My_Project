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
#include "key.h"
#include "delay.h"
#include "LED.h"

/*
************************************************************
*	函数名称：	KEY_Init
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
void KEY_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXIT_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//打开GPIOB的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;				
	GPIO_InitStruct.GPIO_Pin = KEY_PIN;						//将初始化的Pin脚
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;				//可承载的最大频率
	
	GPIO_Init(KEY_PORT, &GPIO_InitStruct);							//初始化GPIO
    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1); 

    EXIT_InitStruct.EXTI_Line = EXTI_Line1;
    EXIT_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXIT_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXIT_InitStruct.EXTI_LineCmd = ENABLE;
    
    EXTI_Init(&EXIT_InitStruct);
    
    NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void EXTI1_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line1) == SET)
    {
        DelayMs(10);
        if(GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN) == RESET)
        {
            if(LED_info.LED_Status == LED_ON)
                LED_Set(LED_OFF);
            else
                LED_Set(LED_ON);
        }
    }
    EXTI_ClearITPendingBit(EXTI_Line1);
}
