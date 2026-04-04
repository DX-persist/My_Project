#ifndef BSP_ADVANCED_TIMER_H
#define BSP_ADVANCED_TIMER_H

#include "stm32f10x.h"

/**
 * @file    bsp_advanced_timer.h
 * @brief   STM32F10x 高级定时器基础时基/更新中断 BSP 接口头文件
 * @details
 * 本文件封装了 STM32F10x 高级定时器 TIM1 / TIM8 的基础时基配置与更新中断功能。
 *
 * 虽然 TIM1 / TIM8 被称为“高级定时器”，具备：
 * - 互补输出
 * - 死区时间
 * - 刹车输入
 * - 重复计数器
 *
 * 但在本模块中，仅使用其“基本定时器/时基定时”能力，即：
 * - 配置 PSC / ARR / 计数模式
 * - 使能更新中断
 * - 在中断中对更新事件进行计数
 *
 * 适用场景：
 * - 周期性任务调度
 * - 简单软件定时
 * - GPIO 周期翻转
 * - 验证高级定时器时基配置与更新中断
 *
 * @note
 * 1. 本模块没有使用高级定时器的 PWM、互补输出、BDTR 等高级特性。
 * 2. 当前主要把 TIM1 / TIM8 当作“具有更新中断的时基定时器”使用。
 */

/**
 * @defgroup BSP_Advanced_Timer_Base BSP 高级定时器基础时基模块
 * @brief    提供 TIM1 / TIM8 的基础时基配置与更新中断功能
 * @{
 */

/** @brief 定时器中断抢占优先级 */
#define PREEMPT_PRIO   2

/** @brief 定时器中断子优先级 */
#define SUB_PRIO       2

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
}bsp_advancedtimer_counter_mode_t;

/**
 * @brief 定时器时钟分频配置
 * @details
 * 该配置对应 TIMx_CR1 中的 CKD 位。
 *
 * @note
 * 这里的 `clock_div` 不是 PSC 预分频器。
 * 真正决定计数器频率的是 `prescaler`。
 */
typedef enum{
    BSP_TIM_CLOCK_DIV_1 = 0, /**< 时钟分频 1 */
    BSP_TIM_CLOCK_DIV_2,     /**< 时钟分频 2 */
    BSP_TIM_CLOCK_DIV_4      /**< 时钟分频 4 */
}bsp_advancedtimer_clock_div_t;

/**
 * @brief 高级定时器基础配置结构体
 */
typedef struct{
    uint16_t prescaler;                         /**< PSC 预分频值，实际分频系数 = prescaler + 1 */
    bsp_advancedtimer_counter_mode_t counter_mode; /**< 计数模式 */
    uint16_t period;                           /**< ARR 自动重装载值 */
    bsp_advancedtimer_clock_div_t clock_div;   /**< 时钟分频（CKD） */
    uint8_t repetition_cnt;                    /**< 重复计数器，仅高级定时器有效 */
}bsp_advancedtimer_config_t;

/**
 * @brief 高级定时器更新事件计数数组
 * @details
 * 在更新中断中，对应定时器的计数值会自增一次。
 * 可用于在主循环中统计时间片或周期事件次数。
 */
extern volatile uint16_t advancedtimer_cnt[BSP_ADVANCED_TIMER_MAX];

/**
 * @brief 初始化高级定时器基础时基与更新中断
 * @details
 * 本函数完成：
 * - 开启 TIM1 / TIM8 时钟
 * - 配置时基参数（PSC / ARR / CounterMode / CKD / RepetitionCounter）
 * - 清除初始化阶段自动产生的更新标志
 * - 使能更新中断
 * - 配置 NVIC
 * - 启动定时器
 *
 * @param[in] timer_id
 * 高级定时器编号。
 * @param[in] config
 * 指向基础配置结构体的指针。
 *
 * @return 无返回值。
 *
 * @note
 * 高级定时器在这里只被当作基础定时器使用，没有启用 PWM、互补输出等功能。
 */
extern void BSP_BaseTIM_Init(bsp_advancedtimer_t timer_id, bsp_advancedtimer_config_t *config);

/**
 * @brief 高级定时器更新中断公共处理函数
 * @details
 * 当对应高级定时器产生更新中断时：
 * - `advancedtimer_cnt[timer_id]` 自增
 * - 清除更新中断挂起位
 *
 * @param[in] timer_id
 * 高级定时器编号。
 *
 * @return 无返回值。
 */
extern void BSP_TIM_IRQHandler(bsp_advancedtimer_t timer_id);

/** @} */

#endif