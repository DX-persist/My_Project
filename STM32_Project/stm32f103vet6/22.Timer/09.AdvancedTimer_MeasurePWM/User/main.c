/**
 * @file    main.c
 * @brief   高级定时器 PWM 输入测量示例程序
 * @details
 * 本示例演示如何使用 BSP 高级定时器模块对外部 PWM 信号进行测量，
 * 并通过串口周期性打印测量结果，包括：
 * - PWM 频率
 * - PWM 占空比
 * - 周期计数值
 * - 高电平计数值
 *
 * 示例流程：
 * 1. 初始化 LED、系统节拍、串口、NVIC 分组
 * 2. 配置 TIM1 为 PWM 输入测量模式
 * 3. 在主循环中每隔 1 秒检查一次是否有新的测量结果
 * 4. 若有新结果，则通过串口输出
 *
 * @note
 * 该示例中使用 `BSP_ADVANCED_TIMER8`，并将输入源配置为 `CHANNEL1`，
 * 即默认从 TIM1_CH2（PA9）输入 PWM 信号。
 */

#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_nvic_group.h"
#include "bsp_advanced_timer.h"

#include <stdio.h>
#include <string.h>

/**
 * @brief 主函数
 * @return
 * 按标准 C 语言约定返回 `int`，但在嵌入式裸机程序中通常不会返回。
 *
 * @details
 * 本函数完成系统初始化，并循环输出 PWM 输入测量结果。
 */
int main(void)
{
    /**
     * @brief 高级定时器基础参数配置
     * @details
     * 当前配置含义：
     * - 预分频：72 - 1
     * - 若定时器输入时钟为 72 MHz，则计数频率为 1 MHz
     * - 即每个计数约为 1 us
     *
     * 这使得：
     * - `period_cnt` 近似等于 PWM 周期的微秒数
     * - `high_cnt`   近似等于高电平持续时间的微秒数
     */
    bsp_advancedtimer_config_t base_config = {
        .prescaler = 72 - 1,
        .counter_mode = BSP_TIM_COUNTER_MODE_UP,
        .period = 0xFFFF,
        .clock_div = BSP_TIM_CLOCK_DIV_1,
        .repetition_cnt = 0
    };

    /**
     * @brief PWM 输入配置
     * @details
     * 当前配置选择：
     * - 从 TIM1_CH2 输入 PWM 信号
     * - 上升沿捕获
     * - 每个边沿都参与捕获
     * - 不使用输入滤波
     */
    bsp_tim_pwm_config_t pwm_config = {
        .pwm_channel = BSP_TIM_PWM_INPUT_CHANNEL1,
        .ic_polarity = BSP_TIM_IC_POLARITY_RISING,
        .ic_prescaler_div = BSP_TIM_IC_PRESCALER_DIV1,
        .ic_filter = 0x00
    };

    /** @brief 上次打印的系统节拍时间 */
    uint32_t last_tick = 0;

    /**
     * @brief TIM1 对应的 PWM 测量结果指针
     * @note
     * `result` 数组由中断更新，这里仅在主循环中读取。
     */
    bsp_tim_ic_result_t *res = &result[BSP_ADVANCED_TIMER8];

    BSP_LED_Init();
    BSP_TimeBase_Init();
    BSP_USART_Config(BSP_USART1);
    BSP_USART_Stdio(BSP_USART1);
    BSP_NVIC_GroupConfig();

    BSP_AdvancedTimer_Config(BSP_ADVANCED_TIMER8, &base_config, &pwm_config);

    printf("25.高级定时器测量PWM\r\n");

    while(1){
        if(BSP_GetTick() - last_tick >= 1000){
            last_tick = BSP_GetTick();

            if(res->g_pwm_update == 1){
                printf("freq = %lu Hz, duty = %.2f%%, period_cnt = %lu, high_cnt = %lu\r\n",
                    res->g_pwm_freq, res->g_pwm_duty, res->g_pwm_period, res->g_pwm_pulse_width);
                res->g_pwm_update = 0;
            }
        }
    }
}
