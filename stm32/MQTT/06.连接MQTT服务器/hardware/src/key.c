/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	led.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2016-11-23
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		LED��ʼ��������
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//Ӳ������
#include "key.h"
#include "delay.h"
#include "LED.h"

/*
************************************************************
*	�������ƣ�	KEY_Init
*
*	�������ܣ�	��������ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void KEY_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXIT_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//��GPIOB��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;				
	GPIO_InitStruct.GPIO_Pin = KEY_PIN;						//����ʼ����Pin��
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;				//�ɳ��ص����Ƶ��
	
	GPIO_Init(KEY_PORT, &GPIO_InitStruct);							//��ʼ��GPIO
    
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
