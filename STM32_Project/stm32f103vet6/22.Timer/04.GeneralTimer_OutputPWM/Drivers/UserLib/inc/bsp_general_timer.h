#ifndef BSP_GENERAL_TIMER_H
#define BSP_GENERAL_TIMER_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include <stdlib.h>

/**
 * @file    bsp_general_timer.h
 * @brief   STM32F10x 通用定时器 PWM / 输出比较 BSP 接口头文件
 * @details
 * 本文件封装了 STM32F10x 通用定时器 TIM2 / TIM3 / TIM4 / TIM5 的
 * 输出比较（Output Compare）与 PWM 输出功能。
 *
 * 本模块适用于：
 * - 普通 PWM 波形输出
 * - 输出比较定时控制
 * - 占空比可调的方波输出
 * - 基础驱动类场景
 *
 * 与高级定时器不同，本模块不涉及：
 * - 互补输出
 * - 死区时间
 * - BKIN 刹车输入
 * - 主输出使能 MOE
 *
 * @note
 * 1. 本模块主要用于普通定时器 PWM 输出，不适用于高级功率驱动控制场景。
 * 2. 若使用 PWM 模式，最终波形频率由 PSC 和 ARR 决定，占空比由 CCR 决定。
 * 3. 当前接口保留了部分与高级定时器风格一致的字段，例如 `oc_output_nstate`，
 *    但普通定时器实际上没有互补输出能力，这一点要特别注意。
 */

/**
 * @defgroup BSP_General_Timer_PWM BSP 通用定时器 PWM 输出模块
 * @brief    提供 TIM2 / TIM3 / TIM4 / TIM5 的 PWM/输出比较配置功能
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
 * @note
 * 这里的 `clock_div` 对应 CKD，不等于 PSC 预分频。
 */
typedef enum{
    BSP_TIM_CLOCK_DIV_1 = 0, /**< 时钟分频 1 */
    BSP_TIM_CLOCK_DIV_2,     /**< 时钟分频 2 */
    BSP_TIM_CLOCK_DIV_4      /**< 时钟分频 4 */
}bsp_generaltimer_clock_div_t;

/**
 * @brief 输出比较通道
 */
typedef enum{
    BSP_TIM_OC_CHANNEL1 = 0, /**< 输出比较通道 1 */
    BSP_TIM_OC_CHANNEL2,     /**< 输出比较通道 2 */
    BSP_TIM_OC_CHANNEL3,     /**< 输出比较通道 3 */
    BSP_TIM_OC_CHANNEL4      /**< 输出比较通道 4 */
} bsp_tim_oc_channel_t;

/**
 * @brief 输出比较模式
 * @details
 * 常用模式包括：
 * - Timing：仅做比较，不直接改变输出电平
 * - Toggle：匹配时翻转输出
 * - PWM1 / PWM2：PWM 模式
 */
typedef enum{
    BSP_TIM_OC_MODE_TIMING = 0, /**< 定时模式 */
    BSP_TIM_OC_MODE_ACTIVE,     /**< 匹配时输出有效电平 */
    BSP_TIM_OC_MODE_INACTIVE,   /**< 匹配时输出无效电平 */
    BSP_TIM_OC_MODE_TOGGLE,     /**< 匹配时翻转输出 */
    BSP_TIM_OC_MODE_PWM1,       /**< PWM 模式 1 */
    BSP_TIM_OC_MODE_PWM2        /**< PWM 模式 2 */
}bsp_tim_oc_mode_t;

/**
 * @brief 主输出状态
 */
typedef enum{
    BSP_TIM_OUTPUT_STATE_ENABLE = 0, /**< 主输出使能 */
    BSP_TIM_OUTPUT_STATE_DISABLE     /**< 主输出关闭 */
}bsp_tim_oc_out_state_t;

/**
 * @brief 互补输出状态
 * @details
 * 普通定时器并不支持互补输出，本枚举仅为接口风格统一而保留。
 *
 * @note
 * 对 TIM2 / TIM3 / TIM4 / TIM5 而言，该字段通常不会产生实际硬件输出效果。
 */
typedef enum{
    BSP_TIM_OUTPUT_NSTATE_ENABLE = 0, /**< 互补输出使能（普通定时器无实际意义） */
    BSP_TIM_OUTPUT_NSTATE_DISABLE     /**< 互补输出关闭 */
}bsp_tim_oc_out_nstate_t;

/**
 * @brief 主输出极性
 */
typedef enum{
    BSP_TIM_OC_POLARITY_HIGH = 0, /**< 高电平有效 */
    BSP_TIM_OC_POLARITY_LOW       /**< 低电平有效 */
}bsp_tim_oc_polarity_t;

/**
 * @brief 互补输出极性
 * @details
 * 普通定时器无互补输出，本枚举主要为接口统一保留。
 */
typedef enum{
    BSP_TIM_OC_NPOLARITY_HIGH = 0, /**< 高电平有效 */
    BSP_TIM_OC_NPOLARITY_LOW       /**< 低电平有效 */
}bsp_tim_oc_npolarity_t;

/**
 * @brief 主输出空闲状态
 * @details
 * 普通定时器中该字段通常意义有限，主要为了与高级定时器接口保持一致。
 */
typedef enum{
    BSP_TIM_OC_IDLE_STATE_SET = 0,   /**< 空闲状态为置位 */
    BSP_TIM_OC_IDLE_STATE_RESET      /**< 空闲状态为复位 */
}bsp_tim_oc_idle_state_t;

/**
 * @brief 互补输出空闲状态
 * @details
 * 普通定时器无互补输出，本枚举主要为接口统一保留。
 */
typedef enum{
    BSP_TIM_OC_NIDLE_STATE_SET = 0,  /**< 空闲状态为置位 */
    BSP_TIM_OC_NIDLE_STATE_RESET     /**< 空闲状态为复位 */
}bsp_tim_oc_nidle_state_t;

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
 * @brief 输出比较 / PWM 配置结构体
 */
typedef struct{
    bsp_tim_oc_channel_t oc_channel;           /**< 输出比较通道 */
    bsp_tim_oc_mode_t oc_mode;                 /**< 输出比较模式 */
    bsp_tim_oc_out_state_t oc_output_state;    /**< 主输出使能状态 */
    bsp_tim_oc_out_nstate_t oc_output_nstate;  /**< 互补输出使能状态（普通定时器无实际意义） */
    uint16_t oc_ccr_value;                     /**< CCR 比较值 / PWM 占空比控制值 */
    bsp_tim_oc_polarity_t oc_polarity;         /**< 主输出极性 */
    bsp_tim_oc_npolarity_t oc_npolarity;       /**< 互补输出极性（普通定时器无实际意义） */
    bsp_tim_oc_idle_state_t oc_idle_state;     /**< 主输出空闲状态 */
    bsp_tim_oc_nidle_state_t oc_nidle_state;   /**< 互补输出空闲状态（普通定时器无实际意义） */
}bsp_tim_oc_config_t;

/**
 * @brief 通用定时器更新事件计数数组
 * @details
 * 当前主要用于在更新中断中简单统计定时器更新次数。
 */
extern volatile uint16_t generaltimer_cnt[BSP_GENERAL_TIMER_MAX];

/**
 * @brief 配置通用定时器 PWM / 输出比较功能
 * @details
 * 内部完成：
 * - GPIO 初始化
 * - 定时器基础时基配置
 * - 输出比较/PWM 配置
 * - CCR 预装载配置
 * - 启动定时器
 *
 * @param[in] timer_id
 * 通用定时器编号。
 * @param[in] base_config
 * 指向基础时基配置结构体的指针。
 * @param[in] oc_config
 * 指向输出比较配置结构体的指针。
 *
 * @return 无返回值。
 */
extern void BSP_GeneralTIM_Config(bsp_generaltimer_t timer_id, bsp_generaltimer_config_t *base_config, bsp_tim_oc_config_t *oc_config);

/**
 * @brief 通用定时器中断公共处理函数
 * @details
 * 当前函数主要处理更新中断，用于统计定时器更新事件次数。
 *
 * @param[in] timer_id
 * 通用定时器编号。
 *
 * @return 无返回值。
 */
extern void BSP_TIM_IRQHandler(bsp_generaltimer_t timer_id);

/** @} */

#endif