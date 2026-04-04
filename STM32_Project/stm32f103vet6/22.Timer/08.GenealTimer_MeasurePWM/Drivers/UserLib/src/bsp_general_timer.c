#include "bsp_general_timer.h"

/**
 * @file    bsp_general_timer.c
 * @brief   STM32F10x 通用定时器 PWM 输入测量 BSP 实现文件
 * @details
 * 本文件实现 TIM2、TIM3、TIM4、TIM5 的 PWM 输入测量功能。
 *
 * 核心功能包括：
 * - GPIO 输入初始化
 * - 通用定时器基础参数配置
 * - PWM Input 模式初始化
 * - 中断优先级配置
 * - 捕获比较中断中计算 PWM 周期、脉宽、频率和占空比
 *
 * @note
 * PWM Input 模式会同时利用两个 CCR 寄存器协同工作：
 * - 一个记录周期
 * - 一个记录高电平时间
 * 因此在不同输入源通道下，CCR1 / CCR2 的含义会互换。
 */

/**
 * @brief 通用定时器硬件资源描述结构体
 */
typedef struct{
    TIM_TypeDef *timer;       /**< 定时器寄存器实例 */
    uint32_t rcc_clk;         /**< RCC 时钟使能位 */

    uint8_t irq_update_channel; /**< 定时器中断号 */
    uint8_t irq_pre_prio;       /**< 中断抢占优先级 */
    uint8_t irq_sub_prio;       /**< 中断子优先级 */

    bsp_gpio_t ch1_gpio;      /**< 通道 1 对应 GPIO */
    bsp_gpio_t ch2_gpio;      /**< 通道 2 对应 GPIO */
}bsp_generaltimer_hw_t;

/** @brief PWM 测量结果全局数组 */
bsp_tim_ic_result_t result[BSP_GENERAL_TIMER_MAX] = {0};

/**
 * @brief 记录每个定时器当前所选的 PWM 输入源通道
 * @details
 * 中断处理函数需要依赖该数组来判断：
 * - 当前周期值存放在哪个 CCR
 * - 当前脉宽值存放在哪个 CCR
 */
static bsp_tim_pwm_input_channel_t s_pwm_input_channel[BSP_GENERAL_TIMER_MAX];

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
    },

    [BSP_GENERAL_TIMER3] = {
        .timer = TIM3,
        .rcc_clk = RCC_APB1Periph_TIM3,

        .irq_update_channel = TIM3_IRQn,
        .irq_pre_prio       = PREEMPT_PRIO,
        .irq_sub_prio       = SUB_PRIO,

        .ch1_gpio = {GPIOA, GPIO_Pin_6, RCC_APB2Periph_GPIOA},
        .ch2_gpio = {GPIOA, GPIO_Pin_7, RCC_APB2Periph_GPIOA},
    },

    [BSP_GENERAL_TIMER4] = {
        .timer = TIM4,
        .rcc_clk = RCC_APB1Periph_TIM4,

        .irq_update_channel = TIM4_IRQn,
        .irq_pre_prio       = PREEMPT_PRIO,
        .irq_sub_prio       = SUB_PRIO,

        .ch1_gpio = {GPIOB, GPIO_Pin_6, RCC_APB2Periph_GPIOB},
        .ch2_gpio = {GPIOB, GPIO_Pin_7, RCC_APB2Periph_GPIOB},
    },

    [BSP_GENERAL_TIMER5] = {
        .timer = TIM5,
        .rcc_clk = RCC_APB1Periph_TIM5,

        .irq_update_channel = TIM5_IRQn,
        .irq_pre_prio       = PREEMPT_PRIO,
        .irq_sub_prio       = SUB_PRIO,

        .ch1_gpio = {GPIOA, GPIO_Pin_0, RCC_APB2Periph_GPIOA},
        .ch2_gpio = {GPIOA, GPIO_Pin_1, RCC_APB2Periph_GPIOA},
    }
};

/**
 * @brief 将 BSP 层计数模式枚举转换为标准库宏定义
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
 * @brief 将 BSP 层时钟分频枚举转换为标准库 CKD 宏
 * @param[in] clock_div BSP 层时钟分频枚举值
 * @return 对应的 TIM_CKD_DIVx 宏，非法值返回 0
 *
 * @note
 * CKD 不等于 PSC，前者不是直接控制计数频率的预分频器。
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
 * @brief 配置定时器中断对应的 NVIC 参数
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
 * @brief 将 PWM 输入源通道枚举转换为标准库通道宏
 * @param[in] ic_channel PWM 输入通道
 * @return TIM_Channel_1 / TIM_Channel_2，非法值返回 0
 */
static uint16_t BSP_TIM_ConvertICChannel(bsp_tim_pwm_input_channel_t ic_channel)
{
    switch(ic_channel){
        case BSP_TIM_PWM_INPUT_CHANNEL1: return TIM_Channel_1;
        case BSP_TIM_PWM_INPUT_CHANNEL2: return TIM_Channel_2;
        default:                  return 0;
    }
}

/**
 * @brief 将 PWM 输入通道转换为对应的捕获比较中断位
 * @param[in] ic_channel PWM 输入通道
 * @return TIM_IT_CC1 / TIM_IT_CC2，非法值返回 0
 */
static uint16_t BSP_TIM_ConvertICITFlag(bsp_tim_pwm_input_channel_t ic_channel)
{
    switch(ic_channel){
        case BSP_TIM_PWM_INPUT_CHANNEL1: return TIM_IT_CC1;
        case BSP_TIM_PWM_INPUT_CHANNEL2: return TIM_IT_CC2;
        default:                  return 0;
    }
}

/**
 * @brief 将 BSP 输入捕获极性枚举转换为标准库极性宏
 * @param[in] ic_polarity 输入捕获极性
 * @return 标准库 TIM_ICPolarity_xxx 宏，非法值返回 0
 */
static uint16_t BSP_TIM_ConvertICPolarity(bsp_tim_ic_polarity_t ic_polarity)
{
    switch(ic_polarity){
        case BSP_TIM_IC_POLARITY_RISING: return TIM_ICPolarity_Rising;
        case BSP_TIM_IC_POLARITY_FALLING: return TIM_ICPolarity_Falling;
        case BSP_TIM_IC_POLARITY_BOTHEDGE: return TIM_ICPolarity_BothEdge;
        default:    return 0;
    }
}

/**
 * @brief 将 BSP 输入捕获预分频枚举转换为标准库宏
 * @param[in] ic_prescaler_div 输入捕获预分频枚举值
 * @return 对应的 TIM_ICPSC_DIVx 宏，非法值返回 0
 */
static uint16_t BSP_TIM_ConvertICPrescaler(bsp_tim_ic_prescaler_t ic_prescaler_div)
{
    switch(ic_prescaler_div){
        case BSP_TIM_IC_PRESCALER_DIV1: return TIM_ICPSC_DIV1;
        case BSP_TIM_IC_PRESCALER_DIV2: return TIM_ICPSC_DIV2;
        case BSP_TIM_IC_PRESCALER_DIV4: return TIM_ICPSC_DIV4;
        case BSP_TIM_IC_PRESCALER_DIV8: return TIM_ICPSC_DIV8;
        default:    return 0;
    }
}

/**
 * @brief 根据输入源通道获取对应的 GPIO 描述信息
 * @param[in] hw 通用定时器硬件资源指针
 * @param[in] ic_channel PWM 输入通道
 * @return 对应 GPIO 描述结构体指针；若参数非法则返回 NULL
 */
static const bsp_gpio_t *BSP_GetGPIO(const bsp_generaltimer_hw_t *hw, bsp_tim_pwm_input_channel_t ic_channel)
{
    const bsp_gpio_t *gpio = NULL;

    switch(ic_channel){
        case BSP_TIM_PWM_INPUT_CHANNEL1: gpio = &hw->ch1_gpio; break;
        case BSP_TIM_PWM_INPUT_CHANNEL2: gpio = &hw->ch2_gpio; break;
        default:                                        break;
    }
    return gpio;
}

/**
 * @brief 初始化 PWM 输入通道对应的 GPIO
 * @details
 * 将定时器输入引脚配置为浮空输入模式。
 *
 * @param[in] timer_id 通用定时器编号
 * @param[in] ic_channel PWM 输入源通道
 * @return 无返回值
 *
 * @note
 * 若输入信号源为开漏输出或线路较长，可根据实际硬件情况改为上拉/下拉输入模式。
 */
static void BSP_General_GPIO_Init(bsp_generaltimer_t timer_id, bsp_tim_pwm_input_channel_t ic_channel)
{
    if(timer_id >= BSP_GENERAL_TIMER_MAX)   return;

    const bsp_generaltimer_hw_t *hw = &bsp_generaltimer_hw[timer_id];
    const bsp_gpio_t *gpio = BSP_GetGPIO(hw, ic_channel);
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);

    RCC_APB2PeriphClockCmd(gpio->rcc_clk, ENABLE);

    GPIO_InitStruct.GPIO_Pin = gpio->gpio_pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(gpio->gpio_port, &GPIO_InitStruct);
}

/**
 * @brief 初始化通用定时器 PWM 输入测量功能
 * @details
 * 本函数完成：
 * - 定时器基础参数配置
 * - PWM Input 模式配置
 * - 触发源选择
 * - 从模式复位配置
 * - 中断使能与 NVIC 配置
 *
 * @param[in] timer_id 通用定时器编号
 * @param[in] base_config 定时器基础配置
 * @param[in] pwm_config PWM 输入配置
 * @return 无返回值
 *
 * @note
 * 在 PWM Input 模式下：
 * - 选择 CH1 输入时，通常 CCR1 保存周期，CCR2 保存高电平时间
 * - 选择 CH2 输入时，通常 CCR2 保存周期，CCR1 保存高电平时间
 */
static void BSP_GeneralTIM_Init(bsp_generaltimer_t timer_id,
        bsp_generaltimer_config_t *base_config,
        bsp_tim_pwm_input_config_t *pwm_config)
{
    if(timer_id >= BSP_GENERAL_TIMER_MAX)  return;

    const bsp_generaltimer_hw_t *hw = &bsp_generaltimer_hw[timer_id];
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_ICInitTypeDef TIM_ICInitStruct;
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_ICStructInit(&TIM_ICInitStruct);

    RCC_APB1PeriphClockCmd(hw->rcc_clk, ENABLE);    /**< 开启 TIMx 时钟 */

    /* 配置定时器基础参数 */
    TIM_TimeBaseInitStruct.TIM_Prescaler = base_config->prescaler;
    TIM_TimeBaseInitStruct.TIM_CounterMode = BSP_TIM_ConvertCounterMode(base_config->counter_mode);
    TIM_TimeBaseInitStruct.TIM_Period = base_config->period;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = BSP_TIM_ConvertClockDivision(base_config->clock_div);
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = base_config->repetition_cnt;
    TIM_TimeBaseInit(hw->timer, &TIM_TimeBaseInitStruct);

    /* 开启 PSC 和 ARR 预装载，在更新事件时同步到实际寄存器 */
    TIM_PrescalerConfig(hw->timer, base_config->prescaler, TIM_PSCReloadMode_Update);
    TIM_ARRPreloadConfig(hw->timer, ENABLE);

    /* 保存当前 PWM 输入源通道，供中断处理函数使用 */
    s_pwm_input_channel[timer_id] = pwm_config->input_channel;

    /* 配置输入捕获参数并切换到 PWM 输入模式 */
    TIM_ICInitStruct.TIM_Channel = BSP_TIM_ConvertICChannel(pwm_config->input_channel);
    TIM_ICInitStruct.TIM_ICPolarity = BSP_TIM_ConvertICPolarity(pwm_config->ic_polarity);
    TIM_ICInitStruct.TIM_ICPrescaler = BSP_TIM_ConvertICPrescaler(pwm_config->ic_prescaler);
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStruct.TIM_ICFilter = pwm_config->ic_filter;
    TIM_PWMIConfig(hw->timer, &TIM_ICInitStruct);

    /**
     * @note
     * 触发源必须与输入源通道保持一致，否则 PWM 周期捕获结果会异常。
     */
    if(pwm_config->input_channel == BSP_TIM_PWM_INPUT_CHANNEL1){
        TIM_SelectInputTrigger(hw->timer, TIM_TS_TI1FP1);
    }else if(pwm_config->input_channel == BSP_TIM_PWM_INPUT_CHANNEL2){
        TIM_SelectInputTrigger(hw->timer, TIM_TS_TI2FP2);
    }

    /**
     * @note
     * 设置从模式为 Reset 后，一旦检测到配置的触发边沿，CNT 自动清零。
     * 这样一个 PWM 周期的长度就能直接通过捕获值表示出来。
     */
    TIM_SelectSlaveMode(hw->timer, TIM_SlaveMode_Reset);

    TIM_ClearFlag(hw->timer, BSP_TIM_ConvertICITFlag(pwm_config->input_channel));

    /* 配置捕获比较中断，并设置 NVIC */
    TIM_ITConfig(hw->timer, BSP_TIM_ConvertICITFlag(pwm_config->input_channel), ENABLE);
    BSP_TIM_NVIC_Config(timer_id);

    /* 启动定时器 */
    TIM_Cmd(hw->timer, ENABLE);
}

/**
 * @brief 对外提供的通用定时器 PWM 输入配置接口
 * @param[in] timer_id 通用定时器编号
 * @param[in] base_config 定时器基础参数配置
 * @param[in] pwm_config PWM 输入测量参数配置
 * @return 无返回值
 *
 * @details
 * 按顺序完成 GPIO 初始化和定时器初始化。
 */
void BSP_GeneralTIM_Config(bsp_generaltimer_t timer_id, bsp_generaltimer_config_t *base_config, bsp_tim_pwm_input_config_t *pwm_config)
{
    BSP_General_GPIO_Init(timer_id, pwm_config->input_channel);
    BSP_GeneralTIM_Init(timer_id, base_config, pwm_config);
}

/**
 * @brief 通用定时器中断公共处理函数
 * @details
 * 当对应的 CC 中断触发后，本函数负责：
 * - 读取 CCR1/CCR2
 * - 计算周期值与高电平脉宽值
 * - 计算占空比与频率
 * - 更新结果标志
 * - 清除中断挂起位
 *
 * @param[in] timer_id 触发中断的通用定时器编号
 * @return 无返回值
 *
 * @note
 * 该函数名称虽然是 `BSP_TIM_IRQHandler`，但当前逻辑处理的是 PWM 输入测量相关的 CC 中断。
 */
void BSP_TIM_IRQHandler(bsp_generaltimer_t timer_id)
{
    if(timer_id >= BSP_GENERAL_TIMER_MAX)  return;

    const bsp_generaltimer_hw_t *hw = &bsp_generaltimer_hw[timer_id];
    bsp_tim_pwm_input_channel_t pwm_channel = s_pwm_input_channel[timer_id];
    uint16_t it_flag = BSP_TIM_ConvertICITFlag(pwm_channel);
    bsp_tim_ic_result_t *res = &result[timer_id];
    uint32_t timer_freq = CLK_FREQ / (hw->timer->PSC + 1);

    if(TIM_GetITStatus(hw->timer, it_flag) != RESET){
        if(pwm_channel == BSP_TIM_PWM_INPUT_CHANNEL1){
            res->g_pwm_period      = TIM_GetCapture1(hw->timer);  /**< CCR1 保存周期 */
            res->g_pwm_pulse_width = TIM_GetCapture2(hw->timer);  /**< CCR2 保存高电平时间 */
        }else{
            res->g_pwm_period      = TIM_GetCapture2(hw->timer);  /**< CCR2 保存周期 */
            res->g_pwm_pulse_width = TIM_GetCapture1(hw->timer);  /**< CCR1 保存高电平时间 */
        }

        if(res->g_pwm_period != 0){
            res->g_pwm_duty = (float)res->g_pwm_pulse_width / res->g_pwm_period * 100.0f;
            res->g_pwm_freq = timer_freq / res->g_pwm_period;
        }else{
            /**
             * @note
             * 理论上周期值不应为 0。这里加入保护逻辑，避免除 0。
             */
            res->g_pwm_duty = 0.0f;
            res->g_pwm_freq = 0;
        }

        res->g_pwm_update = 1;
        TIM_ClearITPendingBit(hw->timer, it_flag);
    }
}
