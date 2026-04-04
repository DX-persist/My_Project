/**
 * @file bsp_advanced_timer.h
 * @brief STM32F10x 高级定时器 BSP 驱动头文件
 * @author
 * @date
 * @version 1.0
 *
 * @details
 * 本模块对 STM32F10x 系列高级定时器 TIM1 和 TIM8 进行了统一封装，
 * 主要用于 PWM 输入捕获测量场景，可测量外部 PWM 信号的：
 * - 周期
 * - 脉宽
 * - 占空比
 * - 频率
 *
 * 模块内部基于 STM32 标准外设库的 PWM Input 模式实现，
 * 支持通过 CH1 或 CH2 作为输入源进行测量，并在捕获中断中完成结果更新。
 *
 * @note
 * 1. 本模块当前主要实现 PWM 输入测量功能，并未覆盖高级定时器全部高级特性。
 * 2. 捕获结果会在中断中更新，因此结果结构体中的成员均建议视为中断共享变量。
 * 3. 频率计算默认使用宏 @ref CLK_FREQ 作为定时器输入时钟频率，若工程时钟配置发生变化，
 *    需要同步修改该宏或改为动态获取时钟频率。
 */

#ifndef BSP_ADVANCED_TIMER_H
#define BSP_ADVANCED_TIMER_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include "bsp_tim_common.h"
#include <stdlib.h>

/**
 * @def UP_PREEMPT_PRIO
 * @brief 更新中断抢占优先级。
 *
 * @note 当前文件中未实际使用该宏。
 */
#define UP_PREEMPT_PRIO	1

/**
 * @def UP_SUB_PRIO
 * @brief 更新中断子优先级。
 *
 * @note 当前文件中未实际使用该宏。
 */
#define UP_SUB_PRIO		0

/**
 * @def CC_PREEPT_PRIO
 * @brief 捕获比较中断抢占优先级。
 */
#define CC_PREEPT_PRIO  1

/**
 * @def CC_SUB_PRIO
 * @brief 捕获比较中断子优先级。
 */
#define CC_SUB_PRIO     1

/**
 * @def CLK_FREQ
 * @brief 定时器输入时钟频率，单位 Hz。
 *
 * @details
 * 当前用于在 PWM 输入捕获完成后计算输入信号频率：
 * @code
 * pwm_freq = CLK_FREQ / (PSC + 1) / period
 * @endcode
 *
 * @warning
 * 若系统时钟树配置改变，或 TIM1/TIM8 实际时钟频率不再为 72MHz，
 * 则该宏需要同步修改，否则频率计算结果将不准确。
 */
#define CLK_FREQ        72000000U

/**
 * @enum bsp_advancedtimer_t
 * @brief 高级定时器编号枚举。
 *
 * 用于标识本模块支持的高级定时器实例。
 */
typedef enum{
	/**< 高级定时器 TIM1 */
	BSP_ADVANCED_TIMER1 = 0,
	/**< 高级定时器 TIM8 */
	BSP_ADVANCED_TIMER8,
	/**< 高级定时器数量上限 */
	BSP_ADVANCED_TIMER_MAX
}bsp_advancedtimer_t;

/**
 * @enum bsp_tim_pwm_input_channel_t
 * @brief PWM 输入捕获通道枚举。
 *
 * PWM 输入模式下，可选择定时器 CH1 或 CH2 作为输入源。
 */
typedef enum{
    /**< 使用通道 1 作为 PWM 输入捕获源 */
    BSP_TIM_PWM_INPUT_CHANNEL1 = 0,
    /**< 使用通道 2 作为 PWM 输入捕获源 */
    BSP_TIM_PWM_INPUT_CHANNEL2
}bsp_tim_pwm_input_channel_t;

/**
 * @enum bsp_tim_ic_polarity_t
 * @brief 输入捕获极性枚举。
 *
 * 用于指定输入信号的有效触发边沿。
 */
typedef enum{
    /**< 上升沿捕获 */
    BSP_TIM_IC_POLARITY_RISING = 0,
    /**< 下降沿捕获 */
    BSP_TIM_IC_POLARITY_FALLING,
    /**< 双边沿捕获 */
    BSP_TIM_IC_POLARITY_BOTHEDGE
}bsp_tim_ic_polarity_t;

/**
 * @enum bsp_tim_ic_prescaler_t
 * @brief 输入捕获分频枚举。
 *
 * 输入捕获分频用于降低捕获事件频率，常用于高频输入信号场景。
 */
typedef enum{
    /**< 每次有效边沿都捕获 */
    BSP_TIM_IC_PRESCALER_DIV1 = 0,
    /**< 每 2 次有效边沿捕获一次 */
    BSP_TIM_IC_PRESCALER_DIV2,
    /**< 每 4 次有效边沿捕获一次 */
    BSP_TIM_IC_PRESCALER_DIV4,
    /**< 每 8 次有效边沿捕获一次 */
    BSP_TIM_IC_PRESCALER_DIV8
}bsp_tim_ic_prescaler_t;

/**
 * @struct bsp_advancedtimer_config_t
 * @brief 高级定时器基础参数配置结构体。
 *
 * 用于配置定时器基础计数单元参数。
 */
typedef struct{
	/**
     * @brief 预分频器值。
     *
     * 定时器计数频率：
     * @code
     * f_cnt = f_tim / (prescaler + 1)
     * @endcode
     */
	uint16_t prescaler;

    /**< 定时器计数模式 */
	bsp_tim_counter_mode_t counter_mode;

    /**< 自动重装载值 ARR */
	uint16_t period;

    /**< 时钟分频配置 */
	bsp_tim_clock_div_t clock_div;

    /**< 重复计数器值 */
	uint8_t repetition_cnt;
}bsp_advancedtimer_config_t;

/**
 * @struct bsp_tim_pwm_config_t
 * @brief PWM 输入捕获配置结构体。
 *
 * 用于描述 PWM 输入模式下的输入源及采样参数。
 */
typedef struct{
    /**< PWM 输入选择通道 */
    bsp_tim_pwm_input_channel_t pwm_channel;

    /**< 输入捕获极性 */
    bsp_tim_ic_polarity_t ic_polarity;

    /**< 输入捕获预分频 */
    bsp_tim_ic_prescaler_t ic_prescaler_div;

    /**
     * @brief 输入数字滤波器配置值。
     *
     * 该值用于滤除毛刺和抖动信号。
     * 取值范围和具体含义参考 STM32 参考手册中 TIMx_CCMRx 的 ICxF 配置。
     */
    uint16_t ic_filter;
}bsp_tim_pwm_config_t;

/**
 * @struct bsp_tim_ic_result_t
 * @brief PWM 输入捕获结果结构体。
 *
 * 中断处理完成后，测量结果会写入该结构体。
 */
typedef struct{
    /**
     * @brief PWM 周期计数值。
     *
     * 单位为定时器计数周期数，不是直接的时间单位。
     */
    volatile uint32_t g_pwm_period;

    /**
     * @brief PWM 高电平脉宽计数值。
     *
     * 单位为定时器计数周期数。
     */
    volatile uint32_t g_pwm_pulse_width;

    /**
     * @brief PWM 占空比，单位为百分比。
     *
     * 计算公式：
     * @code
     * duty = pulse_width / period * 100
     * @endcode
     */
    volatile float g_pwm_duty;

    /**
     * @brief PWM 频率，单位 Hz。
     *
     * 计算公式：
     * @code
     * freq = CLK_FREQ / (PSC + 1) / period
     * @endcode
     */
    volatile uint32_t g_pwm_freq;

    /**
     * @brief PWM 数据更新标志。
     *
     * - 0：无新数据
     * - 1：有新数据可读
     */
    volatile uint8_t g_pwm_update;
}bsp_tim_ic_result_t;

/**
 * @brief 高级定时器 PWM 输入捕获结果数组。
 *
 * 数组下标由 @ref bsp_advancedtimer_t 指定。
 * 每个高级定时器实例对应一个捕获结果结构体。
 */
extern bsp_tim_ic_result_t result[BSP_ADVANCED_TIMER_MAX];

/**
 * @brief 配置高级定时器 PWM 输入捕获功能。
 *
 * 该函数是模块对外提供的统一配置接口，内部将完成：
 * - PWM 输入引脚 GPIO 初始化
 * - 定时器基础计数单元初始化
 * - PWM 输入模式初始化
 * - 中断使能与 NVIC 配置
 * - 启动定时器
 *
 * @param timer_id 高级定时器编号，取值见 @ref bsp_advancedtimer_t
 * @param base_config 指向基础配置结构体的指针
 * @param pwm_config 指向 PWM 输入配置结构体的指针
 *
 * @note
 * 1. 本函数不做空指针检查，调用前应确保传入参数有效。
 * 2. 该函数默认输入 GPIO 使用浮空输入模式。
 * 3. 配置完成后，PWM 测量结果将通过捕获比较中断持续更新。
 */
extern void BSP_AdvancedTimer_Config(bsp_advancedtimer_t timer_id, 
			bsp_advancedtimer_config_t *base_config, 
			bsp_tim_pwm_config_t *pwm_config);

/**
 * @brief 高级定时器捕获比较中断统一处理函数。
 *
 * 该函数在捕获比较中断中完成以下工作：
 * - 读取捕获寄存器，获得 PWM 周期和脉宽
 * - 计算占空比和频率
 * - 更新结果结构体
 * - 置位更新标志
 * - 清除中断标志
 *
 * @param timer_id 高级定时器编号，取值见 @ref bsp_advancedtimer_t
 *
 * @note
 * 一般在具体中断入口函数中调用，例如：
 * @code
 * void TIM1_CC_IRQHandler(void)
 * {
 *     BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER1);
 * }
 * @endcode
 */
extern void BSP_TIM_CC_IRQHandler(bsp_advancedtimer_t timer_id);

#endif