#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_nvic_group.h"
#include "bsp_general_timer.h"

#include <stdio.h>
#include <string.h>

/**
 * @file    main.c
 * @brief   通用定时器 PWM 输出示例程序
 * @details
 * 本示例演示如何使用 STM32F10x 通用定时器 TIM3 产生 PWM 输出波形。
 *
 * 当前示例配置如下：
 * - 使用 TIM3
 * - 使用 CH1 输出
 * - PWM 模式为 PWM1
 * - PSC = 7200 - 1
 * - ARR = 100 - 1
 * - CCR = 50
 *
 * 因此当前 PWM 参数为：
 * - 定时器计数频率：72MHz / 7200 = 10kHz
 * - PWM 频率：10kHz / 100 = 100Hz
 * - 占空比：50 / 100 = 50%
 *
 * 当前输出引脚为：
 * - TIM3_CH1 -> PA6
 */

/**
 * @brief 主函数
 * @return 按标准 C 语言约定返回 int，但在裸机系统中通常不会返回
 */
int main(void)
{
    /**
     * @brief 通用定时器基础时基配置
     * @details
     * 当前配置下：
     * - 输入时钟假设为 72 MHz
     * - PSC = 7200 - 1，则计数频率 = 10 kHz
     * - ARR = 100 - 1，则 PWM 周期 = 100 个计数
     * - 最终 PWM 频率 = 100 Hz
     */
    bsp_generaltimer_config_t base_config = {
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
     * - 使用 CH1
     * - PWM 模式 1
     * - 主输出使能
     * - CCR = 50，对应约 50% 占空比
     * - 主输出高电平有效
     *
     * @note
     * 虽然这里也配置了 `oc_output_nstate`、`oc_npolarity`、`oc_nidle_state`，
     * 但普通定时器没有互补输出，这些字段在当前场景下通常没有实际硬件意义。
     */
    bsp_tim_oc_config_t oc_config = {
        .oc_channel = BSP_TIM_OC_CHANNEL1,
        .oc_mode = BSP_TIM_OC_MODE_PWM1,
        .oc_output_state = BSP_TIM_OUTPUT_STATE_ENABLE,
        .oc_output_nstate = BSP_TIM_OUTPUT_NSTATE_DISABLE,
        .oc_polarity = BSP_TIM_OC_POLARITY_HIGH,
        .oc_npolarity = BSP_TIM_OC_NPOLARITY_HIGH,
        .oc_idle_state = BSP_TIM_OC_IDLE_STATE_SET,
        .oc_nidle_state = BSP_TIM_OC_NIDLE_STATE_SET,
        .oc_ccr_value = 50
    };

    BSP_LED_Init();
    BSP_TimeBase_Init();
    BSP_USART_Config(BSP_USART1);
    BSP_USART_Stdio(BSP_USART1);
    BSP_NVIC_GroupConfig();

    /* 配置 TIM3 PWM 输出功能 */
    BSP_GeneralTIM_Config(BSP_GENERAL_TIMER3, &base_config, &oc_config);

    printf("25.基本定时器定时\r\n");

    while(1){

    }
}