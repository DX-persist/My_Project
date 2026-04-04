/**
 * @file bsp_general_timer.h
 * @brief STM32F10x 通用定时器 BSP 驱动头文件
 * @author
 * @date
 * @version 1.0
 *
 * @details
 * 本模块对 STM32F10x 系列中的通用定时器 TIM2、TIM3、TIM4、TIM5 进行了统一封装，
 * 主要提供以下功能：
 * - 通用定时器基础参数配置
 * - 输出比较通道配置
 * - PWM / 翻转 / 比较输出等模式支持
 * - 更新中断计数处理接口
 *
 * 本模块通过抽象枚举和配置结构体，屏蔽具体 TIMx 硬件差异，
 * 使应用层可以通过统一接口完成不同通用定时器的配置。
 *
 * @note
 * 1. 当前模块主要面向 TIM2~TIM5 通用定时器。
 * 2. 某些参数（如互补输出、重复计数器）通常只在高级定时器中有实际意义，
 *    在通用定时器中配置后可能不会生效。
 * 3. 本模块中的中断处理函数仅处理更新中断标志，不负责 NVIC 初始化和中断使能配置。
 */

#ifndef BSP_GENERAL_TIMER_H
#define BSP_GENERAL_TIMER_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include "bsp_tim_common.h"

#include <stdlib.h>

/**
 * @def PREEMPT_PRIO
 * @brief 定时器中断的抢占优先级。
 */
#define PREEMPT_PRIO	2

/**
 * @def SUB_PRIO
 * @brief 定时器中断的子优先级。
 */
#define SUB_PRIO		2

/**
 * @enum bsp_generaltimer_t
 * @brief 通用定时器编号枚举。
 *
 * 该枚举用于标识本模块支持的通用定时器实例。
 */
typedef enum{
	/**< 通用定时器 TIM2 */
	BSP_GENERAL_TIMER2 = 0,
	/**< 通用定时器 TIM3 */
	BSP_GENERAL_TIMER3,
	/**< 通用定时器 TIM4 */
	BSP_GENERAL_TIMER4,
	/**< 通用定时器 TIM5 */
	BSP_GENERAL_TIMER5,
	/**< 通用定时器总数 */
	BSP_GENERAL_TIMER_MAX	
}bsp_generaltimer_t;

/**
 * @enum bsp_tim_oc_channel_t
 * @brief 输出比较通道枚举。
 *
 * 通用定时器最多支持 4 个输出比较通道。
 */
typedef enum{
    /**< 输出比较通道 1 */
    BSP_TIM_OC_CHANNEL1 = 0,
    /**< 输出比较通道 2 */
    BSP_TIM_OC_CHANNEL2,
    /**< 输出比较通道 3 */
    BSP_TIM_OC_CHANNEL3,
    /**< 输出比较通道 4 */
    BSP_TIM_OC_CHANNEL4,
} bsp_tim_oc_channel_t;

/**
 * @enum bsp_tim_oc_mode_t
 * @brief 输出比较模式枚举。
 *
 * 不同模式下，定时器通道会在比较匹配时产生不同的输出行为。
 */
typedef enum{
    /**< 定时模式，仅产生比较事件，不改变输出 */
    BSP_TIM_OC_MODE_TIMING = 0,
    /**< 激活模式，匹配时输出有效电平 */
    BSP_TIM_OC_MODE_ACTIVE,
    /**< 失活模式，匹配时输出无效电平 */
    BSP_TIM_OC_MODE_INACTIVE,
    /**< 翻转模式，匹配时翻转输出电平 */
    BSP_TIM_OC_MODE_TOGGLE,
    /**< PWM 模式 1 */
    BSP_TIM_OC_MODE_PWM1,
    /**< PWM 模式 2 */
    BSP_TIM_OC_MODE_PWM2
}bsp_tim_oc_mode_t;

/**
 * @enum bsp_tim_oc_out_state_t
 * @brief 输出比较主输出状态枚举。
 */
typedef enum{
    /**< 使能主输出 */
    BSP_TIM_OUTPUT_STATE_ENABLE = 0,
    /**< 禁止主输出 */
    BSP_TIM_OUTPUT_STATE_DISABLE
}bsp_tim_oc_out_state_t;

/**
 * @enum bsp_tim_oc_out_nstate_t
 * @brief 输出比较互补输出状态枚举。
 *
 * @note
 * 对于 TIM2~TIM5 这类通用定时器，互补输出通常不适用。
 */
typedef enum{
    /**< 使能互补输出 */
    BSP_TIM_OUTPUT_NSTATE_ENABLE = 0,
    /**< 禁止互补输出 */
    BSP_TIM_OUTPUT_NSTATE_DISABLE
}bsp_tim_oc_out_nstate_t;

/**
 * @enum bsp_tim_oc_polarity_t
 * @brief 主输出极性枚举。
 */
typedef enum{
    /**< 高电平有效 */
    BSP_TIM_OC_POLARITY_HIGH = 0,
    /**< 低电平有效 */
    BSP_TIM_OC_POLARITY_LOW
}bsp_tim_oc_polarity_t;

/**
 * @enum bsp_tim_oc_npolarity_t
 * @brief 互补输出极性枚举。
 */
typedef enum{
    /**< 高电平有效 */
    BSP_TIM_OC_NPOLARITY_HIGH = 0,
    /**< 低电平有效 */
    BSP_TIM_OC_NPOLARITY_LOW
}bsp_tim_oc_npolarity_t;

/**
 * @enum bsp_tim_oc_idle_state_t
 * @brief 主输出空闲状态枚举。
 *
 * 用于配置定时器未运行或空闲状态下输出引脚的默认状态。
 */
typedef enum{
    /**< 空闲状态输出置位 */
    BSP_TIM_OC_IDLE_STATE_SET = 0,
    /**< 空闲状态输出复位 */
    BSP_TIM_OC_IDLE_STATE_RESET
}bsp_tim_oc_idle_state_t;

/**
 * @enum bsp_tim_oc_nidle_state_t
 * @brief 互补输出空闲状态枚举。
 */
typedef enum{
    /**< 空闲状态输出置位 */
    BSP_TIM_OC_NIDLE_STATE_SET = 0,
    /**< 空闲状态输出复位 */
    BSP_TIM_OC_NIDLE_STATE_RESET
}bsp_tim_oc_nidle_state_t;

/**
 * @struct bsp_generaltimer_config_t
 * @brief 通用定时器基础配置结构体。
 *
 * 该结构体用于描述定时器计数单元的基本参数。
 */
typedef struct{
    /**
     * @brief 预分频器值。
     *
     * 定时器输入时钟频率将被分频为：
     * f_tim_cnt = f_tim / (prescaler + 1)
     */
    uint16_t prescaler;

    /**
     * @brief 计数模式。
     *
     * 包括向上计数、向下计数以及中央对齐模式。
     */
    bsp_tim_counter_mode_t counter_mode;

    /**
     * @brief 自动重装载值（ARR）。
     *
     * 决定计数器的周期上限。
     */
    uint16_t period;

    /**
     * @brief 时钟分频系数。
     *
     * 用于配置采样时钟分频。
     */
    bsp_tim_clock_div_t clock_div;

    /**
     * @brief 重复计数值。
     *
     * @note
     * 该参数通常只在高级定时器中有效，
     * 对 TIM2~TIM5 通用定时器一般无实际作用。
     */
    uint8_t repetition_cnt;
}bsp_generaltimer_config_t;

/**
 * @struct bsp_tim_oc_config_t
 * @brief 输出比较配置结构体。
 *
 * 用于配置某一个输出比较通道的工作模式和电气属性。
 */
typedef struct{
    /**< 输出比较通道编号 */
    bsp_tim_oc_channel_t oc_channel;

	/**< 输出比较模式 */
	bsp_tim_oc_mode_t oc_mode;

    /**< 主输出状态 */
    bsp_tim_oc_out_state_t oc_output_state;

    /**< 互补输出状态 */
    bsp_tim_oc_out_nstate_t oc_output_nstate;

    /**
     * @brief 比较寄存器初值（CCR）。
     *
     * 在 PWM 模式下，该值决定占空比；
     * 在输出比较模式下，该值决定比较匹配点。
     */
    uint16_t oc_ccr_value;

    /**< 主输出极性 */
    bsp_tim_oc_polarity_t oc_polarity;

    /**< 互补输出极性 */
    bsp_tim_oc_npolarity_t oc_npolarity;

    /**< 主输出空闲状态 */
    bsp_tim_oc_idle_state_t oc_idle_state;

    /**< 互补输出空闲状态 */
    bsp_tim_oc_nidle_state_t oc_nidle_state;
}bsp_tim_oc_config_t;

/**
 * @brief 通用定时器更新中断计数数组。
 *
 * 该数组用于记录各个定时器更新中断发生的次数。
 * 数组下标使用 @ref bsp_generaltimer_t 枚举值。
 *
 * @note
 * 该变量在中断环境中会被修改，因此声明为 volatile。
 */
extern volatile uint16_t generaltimer_cnt[BSP_GENERAL_TIMER_MAX];

/**
 * @brief 配置通用定时器及其输出比较通道。
 *
 * 该函数是本模块的主要对外接口，会依次完成：
 * - 定时器输出引脚 GPIO 初始化
 * - 定时器基础计数单元初始化
 * - 输出比较单元初始化
 * - 启动定时器
 *
 * @param timer_id 定时器编号，取值见 @ref bsp_generaltimer_t
 * @param base_config 指向基础配置结构体的指针
 * @param oc_config 指向输出比较配置结构体的指针
 *
 * @note
 * 1. 本函数不做空指针检查，调用前需保证参数有效。
 * 2. 本函数不负责 NVIC 初始化，也不主动使能更新中断。
 * 3. 若需要使用中断，需额外配置 TIM_ITConfig 和 NVIC。
 */
extern void BSP_GeneralTIM_Config(bsp_generaltimer_t timer_id, bsp_generaltimer_config_t *base_config, bsp_tim_oc_config_t *oc_config);

/**
 * @brief 通用定时器统一中断处理函数。
 *
 * 该函数用于处理指定通用定时器的更新中断事件。
 * 若检测到更新中断标志，则对应的软件计数器加 1，
 * 然后清除中断挂起标志位。
 *
 * @param timer_id 定时器编号，取值见 @ref bsp_generaltimer_t
 *
 * @note
 * 一般在实际中断服务函数中调用，例如：
 * @code
 * void TIM2_IRQHandler(void)
 * {
 *     BSP_TIM_IRQHandler(BSP_GENERAL_TIMER2);
 * }
 * @endcode
 */
extern void BSP_TIM_IRQHandler(bsp_generaltimer_t timer_id);

#endif