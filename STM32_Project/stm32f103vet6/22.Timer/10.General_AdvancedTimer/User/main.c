/**
 * @file main.c
 * @brief 通用定时器输出 PWM、高级定时器测量 PWM 的示例程序
 * @author
 * @date
 * @version 1.0
 *
 * @details
 * 本示例演示以下功能：
 * - 使用 TIM3 的 4 个输出比较通道产生 4 路 PWM 波形
 * - 使用 TIM1 的 PWM 输入捕获功能对外部输入 PWM 信号进行测量
 * - 通过 USART1 串口打印被测 PWM 的频率、占空比、周期计数值和高电平计数值
 *
 * 程序整体流程如下：
 * 1. 初始化 LED、系统时基、串口和中断优先级分组
 * 2. 配置 TIM3 输出 4 路不同占空比的 PWM
 * 3. 配置 TIM1 对输入的 PWM 信号进行测量
 * 4. 主循环中每隔 1 秒检查一次是否有新的 PWM 测量结果
 * 5. 若有新结果，则通过串口打印
 *
 * @note
 * 1. 本示例默认 PWM 输出和 PWM 输入已经通过外部连线连接，否则无法测得有效结果。
 * 2. 当前 TIM3 输出配置为：
 *    - CH1: 30%
 *    - CH2: 40%
 *    - CH3: 60%
 *    - CH4: 70%
 * 3. TIM1 当前配置为从 CH1 输入捕获 PWM，因此需要确认外部接线连接到了 TIM1_CH1。
 * 4. 若系统时钟或定时器时钟发生变化，PWM 频率和测量结果会随之变化。
 */

#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_nvic_group.h"
#include "bsp_general_timer.h"
#include "bsp_advanced_timer.h"

#include <stdio.h>
#include <string.h>

/**
 * @brief 主函数。
 *
 * @details
 * 本函数完成系统初始化、PWM 输出配置、PWM 输入测量配置，并在主循环中
 * 周期性打印测量结果。
 *
 * @retval int 程序正常情况下不会返回
 */
int main(void)
{
    /**
     * @brief 高级定时器 TIM1 基础配置参数。
     *
     * @details
     * 该配置用于 PWM 输入测量：
     * - 预分频为 72 - 1，即计数频率约为 1 MHz（若 TIM1 时钟为 72 MHz）
     * - 向上计数
     * - 自动重装载值为 0xFFFF
     * - 时钟分频为 1
     * - 重复计数值为 0
     *
     * 因此在该配置下，计数器分辨率约为 1 us。
     */
	 bsp_advancedtimer_config_t abase_config = {
        .prescaler = 72 - 1,
        .counter_mode = BSP_TIM_COUNTER_MODE_UP,
        .period = 0xFFFF,
        .clock_div = BSP_TIM_CLOCK_DIV_1,
        .repetition_cnt = 0
    };

    /**
     * @brief TIM1 PWM 输入捕获配置参数。
     *
     * @details
     * 当前配置说明：
     * - 从 CH1 输入 PWM
     * - 以上升沿作为主捕获边沿
     * - 输入捕获不分频
     * - 不使用数字滤波
     */
	bsp_tim_pwm_config_t pwm_config = {
        .pwm_channel = BSP_TIM_PWM_INPUT_CHANNEL1,
        .ic_polarity = BSP_TIM_IC_POLARITY_RISING,
        .ic_prescaler_div = BSP_TIM_IC_PRESCALER_DIV1,
        .ic_filter = 0x00
    };

    /**
     * @brief 上一次打印时间戳。
     *
     * @details
     * 用于控制主循环每隔 1000ms 检查并打印一次测量结果。
     */
    uint32_t last_tick = 0;

    /**
     * @brief 指向 TIM1 PWM 输入测量结果结构体的指针。
     *
     * @details
     * 该结果结构体由高级定时器捕获中断更新，
     * 主循环中通过该指针读取最新测量结果。
     */
    bsp_tim_ic_result_t *res = &result[BSP_ADVANCED_TIMER1];

    /**
     * @brief 通用定时器 TIM3 基础配置参数。
     *
     * @details
     * 当前配置：
     * - 预分频：7200 - 1
     * - 计数模式：向上计数
     * - 自动重装载值：100 - 1
     * - 时钟分频：1
     * - 重复计数值：0
     *
     * 若 TIM3 时钟为 72 MHz，则：
     * - 计数频率 = 72 MHz / 7200 = 10 kHz
     * - PWM 周期 = 100 个计数周期
     * - PWM 频率 = 10 kHz / 100 = 100 Hz
     */
	bsp_generaltimer_config_t gbase_config = {
		.prescaler = 7200 - 1,
		.counter_mode = BSP_TIM_COUNTER_MODE_UP,
		.period = 100 - 1,
		.clock_div = BSP_TIM_CLOCK_DIV_1,
		.repetition_cnt = 0
	};

    /**
     * @brief TIM3 通道 1 PWM 输出配置。
     *
     * @details
     * - 通道：CH1
     * - 模式：PWM1
     * - 比较值 CCR = 30
     *
     * 当 ARR = 99 时，占空比约为 30%。
     */
	bsp_tim_oc_config_t oc_config1 = {
		.oc_channel = BSP_TIM_OC_CHANNEL1,
		.oc_mode = BSP_TIM_OC_MODE_PWM1,
		.oc_output_state = BSP_TIM_OUTPUT_STATE_ENABLE,
		.oc_output_nstate = BSP_TIM_OUTPUT_NSTATE_DISABLE,
		.oc_polarity = BSP_TIM_OC_POLARITY_HIGH,
		.oc_npolarity = BSP_TIM_OC_NPOLARITY_HIGH,
		.oc_idle_state = BSP_TIM_OC_IDLE_STATE_SET,
		.oc_nidle_state = BSP_TIM_OC_NIDLE_STATE_SET,
		.oc_ccr_value = 30
	};

    /**
     * @brief TIM3 通道 2 PWM 输出配置。
     *
     * @details
     * 占空比约为 40%。
     */
	bsp_tim_oc_config_t oc_config2 = {
		.oc_channel = BSP_TIM_OC_CHANNEL2,
		.oc_mode = BSP_TIM_OC_MODE_PWM1,
		.oc_output_state = BSP_TIM_OUTPUT_STATE_ENABLE,
		.oc_output_nstate = BSP_TIM_OUTPUT_NSTATE_DISABLE,
		.oc_polarity = BSP_TIM_OC_POLARITY_HIGH,
		.oc_npolarity = BSP_TIM_OC_NPOLARITY_HIGH,
		.oc_idle_state = BSP_TIM_OC_IDLE_STATE_SET,
		.oc_nidle_state = BSP_TIM_OC_NIDLE_STATE_SET,
		.oc_ccr_value = 40
	};

    /**
     * @brief TIM3 通道 3 PWM 输出配置。
     *
     * @details
     * 占空比约为 60%。
     */
	bsp_tim_oc_config_t oc_config3 = {
		.oc_channel = BSP_TIM_OC_CHANNEL3,
		.oc_mode = BSP_TIM_OC_MODE_PWM1,
		.oc_output_state = BSP_TIM_OUTPUT_STATE_ENABLE,
		.oc_output_nstate = BSP_TIM_OUTPUT_NSTATE_DISABLE,
		.oc_polarity = BSP_TIM_OC_POLARITY_HIGH,
		.oc_npolarity = BSP_TIM_OC_NPOLARITY_HIGH,
		.oc_idle_state = BSP_TIM_OC_IDLE_STATE_SET,
		.oc_nidle_state = BSP_TIM_OC_NIDLE_STATE_SET,
		.oc_ccr_value = 60
	};

    /**
     * @brief TIM3 通道 4 PWM 输出配置。
     *
     * @details
     * 占空比约为 70%。
     */
	bsp_tim_oc_config_t oc_config4 = {
		.oc_channel = BSP_TIM_OC_CHANNEL4,
		.oc_mode = BSP_TIM_OC_MODE_PWM1,
		.oc_output_state = BSP_TIM_OUTPUT_STATE_ENABLE,
		.oc_output_nstate = BSP_TIM_OUTPUT_NSTATE_DISABLE,
		.oc_polarity = BSP_TIM_OC_POLARITY_HIGH,
		.oc_npolarity = BSP_TIM_OC_NPOLARITY_HIGH,
		.oc_idle_state = BSP_TIM_OC_IDLE_STATE_SET,
		.oc_nidle_state = BSP_TIM_OC_NIDLE_STATE_SET,
		.oc_ccr_value = 70
	};

    /**
     * @brief 初始化基础外设。
     *
     * @details
     * 依次完成：
     * - LED 初始化
     * - 系统 Tick 时基初始化
     * - USART1 配置
     * - printf 重定向到 USART1
     * - NVIC 中断优先级分组配置
     */
    BSP_LED_Init();
    BSP_TimeBase_Init();
    BSP_USART_Config(BSP_USART1);
    BSP_USART_Stdio(BSP_USART1);
	BSP_NVIC_GroupConfig();

    /**
     * @brief 配置 TIM3 输出 4 路 PWM。
     *
     * @details
     * 分别配置 TIM3 的 CH1~CH4 通道输出不同占空比的 PWM 波形。
     *
     * @note
     * 这里多次调用 BSP_GeneralTIM_Config()，每次配置一个通道。
     */
	BSP_GeneralTIM_Config(BSP_GENERAL_TIMER3, &gbase_config, &oc_config1);
	BSP_GeneralTIM_Config(BSP_GENERAL_TIMER3, &gbase_config, &oc_config2);
	BSP_GeneralTIM_Config(BSP_GENERAL_TIMER3, &gbase_config, &oc_config3);
	BSP_GeneralTIM_Config(BSP_GENERAL_TIMER3, &gbase_config, &oc_config4);

    /**
     * @brief 配置 TIM1 为 PWM 输入测量模式。
     *
     * @details
     * 使用 TIM1_CH1 对外部输入 PWM 波形进行周期和高电平脉宽测量。
     */
	BSP_AdvancedTimer_Config(BSP_ADVANCED_TIMER1, &abase_config, &pwm_config);

    /**
     * @brief 打印示例说明信息。
     */
    printf("25.通用定时器输出PWM波,高级定时器测量PWM\r\n");

    /**
     * @brief 主循环。
     *
     * @details
     * 每隔 1000ms 检查一次 PWM 输入测量结果是否更新：
     * - 若更新标志为 1，则输出当前频率、占空比、周期计数值和高电平计数值
     * - 打印完成后清除更新标志
     */
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