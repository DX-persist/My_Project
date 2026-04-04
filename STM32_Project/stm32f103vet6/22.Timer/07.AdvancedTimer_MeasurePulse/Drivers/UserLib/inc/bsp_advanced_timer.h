#ifndef BSP_ADVANCED_TIMER_H
#define BSP_ADVANCED_TIMER_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include <stdlib.h>

/**
 * @file    bsp_advanced_timer.h
 * @brief   STM32F10x 高级定时器输入捕获测量脉宽 BSP 接口头文件
 * @details
 * 本文件封装了 STM32F10x 高级定时器 TIM1 / TIM8 的输入捕获功能，
 * 主要用于测量单路脉冲信号的高电平脉宽。
 *
 * 与 PWM Input 模式不同，本模块采用“普通输入捕获 + 更新中断溢出计数”的方式实现：
 * - 首先捕获上升沿时间
 * - 再切换为捕获下降沿
 * - 结合计数器溢出次数，计算高电平持续时间
 *
 * 该方案特别适合：
 * - 测量单个脉冲宽度
 * - 测量较长高电平脉冲
 * - 不一定要求完整 PWM 周期信息的场景
 *
 * @note
 * 1. 本模块计算结果的核心是 `pulse_width[]`，单位是“计数值”，不是直接时间单位。
 * 2. 若需换算成 us / ms，需要结合定时器计数频率进一步计算。
 * 3. 当前代码默认使用宏 @ref CLK_FREQ 作为定时器输入时钟频率，默认值为 72 MHz。
 */

/**
 * @defgroup BSP_Advanced_Timer_IC BSP 高级定时器输入捕获模块
 * @brief    提供 TIM1 / TIM8 输入捕获脉宽测量功能
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
 * @brief 定时器输入时钟频率
 * @details
 * 用于将输入捕获计数值换算为实际时间。
 *
 * @note
 * 若系统时钟树发生变化，或者 APB2 定时器时钟不是 72 MHz，
 * 需要同步修改该值，否则脉宽换算结果会不准确。
 */
#define CLK_FREQ         72000000U

/**
 * @brief 高级定时器编号
 */
typedef enum{
    BSP_ADVANCED_TIMER1 = 0, /**< 高级定时器 TIM1 */
    BSP_ADVANCED_TIMER8,     /**< 高级定时器 TIM8 */
    BSP_ADVANCED_TIMER_MAX   /**< 高级定时器数量上限/边界值 */
}bsp_advancedtimer_t;

/**
 * @brief 定时器计数模式
 */
typedef enum{
    BSP_TIM_COUNTER_MODE_UP = 0,           /**< 向上计数模式 */
    BSP_TIM_COUNTER_MODE_DOWN,             /**< 向下计数模式 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED1,  /**< 中央对齐模式 1 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED2,  /**< 中央对齐模式 2 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED3   /**< 中央对齐模式 3 */
}bsp_tim_base_counter_mode_t;

/**
 * @brief 定时器时钟分频配置
 * @note
 * 这里的 clock_div 对应 CKD，不等于 PSC 预分频。
 */
typedef enum{
    BSP_TIM_CLOCK_DIV_1 = 0, /**< 时钟分频 1 */
    BSP_TIM_CLOCK_DIV_2,     /**< 时钟分频 2 */
    BSP_TIM_CLOCK_DIV_4      /**< 时钟分频 4 */
}bsp_tim_base_clock_div_t;

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
 * @brief 高级定时器基础配置结构体
 */
typedef struct{
    uint16_t prescaler;                     /**< PSC 预分频值，实际分频系数 = prescaler + 1 */
    bsp_tim_base_counter_mode_t counter_mode; /**< 计数模式 */
    uint16_t period;                       /**< ARR 自动重装载值 */
    bsp_tim_base_clock_div_t clock_div;    /**< 时钟分频（CKD） */
    uint8_t repetition_cnt;                /**< 重复计数器，仅高级定时器有效 */
}bsp_advancedtimer_config_t;

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
 * 每个通道分别保存：
 * - 上升沿捕获值
 * - 下降沿捕获值
 * - 计算后的高电平脉宽
 * - 当前捕获状态
 * - 期间发生的定时器溢出次数
 *
 * @note
 * `pulse_width[]` 的单位是“计数值”，不是 us。
 * 需要结合计数器 tick 周期进一步换算。
 */
typedef struct{
    volatile uint16_t rising_cnt[BSP_TIM_IC_CHANNEL_MAX];   /**< 上升沿捕获计数值 */
    volatile uint16_t falling_cnt[BSP_TIM_IC_CHANNEL_MAX];  /**< 下降沿捕获计数值 */
    volatile uint16_t pulse_width[BSP_TIM_IC_CHANNEL_MAX];  /**< 计算得到的高电平脉宽计数值 */
    volatile uint8_t capture_state[BSP_TIM_IC_CHANNEL_MAX]; /**< 0: 等待上升沿，1: 等待下降沿 */
    volatile uint16_t over_flow[BSP_TIM_IC_CHANNEL_MAX];    /**< 上升沿与下降沿之间的计数器溢出次数 */
}bsp_tim_ic_result_t;

/**
 * @brief 高级定时器计数辅助数组
 * @details
 * 当前代码中定义了该数组，但未在本模块主要逻辑中使用。
 * 可作为扩展计数用途保留。
 */
extern volatile uint16_t advancedtimer_cnt[BSP_ADVANCED_TIMER_MAX];

/**
 * @brief 高级定时器输入捕获结果数组
 * @details
 * 数组下标由 @ref bsp_advancedtimer_t 指定。
 */
extern bsp_tim_ic_result_t result[BSP_ADVANCED_TIMER_MAX];

/**
 * @brief 配置高级定时器输入捕获功能
 * @details
 * 内部完成：
 * - GPIO 输入初始化
 * - 高级定时器基础配置
 * - 输入捕获配置
 * - 更新中断与捕获比较中断配置
 * - 定时器启动
 *
 * @param[in] timer_id
 * 高级定时器编号。
 * @param[in] base_config
 * 指向基础配置结构体的指针。
 * @param[in] ic_config
 * 指向输入捕获配置结构体的指针。
 *
 * @return 无返回值。
 */
extern void BSP_AdvancedTimer_Config(bsp_advancedtimer_t timer_id,
            bsp_advancedtimer_config_t *base_config,
            bsp_tim_ic_config_t *ic_config);

/**
 * @brief 高级定时器更新中断公共处理函数
 * @details
 * 用于统计上升沿与下降沿之间发生的定时器溢出次数，
 * 从而支持测量超过一个计数周期的长脉宽信号。
 *
 * @param[in] timer_id
 * 高级定时器编号。
 *
 * @return 无返回值。
 */
extern void BSP_TIM_UP_IRQHandler(bsp_advancedtimer_t timer_id);

/**
 * @brief 高级定时器捕获比较中断公共处理函数
 * @details
 * 用于完成：
 * - 首次上升沿捕获
 * - 切换捕获极性
 * - 下降沿捕获
 * - 结合溢出次数计算高电平脉宽
 *
 * @param[in] timer_id
 * 高级定时器编号。
 *
 * @return 无返回值。
 */
extern void BSP_TIM_CC_IRQHandler(bsp_advancedtimer_t timer_id);

/** @} */

#endif