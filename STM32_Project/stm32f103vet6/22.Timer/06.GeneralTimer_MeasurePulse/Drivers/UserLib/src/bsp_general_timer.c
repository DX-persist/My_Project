#include "bsp_general_timer.h"

/**
 * @file    bsp_general_timer.c
 * @brief   STM32F10x 通用定时器输入捕获测量脉宽 BSP 实现文件
 * @details
 * 本文件实现 TIM2 / TIM3 / TIM4 / TIM5 的输入捕获测量脉宽功能。
 *
 * 实现思路如下：
 * - 初始化某一路输入捕获为上升沿触发
 * - 上升沿到来后记录捕获值
 * - 动态切换为下降沿捕获
 * - 下降沿到来后再次记录捕获值
 * - 若中间发生溢出，则通过更新中断累计溢出次数
 * - 最终计算高电平脉宽
 *
 * @note
 * 本模块用于“测量高电平脉宽”，并不直接输出频率或占空比。
 */

/**
 * @brief 通用定时器硬件资源映射结构体
 */
typedef struct{
    TIM_TypeDef *timer;        /**< 定时器寄存器实例 */
    uint32_t rcc_clk;          /**< RCC 时钟使能位 */

    uint8_t irq_update_channel;/**< 定时器中断号 */
    uint8_t irq_pre_prio;      /**< 中断抢占优先级 */
    uint8_t irq_sub_prio;      /**< 中断子优先级 */

    bsp_gpio_t ch1_gpio;       /**< CH1 对应 GPIO */
    bsp_gpio_t ch2_gpio;       /**< CH2 对应 GPIO */
    bsp_gpio_t ch3_gpio;       /**< CH3 对应 GPIO */
    bsp_gpio_t ch4_gpio;       /**< CH4 对应 GPIO */
}bsp_generaltimer_hw_t;

/** @brief 通用定时器辅助计数数组 */
volatile uint16_t generaltimer_cnt[BSP_GENERAL_TIMER_MAX] = {0};

/**
 * @brief 保存每个定时器当前使用的输入捕获通道
 * @details
 * 按 timer_id 索引，表示该定时器当前配置的是 CH1 / CH2 / CH3 / CH4 中的哪一路。
 */
static bsp_tim_ic_channel_t s_ic_channel_map[BSP_GENERAL_TIMER_MAX];

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
 *
 * @note
 * 当前使用的是 TIM2~TIM5 的统一 IRQ 通道配置。
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
 * @brief 将输入捕获通道枚举转换为标准库通道宏
 * @param[in] ic_channel 输入捕获通道
 * @return TIM_Channel_1 / TIM_Channel_2 / TIM_Channel_3 / TIM_Channel_4，非法值返回 0
 */
static uint16_t BSP_TIM_ConvertICChannel(bsp_tim_ic_channel_t ic_channel)
{
    switch(ic_channel){
        case BSP_TIM_IC_CHANNEL1: return TIM_Channel_1;
        case BSP_TIM_IC_CHANNEL2: return TIM_Channel_2;
        case BSP_TIM_IC_CHANNEL3: return TIM_Channel_3;
        case BSP_TIM_IC_CHANNEL4: return TIM_Channel_4;
        default:                  return 0;
    }
}

/**
 * @brief 将输入捕获通道转换为对应的捕获比较中断位
 * @param[in] ic_channel 输入捕获通道
 * @return TIM_IT_CC1 / TIM_IT_CC2 / TIM_IT_CC3 / TIM_IT_CC4，非法值返回 0
 */
static uint16_t BSP_TIM_ConvertICITFlag(bsp_tim_ic_channel_t ic_channel)
{
    switch(ic_channel){
        case BSP_TIM_IC_CHANNEL1: return TIM_IT_CC1;
        case BSP_TIM_IC_CHANNEL2: return TIM_IT_CC2;
        case BSP_TIM_IC_CHANNEL3: return TIM_IT_CC3;
        case BSP_TIM_IC_CHANNEL4: return TIM_IT_CC4;
        default:                  return 0;
    }
}

/**
 * @brief 将输入捕获极性枚举转换为标准库极性宏
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
 * @brief 将输入捕获预分频枚举转换为标准库宏
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
 * @brief 将输入捕获选择枚举转换为标准库宏
 * @param[in] ic_selection 输入捕获映射选择
 * @return 对应的 TIM_ICSelection_xxx 宏，非法值返回 0
 */
static uint16_t BSP_TIM_ConvertICSelection(bsp_tim_ic_selection_t ic_selection)
{
    switch(ic_selection){
        case BSP_TIM_IC_SELECTION_DIRECTTI: return TIM_ICSelection_DirectTI;
        case BSP_TIM_IC_SELECTION_INDIRECTTI: return TIM_ICSelection_IndirectTI;
        case BSP_TIM_IC_SELECTION_TRC: return TIM_ICSelection_TRC;
        default:    return 0;
    }
}

/**
 * @brief 根据通道动态切换捕获极性
 * @details
 * 该函数用于实现：
 * - 先捕获上升沿
 * - 再切换为下降沿捕获
 * - 完成一次脉宽测量后再切回上升沿
 *
 * @param[in] timer 定时器实例
 * @param[in] polarity 新极性
 * @param[in] ic_channel 输入捕获通道
 * @return 无返回值
 *
 * @note
 * 虽然使用的是 `TIM_OCxPolarityConfig()`，但这里的目的实际上是动态修改捕获极性。
 * 这个地方的接口名称比较容易让接手代码的人误解。
 */
static void BSP_TIM_OCxPolarityConfig(TIM_TypeDef *timer, uint16_t polarity, bsp_tim_ic_channel_t ic_channel)
{
    switch(ic_channel){
        case BSP_TIM_IC_CHANNEL1: TIM_OC1PolarityConfig(timer, polarity); break;
        case BSP_TIM_IC_CHANNEL2: TIM_OC2PolarityConfig(timer, polarity); break;
        case BSP_TIM_IC_CHANNEL3: TIM_OC3PolarityConfig(timer, polarity); break;
        case BSP_TIM_IC_CHANNEL4: TIM_OC4PolarityConfig(timer, polarity); break;
        default:    break;
    }
}

/**
 * @brief 根据输入捕获通道读取对应捕获寄存器值
 * @param[in] TIMx 定时器实例
 * @param[in] ic_channel 输入捕获通道
 * @return 当前通道对应的捕获寄存器值
 */
static uint16_t BSP_TIM_GetCapturex(TIM_TypeDef* TIMx, bsp_tim_ic_channel_t ic_channel)
{
    switch(ic_channel){
        case BSP_TIM_IC_CHANNEL1: return TIM_GetCapture1(TIMx);
        case BSP_TIM_IC_CHANNEL2: return TIM_GetCapture2(TIMx);
        case BSP_TIM_IC_CHANNEL3: return TIM_GetCapture3(TIMx);
        case BSP_TIM_IC_CHANNEL4: return TIM_GetCapture4(TIMx);
        default:    return 0;
    }
}

/**
 * @brief 根据输入捕获通道获取对应 GPIO 描述信息
 * @param[in] hw 通用定时器硬件资源结构体指针
 * @param[in] ic_channel 输入捕获通道
 * @return 成功返回 GPIO 描述结构体指针，失败返回 NULL
 */
static const bsp_gpio_t *BSP_GetGPIO(const bsp_generaltimer_hw_t *hw, bsp_tim_ic_channel_t ic_channel)
{
    const bsp_gpio_t *gpio = NULL;

    switch(ic_channel){
        case BSP_TIM_IC_CHANNEL1: gpio = &hw->ch1_gpio; break;
        case BSP_TIM_IC_CHANNEL2: gpio = &hw->ch2_gpio; break;
        case BSP_TIM_IC_CHANNEL3: gpio = &hw->ch3_gpio; break;
        case BSP_TIM_IC_CHANNEL4: gpio = &hw->ch4_gpio; break;
        default:                                        break;
    }
    return gpio;
}

/**
 * @brief 初始化输入捕获通道对应的 GPIO
 * @param[in] timer_id 通用定时器编号
 * @param[in] ic_channel 输入捕获通道
 * @return 无返回值
 *
 * @note
 * 当前配置为浮空输入模式。若硬件电路有特殊要求，可改成上拉/下拉输入。
 */
static void BSP_General_GPIO_Init(bsp_generaltimer_t timer_id, bsp_tim_ic_channel_t ic_channel)
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
 * @brief 初始化通用定时器输入捕获测量功能
 * @details
 * 本函数完成：
 * - 定时器基础参数配置
 * - 输入捕获配置
 * - 清除初始化阶段自动产生的更新标志
 * - 使能更新中断和捕获中断
 * - 启动定时器
 *
 * @param[in] timer_id 通用定时器编号
 * @param[in] base_config 定时器基础参数配置
 * @param[in] ic_config 输入捕获配置
 * @return 无返回值
 *
 * @note
 * PSC 和 ARR 打开预装载后，在初始化阶段通常会自动触发一次更新事件，
 * 所以这里手动清除更新标志，避免误进入中断逻辑。
 */
static void BSP_GeneralTIM_Init(bsp_generaltimer_t timer_id,
        bsp_generaltimer_config_t *base_config,
        bsp_tim_ic_config_t *ic_config)
{
    if(timer_id >= BSP_GENERAL_TIMER_MAX)  return;

    const bsp_generaltimer_hw_t *hw = &bsp_generaltimer_hw[timer_id];
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_ICInitTypeDef TIM_ICInitStruct;
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_ICStructInit(&TIM_ICInitStruct);

    RCC_APB1PeriphClockCmd(hw->rcc_clk, ENABLE);    /**< 开启 TIMx 时钟 */

    /* 初始化 TIM_BaseInitTypeDef 结构体 */
    TIM_TimeBaseInitStruct.TIM_Prescaler = base_config->prescaler;
    TIM_TimeBaseInitStruct.TIM_CounterMode = BSP_TIM_ConvertCounterMode(base_config->counter_mode);
    TIM_TimeBaseInitStruct.TIM_Period = base_config->period;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = BSP_TIM_ConvertClockDivision(base_config->clock_div);
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = base_config->repetition_cnt;
    TIM_TimeBaseInit(hw->timer, &TIM_TimeBaseInitStruct);

    /* 启用 PSC 和 ARR 预装载功能，在下一更新周期同步到实际寄存器 */
    TIM_PrescalerConfig(hw->timer, base_config->prescaler, TIM_PSCReloadMode_Update);
    TIM_ARRPreloadConfig(hw->timer, ENABLE);

    /* 保存当前定时器对应的输入捕获通道 */
    s_ic_channel_map[timer_id] = ic_config->ic_channel;

    /* 配置输入捕获参数 */
    TIM_ICInitStruct.TIM_Channel = BSP_TIM_ConvertICChannel(ic_config->ic_channel);
    TIM_ICInitStruct.TIM_ICPolarity = BSP_TIM_ConvertICPolarity(ic_config->ic_polarity);
    TIM_ICInitStruct.TIM_ICPrescaler = BSP_TIM_ConvertICPrescaler(ic_config->ic_prescaler_div);
    TIM_ICInitStruct.TIM_ICSelection = BSP_TIM_ConvertICSelection(ic_config->ic_selection);
    TIM_ICInitStruct.TIM_ICFilter = ic_config->ic_filter;
    TIM_ICInit(hw->timer, &TIM_ICInitStruct);

    /*
     * 这里需要清空 TIM_IT_Update 标志位，因为 ARR 和 Prescaler
     * 都具有预装载功能，所以硬件会自动发生一次更新事件将数据
     * 写入到寄存器中。
     */
    TIM_ClearFlag(hw->timer, TIM_IT_Update |
            BSP_TIM_ConvertICITFlag(ic_config->ic_channel));

    /* 配置更新中断和输入捕获中断 */
    TIM_ITConfig(hw->timer, TIM_IT_Update |
            BSP_TIM_ConvertICITFlag(ic_config->ic_channel), ENABLE);
    BSP_TIM_NVIC_Config(timer_id);

    /* 启动定时器 */
    TIM_Cmd(hw->timer, ENABLE);
}

/**
 * @brief 对外提供的通用定时器输入捕获配置接口
 * @param[in] timer_id 通用定时器编号
 * @param[in] base_config 定时器基础配置
 * @param[in] ic_config 输入捕获配置
 * @return 无返回值
 *
 * @details
 * 按顺序完成 GPIO 初始化和定时器初始化。
 */
void BSP_GeneralTIM_Config(bsp_generaltimer_t timer_id, bsp_generaltimer_config_t *base_config, bsp_tim_ic_config_t *ic_config)
{
    BSP_General_GPIO_Init(timer_id, ic_config->ic_channel);
    BSP_GeneralTIM_Init(timer_id, base_config, ic_config);
}

/** @brief 输入捕获结果数组 */
volatile bsp_tim_ic_result_t result[BSP_GENERAL_TIMER_MAX] = {0};

/**
 * @brief 通用定时器中断公共处理函数
 * @details
 * 使用输入捕获功能测量脉冲宽度的基本原理如下：
 * 1. 先配置定时器捕获上升沿并开启对应捕获中断
 * 2. 上升沿到来时自动触发中断并将计数值锁存到捕获寄存器
 * 3. 在中断中读取该值，记录为上升沿计数值，并切换为下降沿捕获
 * 4. 下降沿到来时再次触发中断并锁存下降沿计数值
 * 5. 在中断中读取下降沿计数值，结合溢出次数计算高电平脉宽
 *
 * @param[in] timer_id 通用定时器编号
 * @return 无返回值
 *
 * @note
 * 当前函数同时处理：
 * - TIM_IT_Update：用于统计测量期间的溢出次数
 * - TIM_IT_CCx：用于记录上升沿/下降沿并计算脉宽
 */
void BSP_TIM_IRQHandler(bsp_generaltimer_t timer_id)
{
    if(timer_id >= BSP_GENERAL_TIMER_MAX)  return;

    const bsp_generaltimer_hw_t *hw = &bsp_generaltimer_hw[timer_id];
    bsp_tim_ic_channel_t ic_channel = s_ic_channel_map[timer_id];

    /* 处理更新中断：统计脉宽测量期间发生的溢出次数 */
    if(TIM_GetITStatus(hw->timer, TIM_IT_Update) != RESET){
        result[timer_id].over_flow[timer_id]++;
        TIM_ClearITPendingBit(hw->timer, TIM_IT_Update);
    }

    /* 处理输入捕获中断 */
    if(TIM_GetITStatus(hw->timer, BSP_TIM_ConvertICITFlag(ic_channel)) != RESET){
        if(result[timer_id].capture_state == 0){  /**< 当前等待捕获上升沿 */
            result[timer_id].over_flow[timer_id] = 0;   /**< 捕获到上升沿时清空溢出计数 */
            result[timer_id].rising_cnt = BSP_TIM_GetCapturex(hw->timer, ic_channel); /**< 记录上升沿时的计数值 */
            BSP_TIM_OCxPolarityConfig(hw->timer, TIM_ICPolarity_Falling, ic_channel); /**< 切换为下降沿捕获 */
            result[timer_id].capture_state = 1;   /**< 更新状态为等待下降沿 */
        }else{  /**< 当前等待捕获下降沿 */
            result[timer_id].falling_cnt = BSP_TIM_GetCapturex(hw->timer, ic_channel); /**< 记录下降沿时的计数值 */

            if(result[timer_id].falling_cnt >= result[timer_id].rising_cnt){
                result[timer_id].pulse_width = result[timer_id].over_flow[timer_id] * (hw->timer->ARR + 1) +
                    result[timer_id].falling_cnt - result[timer_id].rising_cnt;
            }else{
                if(result[timer_id].over_flow[timer_id] > 0){
                    result[timer_id].pulse_width = (result[timer_id].over_flow[timer_id] - 1) * (hw->timer->ARR + 1) +
                     (hw->timer->ARR + 1 - result[timer_id].rising_cnt) + result[timer_id].falling_cnt;
                }else{
                    result[timer_id].pulse_width = hw->timer->ARR + 1 - result[timer_id].rising_cnt + result[timer_id].falling_cnt;
                }
            }

            BSP_TIM_OCxPolarityConfig(hw->timer, TIM_ICPolarity_Rising, ic_channel); /**< 测量完成后切回上升沿捕获 */
            result[timer_id].capture_state = 0;   /**< 更新状态为等待下一次上升沿 */
        }

        TIM_ClearITPendingBit(hw->timer, BSP_TIM_ConvertICITFlag(ic_channel));
    }
}