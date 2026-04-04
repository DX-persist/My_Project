#ifndef BSP_ADVANCED_TIMER_H
#define BSP_ADVANCED_TIMER_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include <stdlib.h>

/**
 * @file    bsp_advanced_timer.h
 * @brief   STM32F10x 高级定时器 PWM 输入捕获 BSP 接口头文件
 * @details
 * 本文件封装了 STM32F10x 系列高级定时器（当前支持 TIM1、TIM8）的 PWM 输入测量功能，
 * 主要用于测量外部 PWM 信号的：
 * - 周期计数值
 * - 高电平脉宽计数值
 * - 占空比
 * - 频率
 *
 * 该模块基于标准外设库（StdPeriph Library）实现，利用定时器的 PWM Input 模式完成输入信号测量。
 *
 * 工作原理简述：
 * - 使用 `TIM_PWMIConfig()` 将两个输入捕获通道配对工作
 * - 一个通道用于锁存周期
 * - 另一个通道用于锁存高电平时间
 * - 结合从模式复位（Reset Slave Mode），每个 PWM 周期重新计数
 * - 在捕获中断中计算占空比和频率
 *
 * 适用场景：
 * - 测量传感器输出 PWM
 * - 测量编码器类脉冲占空比信号
 * - 外部设备频率/占空比监测
 *
 * @note
 * 1. 当前接口默认定时器输入时钟使用宏 `CLK_FREQ` 表示，值为 72 MHz。
 *    如果芯片时钟树或 APB 分频方式发生变化，应同步修正该宏，避免频率计算错误。
 * 2. PWM 输入模式依赖两个捕获寄存器联合工作，因此“选择 CH1 输入”与“只使用 CH1”不是同一个概念。
 *    实际上底层会自动使用另一通道辅助测量。
 * 3. 高级定时器 TIM1/TIM8 具备更多高级特性，但本模块仅使用其通用定时器输入捕获相关能力。
 */

/**
 * @defgroup BSP_Advanced_Timer BSP 高级定时器模块
 * @brief    提供高级定时器 PWM 输入捕获测量功能
 * @{
 */

/** @brief 更新中断抢占优先级 */
#define UP_PREEMPT_PRIO  1

/** @brief 更新中断子优先级 */
#define UP_SUB_PRIO      0

/** @brief 捕获比较中断抢占优先级 */
#define CC_PREEPT_PRIO   1

/** @brief 捕获比较中断子优先级 */
#define CC_SUB_PRIO      1

/**
 * @brief 定时器计数时钟频率
 * @details
 * 用于频率计算公式：
 * @code
 * freq = CLK_FREQ / (PSC + 1) / period_cnt
 * @endcode
 *
 * 这里默认高级定时器输入时钟为 72 MHz。
 *
 * @note
 * 如果 APB2 定时器时钟不是 72 MHz，则该值必须修改，否则计算出的 PWM 频率会不准确。
 */
#define CLK_FREQ         72000000U

/**
 * @brief 高级定时器实例编号
 */
typedef enum{
    BSP_ADVANCED_TIMER1 = 0, /**< 高级定时器 TIM1 */
    BSP_ADVANCED_TIMER8,     /**< 高级定时器 TIM8 */
    BSP_ADVANCED_TIMER_MAX   /**< 高级定时器数量上限/边界值 */
}bsp_advancedtimer_t;

/**
 * @brief 定时器计数模式
 * @details
 * 对应 STM32 定时器基本计数模式配置。
 */
typedef enum{
    BSP_TIM_COUNTER_MODE_UP = 0,        /**< 向上计数模式 */
    BSP_TIM_COUNTER_MODE_DOWN,          /**< 向下计数模式 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED1, /**< 中央对齐模式1 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED2, /**< 中央对齐模式2 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED3  /**< 中央对齐模式3 */
}bsp_tim_base_counter_mode_t;

/**
 * @brief 定时器时钟分频配置
 * @details
 * 该配置映射到 TIMx_CR1 寄存器中的 CKD 位，通常用于数字滤波采样时钟设置。
 * 对普通计数频率并不是直接的预分频系数，容易与 PSC 混淆。
 *
 * @note
 * 真正决定计数频率的是 Prescaler（PSC），而不是这里的 clock_div。
 */
typedef enum{
    BSP_TIM_CLOCK_DIV_1 = 0, /**< 定时器时钟分频 1 */
    BSP_TIM_CLOCK_DIV_2,     /**< 定时器时钟分频 2 */
    BSP_TIM_CLOCK_DIV_4      /**< 定时器时钟分频 4 */
}bsp_tim_base_clock_div_t;

/**
 * @brief PWM 输入测量所选输入通道
 * @details
 * 该枚举表示将哪一路输入引脚作为 PWM 输入源。
 * 在 PWM 输入模式下，底层会自动使用互补的另一个捕获通道来完成周期/脉宽测量。
 */
typedef enum{
    BSP_TIM_PWM_INPUT_CHANNEL1 = 0, /**< 选择通道 1 作为 PWM 输入源 */
    BSP_TIM_PWM_INPUT_CHANNEL2      /**< 选择通道 2 作为 PWM 输入源 */
}bsp_tim_pwm_input_channel_t;

/**
 * @brief 输入捕获极性配置
 */
typedef enum{
    BSP_TIM_IC_POLARITY_RISING = 0, /**< 上升沿捕获 */
    BSP_TIM_IC_POLARITY_FALLING,    /**< 下降沿捕获 */
    BSP_TIM_IC_POLARITY_BOTHEDGE    /**< 双边沿捕获 */
}bsp_tim_ic_polarity_t;

/**
 * @brief 输入捕获预分频配置
 * @details
 * 用于控制每隔多少个有效边沿进行一次捕获。
 */
typedef enum{
    BSP_TIM_IC_PRESCALER_DIV1 = 0, /**< 每个有效边沿都捕获 */
    BSP_TIM_IC_PRESCALER_DIV2,     /**< 每 2 个有效边沿捕获一次 */
    BSP_TIM_IC_PRESCALER_DIV4,     /**< 每 4 个有效边沿捕获一次 */
    BSP_TIM_IC_PRESCALER_DIV8      /**< 每 8 个有效边沿捕获一次 */
}bsp_tim_ic_prescaler_t;

/**
 * @brief 高级定时器基础配置结构体
 * @details
 * 用于描述定时器的基础计数行为，例如：
 * - 预分频值
 * - 自动重装值
 * - 计数模式
 * - 时钟分频
 * - 重复计数器
 */
typedef struct{
    uint16_t prescaler;                     /**< 预分频系数，实际分频值 = prescaler + 1 */
    bsp_tim_base_counter_mode_t counter_mode; /**< 定时器计数模式 */
    uint16_t period;                       /**< 自动重装载值 ARR，决定计数周期上限 */
    bsp_tim_base_clock_div_t clock_div;    /**< 时钟分频配置（CKD） */
    uint8_t repetition_cnt;                /**< 重复计数器，仅高级定时器有效 */
}bsp_advancedtimer_config_t;

/**
 * @brief PWM 输入捕获配置结构体
 * @details
 * 用于描述 PWM 输入测量模式的参数：
 * - 输入通道
 * - 捕获极性
 * - 捕获预分频
 * - 数字滤波器
 */
typedef struct{
    bsp_tim_pwm_input_channel_t pwm_channel;   /**< PWM 输入源通道 */
    bsp_tim_ic_polarity_t ic_polarity;         /**< 输入捕获极性 */
    bsp_tim_ic_prescaler_t ic_prescaler_div;   /**< 输入捕获预分频 */
    uint16_t ic_filter;                        /**< 输入滤波器值，范围通常为 0~15 */
}bsp_tim_pwm_config_t;

/**
 * @brief PWM 输入测量结果结构体
 * @details
 * 该结构体在中断服务函数中更新，在主循环中读取。
 * 所有成员均声明为 `volatile`，以避免编译器优化导致主循环无法感知中断中更新的数据。
 *
 * @note
 * `g_pwm_period` 和 `g_pwm_pulse_width` 为定时器计数值，不是时间单位。
 * 如需换算实际时间，应结合计数时钟频率进一步计算。
 */
typedef struct{
    volatile uint32_t g_pwm_period;      /**< PWM 周期计数值 */
    volatile uint32_t g_pwm_pulse_width; /**< PWM 高电平脉宽计数值 */
    volatile float g_pwm_duty;           /**< PWM 占空比，单位：% */
    volatile uint32_t g_pwm_freq;        /**< PWM 频率，单位：Hz */
    volatile uint8_t g_pwm_update;       /**< 数据更新标志，1 表示有新数据 */
}bsp_tim_ic_result_t;

/**
 * @brief 保存所有高级定时器的 PWM 测量结果
 * @details
 * 数组下标由 @ref bsp_advancedtimer_t 指定，例如：
 * - `result[BSP_ADVANCED_TIMER1]`
 * - `result[BSP_ADVANCED_TIMER8]`
 */
extern bsp_tim_ic_result_t result[BSP_ADVANCED_TIMER_MAX];

/**
 * @brief 配置高级定时器 PWM 输入测量功能
 * @details
 * 该函数会完成以下工作：
 * - 配置对应通道 GPIO 为浮空输入
 * - 初始化高级定时器基本参数
 * - 配置 PWM 输入模式
 * - 配置从模式复位
 * - 配置捕获比较中断
 * - 启动定时器
 *
 * @param[in] timer_id
 * 要配置的高级定时器编号，取值见 @ref bsp_advancedtimer_t 。
 *
 * @param[in] base_config
 * 指向基础定时器配置结构体的指针，不能为空。
 *
 * @param[in] pwm_config
 * 指向 PWM 输入配置结构体的指针，不能为空。
 *
 * @return
 * 无返回值。
 *
 * @note
 * 本函数不对空指针进行防护检查，调用者应保证 `base_config` 和 `pwm_config` 合法有效。
 */
extern void BSP_AdvancedTimer_Config(bsp_advancedtimer_t timer_id,
            bsp_advancedtimer_config_t *base_config,
            bsp_tim_pwm_config_t *pwm_config);

/**
 * @brief 高级定时器捕获比较中断公共处理函数
 * @details
 * 该函数用于在实际中断入口函数（如 `TIM1_CC_IRQHandler()`）中被调用，
 * 以统一完成 PWM 周期、高电平时间、占空比和频率的计算。
 *
 * @param[in] timer_id
 * 对应的高级定时器编号。
 *
 * @return
 * 无返回值。
 *
 * @note
 * 该函数本身不是 MCU 启动文件中的真实中断向量函数，
 * 需要由用户在对应的 IRQHandler 中进行转调，例如：
 * @code
 * void TIM1_CC_IRQHandler(void)
 * {
 *     BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER1);
 * }
 * @endcode
 */
extern void BSP_TIM_CC_IRQHandler(bsp_advancedtimer_t timer_id);

/** @} */

#endif