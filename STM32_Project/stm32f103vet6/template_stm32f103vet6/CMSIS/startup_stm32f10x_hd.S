.syntax unified
.cpu cortex-m3
.fpu softvfp
.thumb

.global  g_pfnVectors
.global  Reset_Handler

/* External symbols */
.extern  SystemInit
.extern  main

/* Linker symbols */
.word _estack

/* ---------------- Vector Table ---------------- */

.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object

g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler

  /* External Interrupts */
  .word WWDG_IRQHandler
  .word PVD_IRQHandler
  .word TAMPER_IRQHandler
  .word RTC_IRQHandler
  .word FLASH_IRQHandler
  .word RCC_IRQHandler
  .word EXTI0_IRQHandler
  .word EXTI1_IRQHandler
  .word EXTI2_IRQHandler
  .word EXTI3_IRQHandler
  .word EXTI4_IRQHandler
  .word DMA1_Channel1_IRQHandler
  .word DMA1_Channel2_IRQHandler
  .word DMA1_Channel3_IRQHandler
  .word DMA1_Channel4_IRQHandler
  .word DMA1_Channel5_IRQHandler
  .word DMA1_Channel6_IRQHandler
  .word DMA1_Channel7_IRQHandler
  .word ADC1_2_IRQHandler
  .word USB_HP_CAN1_TX_IRQHandler
  .word USB_LP_CAN1_RX0_IRQHandler
  .word CAN1_RX1_IRQHandler
  .word CAN1_SCE_IRQHandler
  .word EXTI9_5_IRQHandler
  .word TIM1_BRK_IRQHandler
  .word TIM1_UP_IRQHandler
  .word TIM1_TRG_COM_IRQHandler
  .word TIM1_CC_IRQHandler
  .word TIM2_IRQHandler
  .word TIM3_IRQHandler
  .word TIM4_IRQHandler
  .word I2C1_EV_IRQHandler
  .word I2C1_ER_IRQHandler
  .word I2C2_EV_IRQHandler
  .word I2C2_ER_IRQHandler
  .word SPI1_IRQHandler
  .word SPI2_IRQHandler
  .word USART1_IRQHandler
  .word USART2_IRQHandler
  .word USART3_IRQHandler
  .word EXTI15_10_IRQHandler
  .word RTCAlarm_IRQHandler
  .word USBWakeUp_IRQHandler
  .word TIM8_BRK_IRQHandler
  .word TIM8_UP_IRQHandler
  .word TIM8_TRG_COM_IRQHandler
  .word TIM8_CC_IRQHandler
  .word ADC3_IRQHandler
  .word FSMC_IRQHandler
  .word SDIO_IRQHandler
  .word TIM5_IRQHandler
  .word SPI3_IRQHandler
  .word UART4_IRQHandler
  .word UART5_IRQHandler
  .word TIM6_IRQHandler
  .word TIM7_IRQHandler
  .word DMA2_Channel1_IRQHandler
  .word DMA2_Channel2_IRQHandler
  .word DMA2_Channel3_IRQHandler
  .word DMA2_Channel4_5_IRQHandler

/* ---------------- Reset ---------------- */

.section .text.Reset_Handler
.type Reset_Handler, %function
Reset_Handler:

  /* 1. 关闭中断 */
  cpsid   i

  /* 2. 初始化 MSP */
  ldr     r0, =_estack
  msr     msp, r0

  /* 3. 拷贝 .data (Flash -> RAM) */
  ldr     r0, =_sidata    /* Flash */
  ldr     r1, =_sdata     /* RAM */
  ldr     r2, =_edata     /* RAM end */

CopyData:
  cmp     r1, r2
  ittt    lt
  ldrlt   r3, [r0], #4
  strlt   r3, [r1], #4
  blt     CopyData

  /* 4. 清零 .bss */
  ldr     r0, =_sbss
  ldr     r1, =_ebss
  movs    r2, #0

ZeroBSS:
  cmp     r0, r1
  it      lt
  strlt   r2, [r0], #4
  blt     ZeroBSS

  /* 5. 初始化时钟系统 */
  bl      SystemInit

  /* 6. 进入 C 语言世界 */
  bl      main

Hang:
  b Hang

/* ---------------- Default Handler ---------------- */

.section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b Infinite_Loop

/* ---------------- Weak aliases ---------------- */

.macro  DEF_IRQ  name
  .weak \name
  .thumb_set \name, Default_Handler
.endm

DEF_IRQ NMI_Handler
DEF_IRQ HardFault_Handler
DEF_IRQ MemManage_Handler
DEF_IRQ BusFault_Handler
DEF_IRQ UsageFault_Handler
DEF_IRQ SVC_Handler
DEF_IRQ DebugMon_Handler
DEF_IRQ PendSV_Handler
DEF_IRQ SysTick_Handler

DEF_IRQ WWDG_IRQHandler
DEF_IRQ PVD_IRQHandler
DEF_IRQ TAMPER_IRQHandler
DEF_IRQ RTC_IRQHandler
DEF_IRQ FLASH_IRQHandler
DEF_IRQ RCC_IRQHandler
DEF_IRQ EXTI0_IRQHandler
DEF_IRQ EXTI1_IRQHandler
DEF_IRQ EXTI2_IRQHandler
DEF_IRQ EXTI3_IRQHandler
DEF_IRQ EXTI4_IRQHandler
DEF_IRQ DMA1_Channel1_IRQHandler
DEF_IRQ DMA1_Channel2_IRQHandler
DEF_IRQ DMA1_Channel3_IRQHandler
DEF_IRQ DMA1_Channel4_IRQHandler
DEF_IRQ DMA1_Channel5_IRQHandler
DEF_IRQ DMA1_Channel6_IRQHandler
DEF_IRQ DMA1_Channel7_IRQHandler
DEF_IRQ ADC1_2_IRQHandler
DEF_IRQ USB_HP_CAN1_TX_IRQHandler
DEF_IRQ USB_LP_CAN1_RX0_IRQHandler
DEF_IRQ CAN1_RX1_IRQHandler
DEF_IRQ CAN1_SCE_IRQHandler
DEF_IRQ EXTI9_5_IRQHandler
DEF_IRQ TIM1_BRK_IRQHandler
DEF_IRQ TIM1_UP_IRQHandler
DEF_IRQ TIM1_TRG_COM_IRQHandler
DEF_IRQ TIM1_CC_IRQHandler
DEF_IRQ TIM2_IRQHandler
DEF_IRQ TIM3_IRQHandler
DEF_IRQ TIM4_IRQHandler
DEF_IRQ I2C1_EV_IRQHandler
DEF_IRQ I2C1_ER_IRQHandler
DEF_IRQ I2C2_EV_IRQHandler
DEF_IRQ I2C2_ER_IRQHandler
DEF_IRQ SPI1_IRQHandler
DEF_IRQ SPI2_IRQHandler
DEF_IRQ USART1_IRQHandler
DEF_IRQ USART2_IRQHandler
DEF_IRQ USART3_IRQHandler
DEF_IRQ EXTI15_10_IRQHandler
DEF_IRQ RTCAlarm_IRQHandler
DEF_IRQ USBWakeUp_IRQHandler
DEF_IRQ TIM8_BRK_IRQHandler
DEF_IRQ TIM8_UP_IRQHandler
DEF_IRQ TIM8_TRG_COM_IRQHandler
DEF_IRQ TIM8_CC_IRQHandler
DEF_IRQ ADC3_IRQHandler
DEF_IRQ FSMC_IRQHandler
DEF_IRQ SDIO_IRQHandler
DEF_IRQ TIM5_IRQHandler
DEF_IRQ SPI3_IRQHandler
DEF_IRQ UART4_IRQHandler
DEF_IRQ UART5_IRQHandler
DEF_IRQ TIM6_IRQHandler
DEF_IRQ TIM7_IRQHandler
DEF_IRQ DMA2_Channel1_IRQHandler
DEF_IRQ DMA2_Channel2_IRQHandler
DEF_IRQ DMA2_Channel3_IRQHandler
DEF_IRQ DMA2_Channel4_5_IRQHandler

