#include "bsp_advanced_timer.h"

/**
 * @file    bsp_advanced_timer.c
 * @brief   STM32F10x 高级定时器 PWM/互补输出 BSP 实现文件
 * @details
 * 本文件实现 TIM1 / TIM8 的高级 PWM 输出功能，支持：
 * - 主输出
 * - 互补输出
 * - BDTR 死区时间配置
 * - Break 刹车输入
 * - 主输出使能
 *
 * 典型应用场景包括：
 * - 电机驱动
 * - 半桥/全桥 PWM 控制
 * - 带保护的功率输出
 *
 * @note
 * 当前实现重点覆盖 CH1~CH3 的主输出与互补输出，CH4 仅有主输出，没有互补输出。
 */

/**
 * @brief 高级定时器硬件资源映射结构体
 */
typedef struct{
    TIM_TypeDef *timer;         /**< 定时器寄存器实例 */
    uint32_t rcc_clk;           /**< RCC 时钟使能位 */

    uint8_t irq_update_channel; /**< 更新中断号 */
    uint8_t irq_pre_prio;       /**< 中断抢占优先级 */
    uint8_t irq_sub_prio;       /**< 中断子优先级 */

    bsp_gpio_t ch1_gpio;        /**< CH1 主输出引脚 */
    bsp_gpio_t ch1n_gpio;       /**< CH1 互补输出引脚 */
    bsp_gpio_t ch2_gpio;        /**< CH2 主输出引脚 */
    bsp_gpio_t ch2n_gpio;       /**< CH2 互补输出引脚 */
    bsp_gpio_t ch3_gpio;        /**< CH3 主输出引脚 */
    bsp_gpio_t ch3n_gpio;       /**< CH3 互补输出引脚 */
    bsp_gpio_t ch4_gpio;        /**< CH4 主输出引脚 */

    bsp_gpio_t bkin_gpio;       /**< 刹车输入 BKIN 引脚 */
}bsp_advancedtimer_hw_t;

/** @brief 高级定时器更新事件计数数组 */
volatile uint16_t advancedtimer_cnt[BSP_ADVANCED_TIMER_MAX] = {0};

/**
 * @brief 高级定时器硬件资源映射表
 */
static const bsp_advancedtimer_hw_t bsp_advancedtimer_hw[BSP_ADVANCED_TIMER_MAX] = {
    [BSP_ADVANCED_TIMER1] = {
        .timer = TIM1,
        .rcc_clk = RCC_APB2Periph_TIM1,

        .irq_update_channel = TIM1_UP_IRQn,
        .irq_pre_prio       = PREEMPT_PRIO,
        .irq_sub_prio       = SUB_PRIO,

        .ch1_gpio  = {GPIOA, GPIO_Pin_8, RCC_APB2Periph_GPIOA},
        .ch1n_gpio = {GPIOB, GPIO_Pin_13, RCC_APB2Periph_GPIOB},
        .ch2_gpio  = {GPIOA, GPIO_Pin_9, RCC_APB2Periph_GPIOA},
        .ch2n_gpio = {GPIOB, GPIO_Pin_14, RCC_APB2Periph_GPIOB},
        .ch3_gpio  = {GPIOA, GPIO_Pin_10, RCC_APB2Periph_GPIOA},
        .ch3n_gpio = {GPIOB, GPIO_Pin_15, RCC_APB2Periph_GPIOB},
        .ch4_gpio  = {GPIOA, GPIO_Pin_11, RCC_APB2Periph_GPIOA},
        .bkin_gpio = {GPIOB, GPIO_Pin_12, RCC_APB2Periph_GPIOB},
    },

    [BSP_ADVANCED_TIMER8] = {
        .timer = TIM8,
        .rcc_clk = RCC_APB2Periph_TIM8,

        .irq_update_channel = TIM8_UP_IRQn,
        .irq_pre_prio       = PREEMPT_PRIO,
        .irq_sub_prio       = SUB_PRIO,

        .ch1_gpio  = {GPIOC, GPIO_Pin_6, RCC_APB2Periph_GPIOC},
        .ch1n_gpio = {GPIOA, GPIO_Pin_7, RCC_APB2Periph_GPIOA},
        .ch2_gpio  = {GPIOC, GPIO_Pin_7, RCC_APB2Periph_GPIOC},
        .ch2n_gpio = {GPIOB, GPIO_Pin_0, RCC_APB2Periph_GPIOB},
        .ch3_gpio  = {GPIOC, GPIO_Pin_8, RCC_APB2Periph_GPIOC},
        .ch3n_gpio = {GPIOB, GPIO_Pin_1, RCC_APB2Periph_GPIOB},
        .ch4_gpio  = {GPIOC, GPIO_Pin_9, RCC_APB2Periph_GPIOC},
        .bkin_gpio = {GPIOA, GPIO_Pin_6, RCC_APB2Periph_GPIOA},
    }
};

/**
 * @brief 根据 OC 通道获取主输出 GPIO 描述信息
 * @param[in] hw 高级定时器硬件资源结构体指针
 * @param[in] oc_channel 输出比较通道
 * @return 成功返回 GPIO 描述结构体指针，失败返回 NULL
 */
static const bsp_gpio_t *BSP_TIM_GetCHGPIO(const bsp_advancedtimer_hw_t *hw, bsp_tim_oc_channel_t oc_channel)
{
    switch(oc_channel){
        case BSP_TIM_OC_CHANNEL1: return &hw->ch1_gpio;
        case BSP_TIM_OC_CHANNEL2: return &hw->ch2_gpio;
        case BSP_TIM_OC_CHANNEL3: return &hw->ch3_gpio;
        case BSP_TIM_OC_CHANNEL4: return &hw->ch4_gpio;
        default: return NULL;
    }
}

/**
 * @brief 根据 OC 通道获取互补输出 GPIO 描述信息
 * @param[in] hw 高级定时器硬件资源结构体指针
 * @param[in] oc_channel 输出比较通道
 * @return 成功返回互补输出 GPIO 描述结构体指针；CH4 或非法参数返回 NULL
 *
 * @note
 * CH4 没有互补输出，因此返回 NULL。
 */
static const bsp_gpio_t *BSP_TIM_GetCHNGPIO(const bsp_advancedtimer_hw_t *hw, bsp_tim_oc_channel_t oc_channel)
{
    switch(oc_channel){
        case BSP_TIM_OC_CHANNEL1: return &hw->ch1n_gpio;
        case BSP_TIM_OC_CHANNEL2: return &hw->ch2n_gpio;
        case BSP_TIM_OC_CHANNEL3: return &hw->ch3n_gpio;
        case BSP_TIM_OC_CHANNEL4: return NULL;
        default: return NULL;
    }
}

/**
 * @brief 初始化 PWM/互补输出相关 GPIO
 * @details
 * 该函数会初始化：
 * - 主输出引脚 CHx
 * - 互补输出引脚 CHxN（若存在）
 * - BKIN 刹车输入引脚
 *
 * @param[in] timer_id 高级定时器编号
 * @param[in] oc_channel 输出比较通道
 * @return 无返回值
 *
 * @note
 * 1. CHx / CHxN 配置为复用推挽输出
 * 2. BKIN 配置为上拉输入模式，便于低电平触发刹车
 * 3. 当前实现里若选择 CH4，`chn_gpio` 为 NULL，理论上应额外判空后再访问。
 *    这是源码里一个容易出错的点，后续建议补上保护。
 */
static void BSP_TIM_GPIO_Init(bsp_advancedtimer_t timer_id, bsp_tim_oc_channel_t oc_channel)
{
    if(timer_id >= BSP_ADVANCED_TIMER_MAX) return;

    const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];
    const bsp_gpio_t *ch_gpio = BSP_TIM_GetCHGPIO(hw, oc_channel);
    const bsp_gpio_t *chn_gpio = BSP_TIM_GetCHNGPIO(hw, oc_channel);
    uint32_t enable_clk = ch_gpio->rcc_clk | chn_gpio->rcc_clk | hw->bkin_gpio.rcc_clk;
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(enable_clk, ENABLE);

    GPIO_InitStruct.GPIO_Pin = ch_gpio->gpio_pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ch_gpio->gpio_port, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = chn_gpio->gpio_pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(chn_gpio->gpio_port, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = hw->bkin_gpio.gpio_pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; /**< 上拉输入，结合低电平刹车触发使用 */
    GPIO_Init(hw->bkin_gpio.gpio_port, &GPIO_InitStruct);
}

/**
 * @brief 将 BSP 计数模式枚举转换为标准库宏
 * @param[in] counter_mode BSP 层计数模式枚举值
 * @return 对应的 TIM_CounterMode_xxx 宏，非法值返回 0
 */
static uint16_t BSP_TIM_ConvertCounterMode(bsp_tim_base_counter_mode_t counter_mode)
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
 * CKD 不等于 PSC，不能直接理解为定时器计数频率分频。
 */
static uint16_t BSP_TIM_ConvertClockDivision(bsp_tim_base_clock_div_t clock_div)
{
    switch(clock_div){
        case BSP_TIM_CLOCK_DIV_1: return TIM_CKD_DIV1;
        case BSP_TIM_CLOCK_DIV_2: return TIM_CKD_DIV2;
        case BSP_TIM_CLOCK_DIV_4: return TIM_CKD_DIV4;
        default: return 0;
    }
}

/**
 * @brief 将 BSP 输出比较模式枚举转换为标准库 OC 模式宏
 * @param[in] mode 输出比较模式
 * @return 对应的 TIM_OCMode_xxx 宏
 *
 * @note
 * 默认返回 PWM1 模式。
 */
static uint16_t BSP_TIM_ConvertOCMode(bsp_tim_oc_mode_t mode)
{
    switch(mode){
        case BSP_TIM_OC_MODE_TIMING:
            return TIM_OCMode_Timing;
        case BSP_TIM_OC_MODE_ACTIVE:
            return TIM_OCMode_Active;
        case BSP_TIM_OC_MODE_INACTIVE:
            return TIM_OCMode_Inactive;
        case BSP_TIM_OC_MODE_TOGGLE:
            return TIM_OCMode_Toggle;
        case BSP_TIM_OC_MODE_PWM1:
            return TIM_OCMode_PWM1;
        case BSP_TIM_OC_MODE_PWM2:
            return TIM_OCMode_PWM2;
        default:
            return TIM_OCMode_PWM1;
    }
}

/**
 * @brief 将 BSP 锁定级别枚举转换为标准库 LOCKLevel 宏
 * @param[in] lock_level 锁定级别
 * @return 对应的 TIM_LOCKLevel_x 宏
 */
static uint16_t BSP_TIM_ConvertLockLevel(bsp_tim_bdtr_lock_level_t lock_level)
{
    switch(lock_level){
        case BSP_TIM_BDTR_LOCK_LEVEL_OFF:
            return TIM_LOCKLevel_OFF;
        case BSP_TIM_BDTR_LOCK_LEVEL_1:
            return TIM_LOCKLevel_1;
        case BSP_TIM_BDTR_LOCK_LEVEL_2:
            return TIM_LOCKLevel_2;
        case BSP_TIM_BDTR_LOCK_LEVEL_3:
            return TIM_LOCKLevel_3;
        default:
            return TIM_LOCKLevel_1;
    }
}

/**
 * @brief 根据通道调用对应的 TIM_OCxInit() 初始化函数
 * @param[in] tim 定时器实例
 * @param[in] channel 输出比较通道
 * @param[in] oc_init 输出比较初始化结构体
 * @return 无返回值
 *
 * @note
 * 当前实现只对 CH1~CH3 调用了 OCxInit，CH4 未处理。
 * 若后续要完整支持 CH4 输出，需要补充 TIM_OC4Init()。
 */
static void BSP_TIM_OCxInit(TIM_TypeDef *tim, bsp_tim_oc_channel_t channel, TIM_OCInitTypeDef *oc_init)
{
    switch (channel){
        case BSP_TIM_OC_CHANNEL1: TIM_OC1Init(tim, oc_init); break;
        case BSP_TIM_OC_CHANNEL2: TIM_OC2Init(tim, oc_init); break;
        case BSP_TIM_OC_CHANNEL3: TIM_OC3Init(tim, oc_init); break;
        default: break;
    }
}

/**
 * @brief 根据通道配置 CCR 预装载功能
 * @param[in] tim 定时器实例
 * @param[in] channel 输出比较通道
 * @param[in] state 预装载配置状态
 * @return 无返回值
 *
 * @note
 * 当前仅支持 CH1~CH3，CH4 未处理。
 */
static void BSP_TIM_OCxPreloadConfig(TIM_TypeDef *tim, bsp_tim_oc_channel_t channel, uint16_t state)
{
    switch (channel){
        case BSP_TIM_OC_CHANNEL1: TIM_OC1PreloadConfig(tim, state); break;
        case BSP_TIM_OC_CHANNEL2: TIM_OC2PreloadConfig(tim, state); break;
        case BSP_TIM_OC_CHANNEL3: TIM_OC3PreloadConfig(tim, state); break;
        default: break;
    }
}

/**
 * @brief 配置高级定时器更新中断对应的 NVIC
 * @param[in] timer_id 高级定时器编号
 * @return 无返回值
 */
static void BSP_TIM_NVIC_Config(bsp_advancedtimer_t timer_id)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];

    NVIC_InitStruct.NVIC_IRQChannel = hw->irq_update_channel;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = hw->irq_pre_prio;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = hw->irq_sub_prio;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
 * @brief 初始化高级定时器 PWM/互补输出功能
 * @details
 * 本函数完成：
 * - 定时器基础时基配置
 * - OC/PWM 参数配置
 * - CCR 预装载配置
 * - BDTR 参数配置
 * - 主输出使能
 * - 启动定时器
 *
 * @param[in] timer_id 高级定时器编号
 * @param[in] base_config 基础时基配置
 * @param[in] oc_config 输出比较/PWM 配置
 * @param[in] bdtr_config BDTR 配置
 * @return 无返回值
 *
 * @note
 * `TIM_CtrlPWMOutputs()` 对高级定时器非常关键，不调用时即使 OC 配好了也可能没有实际波形输出。
 */
static void BSP_AdvancedTIM_Init(bsp_advancedtimer_t timer_id,
            bsp_advancedtimer_config_t *base_config,
            bsp_tim_oc_config_t *oc_config,
            bsp_tim_bdtr_config_t *bdtr_config)
{
    if(timer_id >= BSP_ADVANCED_TIMER_MAX) return;

    const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_BDTRInitTypeDef TIM_BDTRInitStruct;
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_OCStructInit(&TIM_OCInitStruct);
    TIM_BDTRStructInit(&TIM_BDTRInitStruct);

    RCC_APB2PeriphClockCmd(hw->rcc_clk, ENABLE); /**< 开启 TIMx 时钟 */

    /* 初始化 TIM_BaseInitTypeDef 结构体 */
    TIM_TimeBaseInitStruct.TIM_Prescaler = base_config->prescaler;
    TIM_TimeBaseInitStruct.TIM_CounterMode = BSP_TIM_ConvertCounterMode(base_config->counter_mode);
    TIM_TimeBaseInitStruct.TIM_Period = base_config->period;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = BSP_TIM_ConvertClockDivision(base_config->clock_div);
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = base_config->repetition_cnt;
    TIM_TimeBaseInit(hw->timer, &TIM_TimeBaseInitStruct);

    /* 启用 PSC 和 ARR 预装载功能 */
    TIM_PrescalerConfig(hw->timer, base_config->prescaler, ENABLE);
    TIM_ARRPreloadConfig(hw->timer, ENABLE);

    TIM_OCInitStruct.TIM_OCMode = BSP_TIM_ConvertOCMode(oc_config->oc_mode); /**< 输出比较模式 */
    TIM_OCInitStruct.TIM_OutputState = (oc_config->oc_output_state == BSP_TIM_OUTPUT_STATE_ENABLE) ?
                TIM_OutputState_Enable : TIM_OutputState_Disable;             /**< 主输出使能 */
    TIM_OCInitStruct.TIM_OutputNState = (oc_config->oc_output_nstate == BSP_TIM_OUTPUT_NSTATE_ENABLE) ?
                TIM_OutputNState_Enable : TIM_OutputNState_Disable;           /**< 互补输出使能 */
    TIM_OCInitStruct.TIM_Pulse = oc_config->oc_ccr_value;                     /**< CCR 比较值 / 占空比值 */
    TIM_OCInitStruct.TIM_OCPolarity = (oc_config->oc_polarity == BSP_TIM_OC_POLARITY_HIGH) ?
                TIM_OCPolarity_High : TIM_OCPolarity_Low;                     /**< 主输出极性 */
    TIM_OCInitStruct.TIM_OCNPolarity = (oc_config->oc_npolarity == BSP_TIM_OC_NPOLARITY_HIGH) ?
                TIM_OCNPolarity_High : TIM_OCNPolarity_Low;                   /**< 互补输出极性 */
    TIM_OCInitStruct.TIM_OCIdleState = (oc_config->oc_idle_state == BSP_TIM_OC_IDLE_STATE_SET) ?
                TIM_OCIdleState_Set : TIM_OCIdleState_Reset;                  /**< 主输出空闲状态 */
    TIM_OCInitStruct.TIM_OCNIdleState = (oc_config->oc_nidle_state == BSP_TIM_OC_NIDLE_STATE_SET) ?
                TIM_OCNIdleState_Set : TIM_OCNIdleState_Reset;                /**< 互补输出空闲状态 */
    BSP_TIM_OCxInit(hw->timer, oc_config->oc_channel, &TIM_OCInitStruct);

    /*
     * 配置 CCR 是否写入预装载寄存器。
     * 若启用，则修改占空比后在下一次更新事件统一生效，可避免当前周期内立刻跳变导致毛刺。
     */
    BSP_TIM_OCxPreloadConfig(hw->timer, oc_config->oc_channel, TIM_OCPreload_Enable);

    /* 配置 BDTR 参数 */
    TIM_BDTRInitStruct.TIM_OSSIState = (bdtr_config->bdtr_ossi_state == BSP_TIM_BDTR_OSSI_STATE_ENABLE) ?
                TIM_OSSIState_Enable : TIM_OSSIState_Disable;
    TIM_BDTRInitStruct.TIM_OSSRState = (bdtr_config->bdtr_ossr_state == BSP_TIM_BDTR_OSSR_STATE_ENABLE) ?
                TIM_OSSRState_Enable : TIM_OSSRState_Disable;
    TIM_BDTRInitStruct.TIM_Break = (bdtr_config->bdtr_break == BSP_TIM_BDTR_BREAK_ENABLE) ?
                TIM_Break_Enable : TIM_Break_Disable;
    TIM_BDTRInitStruct.TIM_BreakPolarity = (bdtr_config->bdtr_break_polarity == BSP_TIM_BDTR_BREAK_POLARITY_HIGH) ?
                TIM_BreakPolarity_High : TIM_BreakPolarity_Low;
    TIM_BDTRInitStruct.TIM_DeadTime = bdtr_config->bdtr_dead_time;
    TIM_BDTRInitStruct.TIM_LOCKLevel = BSP_TIM_ConvertLockLevel(bdtr_config->bdtr_lock_level);
    TIM_BDTRInitStruct.TIM_AutomaticOutput = (bdtr_config->bdtr_auto_output == BSP_TIM_BDTR_AUTO_OUTPUT_ENABLE) ?
                TIM_AutomaticOutput_Enable : TIM_AutomaticOutput_Disable;
    TIM_BDTRConfig(hw->timer, &TIM_BDTRInitStruct);

    TIM_CtrlPWMOutputs(hw->timer, ENABLE); /**< 使能高级定时器主输出 */

#if 0
    /*
     * 若需要使用更新中断，可打开以下代码：
     * - 清除初始化阶段自动产生的更新标志
     * - 使能更新中断
     * - 配置 NVIC
     */
    TIM_ClearFlag(hw->timer, TIM_IT_Update);
    TIM_ITConfig(hw->timer, TIM_IT_Update, ENABLE);
    BSP_TIM_NVIC_Config(timer_id);
#endif

    TIM_Cmd(hw->timer, ENABLE);
}

/**
 * @brief 对外提供的高级定时器 PWM/互补输出配置接口
 * @param[in] timer_id 高级定时器编号
 * @param[in] base_config 基础配置
 * @param[in] oc_config 输出比较/PWM 配置
 * @param[in] bdtr_config BDTR 配置
 * @return 无返回值
 */
void BSP_AdvancedTimer_Config(bsp_advancedtimer_t timer_id,
            bsp_advancedtimer_config_t *base_config,
            bsp_tim_oc_config_t *oc_config,
            bsp_tim_bdtr_config_t *bdtr_config)
{
    BSP_TIM_GPIO_Init(timer_id, oc_config->oc_channel);
    BSP_AdvancedTIM_Init(timer_id, base_config, oc_config, bdtr_config);
}

/**
 * @brief 高级定时器中断公共处理函数
 * @details
 * 当前主要用于处理更新中断，并对更新次数进行计数。
 *
 * @param[in] timer_id 高级定时器编号
 * @return 无返回值
 *
 * @note
 * 当前主初始化流程中默认未开启更新中断，因此此函数通常不会被触发，
 * 除非用户打开了相关中断配置代码。
 */
void BSP_TIM_IRQHandler(bsp_advancedtimer_t timer_id)
{
    if(timer_id >= BSP_ADVANCED_TIMER_MAX) return;

    const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];

    if(TIM_GetITStatus(hw->timer, TIM_IT_Update) != RESET){
        advancedtimer_cnt[timer_id]++;
        TIM_ClearITPendingBit(hw->timer, TIM_IT_Update);
    }
}