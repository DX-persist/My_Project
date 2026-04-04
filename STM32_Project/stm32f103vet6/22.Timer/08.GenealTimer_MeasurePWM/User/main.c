#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_nvic_group.h"
#include "bsp_general_timer.h"

#include <stdio.h>
#include <string.h>

/**
 * @file    main.c
 * @brief   通用定时器 PWM 输入测量示例程序
 * @details
 * 本示例演示如何使用 STM32F10x 通用定时器模块测量外部 PWM 信号的频率和占空比。
 *
 * 当前示例配置如下：
 * - 使用 TIM4
 * - 选择 TIM4_CH2 作为 PWM 输入源
 * - PWM 输入引脚为 PB7
 * - 串口输出使用 USART1
 *
 * 程序运行后，每隔 1 秒检查一次是否有新的测量结果。
 * 若有，则通过串口打印：
 * - PWM 频率
 * - PWM 占空比
 * - 周期计数值
 * - 高电平计数值
 */

/**
 * @brief 主函数
 * @return 按标准 C 语言约定返回 int，但裸机程序通常不会返回
 */
int main(void)
{
    /** @brief 上次串口打印的系统节拍值 */
    uint32_t last_tick = 0;

    /**
     * @brief 通用定时器基础参数配置
     * @details
     * 当前预分频配置为 36 - 1。
     *
     * 若定时器输入时钟为 72 MHz，则计数频率为：
     * @code
     * 72 MHz / 36 = 2 MHz
     * @endcode
     *
     * 即：
     * - 1 tick = 0.5 us
     *
     *
     */
    bsp_generaltimer_config_t base_config = {
        .prescaler = 36 - 1,
        .counter_mode = BSP_TIM_COUNTER_MODE_UP,
        .period = 0xFFFF,
        .clock_div = BSP_TIM_CLOCK_DIV_1,
        .repetition_cnt = 0
    };

    /**
     * @brief PWM 输入配置
     * @details
     * 当前配置表示：
     * - 从 CH2 引脚输入 PWM
     * - 使用上升沿捕获
     * - 每个有效边沿都参与捕获
     * - 不使用输入滤波
     */
    bsp_tim_pwm_input_config_t pwm_config = {
        .input_channel = BSP_TIM_PWM_INPUT_CHANNEL2,
        .ic_polarity = BSP_TIM_IC_POLARITY_RISING,
        .ic_prescaler = BSP_TIM_IC_PRESCALER_DIV1,
        .ic_filter = 0
    };

    /**
     * @brief 指向 TIM4 测量结果槽位的指针
     * @details
     * 由于当前初始化的是 TIM4，因此这里读取 result[BSP_GENERAL_TIMER4]。
     */
    bsp_tim_ic_result_t *res = &result[BSP_GENERAL_TIMER4];

    BSP_LED_Init();
    BSP_TimeBase_Init();
    BSP_USART_Config(BSP_USART1);
    BSP_USART_Stdio(BSP_USART1);
    BSP_NVIC_GroupConfig();

    /* 配置 TIM4 PWM 输入测量功能 */
    BSP_GeneralTIM_Config(BSP_GENERAL_TIMER4, &base_config, &pwm_config);

    printf("25.通用定时器测量PWM\r\n");

    while(1){
        if(BSP_GetTick() - last_tick > 1000){
            last_tick = BSP_GetTick();

            if(res->g_pwm_update == 1){
                printf("freq = %lu Hz, duty = %.2f%%, period_cnt = %lu, high_cnt = %lu\r\n",
                    res->g_pwm_freq, res->g_pwm_duty, res->g_pwm_period, res->g_pwm_pulse_width);
                res->g_pwm_update = 0;
            }
        }
    }
}
