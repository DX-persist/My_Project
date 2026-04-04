#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_nvic_group.h"
#include "bsp_advanced_timer.h"

#include <stdio.h>
#include <string.h>

/**
 * @file    main.c
 * @brief   高级定时器 PWM 互补输出与 BDTR 配置示例程序
 * @details
 * 本示例演示如何使用 STM32F10x 高级定时器 TIM1 产生 PWM 输出，
 * 同时开启互补输出，并配置 BDTR（死区、刹车、自动输出等）参数。
 *
 * 当前示例配置如下：
 * - 使用 TIM1
 * - 使用 CH2 主输出与 CH2N 互补输出
 * - PWM 模式为 PWM1
 * - CCR = 50
 * - PSC = 7200 - 1
 * - ARR = 100 - 1
 *
 * 因此当前 PWM 参数为：
 * - 定时器计数频率：72MHz / 7200 = 10kHz
 * - PWM 周期计数：100
 * - PWM 频率：10kHz / 100 = 100Hz
 * - 占空比：50 / 100 = 50%
 *
 * @note
 * 由于开启了互补输出和死区时间，本示例更适合作为高级 PWM 驱动模板使用。
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
     * - 定时器输入时钟假设为 72 MHz
     * - PSC = 7200 - 1，则计数频率 = 10 kHz
     * - ARR = 100 - 1，则 PWM 周期 = 100 个计数
     * - 最终 PWM 频率 = 100 Hz
     */
    bsp_advancedtimer_config_t base_config = {
        .prescaler = 7200 - 1,
        .counter_mode = BSP_TIM_COUNTER_MODE_UP,
        .period = 100 - 1,
        .clock_div = BSP_TIM_CLOCK_DIV_1,
        .repetition_cnt = 0
    };

    /**
     * @brief 输出比较/PWM 配置
     * @details
     * 当前配置表示：
     * - 使用 CH2
     * - PWM 模式 1
     * - 主输出使能
     * - 互补输出使能
     * - CCR = 50，对应 50% 占空比
     * - 主输出高电平有效
     * - 互补输出低电平有效
     * - 空闲状态分别按配置输出
     */
    bsp_tim_oc_config_t oc_config = {
        .oc_channel = BSP_TIM_OC_CHANNEL1,
        .oc_mode = BSP_TIM_OC_MODE_PWM1,
        .oc_output_state = BSP_TIM_OUTPUT_STATE_ENABLE,
        .oc_output_nstate = BSP_TIM_OUTPUT_NSTATE_ENABLE,
        .oc_polarity = BSP_TIM_OC_POLARITY_HIGH,
        .oc_npolarity = BSP_TIM_OC_NPOLARITY_LOW,
        .oc_idle_state = BSP_TIM_OC_IDLE_STATE_RESET,
        .oc_nidle_state = BSP_TIM_OC_NIDLE_STATE_SET,
        .oc_ccr_value = 50
    };

    /**
     * @brief BDTR 配置
     * @details
     * 当前设置：
     * - OSSI 使能
     * - OSSR 使能
     * - 不加锁
     * - 刹车功能使能
     * - 刹车低电平触发
     * - 自动输出使能
     * - 死区时间 = 11
     *
     * @note
     * `bdtr_dead_time = 11` 是寄存器编码值，不应直接简单理解为 11us。
     * 具体实际死区时间需结合定时器时钟与芯片参考手册中的 DTG 编码规则计算。
     */
    bsp_tim_bdtr_config_t bdtr_config = {
        .bdtr_ossi_state = BSP_TIM_BDTR_OSSI_STATE_ENABLE,
        .bdtr_ossr_state = BSP_TIM_BDTR_OSSR_STATE_ENABLE,
        .bdtr_lock_level = BSP_TIM_BDTR_LOCK_LEVEL_OFF,
        .bdtr_break = BSP_TIM_BDTR_BREAK_ENABLE,
        .bdtr_break_polarity = BSP_TIM_BDTR_BREAK_POLARITY_LOW,
        .bdtr_auto_output = BSP_TIM_BDTR_AUTO_OUTPUT_ENABLE,
        .bdtr_dead_time = 11
    };

    BSP_LED_Init();
    BSP_TimeBase_Init();
    BSP_USART_Config(BSP_USART1);
    BSP_USART_Stdio(BSP_USART1);
    BSP_NVIC_GroupConfig();

    /* 配置 TIM1 高级 PWM / 互补输出 / BDTR 功能 */
    BSP_AdvancedTimer_Config(BSP_ADVANCED_TIMER1, &base_config, &oc_config, &bdtr_config);

    printf("25.基本定时器定时\r\n");

    while(1){

    }
}
