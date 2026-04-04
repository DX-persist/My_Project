#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_nvic_group.h"
#include "bsp_advanced_timer.h"

#include <stdio.h>
#include <string.h>

/**
 * @file    main.c
 * @brief   高级定时器基础时基定时示例程序
 * @details
 * 本示例演示如何使用 STM32F10x 高级定时器 TIM8 作为基础时基定时器使用，
 * 并通过更新中断计数实现简单的周期性 LED 翻转。
 *
 * 当前示例配置如下：
 * - 使用 TIM8
 * - PSC = 72 - 1
 * - ARR = 1000 - 1
 * - 向上计数模式
 *
 * 若定时器输入时钟为 72 MHz，则：
 * - 计数频率 = 72MHz / 72 = 1MHz
 * - 单次更新周期 = 1000 / 1MHz = 1ms
 *
 * 程序中每发生 1000 次更新中断，就翻转一次红色 LED，
 * 因此 LED 翻转周期约为 1 秒。
 */

/**
 * @brief 主函数
 * @return 按标准 C 语言约定返回 int，但裸机系统中通常不会返回
 */
int main(void)
{
    /**
     * @brief 高级定时器基础时基配置
     * @details
     * 当前配置下：
     * - 预分频后计数频率为 1 MHz
     * - ARR = 1000 - 1，对应 1000 个计数一次更新
     * - 因此更新事件周期为 1 ms
     */
    bsp_advancedtimer_config_t advanced_config = {
        .prescaler = 72 - 1,
        .counter_mode = BSP_TIM_COUNTER_MODE_UP,
        .period = 1000 - 1,
        .clock_div = BSP_TIM_CLOCK_DIV_1,
        .repetition_cnt = 0
    };

    BSP_LED_Init();
    BSP_TimeBase_Init();
    BSP_USART_Config(BSP_USART1);
    BSP_USART_Stdio(BSP_USART1);
    BSP_NVIC_GroupConfig();

    /* 初始化 TIM8 基础时基与更新中断 */
    BSP_BaseTIM_Init(BSP_ADVANCED_TIMER8, &advanced_config);

    printf("25.基本定时器定时\r\n");

    while(1){
        /**
         * @note
         * 当前 `advancedtimer_cnt[BSP_ADVANCED_TIMER8]` 每 1ms 自增一次。
         * 当其累积到 1000 时，说明经过约 1 秒。
         */
        if(advancedtimer_cnt[BSP_ADVANCED_TIMER8] == 1000){
            advancedtimer_cnt[BSP_ADVANCED_TIMER8] = 0;
            BSP_LED_Toggle(LED_RED);
        }
    }
}