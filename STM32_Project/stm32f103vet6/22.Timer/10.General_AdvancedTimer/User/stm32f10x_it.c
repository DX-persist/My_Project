/**
  ******************************************************************************
  * @file    stm32f10x_it.c
  * @author
  * @version V1.0.0
  * @date
  * @brief   系统异常与外设中断服务函数实现文件
  *
  * @details
  * 本文件实现了 STM32F10x 工程中使用到的中断服务函数（ISR），包括：
  * - Cortex-M3 内核异常处理函数
  * - USART 串口中断处理函数
  * - TIM1 / TIM8 高级定时器捕获比较中断处理函数
  *
  * 文件中大部分中断入口采用“中断入口 + BSP 统一处理接口”的方式组织，
  * 即具体 IRQHandler 只负责响应中断入口，再调用底层 BSP 模块中对应的
  * 公共处理函数，从而降低中断文件与具体外设实现之间的耦合度。
  *
  * @note
  * 1. 本文件中的 HardFault / MemManage / BusFault / UsageFault 默认均进入死循环，
  *    便于调试阶段定位异常问题。
  * 2. 若项目需要更复杂的异常记录或故障恢复机制，可在对应异常处理函数中扩展。
  * 3. 某些中断处理函数当前被保留但未启用，例如 SysTick、EXTI、TIM1/TIM8 更新中断。
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"

/**
  * @addtogroup STM32F10x_Interrupt_Handlers
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
  * @brief  非屏蔽中断（NMI）处理函数。
  *
  * @details
  * NMI（Non-Maskable Interrupt）属于高优先级异常，通常用于处理严重硬件故障、
  * 时钟安全系统异常等不可屏蔽事件。
  *
  * @param  None
  * @retval None
  *
  * @note
  * 当前实现为空函数。如项目中使用时钟安全系统（CSS）或其他 NMI 源，
  * 可在此补充具体处理逻辑。
  */
void NMI_Handler(void)
{
}

/**
  * @brief  硬件故障异常处理函数。
  *
  * @details
  * 当系统发生严重错误，例如非法访问、总线异常升级、执行异常指令等，
  * 可能触发 Hard Fault。当前实现进入死循环，便于在调试器中停住分析。
  *
  * @param  None
  * @retval None
  *
  * @warning
  * 若程序进入该函数，通常说明系统运行出现严重异常。
  */
void HardFault_Handler(void)
{
  /* 发生 Hard Fault 时进入死循环，便于调试定位问题 */
  while (1)
  {
  }
}

/**
  * @brief  存储器管理异常处理函数。
  *
  * @details
  * 当发生存储器保护相关异常时进入该函数，例如 MPU 访问违规等。
  * Cortex-M3 在具体配置下可能并不总是启用该异常。
  *
  * @param  None
  * @retval None
  *
  * @warning
  * 进入该函数说明存在非法内存访问或内存保护配置异常。
  */
void MemManage_Handler(void)
{
  /* 发生 Memory Manage Fault 时进入死循环 */
  while (1)
  {
  }
}

/**
  * @brief  总线故障异常处理函数。
  *
  * @details
  * 当发生总线访问错误时进入该函数，例如访问不存在的外设地址、
  * 非法取指或数据总线错误等。
  *
  * @param  None
  * @retval None
  *
  * @warning
  * 进入该函数说明系统总线访问存在异常。
  */
void BusFault_Handler(void)
{
  /* 发生 Bus Fault 时进入死循环 */
  while (1)
  {
  }
}

/**
  * @brief  用法故障异常处理函数。
  *
  * @details
  * 当 CPU 执行非法指令、状态不一致或其他使用错误时进入该函数。
  *
  * @param  None
  * @retval None
  *
  * @warning
  * 进入该函数通常表示程序流异常、非法指令或栈损坏等问题。
  */
void UsageFault_Handler(void)
{
  /* 发生 Usage Fault 时进入死循环 */
  while (1)
  {
  }
}

/**
  * @brief  系统服务调用异常处理函数。
  *
  * @details
  * SVC（Supervisor Call）通常用于操作系统内核服务调用或特权切换。
  * 当前裸机工程中未使用，函数为空实现。
  *
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  调试监视异常处理函数。
  *
  * @details
  * 用于调试相关异常处理，通常在普通裸机应用中不需要特殊实现。
  *
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  PendSV 异常处理函数。
  *
  * @details
  * PendSV 常用于实时操作系统中的任务切换。
  * 当前工程中未启用，保持空实现。
  *
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  SysTick 系统滴答中断处理函数。
  *
  * @details
  * SysTick 常用于系统节拍、延时基准或操作系统时基。
  * 当前该处理函数被条件编译屏蔽，说明项目当前未启用该中断处理逻辑，
  * 或在其他文件中另行实现。
  *
  * @param  None
  * @retval None
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
  * @brief  外部中断统一处理辅助函数。
  *
  * @details
  * 该函数用于统一处理指定 EXTI 线的中断：
  * - 检查中断状态
  * - 调用 BSP 层回调函数
  * - 清除中断挂起标志
  *
  * @param  EXTI_Line 外部中断线编号
  * @retval None
  *
  * @note
  * 当前该函数及相关 EXTI 中断入口被条件编译屏蔽，
  * 表明项目当前未启用对应 EXTI 中断功能。
  */
#if 0
static void BSP_EXTI_CommandHandler(uint32_t EXTI_Line)
{
  if(EXTI_GetITStatus(EXTI_Line) != RESET){
    BSP_EXTI_Callback(EXTI_Line);
    EXTI_ClearITPendingBit(EXTI_Line);
  }
}
#endif

/**
  * @brief  EXTI0 外部中断处理函数。
  *
  * @details
  * 当前通过统一辅助函数处理 EXTI_Line0 对应中断。
  *
  * @param  None
  * @retval None
  */
#if 0
void EXTI0_IRQHandler(void)
{
  BSP_EXTI_CommandHandler(EXTI_Line0);
}
#endif

/**
  * @brief  EXTI15~10 外部中断处理函数。
  *
  * @details
  * 当前示例中仅处理 EXTI_Line13。
  *
  * @param  None
  * @retval None
  */
#if 0
void EXTI15_10_IRQHandler(void)
{
	BSP_EXTI_CommandHandler(EXTI_Line13);
}
#endif

/**
  * @brief  USART1 中断服务函数。
  *
  * @details
  * 该函数为 USART1 的实际中断入口，
  * 内部调用 BSP 串口模块统一中断处理接口处理收发事件。
  *
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
	BSP_USART_IRQHandler(BSP_USART1);	
}

/**
  * @brief  USART2 中断服务函数。
  *
  * @details
  * 该函数为 USART2 的实际中断入口，
  * 内部调用 BSP 串口模块统一中断处理接口处理收发事件。
  *
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{
	BSP_USART_IRQHandler(BSP_USART2);	
}

/**
  * @brief  USART3 中断服务函数。
  *
  * @details
  * 该函数为 USART3 的实际中断入口，
  * 内部调用 BSP 串口模块统一中断处理接口处理收发事件。
  *
  * @param  None
  * @retval None
  */
void USART3_IRQHandler(void)
{
	BSP_USART_IRQHandler(BSP_USART3);	
}

/**
  * @brief  UART4 中断服务函数。
  *
  * @details
  * 该函数为 UART4 的实际中断入口，
  * 内部调用 BSP 串口模块统一中断处理接口处理收发事件。
  *
  * @param  None
  * @retval None
  */
void UART4_IRQHandler(void)
{
	BSP_USART_IRQHandler(BSP_UART4);	
}

/**
  * @brief  UART5 中断服务函数。
  *
  * @details
  * 该函数为 UART5 的实际中断入口，
  * 内部调用 BSP 串口模块统一中断处理接口处理收发事件。
  *
  * @param  None
  * @retval None
  */
void UART5_IRQHandler(void)
{
	BSP_USART_IRQHandler(BSP_UART5);	
}

/**
  * @brief  TIM1 更新中断服务函数。
  *
  * @details
  * 该函数是 TIM1 更新中断入口。
  * 当前对高级定时器更新中断处理接口的调用被注释掉，
  * 说明项目暂未启用 TIM1 更新中断业务逻辑。
  *
  * @param  None
  * @retval None
  *
  * @note
  * 若后续需要处理 TIM1 更新事件，可取消注释对应 BSP 调用。
  */
void TIM1_UP_IRQHandler(void)
{
	//BSP_TIM_UP_IRQHandler(BSP_ADVANCED_TIMER1);
}

/**
  * @brief  TIM1 捕获比较中断服务函数。
  *
  * @details
  * 该函数为 TIM1 捕获比较中断实际入口，
  * 内部调用高级定时器 BSP 模块统一处理中断。
  * 主要用于 PWM 输入捕获测量场景。
  *
  * @param  None
  * @retval None
  */
void TIM1_CC_IRQHandler(void)
{
  BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER1);
}

/**
  * @brief  TIM8 更新中断服务函数。
  *
  * @details
  * 该函数是 TIM8 更新中断入口。
  * 当前对高级定时器更新中断处理接口的调用被注释掉，
  * 说明项目暂未启用 TIM8 更新中断业务逻辑。
  *
  * @param  None
  * @retval None
  */
void TIM8_UP_IRQHandler(void)
{
	//BSP_TIM_UP_IRQHandler(BSP_ADVANCED_TIMER8);
}

/**
  * @brief  TIM8 捕获比较中断服务函数。
  *
  * @details
  * 该函数为 TIM8 捕获比较中断实际入口，
  * 内部调用高级定时器 BSP 模块统一处理中断。
  * 主要用于 PWM 输入捕获测量场景。
  *
  * @param  None
  * @retval None
  */
void TIM8_CC_IRQHandler(void)
{
  BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER8);
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/