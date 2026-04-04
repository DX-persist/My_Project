/**
  ******************************************************************************
  * @file    stm32f10x_it.c
  * @brief   STM32F10x 异常与外设中断服务函数实现文件
  * @details
  * 本文件实现 Cortex-M3 内核异常处理函数以及工程中实际使用的外设中断服务函数。
  *
  * 当前工程接入的主要中断包括：
  * - USART1 / USART2 / USART3 / UART4 / UART5 串口中断
  * - TIM1 / TIM8 更新中断
  * - TIM1 / TIM8 捕获比较中断
  *
  * 对于 TIM1 / TIM8：
  * - 更新中断用于统计输入捕获期间的溢出次数
  * - 捕获比较中断用于记录上升沿/下降沿时间并计算脉宽
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"

/**
  * @brief NMI 异常处理函数
  */
void NMI_Handler(void)
{
}

/**
  * @brief HardFault 异常处理函数
  * @details 严重异常发生后进入死循环，便于调试定位。
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

void MemManage_Handler(void)
{
  while (1)
  {
  }
}

void BusFault_Handler(void)
{
  while (1)
  {
  }
}

void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

#if 0
/**
  * @brief SysTick 中断处理函数
  * @details 当前屏蔽，表示系统节拍处理可能已在其他模块中定义。
  */
void SysTick_Handler(void)
{
}
#endif

/**
  * @brief USART1 中断服务函数
  * @details 将 USART1 中断统一转交给 BSP 串口公共处理函数。
  */
void USART1_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_USART1);
}

void USART2_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_USART2);
}

void USART3_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_USART3);
}

void UART4_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_UART4);
}

void UART5_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_UART5);
}

/**
  * @brief TIM1 更新中断服务函数
  * @details
  * 用于统计输入捕获测量过程中发生的计数器溢出次数。
  */
void TIM1_UP_IRQHandler(void)
{
    BSP_TIM_UP_IRQHandler(BSP_ADVANCED_TIMER1);
}

/**
  * @brief TIM1 捕获比较中断服务函数
  * @details
  * 用于处理 TIM1 输入捕获事件，包括上升沿/下降沿捕获和脉宽计算。
  */
void TIM1_CC_IRQHandler(void)
{
    BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER1);
}

/**
  * @brief TIM8 更新中断服务函数
  * @details
  * 用于统计 TIM8 输入捕获测量期间发生的计数器溢出次数。
  */
void TIM8_UP_IRQHandler(void)
{
    BSP_TIM_UP_IRQHandler(BSP_ADVANCED_TIMER8);
}

/**
  * @brief TIM8 捕获比较中断服务函数
  * @details
  * 用于处理 TIM8 输入捕获事件，包括上升沿/下降沿捕获和脉宽计算。
  */
void TIM8_CC_IRQHandler(void)
{
    BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER8);
}