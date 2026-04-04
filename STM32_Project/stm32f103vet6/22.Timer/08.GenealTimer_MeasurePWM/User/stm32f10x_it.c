/**
  ******************************************************************************
  * @file    stm32f10x_it.c
  * @author  MCD Application Team / User Modified
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   STM32F10x 异常与外设中断服务函数实现文件
  * @details
  * 本文件实现 Cortex-M3 内核异常处理函数以及工程中实际使用的外设中断服务函数。
  *
  * 当前工程已接入的外设中断包括：
  * - USART1 / USART2 / USART3 / UART4 / UART5 串口中断
  * - TIM2 / TIM3 / TIM4 / TIM5 定时器中断
  *
  * 对于定时器中断，本文件不直接处理业务逻辑，而是将中断统一转交给 BSP 层：
  * @code
  * BSP_TIM_IRQHandler(BSP_GENERAL_TIMERx);
  * @endcode
  *
  * 这种“中断入口 + BSP 分发”的结构更利于维护和复用。
  *
  * @note
  * 中断处理函数名称必须与启动文件 `startup_stm32f10x_xx.s` 中的向量表名称完全一致。
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  NMI 异常处理函数
  * @details
  * 当前工程未对 NMI 做特殊处理，因此函数体为空。
  */
void NMI_Handler(void)
{
}

/**
  * @brief  Hard Fault 异常处理函数
  * @details
  * 出现严重异常时进入死循环，便于开发阶段调试定位。
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief  Memory Manage 异常处理函数
  * @details
  * 出现内存管理异常时进入死循环，便于调试。
  */
void MemManage_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief  Bus Fault 异常处理函数
  * @details
  * 出现总线错误时进入死循环，便于调试。
  */
void BusFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief  Usage Fault 异常处理函数
  * @details
  * 出现程序使用错误时进入死循环，便于调试。
  */
void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief  SVCall 异常处理函数
  */
void SVC_Handler(void)
{
}

/**
  * @brief  Debug Monitor 异常处理函数
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  PendSV 异常处理函数
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  SysTick 中断处理函数
  * @details
  * 当前被 `#if 0` 屏蔽，说明系统节拍处理可能已在其他模块中实现，
  * 避免重复定义。
  */
#if 0
void SysTick_Handler(void)
{
}
#endif

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/******************************************************************************/

/**
  * @brief  EXTI 中断公共处理逻辑示例
  * @details
  * 当前未启用，保留作模板参考。
  */
#if 0
static void BSP_EXTI_CommandHandler(uint32_t EXTI_Line)
{
  if(EXTI_GetITStatus(EXTI_Line) != RESET){
    BSP_EXTI_Callback(EXTI_Line);
    EXTI_ClearITPendingBit(EXTI_Line);
  }
}

void EXTI0_IRQHandler(void)
{
  BSP_EXTI_CommandHandler(EXTI_Line0);
}

void EXTI15_10_IRQHandler(void)
{
  BSP_EXTI_CommandHandler(EXTI_Line13);
}
#endif

/**
  * @brief USART1 中断服务函数
  * @details
  * 将 USART1 中断转交给 BSP 串口公共处理函数。
  */
void USART1_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_USART1);
}

/**
  * @brief USART2 中断服务函数
  * @details
  * 将 USART2 中断转交给 BSP 串口公共处理函数。
  */
void USART2_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_USART2);
}

/**
  * @brief USART3 中断服务函数
  * @details
  * 将 USART3 中断转交给 BSP 串口公共处理函数。
  */
void USART3_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_USART3);
}

/**
  * @brief UART4 中断服务函数
  * @details
  * 将 UART4 中断转交给 BSP 串口公共处理函数。
  */
void UART4_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_UART4);
}

/**
  * @brief UART5 中断服务函数
  * @details
  * 将 UART5 中断转交给 BSP 串口公共处理函数。
  */
void UART5_IRQHandler(void)
{
    BSP_USART_IRQHandler(BSP_UART5);
}

/**
  * @brief TIM2 中断服务函数
  * @details
  * 将 TIM2 中断统一转交给通用定时器公共中断处理函数。
  */
void TIM2_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER2);
}

/**
  * @brief TIM3 中断服务函数
  * @details
  * 将 TIM3 中断统一转交给通用定时器公共中断处理函数。
  */
void TIM3_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER3);
}

/**
  * @brief TIM4 中断服务函数
  * @details
  * 将 TIM4 中断统一转交给通用定时器公共中断处理函数。
  */
void TIM4_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER4);
}

/**
  * @brief TIM5 中断服务函数
  * @details
  * 将 TIM5 中断统一转交给通用定时器公共中断处理函数。
  */
void TIM5_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER5);
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
