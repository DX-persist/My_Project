/**
  ******************************************************************************
  * @file    stm32f10x_it.c
  * @brief   STM32F10x 异常与外设中断服务函数实现文件
  * @details
  * 本文件实现 Cortex-M3 内核异常处理函数以及工程中实际使用的外设中断服务函数。
  *
  * 当前工程接入的中断包括：
  * - USART1 / USART2 / USART3 / UART4 / UART5 串口中断
  * - TIM2 / TIM3 / TIM4 / TIM5 定时器中断
  *
  * 对于 TIM2~TIM5：
  * - 更新中断用于统计输入捕获测量期间的溢出次数
  * - 捕获比较中断用于记录上升沿/下降沿并计算脉宽
  *
  * @note
  * 定时器中断入口最终统一转交给 BSP_TIM_IRQHandler() 处理。
  ******************************************************************************
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

/**
  * @brief Memory Manage 异常处理函数
  */
void MemManage_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief Bus Fault 异常处理函数
  */
void BusFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief Usage Fault 异常处理函数
  */
void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief SVCall 异常处理函数
  */
void SVC_Handler(void)
{
}

/**
  * @brief Debug Monitor 异常处理函数
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief PendSV 异常处理函数
  */
void PendSV_Handler(void)
{
}

#if 0
/**
  * @brief SysTick 中断处理函数
  * @details 当前屏蔽，说明系统节拍处理可能已在其他模块中定义。
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

/**
  * @brief USART2 中断服务函数
  * @details 将 USART2 中断统一转交给 BSP 串口公共处理函数。
  */
void USART2_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_USART2);
}

/**
  * @brief USART3 中断服务函数
  * @details 将 USART3 中断统一转交给 BSP 串口公共处理函数。
  */
void USART3_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_USART3);
}

/**
  * @brief UART4 中断服务函数
  * @details 将 UART4 中断统一转交给 BSP 串口公共处理函数。
  */
void UART4_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_UART4);
}

/**
  * @brief UART5 中断服务函数
  * @details 将 UART5 中断统一转交给 BSP 串口公共处理函数。
  */
void UART5_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_UART5);
}

/**
  * @brief TIM2 中断服务函数
  * @details 将 TIM2 中断统一转交给通用定时器公共处理函数。
  */
void TIM2_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER2);
}

/**
  * @brief TIM3 中断服务函数
  * @details 将 TIM3 中断统一转交给通用定时器公共处理函数。
  */
void TIM3_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER3);
}

/**
  * @brief TIM4 中断服务函数
  * @details 将 TIM4 中断统一转交给通用定时器公共处理函数。
  */
void TIM4_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER4);
}

/**
  * @brief TIM5 中断服务函数
  * @details 将 TIM5 中断统一转交给通用定时器公共处理函数。
  */
void TIM5_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER5);
}