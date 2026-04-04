#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_nvic_group.h"
#include "bsp_advanced_timer.h"

#include <stdio.h>
#include <string.h>

/**
 * @file    main.c
 * @brief   高级定时器输入捕获测量脉宽示例程序
 * @details
 * 本示例演示如何使用 STM32F10x 高级定时器 TIM8 的输入捕获功能测量外部脉冲高电平宽度，
 * 并通过串口周期性输出测量结果。
 *
 * 当前示例配置如下：
 * - 使用 TIM8
 * - 选择 CH4 作为输入捕获通道
 * - 输入引脚为 PC9
 * - 串口输出使用 USART1
 *
 * 程序运行后，每隔 1 秒通过串口打印一次当前测得的脉宽值。
 */

/**
 * @brief 主函数
 * @return 按标准 C 语言约定返回 int，但在裸机系统中通常不会返回
 */
int main(void)
{
    /**
     * @brief 高级定时器基础配置
     * @details
     * 当前设置：
     * - PSC = 72 - 1
     * - 若定时器输入时钟为 72 MHz，则计数频率为 1 MHz
     * - 即 1 tick = 1 us
     */
    bsp_advancedtimer_config_t base_config = {
        .prescaler = 72 - 1,
        .counter_mode = BSP_TIM_COUNTER_MODE_UP,
        .period = 0xFFFF,
        .clock_div = BSP_TIM_CLOCK_DIV_1,
        .repetition_cnt = 0
    };

    /**
     * @brief 输入捕获配置
     * @details
     * 当前配置表示：
     * - 使用 CH4 输入捕获
     * - 初始捕获上升沿
     * - 输入映射方式为 DirectTI
     * - 每个有效边沿都参与捕获
     * - 不使用滤波
     */
    bsp_tim_ic_config_t ic_config = {
        .ic_channel = BSP_TIM_IC_CHANNEL4,
        .ic_polarity = BSP_TIM_IC_POLARITY_RISING,
        .ic_prescaler_div = BSP_TIM_IC_PRESCALER_DIV1,
        .ic_selection = BSP_TIM_IC_SELECTION_DIRECTTI,
        .ic_filter = 0x00
    };

    /** @brief 上次打印的系统节拍值 */
    uint32_t last_tick = 0;

    /**
     * @brief 定时器计数频率
     * @details
     * 当前配置下：
     * @code
     * counter_freq = 72MHz / 72 = 1MHz
     * @endcode
     */
    uint32_t counter_freq = CLK_FREQ / (base_config.prescaler + 1);

    /**
     * @brief 单个计数对应的时间（单位：us）
     * @details
     * 这里计算结果理论上为 1 us。
     *
     * @note
     * 当前写法使用浮点换算后再转 uint16_t。
     * 在本示例配置下没有问题，但如果后续计数频率较高，结果可能小于 1 us，
     * 此时该变量将不再适合作为整数 us 表示。
     */
    uint16_t period = (uint16_t)((float)(1.0 / counter_freq) * 1000000);

    BSP_LED_Init();
    BSP_TimeBase_Init();
    BSP_USART_Config(BSP_USART1);
    BSP_USART_Stdio(BSP_USART1);
    BSP_NVIC_GroupConfig();

    /* 配置 TIM8 输入捕获功能 */
    BSP_AdvancedTimer_Config(BSP_ADVANCED_TIMER8, &base_config, &ic_config);

    printf("25.高级定时器测量脉宽\r\n");

    while(1){
        if(BSP_GetTick() - last_tick >= 1000){
            last_tick = BSP_GetTick();

            printf("the width of pulse is %dus\r\n",
                    result[BSP_ADVANCED_TIMER8].pulse_width[ic_config.ic_channel] * period);
        }
    }
}