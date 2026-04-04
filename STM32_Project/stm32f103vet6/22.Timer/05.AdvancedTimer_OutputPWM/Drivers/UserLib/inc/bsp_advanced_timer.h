#ifndef BSP_ADVANCED_TIMER_H
#define BSP_ADVANCED_TIMER_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include <stdlib.h>

/**
 * @file    bsp_advanced_timer.h
 * @brief   STM32F10x 高级定时器 PWM/互补输出 BSP 接口头文件
 * @details
 * 本文件封装了 STM32F10x 高级定时器 TIM1 / TIM8 的输出比较、PWM 输出、
 * 互补输出以及 BDTR（Break and Dead-Time Register）相关功能。
 *
 * 本模块主要适用于：
 * - PWM 波形输出
 * - 带互补输出的半桥/全桥驱动
 * - 电机控制场景
 * - 需要刹车输入 BKIN 的保护型输出场景
 * - 需要死区时间插入的功率驱动场景
 *
 * 模块主要支持以下特性：
 * - 基本定时器时基配置
 * - 输出比较模式配置
 * - 主输出与互补输出使能
 * - 输出有效电平配置
 * - 空闲状态输出配置
 * - BDTR 死区/刹车/自动输出配置
 *
 * @note
 * 1. 互补输出只存在于高级定时器 TIM1 / TIM8 的 CH1~CH3，CH4 没有互补输出。
 * 2. 若使用 PWM 输出，最终是否真正输出到引脚，不仅取决于 OC 配置，
 *    还取决于主输出使能 `TIM_CtrlPWMOutputs()`。
 * 3. BKIN 为刹车输入，用于出现故障时快速关闭 PWM 输出，常用于功率驱动保护。
 */

/**
 * @defgroup BSP_Advanced_Timer_PWM BSP 高级定时器 PWM 输出模块
 * @brief    提供 TIM1 / TIM8 的 PWM、互补输出与 BDTR 配置功能
 * @{
 */

/** @brief 定时器中断抢占优先级 */
#define PREEMPT_PRIO    2

/** @brief 定时器中断子优先级 */
#define SUB_PRIO        2

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
 * 这里的时钟分频对应 CKD，不等于 PSC 预分频。
 */
typedef enum{
    BSP_TIM_CLOCK_DIV_1 = 0, /**< 时钟分频 1 */
    BSP_TIM_CLOCK_DIV_2,     /**< 时钟分频 2 */
    BSP_TIM_CLOCK_DIV_4      /**< 时钟分频 4 */
}bsp_tim_base_clock_div_t;

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
 * 用于指定通道的输出行为，例如：
 * - Timing：仅比较，不改变输出
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
 * @note
 * 仅 CH1~CH3 具有互补输出，CH4 无互补输出引脚。
 */
typedef enum{
    BSP_TIM_OUTPUT_NSTATE_ENABLE = 0, /**< 互补输出使能 */
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
 */
typedef enum{
    BSP_TIM_OC_NPOLARITY_HIGH = 0, /**< 高电平有效 */
    BSP_TIM_OC_NPOLARITY_LOW       /**< 低电平有效 */
}bsp_tim_oc_npolarity_t;

/**
 * @brief 主输出空闲状态
 * @details
 * 当 MOE 被关闭或进入空闲状态时，输出通道维持的电平状态。
 */
typedef enum{
    BSP_TIM_OC_IDLE_STATE_SET = 0,   /**< 空闲状态输出为置位 */
    BSP_TIM_OC_IDLE_STATE_RESET      /**< 空闲状态输出为复位 */
}bsp_tim_oc_idle_state_t;

/**
 * @brief 互补输出空闲状态
 */
typedef enum{
    BSP_TIM_OC_NIDLE_STATE_SET = 0,  /**< 空闲状态输出为置位 */
    BSP_TIM_OC_NIDLE_STATE_RESET     /**< 空闲状态输出为复位 */
}bsp_tim_oc_nidle_state_t;

/**
 * @brief OSSR 状态配置
 * @details
 * OSSR（Off-State Selection for Run mode）用于运行模式下失效输出状态控制。
 */
typedef enum{
    BSP_TIM_BDTR_OSSR_STATE_ENABLE = 0, /**< 运行模式失效输出状态使能 */
    BSP_TIM_BDTR_OSSR_STATE_DISABLE     /**< 运行模式失效输出状态关闭 */
}bsp_tim_bdtr_ossr_state_t;

/**
 * @brief OSSI 状态配置
 * @details
 * OSSI（Off-State Selection for Idle mode）用于空闲模式下失效输出状态控制。
 */
typedef enum{
    BSP_TIM_BDTR_OSSI_STATE_ENABLE = 0, /**< 空闲模式失效输出状态使能 */
    BSP_TIM_BDTR_OSSI_STATE_DISABLE     /**< 空闲模式失效输出状态关闭 */
}bsp_tim_bdtr_ossi_state_t;

/**
 * @brief 锁定级别
 * @details
 * 锁定级别用于限制后续对部分定时器配置位的修改能力。
 */
typedef enum{
    BSP_TIM_BDTR_LOCK_LEVEL_OFF = 0, /**< 不锁定 */
    BSP_TIM_BDTR_LOCK_LEVEL_1,       /**< 锁定级别 1 */
    BSP_TIM_BDTR_LOCK_LEVEL_2,       /**< 锁定级别 2 */
    BSP_TIM_BDTR_LOCK_LEVEL_3        /**< 锁定级别 3 */
}bsp_tim_bdtr_lock_level_t;

/**
 * @brief 刹车功能使能状态
 */
typedef enum{
    BSP_TIM_BDTR_BREAK_ENABLE = 0, /**< 刹车输入使能 */
    BSP_TIM_BDTR_BREAK_DISABLE     /**< 刹车输入关闭 */
}bsp_tim_bdtr_break_t;

/**
 * @brief 刹车输入有效极性
 */
typedef enum{
    BSP_TIM_BDTR_BREAK_POLARITY_LOW = 0, /**< 低电平触发刹车 */
    BSP_TIM_BDTR_BREAK_POLARITY_HIGH     /**< 高电平触发刹车 */
}bsp_tim_bdtr_break_polarity_t;

/**
 * @brief 自动输出使能状态
 * @details
 * 自动输出允许在 Break 条件解除后自动重新使能输出。
 */
typedef enum{
    BSP_TIM_BDTR_AUTO_OUTPUT_ENABLE = 0, /**< 自动输出使能 */
    BSP_TIM_BDTR_AUTO_OUTPUT_DISABLE     /**< 自动输出关闭 */
}bsp_tim_bdtr_auto_output_t;

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
 * @brief 输出比较/PWM 配置结构体
 */
typedef struct{
    bsp_tim_oc_channel_t oc_channel;           /**< 输出比较通道 */
    bsp_tim_oc_mode_t oc_mode;                 /**< 输出比较模式 */
    bsp_tim_oc_out_state_t oc_output_state;    /**< 主输出使能状态 */
    bsp_tim_oc_out_nstate_t oc_output_nstate;  /**< 互补输出使能状态 */
    uint16_t oc_ccr_value;                     /**< CCR 比较值 / PWM 占空比控制值 */
    bsp_tim_oc_polarity_t oc_polarity;         /**< 主输出极性 */
    bsp_tim_oc_npolarity_t oc_npolarity;       /**< 互补输出极性 */
    bsp_tim_oc_idle_state_t oc_idle_state;     /**< 主输出空闲状态 */
    bsp_tim_oc_nidle_state_t oc_nidle_state;   /**< 互补输出空闲状态 */
}bsp_tim_oc_config_t;

/**
 * @brief BDTR（Break and Dead-Time）配置结构体
 */
typedef struct{
    bsp_tim_bdtr_ossr_state_t bdtr_ossr_state;           /**< 运行模式失效输出状态 */
    bsp_tim_bdtr_ossi_state_t bdtr_ossi_state;           /**< 空闲模式失效输出状态 */
    bsp_tim_bdtr_lock_level_t bdtr_lock_level;           /**< 锁定级别 */
    uint16_t bdtr_dead_time;                             /**< 死区时间 */
    bsp_tim_bdtr_break_t bdtr_break;                     /**< 刹车功能使能状态 */
    bsp_tim_bdtr_break_polarity_t bdtr_break_polarity;  /**< 刹车输入极性 */
    bsp_tim_bdtr_auto_output_t bdtr_auto_output;         /**< 自动输出状态 */
}bsp_tim_bdtr_config_t;

/**
 * @brief 高级定时器辅助计数数组
 * @details
 * 当前主要在更新中断中累加，用于简单观察定时器更新事件次数。
 */
extern volatile uint16_t advancedtimer_cnt[BSP_ADVANCED_TIMER_MAX];

/**
 * @brief 配置高级定时器 PWM / 输出比较 / 互补输出功能
 * @details
 * 内部完成：
 * - GPIO 初始化
 * - 高级定时器基础时基配置
 * - OC/PWM 配置
 * - BDTR 配置
 * - 主输出使能
 * - 启动定时器
 *
 * @param[in] timer_id
 * 高级定时器编号。
 * @param[in] base_config
 * 指向基础配置结构体的指针。
 * @param[in] oc_config
 * 指向输出比较配置结构体的指针。
 * @param[in] bdtr_config
 * 指向 BDTR 配置结构体的指针。
 *
 * @return 无返回值。
 */
extern void BSP_AdvancedTimer_Config(bsp_advancedtimer_t timer_id,
            bsp_advancedtimer_config_t *base_config,
            bsp_tim_oc_config_t *oc_config,
            bsp_tim_bdtr_config_t *bdtr_config);

/**
 * @brief 高级定时器中断公共处理函数
 * @details
 * 当前函数主要处理更新中断，用于统计定时器更新事件次数。
 *
 * @param[in] timer_id
 * 高级定时器编号。
 *
 * @return 无返回值。
 */
extern void BSP_TIM_IRQHandler(bsp_advancedtimer_t timer_id);

/** @} */

#endif