#ifndef BSP_GENERAL_TIMER_H
#define BSP_GENERAL_TIMER_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include <stdlib.h>

/**
 * @file    bsp_general_timer.h
 * @brief   STM32F10x 通用定时器 PWM 输入测量 BSP 接口头文件
 * @details
 * 本文件封装了 STM32F10x 系列通用定时器（TIM2、TIM3、TIM4、TIM5）的 PWM 输入测量接口，
 * 用于测量外部 PWM 信号的周期、高电平脉宽、占空比和频率。
 *
 * 模块基于 STM32 标准外设库的 PWM Input 模式实现，适合如下场景：
 * - 外部设备 PWM 输出频率检测
 * - 占空比测量
 * - 传感器或执行器的 PWM 状态采样
 *
 * 工作流程概述：
 * - 配置定时器基础参数（PSC、ARR、计数模式等）
 * - 选择 CH1 或 CH2 作为 PWM 输入源
 * - 通过 TIM_PWMIConfig() 进入 PWM 输入模式
 * - 配置从模式复位，使计数器在周期边沿自动清零
 * - 在捕获比较中断中读取 CCR1/CCR2 并计算结果
 *
 * @note
 * 1. 本模块中频率计算默认依赖宏 @ref CLK_FREQ ，其值为 72 MHz。
 *    若芯片时钟树、APB1 分频或定时器实际输入时钟发生变化，需要同步修改该值。
 * 2. PWM Input 模式内部会联动两个捕获寄存器工作，因此选择 CH1/CH2 作为输入源，
 *    并不意味着底层只使用单一通道寄存器。
 * 3. 结果结构体成员使用 volatile 修饰，原因是这些变量会在中断中更新、主循环中读取。
 */

/**
 * @defgroup BSP_General_Timer BSP 通用定时器模块
 * @brief    提供通用定时器 PWM 输入测量功能
 * @{
 */

/** @brief 定时器中断抢占优先级 */
#define PREEMPT_PRIO   2

/** @brief 定时器中断子优先级 */
#define SUB_PRIO       2

/**
 * @brief 定时器输入时钟频率
 * @details
 * 本宏用于计算 PWM 输入信号频率：
 * @code
 * g_pwm_freq = CLK_FREQ / (PSC + 1) / period_cnt
 * @endcode
 *
 * @note
 * 对于 STM32F10x，若 APB1 预分频不为 1，则定时器时钟可能是 PCLK1 的 2 倍。
 * 因此在修改系统时钟配置后，应重新核对本宏是否仍然正确。
 */
#define CLK_FREQ       72000000U

/**
 * @brief 通用定时器编号
 */
typedef enum{
    BSP_GENERAL_TIMER2 = 0, /**< 通用定时器 TIM2 */
    BSP_GENERAL_TIMER3,     /**< 通用定时器 TIM3 */
    BSP_GENERAL_TIMER4,     /**< 通用定时器 TIM4 */
    BSP_GENERAL_TIMER5,     /**< 通用定时器 TIM5 */
    BSP_GENERAL_TIMER_MAX   /**< 通用定时器数量上限/边界值 */
}bsp_generaltimer_t;

/**
 * @brief 定时器计数模式
 */
typedef enum{
    BSP_TIM_COUNTER_MODE_UP = 0,        /**< 向上计数模式 */
    BSP_TIM_COUNTER_MODE_DOWN,          /**< 向下计数模式 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED1, /**< 中央对齐模式 1 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED2, /**< 中央对齐模式 2 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED3  /**< 中央对齐模式 3 */
}bsp_generaltimer_counter_mode_t;

/**
 * @brief 定时器时钟分频配置
 * @details
 * 该值映射到 TIMx_CR1 寄存器中的 CKD 位。
 *
 * @note
 * 这里的 clock_div 不是 PSC 预分频器，二者含义不同：
 * - PSC 决定计数频率
 * - CKD 主要影响采样与数字滤波相关时钟
 */
typedef enum{
    BSP_TIM_CLOCK_DIV_1 = 0, /**< 定时器时钟分频 1 */
    BSP_TIM_CLOCK_DIV_2,     /**< 定时器时钟分频 2 */
    BSP_TIM_CLOCK_DIV_4      /**< 定时器时钟分频 4 */
}bsp_generaltimer_clock_div_t;

/**
 * @brief PWM 输入源通道选择
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
 * @brief 输入捕获选择方式
 * @details
 * 当前模块内部固定使用 DirectTI 模式，该枚举主要为接口扩展保留。
 */
typedef enum{
    BSP_TIM_IC_SELECTION_DIRECTTI = 0, /**< 直接映射到 TIx */
    BSP_TIM_IC_SELECTION_INDIRECTTI,   /**< 间接映射到另一路 TI */
    BSP_TIM_IC_SELECTION_TRC           /**< 映射到触发控制器 */
}bsp_tim_ic_selection_t;

/**
 * @brief 输入捕获预分频配置
 */
typedef enum{
    BSP_TIM_IC_PRESCALER_DIV1 = 0, /**< 每个有效边沿都捕获 */
    BSP_TIM_IC_PRESCALER_DIV2,     /**< 每 2 个有效边沿捕获一次 */
    BSP_TIM_IC_PRESCALER_DIV4,     /**< 每 4 个有效边沿捕获一次 */
    BSP_TIM_IC_PRESCALER_DIV8      /**< 每 8 个有效边沿捕获一次 */
}bsp_tim_ic_prescaler_t;

/**
 * @brief 通用定时器基础配置结构体
 */
typedef struct{
    uint16_t prescaler;                        /**< PSC 预分频值，实际分频系数 = prescaler + 1 */
    bsp_generaltimer_counter_mode_t counter_mode; /**< 定时器计数模式 */
    uint16_t period;                          /**< 自动重装载寄存器 ARR 的值 */
    bsp_generaltimer_clock_div_t clock_div;   /**< 时钟分频配置（CKD） */
    uint8_t repetition_cnt;                   /**< 重复计数器，通用定时器通常不使用，保留接口兼容 */
}bsp_generaltimer_config_t;

/**
 * @brief PWM 输入测量配置结构体
 */
typedef struct{
    bsp_tim_pwm_input_channel_t input_channel; /**< PWM 输入源通道 */
    bsp_tim_ic_polarity_t ic_polarity;         /**< 输入捕获极性 */
    bsp_tim_ic_prescaler_t ic_prescaler;       /**< 输入捕获预分频 */
    uint16_t ic_filter;                        /**< 输入滤波器值 */
}bsp_tim_pwm_input_config_t;

/**
 * @brief PWM 输入测量结果结构体
 * @details
 * 该结构体在中断中更新，在主循环中读取。
 */
typedef struct{
    volatile uint32_t g_pwm_period;      /**< PWM 周期计数值 */
    volatile uint32_t g_pwm_pulse_width; /**< PWM 高电平脉宽计数值 */
    volatile float g_pwm_duty;           /**< PWM 占空比，单位：% */
    volatile uint32_t g_pwm_freq;        /**< PWM 频率，单位：Hz */
    volatile uint8_t g_pwm_update;       /**< 数据更新标志，1 表示有新数据 */
}bsp_tim_ic_result_t;

/**
 * @brief 通用定时器 PWM 测量结果数组
 * @details
 * 使用 @ref bsp_generaltimer_t 作为下标访问。
 */
extern bsp_tim_ic_result_t result[BSP_GENERAL_TIMER_MAX];

/**
 * @brief 配置通用定时器 PWM 输入测量功能
 * @details
 * 内部完成：
 * - GPIO 输入初始化
 * - 定时器基础参数初始化
 * - PWM Input 模式配置
 * - 中断配置
 * - 定时器启动
 *
 * @param[in] timer_id
 * 要配置的通用定时器编号。
 * @param[in] base_config
 * 指向基础参数配置结构体的指针。
 * @param[in] pwm_config
 * 指向 PWM 输入配置结构体的指针。
 *
 * @return 无返回值。
 *
 * @note
 * 本函数不对空指针进行防护检查，调用者应保证传入参数有效。
 */
extern void BSP_GeneralTIM_Config(bsp_generaltimer_t timer_id,
        bsp_generaltimer_config_t *base_config,
        bsp_tim_pwm_input_config_t *pwm_config);

/**
 * @brief 通用定时器捕获比较中断公共处理函数
 * @details
 * 在实际 IRQHandler 中调用该函数，用于统一处理 TIM2~TIM5 的 PWM 输入捕获结果。
 *
 * @param[in] timer_id
 * 触发中断的通用定时器编号。
 *
 * @return 无返回值。
 *
 * @note
 * 该函数本身不是启动文件中的真实中断入口，需要在对应的 IRQHandler 中转调。
 */
extern void BSP_TIM_IRQHandler(bsp_generaltimer_t timer_id);

/** @} */

#endif
