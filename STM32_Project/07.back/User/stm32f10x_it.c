/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTI
  
  AL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
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
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
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

#if 0
static void BSP_USART_CommandHandler(bsp_usart_t id)
{
  if(id >= BSP_USART_MAX) return;

  const bsp_usart_hw_t *hw = &bsp_usart_hw[id];
  if(USART_GetITStatus(hw->usartx, hw->irq_it) != RESET){
    BSP_USART_Callback(hw->usartx);
    //USART_ClearITPendingBit(hw->usartx, hw->irq_it);
  }

  // 2. 处理 ORE 溢出中断
  // 即使上面处理了 RXNE，如果发生了溢出，ORE标志可能仍未清除，或者是在无 RXNE 时触发的 ORE
  if(USART_GetFlagStatus(hw->usartx, USART_FLAG_ORE) != RESET){
    // 读取数据寄存器 (DR)。序列：先读SR(GetFlagStatus已做)，再读DR。这会清除 ORE。
    // 使用 volatile 防止编译器优化掉“无用的”读取
    volatile uint16_t temp = USART_ReceiveData(hw->usartx);
    (void)temp; // 防止编译器警告变量未使用
  }
}
#endif

static void BSP_USART_CommandHandler(bsp_usart_t id)
{
    if(id >= BSP_USART_MAX) return;

    const bsp_usart_hw_t *hw = &bsp_usart_hw[id];
    USART_TypeDef *usart = hw->usartx;

    // 读取状态寄存器
    // 这一步对于清除 ORE 至关重要
    if(USART_GetITStatus(usart, hw->irq_it) != RESET)
    {
        // 读 DR 寄存器，这会自动清除 RXNE 标志位
        uint16_t data = USART_ReceiveData(usart);
        
        // 直接在这里处理数据，不要跳去 Callback 再读一次 DR
        BSP_USART_SendByte(id, (uint8_t)data);
        
        // 不需要 ClearITPendingBit
    }
    // 处理 ORE (溢出错误)
    // 如果 RXNE 没有触发，但是 ORE 触发了 (数据发太快没来得及读)
    else if(USART_GetFlagStatus(usart, USART_FLAG_ORE) != RESET)
    {
        //只读数据不处理，纯粹为了清除 ORE 标志位
        // 序列: 读 SR (上面的 GetFlagStatus 已做) -> 读 DR
        (void)USART_ReceiveData(usart); 
    }
}

void USART1_IRQHandler(void)
{
  // if(USART_GetITStatus(USART1, USART_FLAG_RXNE) != RESET){
  //   uint8_t ch = USART_ReceiveData(USART1);
  //   USART_SendSData(USART1, (uint16_t)ch);
  // }
  BSP_USART_CommandHandler(BSP_USART1);
}

void USART2_IRQHandler(void)
{
  BSP_USART_CommandHandler(BSP_USART2);
}

void USART3_IRQHandler(void)
{
  BSP_USART_CommandHandler(BSP_USART3);
}

void UART4_IRQHandler(void)
{
  BSP_USART_CommandHandler(BSP_UART4);
}

void UART5_IRQHandler(void)
{
  BSP_USART_CommandHandler(BSP_UART5);
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
