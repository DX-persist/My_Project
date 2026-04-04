#include "bsp_general_timer.h"

/**
 * @file    bsp_general_timer.c
 * @brief   STM32F10x 通用定时器 PWM / 输出比较 BSP 实现文件
 * @details
 * 本文件实现 TIM2 / TIM3 / TIM4 / TIM5 的 PWM 输出与输出比较功能。
 *
 * 核心功能包括：
 * - GPIO 输出初始化
 * - 定时器基础参数配置
 * - OC/PWM 模式配置
 * - CCR 预装载配置
 * - 启动定时器
 *
 * @note
 * 当前实现支持 CH1~CH4 的普通输出，不涉及：
 * - 互补输出
 * - 刹车输入
 * - 死区时间
 * - 主输出使能 MOE
 */

/**
 * @brief 通用定时器硬件资源映射结构体
 */
typedef struct{
    TIM_TypeDef *timer;         /**< 定时器寄存器实例 */
    uint32_t rcc_clk;           /**< RCC 时钟使能位 */

    uint8_t irq_update_channel; /**< 更新中断号 */
    uint8_t irq_pre_prio;       /**< 中断抢占优先级 */
    uint8_t irq_sub_prio;       /**< 中断子优先级 */

    bsp_gpio_t ch1_gpio;        /**< CH1 输出引脚 */
    bsp_gpio_t ch2_gpio;        /**< CH2 输出引脚 */
    bsp_gpio_t ch3_gpio;        /**< CH3 输出引脚 */
    bsp_gpio_t ch4_gpio;        /**< CH4 输出引脚 */
}bsp_generaltimer_hw_t;

/** @brief 通用定时器更新事件计数数组 */
volatile uint16_t generaltimer_cnt[BSP_GENERAL_TIMER_MAX] = {0};

/**
 * @brief 通用定时器硬件资源映射表
 */
static const bsp_generaltimer_hw_t bsp_generaltimer_hw[BSP_GENERAL_TIMER_MAX] = {
    [BSP_GENERAL_TIMER2] = {
        .timer = TIM2,
        .rcc_clk = RCC_APB1Periph_TIM2,

        .irq_update_channel = TIM2_IRQn,
        .irq_pre_prio       = PREEMPT_PRIO,
        .irq_sub_prio       = SUB_PRIO,

        .ch1_gpio = {GPIOA, GPIO_Pin_0, RCC_APB2Periph_GPIOA},
        .ch2_gpio = {GPIOA, GPIO_Pin_1, RCC_APB2Periph_GPIOA},
        .ch3_gpio = {GPIOA, GPIO_Pin_2, RCC_APB2Periph_GPIOA},
        .ch4_gpio = {GPIOA, GPIO_Pin_3, RCC_APB2Periph_GPIOA},
    },

    [BSP_GENERAL_TIMER3] = {
        .timer = TIM3,
        .rcc_clk = RCC_APB1Periph_TIM3,

        .irq_update_channel = TIM3_IRQn,
        .irq_pre_prio       = PREEMPT_PRIO,
        .irq_sub_prio       = SUB_PRIO,

        .ch1_gpio = {GPIOA, GPIO_Pin_6, RCC_APB2Periph_GPIOA},
        .ch2_gpio = {GPIOA, GPIO_Pin_7, RCC_APB2Periph_GPIOA},
        .ch3_gpio = {GPIOB, GPIO_Pin_0, RCC_APB2Periph_GPIOB},
        .ch4_gpio = {GPIOB, GPIO_Pin_1, RCC_APB2Periph_GPIOB},
    },

    [BSP_GENERAL_TIMER4] = {
        .timer = TIM4,
        .rcc_clk = RCC_APB1Periph_TIM4,

        .irq_update_channel = TIM4_IRQn,
        .irq_pre_prio       = PREEMPT_PRIO,
        .irq_sub_prio       = SUB_PRIO,

        .ch1_gpio = {GPIOB, GPIO_Pin_6, RCC_APB2Periph_GPIOB},
        .ch2_gpio = {GPIOB, GPIO_Pin_7, RCC_APB2Periph_GPIOB},
        .ch3_gpio = {GPIOB, GPIO_Pin_8, RCC_APB2Periph_GPIOB},
        .ch4_gpio = {GPIOB, GPIO_Pin_9, RCC_APB2Periph_GPIOB},
    },

    [BSP_GENERAL_TIMER5] = {
        .timer = TIM5,
        .rcc_clk = RCC_APB1Periph_TIM5,

        .irq_update_channel = TIM5_IRQn,
        .irq_pre_prio       = PREEMPT_PRIO,
        .irq_sub_prio       = SUB_PRIO,

        .ch1_gpio = {GPIOA, GPIO_Pin_0, RCC_APB2Periph_GPIOA},
        .ch2_gpio = {GPIOA, GPIO_Pin_1, RCC_APB2Periph_GPIOA},
        .ch3_gpio = {GPIOA, GPIO_Pin_2, RCC_APB2Periph_GPIOA},
        .ch4_gpio = {GPIOA, GPIO_Pin_3, RCC_APB2Periph_GPIOA},
    }
};

/**
 * @brief 将 BSP 计数模式枚举转换为标准库宏
 * @param[in] counter_mode BSP 层计数模式枚举值
 * @return 对应的 TIM_CounterMode_xxx 宏，非法值返回 0
 */
static uint16_t BSP_TIM_ConvertCounterMode(bsp_generaltimer_counter_mode_t counter_mode)
{
    switch(counter_mode){
        case BSP_TIM_COUNTER_MODE_UP: return TIM_CounterMode_Up;
        case BSP_TIM_COUNTER_MODE_DOWN: return TIM_CounterMode_Down;
        case BSP_TIM_COUNTER_MODE_CENTER_ALIGNED1:  return TIM_CounterMode_CenterAligned1;
        case BSP_TIM_COUNTER_MODE_CENTER_ALIGNED2:  return TIM_CounterMode_CenterAligned2;
        case BSP_TIM_COUNTER_MODE_CENTER_ALIGNED3:  return TIM_CounterMode_CenterAligned3;
        default:    return 0;
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
static uint16_t BSP_TIM_ConvertClockDivision(bsp_generaltimer_clock_div_t clock_div)
{
    switch(clock_div){
        case BSP_TIM_CLOCK_DIV_1:   return TIM_CKD_DIV1;
        case BSP_TIM_CLOCK_DIV_2:   return TIM_CKD_DIV2;
        case BSP_TIM_CLOCK_DIV_4:   return TIM_CKD_DIV4;
        default:    return 0;
    }
}

/**
 * @brief 配置通用定时器更新中断对应的 NVIC
 * @param[in] timer_id 通用定时器编号
 * @return 无返回值
 */
static void BSP_TIM_NVIC_Config(bsp_generaltimer_t timer_id)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    const bsp_generaltimer_hw_t *hw = &bsp_generaltimer_hw[timer_id];

    NVIC_InitStruct.NVIC_IRQChannel = hw->irq_update_channel;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = hw->irq_pre_prio;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = hw->irq_sub_prio;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
 * @brief 将 BSP 输出比较模式枚举转换为标准库 OC 模式宏
 * @param[in] oc_mode 输出比较模式
 * @return 对应的 TIM_OCMode_xxx 宏，非法值返回 0
 */
static uint16_t BSP_TIM_ConvertOCMode(bsp_tim_oc_mode_t oc_mode)
{
    switch(oc_mode){
        case BSP_TIM_OC_MODE_TIMING:    return TIM_OCMode_Timing;
        case BSP_TIM_OC_MODE_ACTIVE:    return TIM_OCMode_Active;
        case BSP_TIM_OC_MODE_INACTIVE:  return TIM_OCMode_Inactive;
        case BSP_TIM_OC_MODE_TOGGLE:    return TIM_OCMode_Toggle;
        case BSP_TIM_OC_MODE_PWM1:      return TIM_OCMode_PWM1;
        case BSP_TIM_OC_MODE_PWM2:      return TIM_OCMode_PWM2;
        default:                        return 0;
    }
}

/**
 * @brief 根据通道调用对应的 TIM_OCxInit() 初始化函数
 * @param[in] timer 定时器实例
 * @param[in] oc_init 输出比较初始化结构体
 * @param[in] oc_channel 输出比较通道
 * @return 无返回值
 */
static void BSP_TIM_OCxInit(TIM_TypeDef *timer, TIM_OCInitTypeDef *oc_init, bsp_tim_oc_channel_t oc_channel)
{
    switch(oc_channel){
       case BSP_TIM_OC_CHANNEL1: TIM_OC1Init(timer, oc_init); break;
       case BSP_TIM_OC_CHANNEL2: TIM_OC2Init(timer, oc_init); break;
       case BSP_TIM_OC_CHANNEL3: TIM_OC3Init(timer, oc_init); break;
       case BSP_TIM_OC_CHANNEL4: TIM_OC4Init(timer, oc_init); break;
       default:                                               break;
    }
}

/**
 * @brief 根据通道配置 CCR 预装载功能
 * @param[in] timer 定时器实例
 * @param[in] state 预装载状态
 * @param[in] oc_channel 输出比较通道
 * @return 无返回值
 *
 * @note
 * 使能 CCR 预装载后，占空比更新会在下一次更新事件统一生效，
 * 可避免当前周期立即跳变导致波形毛刺。
 */
static void BSP_TIM_OCxPreloadConfig(TIM_TypeDef *timer, uint16_t state, bsp_tim_oc_channel_t oc_channel)
{
    switch(oc_channel){
       case BSP_TIM_OC_CHANNEL1: TIM_OC1PreloadConfig(timer, state); break;
       case BSP_TIM_OC_CHANNEL2: TIM_OC2PreloadConfig(timer, state); break;
       case BSP_TIM_OC_CHANNEL3: TIM_OC3PreloadConfig(timer, state); break;
       case BSP_TIM_OC_CHANNEL4: TIM_OC4PreloadConfig(timer, state); break;
       default:                                                      break;
    }
}

/**
 * @brief 根据输出通道获取对应 GPIO 描述信息
 * @param[in] hw 通用定时器硬件资源结构体指针
 * @param[in] oc_channel 输出比较通道
 * @return 成功返回 GPIO 描述结构体指针，失败返回 NULL
 */
static const bsp_gpio_t *BSP_GetGPIO(const bsp_generaltimer_hw_t *hw, bsp_tim_oc_channel_t oc_channel)
{
    const bsp_gpio_t *gpio = NULL;

    switch(oc_channel){
        case BSP_TIM_OC_CHANNEL1: gpio = &hw->ch1_gpio; break;
        case BSP_TIM_OC_CHANNEL2: gpio = &hw->ch2_gpio; break;
        case BSP_TIM_OC_CHANNEL3: gpio = &hw->ch3_gpio; break;
        case BSP_TIM_OC_CHANNEL4: gpio = &hw->ch4_gpio; break;
        default:                                        break;
    }
    return gpio;
}

/**
 * @brief 初始化 PWM/输出比较通道对应的 GPIO
 * @param[in] timer_id 通用定时器编号
 * @param[in] oc_channel 输出比较通道
 * @return 无返回值
 *
 * @note
 * 当前配置为复用推挽输出模式，适用于 PWM 波形输出。
 */
static void BSP_General_GPIO_Init(bsp_generaltimer_t timer_id, bsp_tim_oc_channel_t oc_channel)
{
    if(timer_id >= BSP_GENERAL_TIMER_MAX)   return;

    const bsp_generaltimer_hw_t *hw = &bsp_generaltimer_hw[timer_id];
    const bsp_gpio_t *gpio = BSP_GetGPIO(hw, oc_channel);
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);

    RCC_APB2PeriphClockCmd(gpio->rcc_clk, ENABLE);

    GPIO_InitStruct.GPIO_Pin = gpio->gpio_pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(gpio->gpio_port, &GPIO_InitStruct);
}

/**
 * @brief 初始化通用定时器 PWM/输出比较功能
 * @details
 * 本函数完成：
 * - 定时器基础时基配置
 * - OC/PWM 参数配置
 * - CCR 预装载配置
 * - 启动定时器
 *
 * @param[in] timer_id 通用定时器编号
 * @param[in] base_config 定时器基础时基配置
 * @param[in] oc_config 输出比较/PWM 配置
 * @return 无返回值
 *
 * @note
 * 当前实现虽然保留了部分高级定时器风格字段，例如 OutputNState，
 * 但对普通定时器而言，这些字段通常没有实际输出意义。
 */
static void BSP_GeneralTIM_Init(bsp_generaltimer_t timer_id,
        bsp_generaltimer_config_t *base_config,
        bsp_tim_oc_config_t *oc_config)
{
    if(timer_id >= BSP_GENERAL_TIMER_MAX)  return;

    const bsp_generaltimer_hw_t *hw = &bsp_generaltimer_hw[timer_id];
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_OCStructInit(&TIM_OCInitStruct);

    RCC_APB1PeriphClockCmd(hw->rcc_clk, ENABLE);    /**< 开启 TIMx 时钟 */

    /* 初始化 TIM_BaseInitTypeDef 结构体 */
    TIM_TimeBaseInitStruct.TIM_Prescaler = base_config->prescaler;
    TIM_TimeBaseInitStruct.TIM_CounterMode = BSP_TIM_ConvertCounterMode(base_config->counter_mode);
    TIM_TimeBaseInitStruct.TIM_Period = base_config->period;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = BSP_TIM_ConvertClockDivision(base_config->clock_div);
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = base_config->repetition_cnt;
    TIM_TimeBaseInit(hw->timer, &TIM_TimeBaseInitStruct);

    /* 启用 PSC 和 ARR 预装载功能，在下一更新周期同步数据到寄存器中 */
    TIM_PrescalerConfig(hw->timer, base_config->prescaler, TIM_PSCReloadMode_Update);
    TIM_ARRPreloadConfig(hw->timer, ENABLE);

    /* 配置 TIM_OCInitTypeDef 结构体 */
    TIM_OCInitStruct.TIM_OCMode = BSP_TIM_ConvertOCMode(oc_config->oc_mode);
    TIM_OCInitStruct.TIM_OutputState = (oc_config->oc_output_state == BSP_TIM_OUTPUT_STATE_ENABLE) ? TIM_OutputState_Enable : TIM_OutputState_Disable;
    TIM_OCInitStruct.TIM_OutputNState = (oc_config->oc_output_nstate == BSP_TIM_OUTPUT_NSTATE_ENABLE) ? TIM_OutputNState_Enable: TIM_OutputNState_Disable;
    TIM_OCInitStruct.TIM_Pulse = oc_config->oc_ccr_value;
    TIM_OCInitStruct.TIM_OCPolarity = (oc_config->oc_polarity == BSP_TIM_OC_POLARITY_HIGH) ? TIM_OCPolarity_High : TIM_OCPolarity_Low;
    TIM_OCInitStruct.TIM_OCNPolarity = (oc_config->oc_npolarity == BSP_TIM_OC_NPOLARITY_HIGH) ? TIM_OCNPolarity_High : TIM_OCNPolarity_Low;
    TIM_OCInitStruct.TIM_OCIdleState = (oc_config->oc_idle_state == BSP_TIM_OC_IDLE_STATE_SET) ? TIM_OCIdleState_Set : TIM_OCIdleState_Reset;
    TIM_OCInitStruct.TIM_OCNIdleState = (oc_config->oc_nidle_state == BSP_TIM_OC_NIDLE_STATE_SET) ? TIM_OCNIdleState_Set : TIM_OCNIdleState_Reset;
    BSP_TIM_OCxInit(hw->timer, &TIM_OCInitStruct, oc_config->oc_channel);

    BSP_TIM_OCxPreloadConfig(hw->timer, TIM_OCPreload_Enable, oc_config->oc_channel); /**< 启用 CCR 预装载功能 */

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
 * @brief 对外提供的通用定时器 PWM/输出比较配置接口
 * @param[in] timer_id 通用定时器编号
 * @param[in] base_config 基础配置
 * @param[in] oc_config 输出比较/PWM 配置
 * @return 无返回值
 */
void BSP_GeneralTIM_Config(bsp_generaltimer_t timer_id, bsp_generaltimer_config_t *base_config, bsp_tim_oc_config_t *oc_config)
{
    BSP_General_GPIO_Init(timer_id, oc_config->oc_channel);
    BSP_GeneralTIM_Init(timer_id, base_config, oc_config);
}

/**
 * @brief 通用定时器中断公共处理函数
 * @details
 * 当前主要用于处理更新中断，并对更新次数进行计数。
 *
 * @param[in] timer_id 通用定时器编号
 * @return 无返回值
 *
 * @note
 * 当前主初始化流程默认未开启更新中断，因此此函数通常不会被触发，
 * 除非用户手动打开了相关中断配置代码。
 */
void BSP_TIM_IRQHandler(bsp_generaltimer_t timer_id)
{
    if(timer_id >= BSP_GENERAL_TIMER_MAX)  return;

    const bsp_generaltimer_hw_t *hw = &bsp_generaltimer_hw[timer_id];

    if(TIM_GetITStatus(hw->timer, TIM_IT_Update) != RESET){
        generaltimer_cnt[timer_id]++;
        TIM_ClearITPendingBit(hw->timer, TIM_IT_Update);
    }
}