#ifndef BSP_GENERAL_TIMER_H
#define BSP_GENERAL_TIMER_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include <stdlib.h>

/**
 * @file    bsp_general_timer.h
 * @brief   STM32F10x 通用定时器输入捕获测量脉宽 BSP 接口头文件
 * @details
 * 本文件封装了 STM32F10x 通用定时器 TIM2 / TIM3 / TIM4 / TIM5 的输入捕获功能，
 * 用于测量外部脉冲信号的高电平脉宽。
 *
 * 本模块采用“普通输入捕获 + 更新中断溢出计数”的方式实现：
 * - 先捕获上升沿
 * - 再切换为下降沿捕获
 * - 记录下降沿
 * - 结合计数器溢出次数计算完整脉宽
 *
 * 与 PWM Input 模式不同，本模块更适合：
 * - 测量单个脉冲宽度
 * - 测量较长高电平脉冲
 * - 只关心脉宽、不关心周期和占空比的场景
 *
 * @note
 * 1. 本模块输出的 `pulse_width` 是计数值，不是直接的时间值。
 * 2. 若要换算为 us / ms，需要结合定时器计数频率进一步计算。
 * 3. 当前默认使用 @ref CLK_FREQ 表示定时器输入时钟频率，默认值为 72 MHz。
 */

/**
 * @defgroup BSP_General_Timer_IC BSP 通用定时器输入捕获模块
 * @brief    提供 TIM2 / TIM3 / TIM4 / TIM5 输入捕获测量脉宽功能
 * @{
 */

/** @brief 定时器中断抢占优先级 */
#define PREEMPT_PRIO    2

/** @brief 定时器中断子优先级 */
#define SUB_PRIO        2

/**
 * @brief 定时器输入时钟频率
 * @details
 * 用于将输入捕获计数值换算为实际时间。
 *
 * @note
 * 若系统时钟树、APB1 分频或定时器输入时钟发生变化，
 * 需要同步修改该值，否则时间换算结果会不准确。
 */
#define CLK_FREQ        72000000U

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
    BSP_TIM_COUNTER_MODE_UP = 0,           /**< 向上计数模式 */
    BSP_TIM_COUNTER_MODE_DOWN,             /**< 向下计数模式 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED1,  /**< 中央对齐模式 1 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED2,  /**< 中央对齐模式 2 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED3   /**< 中央对齐模式 3 */
}bsp_generaltimer_counter_mode_t;

/**
 * @brief 定时器时钟分频配置
 * @note
 * 这里的 clock_div 对应 CKD，不等于 PSC 预分频。
 */
typedef enum{
    BSP_TIM_CLOCK_DIV_1 = 0, /**< 时钟分频 1 */
    BSP_TIM_CLOCK_DIV_2,     /**< 时钟分频 2 */
    BSP_TIM_CLOCK_DIV_4      /**< 时钟分频 4 */
}bsp_generaltimer_clock_div_t;

/**
 * @brief 输入捕获通道
 */
typedef enum{
    BSP_TIM_IC_CHANNEL1 = 0, /**< 输入捕获通道 1 */
    BSP_TIM_IC_CHANNEL2,     /**< 输入捕获通道 2 */
    BSP_TIM_IC_CHANNEL3,     /**< 输入捕获通道 3 */
    BSP_TIM_IC_CHANNEL4,     /**< 输入捕获通道 4 */
    BSP_TIM_IC_CHANNEL_MAX   /**< 输入捕获通道数量上限/边界值 */
}bsp_tim_ic_channel_t;

/**
 * @brief 输入捕获极性配置
 */
typedef enum{
    BSP_TIM_IC_POLARITY_RISING = 0, /**< 上升沿捕获 */
    BSP_TIM_IC_POLARITY_FALLING,    /**< 下降沿捕获 */
    BSP_TIM_IC_POLARITY_BOTHEDGE    /**< 双边沿捕获 */
}bsp_tim_ic_polarity_t;

/**
 * @brief 输入捕获映射选择
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
    uint16_t prescaler;                           /**< PSC 预分频值，实际分频系数 = prescaler + 1 */
    bsp_generaltimer_counter_mode_t counter_mode;/**< 计数模式 */
    uint16_t period;                             /**< ARR 自动重装载值 */
    bsp_generaltimer_clock_div_t clock_div;      /**< 时钟分频（CKD） */
    uint8_t repetition_cnt;                      /**< 重复计数器，通用定时器通常无效，此处为兼容保留 */
}bsp_generaltimer_config_t;

/**
 * @brief 输入捕获配置结构体
 */
typedef struct{
    bsp_tim_ic_channel_t ic_channel;             /**< 输入捕获通道 */
    bsp_tim_ic_polarity_t ic_polarity;           /**< 输入捕获极性 */
    bsp_tim_ic_selection_t ic_selection;         /**< 输入映射方式 */
    bsp_tim_ic_prescaler_t ic_prescaler_div;     /**< 输入捕获预分频 */
    uint16_t ic_filter;                          /**< 输入滤波器配置 */
}bsp_tim_ic_config_t;

/**
 * @brief 输入捕获结果结构体
 * @details
 * 本结构体用于保存某个定时器对应通道的测量结果：
 * - 上升沿捕获值
 * - 下降沿捕获值
 * - 计算后的脉宽
 * - 当前捕获状态
 * - 定时器溢出次数
 *
 * @note
 * 当前结构体中：
 * - `rising_cnt` / `falling_cnt` / `pulse_width` 针对当前定时器当前配置通道
 * - `over_flow[]` 以定时器编号为下标记录溢出次数
 *
 * 这种设计可以工作，但 `over_flow[]` 放在结果结构体里且按 timer_id 索引，语义上稍绕。
 */
typedef struct{
    volatile uint16_t rising_cnt;             /**< 上升沿捕获计数值 */
    volatile uint16_t falling_cnt;            /**< 下降沿捕获计数值 */
    volatile uint16_t pulse_width;            /**< 计算得到的高电平脉宽计数值 */
    volatile uint8_t capture_state;           /**< 0: 等待上升沿，1: 等待下降沿 */
    volatile uint16_t over_flow[BSP_GENERAL_TIMER_MAX]; /**< 测量期间定时器溢出次数，按 timer_id 索引 */
}bsp_tim_ic_result_t;

/**
 * @brief 通用定时器辅助计数数组
 * @details
 * 当前代码中定义了该数组，但未在主要逻辑中使用，可作为扩展用途保留。
 */
extern volatile uint16_t generaltimer_cnt[BSP_GENERAL_TIMER_MAX];

/**
 * @brief 通用定时器输入捕获结果数组
 * @details
 * 数组下标由 @ref bsp_generaltimer_t 指定。
 */
extern volatile bsp_tim_ic_result_t result[BSP_GENERAL_TIMER_MAX];

/**
 * @brief 配置通用定时器输入捕获功能
 * @details
 * 内部完成：
 * - GPIO 输入初始化
 * - 定时器基础参数初始化
 * - 输入捕获初始化
 * - 更新中断和捕获比较中断配置
 * - 定时器启动
 *
 * @param[in] timer_id
 * 通用定时器编号。
 * @param[in] base_config
 * 指向基础配置结构体的指针。
 * @param[in] ic_config
 * 指向输入捕获配置结构体的指针。
 *
 * @return 无返回值。
 */
extern void BSP_GeneralTIM_Config(bsp_generaltimer_t timer_id,
        bsp_generaltimer_config_t *base_config,
        bsp_tim_ic_config_t *ic_config);

/**
 * @brief 通用定时器中断公共处理函数
 * @details
 * 该函数统一处理：
 * - 更新中断：统计测量期间发生的溢出次数
 * - 捕获比较中断：记录上升沿/下降沿并计算脉宽
 *
 * @param[in] timer_id
 * 发生中断的通用定时器编号。
 *
 * @return 无返回值。
 */
extern void BSP_TIM_IRQHandler(bsp_generaltimer_t timer_id);

/** @} */

#endif