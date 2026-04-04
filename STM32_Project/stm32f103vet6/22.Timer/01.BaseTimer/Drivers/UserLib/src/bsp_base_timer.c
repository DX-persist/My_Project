#include "bsp_base_timer.h"

/**
 * @file    bsp_base_timer.c
 * @brief   STM32F10x 基本定时器基础时基/更新中断 BSP 实现文件
 * @details
 * 本文件实现 TIM6 / TIM7 的基础时基初始化与更新中断处理功能。
 *
 * 当前实现只使用基本定时器的以下能力：
 * - 基础时基计数
 * - 更新事件
 * - 更新中断
 *
 * 典型用途：
 * - 周期性翻转 LED
 * - 作为简单软件定时节拍源
 * - 基础定时中断实验
 *
 * @note
 * TIM6 / TIM7 没有外部输入输出通道，因此不能用于 PWM、输入捕获等功能。
 */

/**
 * @brief 基本定时器硬件资源映射结构体
 */
typedef struct{
    TIM_TypeDef *timer;         /**< 定时器寄存器实例 */
    uint32_t rcc_clk;           /**< RCC 时钟使能位 */

    uint8_t irq_update_channel; /**< 更新中断号 */
    uint8_t irq_pre_prio;       /**< 中断抢占优先级 */
    uint8_t irq_sub_prio;       /**< 中断子优先级 */
}bsp_basetimer_hw_t;

/** @brief 基本定时器更新事件计数数组 */
volatile uint16_t basetimer_cnt[BSP_BASE_TIMER_MAX] = {0};

/**
 * @brief 基本定时器硬件资源映射表
 */
static const bsp_basetimer_hw_t bsp_basetimer_hw[BSP_BASE_TIMER_MAX] = {
    [BSP_BASE_TIMER6] = {
        .timer = TIM6,
        .rcc_clk = RCC_APB1Periph_TIM6,

        .irq_update_channel = TIM6_IRQn,
        .irq_pre_prio       = PREEMPT_PRIO,
        .irq_sub_prio       = SUB_PRIO,
    },

    [BSP_BASE_TIMER7] = {
        .timer = TIM7,
        .rcc_clk = RCC_APB1Periph_TIM7,

        .irq_update_channel = TIM7_IRQn,
        .irq_pre_prio       = PREEMPT_PRIO,
        .irq_sub_prio       = SUB_PRIO,
    }
};

/**
 * @brief 将 BSP 计数模式枚举转换为标准库宏
 * @param[in] counter_mode BSP 层计数模式枚举值
 * @return 对应的 TIM_CounterMode_xxx 宏，非法值返回 0
 *
 * @note
 * TIM6 / TIM7 实际通常工作在向上计数模式，这里保留完整枚举转换主要是为了接口统一。
 */
static uint16_t BSP_TIM_ConvertCounterMode(bsp_basetimer_counter_mode_t counter_mode)
{
    switch(counter_mode){
        case BSP_TIM_COUNTER_MODE_UP: return TIM_CounterMode_Up;
        case BSP_TIM_COUNTER_MODE_DOWN: return TIM_CounterMode_Down;
        case BSP_TIM_COUNTER_MODE_CENTER_ALIGNED1: return TIM_CounterMode_CenterAligned1;
        case BSP_TIM_COUNTER_MODE_CENTER_ALIGNED2: return TIM_CounterMode_CenterAligned2;
        case BSP_TIM_COUNTER_MODE_CENTER_ALIGNED3: return TIM_CounterMode_CenterAligned3;
        default: return 0;
    }
}

/**
 * @brief 将 BSP 时钟分频枚举转换为标准库 CKD 宏
 * @param[in] clock_div BSP 层时钟分频枚举值
 * @return 对应的 TIM_CKD_DIVx 宏，非法值返回 0
 *
 * @note
 * CKD 并不直接决定定时器计数频率，真正决定计数频率的是 PSC。
 */
static uint16_t BSP_TIM_ConvertClockDivision(bsp_basetimer_clock_div_t clock_div)
{
    switch(clock_div){
        case BSP_TIM_CLOCK_DIV_1: return TIM_CKD_DIV1;
        case BSP_TIM_CLOCK_DIV_2: return TIM_CKD_DIV2;
        case BSP_TIM_CLOCK_DIV_4: return TIM_CKD_DIV4;
        default: return 0;
    }
}

/**
 * @brief 配置基本定时器更新中断对应的 NVIC
 * @param[in] timer_id 基本定时器编号
 * @return 无返回值
 */
static void BSP_TIM_NVIC_Config(bsp_basetimer_t timer_id)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    const bsp_basetimer_hw_t *hw = &bsp_basetimer_hw[timer_id];

    NVIC_InitStruct.NVIC_IRQChannel = hw->irq_update_channel;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = hw->irq_pre_prio;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = hw->irq_sub_prio;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
 * @brief 初始化基本定时器基础时基与更新中断
 * @details
 * 本函数完成：
 * - 使能 TIMx 时钟
 * - 初始化 TIM_TimeBaseInitTypeDef
 * - 配置 PSC、ARR、计数模式、时钟分频、重复计数器
 * - 使能 ARR 预装载功能
 * - 清除初始化阶段自动产生的更新标志
 * - 使能更新中断
 * - 配置 NVIC
 * - 启动定时器
 *
 * @param[in] timer_id 基本定时器编号
 * @param[in] config 基础时基配置结构体指针
 * @return 无返回值
 *
 * @note
 * 由于 ARR 和 PSC 预装载机制，初始化阶段通常会自动触发一次更新事件，
 * 因此这里需要手动清除 `TIM_IT_Update` 标志位，避免上电后立即误进入中断。
 */
void BSP_BaseTIM_Init(bsp_basetimer_t timer_id, bsp_basetimer_config_t *config)
{
    if(timer_id >= BSP_BASE_TIMER_MAX) return;

    const bsp_basetimer_hw_t *hw = &bsp_basetimer_hw[timer_id];
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);

    RCC_APB1PeriphClockCmd(hw->rcc_clk, ENABLE); /**< 开启 TIMx 的时钟 */

    /* 初始化 TIM_BaseInitTypeDef 结构体 */
    TIM_TimeBaseInitStruct.TIM_Prescaler = config->prescaler;
    TIM_TimeBaseInitStruct.TIM_CounterMode = BSP_TIM_ConvertCounterMode(config->counter_mode);
    TIM_TimeBaseInitStruct.TIM_Period = config->period;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = BSP_TIM_ConvertClockDivision(config->clock_div);
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = config->repetition_cnt;
    TIM_TimeBaseInit(hw->timer, &TIM_TimeBaseInitStruct);

    /* 启用 ARR 预装载功能 */
    TIM_ARRPreloadConfig(hw->timer, ENABLE);

    /*
     * 这里需要清空 TIM_IT_Update 标志位，因为 ARR 和 Prescaler
     * 都具有预装载功能，所以硬件会自动发生一次更新事件将数据
     * 写入到寄存器中。
     */
    TIM_ClearFlag(hw->timer, TIM_IT_Update);

    /* 配置更新中断并设置 NVIC */
    TIM_ITConfig(hw->timer, TIM_IT_Update, ENABLE);
    BSP_TIM_NVIC_Config(timer_id);

    /* 启动定时器 */
    TIM_Cmd(hw->timer, ENABLE);
}

/**
 * @brief 基本定时器中断公共处理函数
 * @details
 * 当前仅处理更新中断：
 * - 更新计数数组 `basetimer_cnt[timer_id]`
 * - 清除中断挂起位
 *
 * @param[in] timer_id 基本定时器编号
 * @return 无返回值
 */
void BSP_TIM_IRQHandler(bsp_basetimer_t timer_id)
{
    if(timer_id >= BSP_BASE_TIMER_MAX) return;

    const bsp_basetimer_hw_t *hw = &bsp_basetimer_hw[timer_id];

    if(TIM_GetITStatus(hw->timer, TIM_IT_Update) != RESET){
        basetimer_cnt[timer_id]++;
        TIM_ClearITPendingBit(hw->timer, TIM_IT_Update);
    }
}