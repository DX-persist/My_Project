#include "bsp_advanced_timer.h"

/**
 * @file    bsp_advanced_timer.c
 * @brief   STM32F10x 高级定时器输入捕获测量脉宽 BSP 实现文件
 * @details
 * 本文件实现高级定时器 TIM1 / TIM8 的输入捕获脉宽测量功能。
 *
 * 实现思路如下：
 * - 先配置某一路输入捕获为上升沿触发
 * - 在第一次捕获到上升沿时记录计数值
 * - 随后切换该通道的输入捕获极性为下降沿
 * - 在下降沿到来时再次捕获计数值
 * - 若中间发生溢出，则通过更新中断累计溢出次数
 * - 最终计算高电平脉宽
 *
 * @note
 * 本模块适合“测量高电平脉宽”场景，不直接输出频率或占空比。
 */

/**
 * @brief 高级定时器硬件资源映射结构体
 */
typedef struct{
    TIM_TypeDef *timer;        /**< 定时器寄存器实例 */
    uint32_t rcc_clk;          /**< RCC 时钟使能位 */

    uint8_t irq_update_channel; /**< 更新中断号 */
    uint8_t irq_cc_channel;     /**< 捕获比较中断号 */
    uint8_t irq_up_pre_prio;    /**< 更新中断抢占优先级 */
    uint8_t irq_up_sub_prio;    /**< 更新中断子优先级 */
    uint8_t irq_cc_pre_prio;    /**< 捕获比较中断抢占优先级 */
    uint8_t irq_cc_sub_prio;    /**< 捕获比较中断子优先级 */

    bsp_gpio_t ch1_gpio;       /**< CH1 对应 GPIO */
    bsp_gpio_t ch2_gpio;       /**< CH2 对应 GPIO */
    bsp_gpio_t ch3_gpio;       /**< CH3 对应 GPIO */
    bsp_gpio_t ch4_gpio;       /**< CH4 对应 GPIO */
}bsp_advancedtimer_hw_t;

/** @brief 高级定时器辅助计数数组 */
volatile uint16_t advancedtimer_cnt[BSP_ADVANCED_TIMER_MAX] = {0};

/**
 * @brief 保存每个高级定时器当前使用的输入捕获通道
 * @note
 * 该数组按定时器编号索引。
 */
static bsp_tim_ic_channel_t s_ic_channel[BSP_ADVANCED_TIMER_MAX];

/**
 * @brief 高级定时器硬件资源映射表
 */
static const bsp_advancedtimer_hw_t bsp_advancedtimer_hw[BSP_ADVANCED_TIMER_MAX] = {
    [BSP_ADVANCED_TIMER1] = {
        .timer = TIM1,
        .rcc_clk = RCC_APB2Periph_TIM1,

        .irq_update_channel = TIM1_UP_IRQn,
        .irq_cc_channel     = TIM1_CC_IRQn,
        .irq_up_pre_prio    = UP_PREEMPT_PRIO,
        .irq_up_sub_prio    = UP_SUB_PRIO,
        .irq_cc_pre_prio    = CC_PREEPT_PRIO,
        .irq_cc_sub_prio    = CC_SUB_PRIO,

        .ch1_gpio = {GPIOA, GPIO_Pin_8, RCC_APB2Periph_GPIOA},
        .ch2_gpio = {GPIOA, GPIO_Pin_9, RCC_APB2Periph_GPIOA},
        .ch3_gpio = {GPIOA, GPIO_Pin_10, RCC_APB2Periph_GPIOA},
        .ch4_gpio = {GPIOA, GPIO_Pin_11, RCC_APB2Periph_GPIOA},
    },

    [BSP_ADVANCED_TIMER8] = {
        .timer = TIM8,
        .rcc_clk = RCC_APB2Periph_TIM8,

        .irq_update_channel = TIM8_UP_IRQn,
        .irq_cc_channel = TIM8_CC_IRQn,
        .irq_up_pre_prio = UP_PREEMPT_PRIO,
        .irq_up_sub_prio = UP_SUB_PRIO,
        .irq_cc_pre_prio = CC_PREEPT_PRIO,
        .irq_cc_sub_prio = CC_SUB_PRIO,

        .ch1_gpio = {GPIOC, GPIO_Pin_6, RCC_APB2Periph_GPIOC},
        .ch2_gpio = {GPIOC, GPIO_Pin_7, RCC_APB2Periph_GPIOC},
        .ch3_gpio = {GPIOC, GPIO_Pin_8, RCC_APB2Periph_GPIOC},
        .ch4_gpio = {GPIOC, GPIO_Pin_9, RCC_APB2Periph_GPIOC},
    }
};

/**
 * @brief 根据输入捕获通道获取对应 GPIO 描述信息
 * @param[in] hw 高级定时器硬件资源结构体指针
 * @param[in] ic_channel 输入捕获通道
 * @return 成功返回 GPIO 描述结构体指针，失败返回 NULL
 */
static const bsp_gpio_t *BSP_TIM_GetCHGPIO(const bsp_advancedtimer_hw_t *hw, bsp_tim_ic_channel_t ic_channel)
{
    switch(ic_channel){
        case BSP_TIM_IC_CHANNEL1: return &hw->ch1_gpio;
        case BSP_TIM_IC_CHANNEL2: return &hw->ch2_gpio;
        case BSP_TIM_IC_CHANNEL3: return &hw->ch3_gpio;
        case BSP_TIM_IC_CHANNEL4: return &hw->ch4_gpio;
        default: return NULL;
    }
}

/**
 * @brief 初始化输入捕获通道对应的 GPIO
 * @param[in] timer_id 高级定时器编号
 * @param[in] ic_channel 输入捕获通道
 * @return 无返回值
 *
 * @note
 * 当前配置为浮空输入模式。
 */
static void BSP_TIM_GPIO_Init(bsp_advancedtimer_t timer_id, bsp_tim_ic_channel_t ic_channel)
{
    if(timer_id >= BSP_ADVANCED_TIMER_MAX) return;

    const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];
    const bsp_gpio_t *ch_gpio = BSP_TIM_GetCHGPIO(hw, ic_channel);
    uint32_t enable_clk = ch_gpio->rcc_clk;
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(enable_clk, ENABLE);

    GPIO_InitStruct.GPIO_Pin = ch_gpio->gpio_pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(ch_gpio->gpio_port, &GPIO_InitStruct);
}

/**
 * @brief 将 BSP 计数模式枚举转换为标准库宏
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
 * @brief 将 BSP 输入捕获通道枚举转换为标准库通道宏
 */
static uint16_t BSP_TIM_ConvertICChannel(bsp_tim_ic_channel_t ic_channel)
{
    switch(ic_channel){
        case BSP_TIM_IC_CHANNEL1: return TIM_Channel_1;
        case BSP_TIM_IC_CHANNEL2: return TIM_Channel_2;
        case BSP_TIM_IC_CHANNEL3: return TIM_Channel_3;
        case BSP_TIM_IC_CHANNEL4: return TIM_Channel_4;
        default: return 0;
    }
}

/**
 * @brief 将输入捕获通道转换为对应捕获比较中断位
 */
static uint16_t BSP_TIM_ConvertICITFlag(bsp_tim_ic_channel_t ic_channel)
{
    switch(ic_channel){
        case BSP_TIM_IC_CHANNEL1: return TIM_IT_CC1;
        case BSP_TIM_IC_CHANNEL2: return TIM_IT_CC2;
        case BSP_TIM_IC_CHANNEL3: return TIM_IT_CC3;
        case BSP_TIM_IC_CHANNEL4: return TIM_IT_CC4;
        default: return 0;
    }
}

/**
 * @brief 将 BSP 输入捕获极性枚举转换为标准库极性宏
 */
static uint16_t BSP_TIM_ConvertICPolarity(bsp_tim_ic_polarity_t ic_polarity)
{
    switch(ic_polarity){
        case BSP_TIM_IC_POLARITY_RISING: return TIM_ICPolarity_Rising;
        case BSP_TIM_IC_POLARITY_FALLING: return TIM_ICPolarity_Falling;
        case BSP_TIM_IC_POLARITY_BOTHEDGE: return TIM_ICPolarity_BothEdge;
        default: return 0;
    }
}

/**
 * @brief 将 BSP 输入捕获预分频枚举转换为标准库宏
 */
static uint16_t BSP_TIM_ConvertICPrescaler(bsp_tim_ic_prescaler_t ic_prescaler_div)
{
    switch(ic_prescaler_div){
        case BSP_TIM_IC_PRESCALER_DIV1: return TIM_ICPSC_DIV1;
        case BSP_TIM_IC_PRESCALER_DIV2: return TIM_ICPSC_DIV2;
        case BSP_TIM_IC_PRESCALER_DIV4: return TIM_ICPSC_DIV4;
        case BSP_TIM_IC_PRESCALER_DIV8: return TIM_ICPSC_DIV8;
        default: return 0;
    }
}

/**
 * @brief 将 BSP 输入捕获映射枚举转换为标准库宏
 */
static uint16_t BSP_TIM_ConvertICSelection(bsp_tim_ic_selection_t ic_selection)
{
    switch(ic_selection){
        case BSP_TIM_IC_SELECTION_DIRECTTI: return TIM_ICSelection_DirectTI;
        case BSP_TIM_IC_SELECTION_INDIRECTTI: return TIM_ICSelection_IndirectTI;
        case BSP_TIM_IC_SELECTION_TRC: return TIM_ICSelection_TRC;
        default: return 0;
    }
}

/**
 * @brief 根据通道读取对应的捕获寄存器值
 * @param[in] TIMx 定时器寄存器实例
 * @param[in] ic_channel 输入捕获通道
 * @return 当前捕获寄存器值
 */
static uint16_t BSP_TIM_GetCapturex(TIM_TypeDef* TIMx, bsp_tim_ic_channel_t ic_channel)
{
    switch(ic_channel){
        case BSP_TIM_IC_CHANNEL1: return TIM_GetCapture1(TIMx);
        case BSP_TIM_IC_CHANNEL2: return TIM_GetCapture2(TIMx);
        case BSP_TIM_IC_CHANNEL3: return TIM_GetCapture3(TIMx);
        case BSP_TIM_IC_CHANNEL4: return TIM_GetCapture4(TIMx);
        default: return 0;
    }
}

/**
 * @brief 根据通道动态修改输出比较极性配置
 * @details
 * 这里实际用于切换捕获极性，以实现“先测上升沿，再测下降沿”。
 *
 * @param[in] timer 定时器实例
 * @param[in] polarity 新极性
 * @param[in] ic_channel 输入捕获通道
 * @return 无返回值
 *
 * @note
 * 虽然函数名里是 OCxPolarityConfig，但在标准库中该接口同样用于对应通道极性切换。
 * 这里是一个容易让人误解的点。
 */
static void BSP_TIM_OCxPolarityConfig(TIM_TypeDef *timer, uint16_t polarity, bsp_tim_ic_channel_t ic_channel)
{
    switch(ic_channel){
        case BSP_TIM_IC_CHANNEL1: TIM_OC1PolarityConfig(timer, polarity); break;
        case BSP_TIM_IC_CHANNEL2: TIM_OC2PolarityConfig(timer, polarity); break;
        case BSP_TIM_IC_CHANNEL3: TIM_OC3PolarityConfig(timer, polarity); break;
        case BSP_TIM_IC_CHANNEL4: TIM_OC4PolarityConfig(timer, polarity); break;
        default: break;
    }
}

/**
 * @brief 配置高级定时器更新中断和捕获比较中断的 NVIC
 * @param[in] timer_id 高级定时器编号
 * @return 无返回值
 */
static void BSP_TIM_NVIC_Config(bsp_advancedtimer_t timer_id)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];

    NVIC_InitStruct.NVIC_IRQChannel = hw->irq_update_channel;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = hw->irq_up_pre_prio;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = hw->irq_up_sub_prio;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = hw->irq_cc_channel;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = hw->irq_cc_pre_prio;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = hw->irq_cc_sub_prio;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
 * @brief 初始化高级定时器输入捕获测量功能
 * @details
 * 本函数完成：
 * - 基础计数参数初始化
 * - 输入捕获初始化
 * - 清除初始更新标志和捕获标志
 * - 使能更新中断和捕获中断
 * - 启动定时器
 *
 * @param[in] timer_id 高级定时器编号
 * @param[in] base_config 基础配置
 * @param[in] ic_config 输入捕获配置
 * @return 无返回值
 *
 * @note
 * PSC 和 ARR 开启预装载后，初始化阶段通常会自动触发一次更新事件，
 * 所以这里需要手动清除更新中断标志，避免误进入中断。
 */
static void BSP_AdvancedTIM_Init(bsp_advancedtimer_t timer_id,
            bsp_advancedtimer_config_t *base_config,
            bsp_tim_ic_config_t *ic_config)
{
    if(timer_id >= BSP_ADVANCED_TIMER_MAX) return;

    const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_ICInitTypeDef TIM_ICInitStruct;
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_ICStructInit(&TIM_ICInitStruct);

    RCC_APB2PeriphClockCmd(hw->rcc_clk, ENABLE);

    TIM_TimeBaseInitStruct.TIM_Prescaler = base_config->prescaler;
    TIM_TimeBaseInitStruct.TIM_CounterMode = BSP_TIM_ConvertCounterMode(base_config->counter_mode);
    TIM_TimeBaseInitStruct.TIM_Period = base_config->period;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = BSP_TIM_ConvertClockDivision(base_config->clock_div);
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = base_config->repetition_cnt;
    TIM_TimeBaseInit(hw->timer, &TIM_TimeBaseInitStruct);

    TIM_PrescalerConfig(hw->timer, base_config->prescaler, ENABLE);
    TIM_ARRPreloadConfig(hw->timer, ENABLE);

    s_ic_channel[timer_id] = ic_config->ic_channel;

    TIM_ICInitStruct.TIM_Channel = BSP_TIM_ConvertICChannel(ic_config->ic_channel);
    TIM_ICInitStruct.TIM_ICPrescaler = BSP_TIM_ConvertICPrescaler(ic_config->ic_prescaler_div);
    TIM_ICInitStruct.TIM_ICPolarity = BSP_TIM_ConvertICPolarity(ic_config->ic_polarity);
    TIM_ICInitStruct.TIM_ICSelection = BSP_TIM_ConvertICSelection(ic_config->ic_selection);
    TIM_ICInitStruct.TIM_ICFilter = ic_config->ic_filter;
    TIM_ICInit(hw->timer, &TIM_ICInitStruct);

    TIM_ClearFlag(hw->timer, TIM_IT_Update |
            BSP_TIM_ConvertICITFlag(ic_config->ic_channel));

    TIM_ITConfig(hw->timer, TIM_IT_Update |
            BSP_TIM_ConvertICITFlag(ic_config->ic_channel), ENABLE);
    BSP_TIM_NVIC_Config(timer_id);

    TIM_Cmd(hw->timer, ENABLE);
}

/**
 * @brief 对外提供的高级定时器输入捕获配置接口
 * @param[in] timer_id 高级定时器编号
 * @param[in] base_config 基础配置
 * @param[in] ic_config 输入捕获配置
 * @return 无返回值
 */
void BSP_AdvancedTimer_Config(bsp_advancedtimer_t timer_id,
            bsp_advancedtimer_config_t *base_config,
            bsp_tim_ic_config_t *ic_config)
{
    BSP_TIM_GPIO_Init(timer_id, ic_config->ic_channel);
    BSP_AdvancedTIM_Init(timer_id, base_config, ic_config);
}

/** @brief 输入捕获结果数组 */
bsp_tim_ic_result_t result[BSP_ADVANCED_TIMER_MAX] = {0};

/**
 * @brief 高级定时器更新中断公共处理函数
 * @details
 * 当等待下降沿期间，如果计数器发生溢出，则通过该函数累加溢出次数，
 * 用于后续计算完整脉宽。
 *
 * @param[in] timer_id 高级定时器编号
 * @return 无返回值
 */
void BSP_TIM_UP_IRQHandler(bsp_advancedtimer_t timer_id)
{
    if(timer_id >= BSP_ADVANCED_TIMER_MAX) return;

    const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];
    bsp_tim_ic_channel_t ic_channel = s_ic_channel[timer_id];
    bsp_tim_ic_result_t *res = &result[timer_id];

    if(TIM_GetITStatus(hw->timer, TIM_IT_Update) != RESET){
        res->over_flow[ic_channel]++;
        TIM_ClearITPendingBit(hw->timer, TIM_IT_Update);
    }
}

/**
 * @brief 高级定时器捕获比较中断公共处理函数
 * @details
 * 流程如下：
 * - 第一次进入：认为捕获到上升沿，记录 rising_cnt，并切换成下降沿捕获
 * - 第二次进入：认为捕获到下降沿，记录 falling_cnt，并结合 over_flow 计算 pulse_width
 * - 最后切回上升沿捕获，准备下一次测量
 *
 * @param[in] timer_id 高级定时器编号
 * @return 无返回值
 *
 * @note
 * 脉宽计算时需要考虑三种情况：
 * 1. 下降沿捕获值 >= 上升沿捕获值，且期间可能有整周期溢出
 * 2. 下降沿捕获值 < 上升沿捕获值，且期间至少溢出一次
 * 3. 下降沿捕获值 < 上升沿捕获值，但 over_flow 为 0，说明仅跨越了当前 ARR 边界
 */
void BSP_TIM_CC_IRQHandler(bsp_advancedtimer_t timer_id)
{
    if(timer_id >= BSP_ADVANCED_TIMER_MAX) return;

    const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];
    bsp_tim_ic_channel_t ic_channel = s_ic_channel[timer_id];
    uint16_t it_flag = BSP_TIM_ConvertICITFlag(ic_channel);
    bsp_tim_ic_result_t *res = &result[timer_id];
    uint32_t period = hw->timer->ARR + 1;

    if(TIM_GetITStatus(hw->timer, it_flag) != RESET){
        if(res->capture_state[ic_channel] == 0){
            res->rising_cnt[ic_channel] = BSP_TIM_GetCapturex(hw->timer, ic_channel);
            BSP_TIM_OCxPolarityConfig(hw->timer, TIM_ICPolarity_Falling, ic_channel);
            res->over_flow[ic_channel] = 0;
            res->capture_state[ic_channel] = 1;
        }else if(res->capture_state[ic_channel] == 1){
            res->falling_cnt[ic_channel] = BSP_TIM_GetCapturex(hw->timer, ic_channel);

            if(res->falling_cnt[ic_channel] >= res->rising_cnt[ic_channel]){
                res->pulse_width[ic_channel] = res->over_flow[ic_channel] * period +
                             res->falling_cnt[ic_channel] - res->rising_cnt[ic_channel];
            }else{
                if(res->over_flow[ic_channel] > 0){
                    res->pulse_width[ic_channel] = (res->over_flow[ic_channel] - 1) * period +
                            period - res->rising_cnt[ic_channel] + res->falling_cnt[ic_channel];
                }else{
                    res->pulse_width[ic_channel] = period - res->rising_cnt[ic_channel] +
                            res->falling_cnt[ic_channel];
                }
            }

            BSP_TIM_OCxPolarityConfig(hw->timer, TIM_ICPolarity_Rising, ic_channel);
            res->capture_state[ic_channel] = 0;
        }

        TIM_ClearITPendingBit(hw->timer, it_flag);
    }
}