#ifndef BSP_BASE_TIMER_H
#define BSP_BASE_TIMER_H

#include "stm32f10x.h"

/**
 * @file    bsp_base_timer.h
 * @brief   STM32F10x 基本定时器基础时基/更新中断 BSP 接口头文件
 * @details
 * 本文件封装了 STM32F10x 基本定时器 TIM6 / TIM7 的基础时基配置与更新中断功能。
 *
 * 基本定时器与通用定时器、高级定时器不同，通常只具备最基础的：
 * - 时基计数功能
 * - 更新事件
 * - 更新中断
 *
 * 不支持：
 * - 输入捕获
 * - 输出比较
 * - PWM 输出
 * - 编码器接口
 *
 * 因此 TIM6 / TIM7 很适合作为：
 * - 软件定时节拍源
 * - 周期性任务触发器
 * - 简单 LED 闪烁控制
 * - 基础中断定时实验
 *
 * @note
 * 1. 本模块只使用 TIM6 / TIM7 的时基和更新中断功能。
 * 2. 当前通过软件计数数组 `basetimer_cnt[]` 记录中断触发次数。
 */

/**
 * @defgroup BSP_Base_Timer BSP 基本定时器模块
 * @brief    提供 TIM6 / TIM7 的基础时基配置与更新中断功能
 * @{
 */

/** @brief 定时器中断抢占优先级 */
#define PREEMPT_PRIO    2

/** @brief 定时器中断子优先级 */
#define SUB_PRIO        2

/**
 * @brief 基本定时器编号
 */
typedef enum{
    BSP_BASE_TIMER6 = 0, /**< 基本定时器 TIM6 */
    BSP_BASE_TIMER7,     /**< 基本定时器 TIM7 */
    BSP_BASE_TIMER_MAX   /**< 基本定时器数量上限/边界值 */
}bsp_basetimer_t;

/**
 * @brief 定时器计数模式
 * @details
 * 该枚举沿用了通用定时器/高级定时器接口风格。
 *
 * @note
 * 对 TIM6 / TIM7 这类基本定时器来说，实际通常只使用向上计数模式。
 */
typedef enum{
    BSP_TIM_COUNTER_MODE_UP = 0,           /**< 向上计数模式 */
    BSP_TIM_COUNTER_MODE_DOWN,             /**< 向下计数模式 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED1,  /**< 中央对齐模式 1 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED2,  /**< 中央对齐模式 2 */
    BSP_TIM_COUNTER_MODE_CENTER_ALIGNED3   /**< 中央对齐模式 3 */
}bsp_basetimer_counter_mode_t;

/**
 * @brief 定时器时钟分频配置
 * @details
 * 该配置对应 TIMx_CR1 寄存器中的 CKD 位。
 *
 * @note
 * 真正决定计数器频率的是 `prescaler`，不是 `clock_div`。
 */
typedef enum{
    BSP_TIM_CLOCK_DIV_1 = 0, /**< 时钟分频 1 */
    BSP_TIM_CLOCK_DIV_2,     /**< 时钟分频 2 */
    BSP_TIM_CLOCK_DIV_4      /**< 时钟分频 4 */
}bsp_basetimer_clock_div_t;

/**
 * @brief 基本定时器基础配置结构体
 */
typedef struct{
    uint16_t prescaler;                       /**< PSC 预分频值，实际分频系数 = prescaler + 1 */
    bsp_basetimer_counter_mode_t counter_mode; /**< 计数模式 */
    uint16_t period;                         /**< ARR 自动重装载值 */
    bsp_basetimer_clock_div_t clock_div;     /**< 时钟分频（CKD） */
    uint8_t repetition_cnt;                  /**< 重复计数器，基本定时器通常无效，仅作接口兼容保留 */
}bsp_basetimer_config_t;

/**
 * @brief 基本定时器更新事件计数数组
 * @details
 * 每当对应定时器进入一次更新中断，对应元素加 1。
 * 可用于在主循环中实现简易周期计数。
 */
extern volatile uint16_t basetimer_cnt[BSP_BASE_TIMER_MAX];

/**
 * @brief 初始化基本定时器基础时基与更新中断
 * @details
 * 本函数完成：
 * - 开启 TIM6 / TIM7 时钟
 * - 配置时基参数（PSC / ARR / CounterMode / CKD / RepetitionCounter）
 * - 清除初始化阶段自动产生的更新标志
 * - 使能更新中断
 * - 配置 NVIC
 * - 启动定时器
 *
 * @param[in] timer_id
 * 基本定时器编号。
 * @param[in] config
 * 指向基础配置结构体的指针。
 *
 * @return 无返回值。
 */
extern void BSP_BaseTIM_Init(bsp_basetimer_t timer_id, bsp_basetimer_config_t *config);

/**
 * @brief 基本定时器更新中断公共处理函数
 * @details
 * 当对应基本定时器产生更新中断时：
 * - `basetimer_cnt[timer_id]` 自增
 * - 清除更新中断挂起位
 *
 * @param[in] timer_id
 * 基本定时器编号。
 *
 * @return 无返回值。
 */
extern void BSP_TIM_IRQHandler(bsp_basetimer_t timer_id);

/** @} */

#endif