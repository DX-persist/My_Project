#ifndef BSP_GENERAL_TIMER_H
#define BSP_GENERAL_TIMER_H

#include "stm32f10x.h"

/**
 * @file    bsp_general_timer.h
 * @brief   STM32F10x 通用定时器基础时基/更新中断 BSP 接口头文件
 * @details
 * 本文件封装了 STM32F10x 通用定时器 TIM2 / TIM3 / TIM4 / TIM5 的
 * 基础时基配置与更新中断功能。
 *
 * 虽然 TIM2 / TIM3 / TIM4 / TIM5 通常也常用于：
 * - PWM 输出
 * - 输入捕获
 * - 编码器接口
 *
 * 但在本模块中，仅使用其“基础定时器/时基定时”能力，即：
 * - 配置 PSC / ARR / 计数模式
 * - 使能更新中断
 * - 在中断中对更新事件进行计数
 *
 * 适用场景：
 * - 周期性任务调度
 * - 简单软件定时
 * - GPIO 周期翻转
 * - 验证通用定时器时基配置与更新中断
 *
 * @note
 * 1. 本模块没有使用 PWM、输入捕获、编码器模式等通用定时器高级功能。
 * 2. 当前主要把 TIM2 / TIM3 / TIM4 / TIM5 当作“具有更新中断的时基定时器”使用。
 */

/**
 * @defgroup BSP_General_Timer_Base BSP 通用定时器基础时基模块
 * @brief    提供 TIM2 / TIM3 / TIM4 / TIM5 的基础时基配置与更新中断功能
 * @{
 */

/** @brief 定时器中断抢占优先级 */
#define PREEMPT_PRIO    2

/** @brief 定时器中断子优先级 */
#define SUB_PRIO        2

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
 * @details
 * 该配置对应 TIMx_CR1 寄存器中的 CKD 位。
 *
 * @note
 * 这里的 `clock_div` 不是 PSC 预分频器。
 * 真正决定计数器频率的是 `prescaler`。
 */
typedef enum{
    BSP_TIM_CLOCK_DIV_1 = 0, /**< 时钟分频 1 */
    BSP_TIM_CLOCK_DIV_2,     /**< 时钟分频 2 */
    BSP_TIM_CLOCK_DIV_4      /**< 时钟分频 4 */
}bsp_generaltimer_clock_div_t;

/**
 * @brief 通用定时器基础配置结构体
 */
typedef struct{
    uint16_t prescaler;                           /**< PSC 预分频值，实际分频系数 = prescaler + 1 */
    bsp_generaltimer_counter_mode_t counter_mode;/**< 计数模式 */
    uint16_t period;                             /**< ARR 自动重装载值 */
    bsp_generaltimer_clock_div_t clock_div;      /**< 时钟分频（CKD） */
    uint8_t repetition_cnt;                      /**< 重复计数器，普通定时器通常无效，仅作兼容保留 */
}bsp_generaltimer_config_t;

/**
 * @brief 通用定时器更新事件计数数组
 * @details
 * 在更新中断中，对应定时器的计数值会自增一次。
 * 可用于在主循环中统计时间片或周期事件次数。
 */
extern volatile uint16_t generaltimer_cnt[BSP_GENERAL_TIMER_MAX];

/**
 * @brief 初始化通用定时器基础时基与更新中断
 * @details
 * 本函数完成：
 * - 开启 TIM2 / TIM3 / TIM4 / TIM5 时钟
 * - 配置时基参数（PSC / ARR / CounterMode / CKD / RepetitionCounter）
 * - 清除初始化阶段自动产生的更新标志
 * - 使能更新中断
 * - 配置 NVIC
 * - 启动定时器
 *
 * @param[in] timer_id
 * 通用定时器编号。
 * @param[in] config
 * 指向基础配置结构体的指针。
 *
 * @return 无返回值。
 */
extern void BSP_BaseTIM_Init(bsp_generaltimer_t timer_id, bsp_generaltimer_config_t *config);

/**
 * @brief 通用定时器更新中断公共处理函数
 * @details
 * 当对应通用定时器产生更新中断时：
 * - `generaltimer_cnt[timer_id]` 自增
 * - 清除更新中断挂起位
 *
 * @param[in] timer_id
 * 通用定时器编号。
 *
 * @return 无返回值。
 */
extern void BSP_TIM_IRQHandler(bsp_generaltimer_t timer_id);

/** @} */

#endif