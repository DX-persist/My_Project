/**
  ******************************************************************************
  * @file    stm32f10x_it.c
  * @author  MCD Application Team / User Modified
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   STM32F10x 中断服务函数实现文件
  * @details
  * 本文件用于实现 STM32F10x 系列 MCU 的异常处理函数（Exception Handlers）
  * 与外设中断服务函数（Interrupt Service Routines, ISR）。
  *
  * 文件内容主要分为两部分：
  * - Cortex-M3 内核异常处理函数
  * - 具体外设中断处理函数
  *
  * 对于本工程而言，当前已实际使用并接入的中断包括：
  * - USART1 / USART2 / USART3 / UART4 / UART5 串口中断
  * - TIM1 / TIM8 捕获比较中断
  *
  * 其中大多数外设中断函数本身只作为“中断入口”，
  * 实际的功能处理被转交给 BSP 层对应的公共处理函数，例如：
  * - `BSP_USART_IRQHandler()`
  * - `BSP_TIM_CC_IRQHandler()`
  *
  * 这种设计方式的优点是：
  * - 统一各外设实例的处理逻辑
  * - 降低重复代码
  * - 让中断入口函数保持简洁
  * - 便于后期移植和维护
  *
  * @note
  * 1. 本文件中的 IRQHandler 名称必须与启动文件
  *    `startup_stm32f10x_xx.s` 中断向量表定义完全一致，否则中断无法正确进入。
  * 2. 某些异常处理函数采用死循环，这是嵌入式工程中的常见调试手段，
  *    便于在异常发生时停留现场，供调试器分析。
  * 3. 本文件保留了 ST 官方模板中的部分注释与结构，同时进行了工程化注释增强。
  *
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"

/**
  * @defgroup STM32F10x_Interrupts 中断服务函数模块
  * @brief    管理 Cortex-M3 异常与 STM32 外设中断服务函数
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
  * @brief  非屏蔽中断（NMI）处理函数
  * @details
  * NMI（Non-Maskable Interrupt）为不可屏蔽中断，优先级高，
  * 通常用于严重硬件事件处理，例如时钟安全系统故障等。
  *
  * 当前工程未对 NMI 做额外处理，因此函数体为空。
  *
  * @param  None
  * @retval None
  *
  * @note
  * 如果后续工程需要监测时钟失效、外部严重错误等，可在此处加入日志、
  * LED 指示或错误恢复逻辑。
  */
void NMI_Handler(void)
{
}

/**
  * @brief  Hard Fault 异常处理函数
  * @details
  * Hard Fault 是 Cortex-M3 内核中的严重错误异常，常见触发原因包括：
  * - 非法访问地址
  * - 执行了非法指令
  * - 总线错误升级为 HardFault
  * - 使用错误未被单独捕获而升级
  *
  * 该函数进入死循环，便于开发阶段通过调试器观察异常现场。
  *
  * @param  None
  * @retval None
  *
  * @note
  * 在正式产品中，通常会在此处增加：
  * - 看门狗复位前记录错误信息
  * - 保存寄存器上下文
  * - 输出调试日志
  * - 系统复位策略
  */
void HardFault_Handler(void)
{
  /* 发生 HardFault 后停留在此，便于调试定位问题 */
  while (1)
  {
  }
}

/**
  * @brief  Memory Manage 异常处理函数
  * @details
  * 该异常通常与内存访问权限错误相关，例如 MPU 配置非法访问等。
  * 在 STM32F1 常规裸机开发中较少主动使用 MPU，因此此异常通常不常见。
  *
  * 当前处理策略为进入死循环。
  *
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* 发生内存管理异常后停留在此，便于调试 */
  while (1)
  {
  }
}

/**
  * @brief  Bus Fault 异常处理函数
  * @details
  * Bus Fault 一般表示总线访问异常，例如：
  * - 访问不存在的外设地址
  * - 访问非法存储区
  * - 总线事务异常
  *
  * 当前处理策略为进入死循环。
  *
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* 发生总线错误后停留在此，便于调试 */
  while (1)
  {
  }
}

/**
  * @brief  Usage Fault 异常处理函数
  * @details
  * Usage Fault 表示程序使用错误，常见原因包括：
  * - 执行未定义指令
  * - 非法状态切换
  * - 除 0（如果相关检测开启）
  *
  * 当前处理策略为进入死循环。
  *
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* 发生用法错误后停留在此，便于调试 */
  while (1)
  {
  }
}

/**
  * @brief  SVCall 异常处理函数
  * @details
  * SVC（Supervisor Call）通常用于操作系统内核服务调用。
  * 在裸机工程中如果未使用 RTOS，一般不需要额外实现。
  *
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  Debug Monitor 异常处理函数
  * @details
  * 用于调试监视功能相关异常。
  * 当前工程未使用该功能，因此保持空实现。
  *
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  PendSV 异常处理函数
  * @details
  * PendSV 常用于 RTOS 中进行任务切换。
  * 当前裸机工程未使用任务调度，因此保持空实现。
  *
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  SysTick 中断处理函数
  * @details
  * 当前该函数被 `#if 0` 屏蔽，表示本文件未启用 SysTick 的默认处理逻辑。
  *
  * 一般来说，如果工程中已有独立的系统节拍模块（例如 `bsp_delay.c`、
  * `bsp_systick.c` 或其他时间基准模块）实现了 `SysTick_Handler()`，
  * 则不能在这里再次定义，否则会产生重复定义错误。
  *
  * @note
  * 此处使用 `#if 0` 而不是删除代码，是为了保留 ST 模板结构，便于后续恢复或参考。
  */
#if 0
void SysTick_Handler(void)
{
}
#endif

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  在此添加工程实际使用的外设中断处理函数。                                  */
/*  具体中断函数名称请参考启动文件 startup_stm32f10x_xx.s。                   */
/******************************************************************************/

/**
  * @brief  外设中断模板示例
  * @details
  * 这是 ST 官方模板中保留的示例结构，用于提醒开发者按需添加外设中断。
  *
  * @code
  * void PPP_IRQHandler(void)
  * {
  * }
  * @endcode
  *
  * 当前仅作为注释保留，不参与编译。
  */

/*void PPP_IRQHandler(void)
{
}*/

#if 0
/**
  * @brief  EXTI 中断公共处理函数
  * @details
  * 该函数封装了 EXTI 中断状态检测、回调触发与挂起位清除流程，
  * 适合多个 EXTI 中断入口复用。
  *
  * @param[in] EXTI_Line
  * 需要处理的 EXTI 线号，例如 `EXTI_Line0`、`EXTI_Line13`。
  *
  * @retval None
  *
  * @note
  * 当前该部分代码被 `#if 0` 屏蔽，说明当前工程暂未启用该 EXTI 处理逻辑。
  * 若后续恢复使用，应确保：
  * - 已正确配置 GPIO 和 EXTI
  * - 已实现 `BSP_EXTI_Callback()`
  * - 已在 NVIC 中开启对应 EXTI 中断
  */
static void BSP_EXTI_CommandHandler(uint32_t EXTI_Line)
{
  if(EXTI_GetITStatus(EXTI_Line) != RESET){
    BSP_EXTI_Callback(EXTI_Line);
    EXTI_ClearITPendingBit(EXTI_Line);
  }
}

/**
  * @brief  EXTI0 中断服务函数
  * @details
  * 用于处理 EXTI Line0 对应的外部中断请求。
  */
void EXTI0_IRQHandler(void)
{
  BSP_EXTI_CommandHandler(EXTI_Line0);
}

/**
  * @brief  EXTI15_10 中断服务函数
  * @details
  * 用于处理 EXTI Line10 ~ Line15 共用中断入口。
  * 当前代码中仅实际转调处理 `EXTI_Line13`。
  *
  * @note
  * 由于 EXTI15_10 是多个线共享同一个 IRQ，因此如果后续启用更多 EXTI 线，
  * 需要在此函数中逐个检查对应线的中断状态。
  */
void EXTI15_10_IRQHandler(void)
{
  BSP_EXTI_CommandHandler(EXTI_Line13);
}
#endif

/**
  * @brief  USART1 中断服务函数
  * @details
  * 当 USART1 发生接收、发送完成、空闲中断或其他已使能的串口中断事件时，
  * CPU 会进入本函数。
  *
  * 本函数本身不直接处理具体业务，而是转调 BSP 层统一串口中断处理接口：
  * `BSP_USART_IRQHandler(BSP_USART1)`。
  *
  * @param  None
  * @retval None
  *
  * @note
  * 这种“IRQ 入口 + BSP 分发”的设计方式能让不同 USART 的中断处理流程保持一致。
  */
void USART1_IRQHandler(void)
{
  BSP_USART_IRQHandler(BSP_USART1);
}

/**
  * @brief  USART2 中断服务函数
  * @details
  * 进入该函数后，将 USART2 的中断处理统一转交给 BSP 串口驱动层。
  *
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{
  BSP_USART_IRQHandler(BSP_USART2);
}

/**
  * @brief  USART3 中断服务函数
  * @details
  * 进入该函数后，将 USART3 的中断处理统一转交给 BSP 串口驱动层。
  *
  * @param  None
  * @retval None
  */
void USART3_IRQHandler(void)
{
  BSP_USART_IRQHandler(BSP_USART3);
}

/**
  * @brief  UART4 中断服务函数
  * @details
  * 进入该函数后，将 UART4 的中断处理统一转交给 BSP 串口驱动层。
  *
  * @param  None
  * @retval None
  */
void UART4_IRQHandler(void)
{
  BSP_USART_IRQHandler(BSP_UART4);
}

/**
  * @brief  UART5 中断服务函数
  * @details
  * 进入该函数后，将 UART5 的中断处理统一转交给 BSP 串口驱动层。
  *
  * @param  None
  * @retval None
  */
void UART5_IRQHandler(void)
{
  BSP_USART_IRQHandler(BSP_UART5);
}

/**
  * @brief  TIM1 更新中断服务函数
  * @details
  * TIM1_UP_IRQHandler 对应 TIM1 的更新事件中断，
  * 通常与如下事件相关：
  * - 计数器溢出/更新
  * - 重载更新
  * - 重复计数更新
  *
  * 当前函数中实际处理逻辑被注释掉，说明本工程当前未启用 TIM1 更新中断处理。
  *
  * @param  None
  * @retval None
  *
  * @note
  * 当前 PWM 输入测量使用的是捕获比较中断（CC），不是更新中断（UP）。
  * 因此这里保留空实现属于正常设计，不是遗漏。
  */
void TIM1_UP_IRQHandler(void)
{
  //BSP_TIM_UP_IRQHandler(BSP_ADVANCED_TIMER1);
}

/**
  * @brief  TIM1 捕获比较中断服务函数
  * @details
  * 当 TIM1 的捕获比较事件发生时进入本函数。
  *
  * 在当前工程中，该中断主要用于高级定时器 PWM 输入测量，
  * 实际处理逻辑由 BSP 层统一函数完成：
  * `BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER1)`
  *
  * @param  None
  * @retval None
  *
  * @note
  * 这里的 “CC” 表示 Capture/Compare。
  * 对于本工程中的 PWM 输入测量场景，核心是“输入捕获”而非输出比较。
  */
void TIM1_CC_IRQHandler(void)
{
  BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER1);
}

/**
  * @brief  TIM8 更新中断服务函数
  * @details
  * TIM8_UP_IRQHandler 对应 TIM8 的更新事件中断。
  * 当前工程未启用对应实际处理，因此仅保留占位函数。
  *
  * @param  None
  * @retval None
  */
void TIM8_UP_IRQHandler(void)
{
  //BSP_TIM_UP_IRQHandler(BSP_ADVANCED_TIMER8);
}

/**
  * @brief  TIM8 捕获比较中断服务函数
  * @details
  * 当 TIM8 发生捕获比较中断时进入本函数，
  * 实际中断处理逻辑转交给 BSP 高级定时器模块统一处理。
  *
  * @param  None
  * @retval None
  *
  * @note
  * 如果当前工程并未实际初始化 TIM8，对应中断一般不会触发；
  * 保留该函数可以便于后续扩展使用 TIM8。
  */
void TIM8_CC_IRQHandler(void)
{
  BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER8);
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/