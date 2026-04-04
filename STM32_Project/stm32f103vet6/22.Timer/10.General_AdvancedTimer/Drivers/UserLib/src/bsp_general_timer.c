/**
 * @file bsp_general_timer.c
 * @brief STM32F10x 通用定时器 BSP 驱动源文件
 * @author
 * @date
 * @version 1.0
 *
 * @details
 * 本文件实现了 STM32F10x 通用定时器 TIM2~TIM5 的底层配置逻辑，
 * 包括：
 * - 硬件资源映射表建立
 * - GPIO 复用输出配置
 * - 定时器基础参数初始化
 * - 输出比较单元初始化
 * - 更新中断统一处理
 *
 * 通过对标准外设库接口进行再次封装，
 * 提高应用层对多个通用定时器的复用性和可维护性。
 */

#include "bsp_general_timer.h"

/**
 * @struct bsp_generaltimer_hw_t
 * @brief 通用定时器硬件资源描述结构体。
 *
 * 该结构体用于保存某个定时器实例所需的底层硬件信息，包括：
 * - TIM 外设指针
 * - RCC 时钟使能位
 * - 更新中断信息
 * - 4 个输出比较通道对应的 GPIO 描述
 *
 * 该结构体仅在当前源文件内部使用。
 */
typedef struct{
    /**< TIM 外设寄存器基地址 */
    TIM_TypeDef *timer;

    /**< TIM 外设 RCC 时钟使能位 */
    uint32_t rcc_clk;

    /**< 更新中断通道号 */
    uint8_t irq_update_channel;

    /**< 中断抢占优先级 */
    uint8_t irq_pre_prio;

    /**< 中断子优先级 */
    uint8_t irq_sub_prio;

    /**< 通道 1 对应 GPIO */
    bsp_gpio_t ch1_gpio;

    /**< 通道 2 对应 GPIO */
    bsp_gpio_t ch2_gpio;

    /**< 通道 3 对应 GPIO */
    bsp_gpio_t ch3_gpio;

    /**< 通道 4 对应 GPIO */
    bsp_gpio_t ch4_gpio;
}bsp_generaltimer_hw_t;

/**
 * @brief 通用定时器更新中断计数数组定义。
 *
 * 每个元素分别表示一个定时器自启动以来发生的更新中断次数。
 */
volatile uint16_t generaltimer_cnt[BSP_GENERAL_TIMER_MAX] = {0};

/**
 * @brief 通用定时器硬件资源映射表。
 *
 * 该表建立了逻辑定时器编号和具体硬件资源之间的映射关系。
 * 上层调用者仅需给出 @ref bsp_generaltimer_t 编号，
 * 即可在底层定位到对应的 TIMx 外设及 GPIO 资源。
 *
 * @note
 * TIM2 和 TIM5 当前都映射到了 PA0~PA3，
 * 实际项目中如果同时使用，需确认芯片封装、引脚复用和是否存在冲突。
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
 * @brief 将 BSP 抽象层计数模式转换为 STM32 标准库计数模式宏。
 *
 * @param counter_mode BSP 抽象层计数模式
 * @return 对应的 TIM_CounterMode_xxx 宏值；若参数非法则返回 0
 */
static uint16_t BSP_TIM_ConvertCounterMode(bsp_tim_counter_mode_t counter_mode)
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
 * @brief 将 BSP 抽象层时钟分频配置转换为 STM32 标准库宏。
 *
 * @param clock_div BSP 抽象层时钟分频枚举值
 * @return 对应的 TIM_CKD_DIVx 宏值；若参数非法则返回 0
 */
static uint16_t BSP_TIM_ConvertClockDivision(bsp_tim_clock_div_t clock_div)
{
    switch(clock_div){
        case BSP_TIM_CLOCK_DIV_1:   return TIM_CKD_DIV1;
        case BSP_TIM_CLOCK_DIV_2:   return TIM_CKD_DIV2;
        case BSP_TIM_CLOCK_DIV_4:   return TIM_CKD_DIV4;
        default:    return 0;
    }
}

/**
 * @brief 将 BSP 抽象层输出比较模式转换为 STM32 标准库宏。
 *
 * @param oc_mode BSP 输出比较模式
 * @return 对应的 TIM_OCMode_xxx 宏值；若参数非法则返回 0
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
 * @brief 根据通道号调用对应的输出比较初始化函数。
 *
 * STM32 标准外设库中，不同通道对应不同的初始化函数，
 * 本函数将这部分差异统一封装起来。
 *
 * @param timer 定时器实例指针
 * @param oc_init 输出比较初始化结构体指针
 * @param oc_channel 输出比较通道号
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
 * @brief 根据通道号配置对应输出比较通道的预装载功能。
 *
 * @param timer 定时器实例指针
 * @param state 预装载状态
 * @param oc_channel 输出比较通道号
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
 * @brief 获取指定定时器通道对应的 GPIO 资源描述。
 *
 * @param hw 定时器硬件资源描述结构体指针
 * @param oc_channel 输出比较通道号
 * @return 对应 GPIO 资源描述指针；若通道非法则返回 NULL
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
 * @brief 初始化指定定时器输出比较通道对应的 GPIO。
 *
 * 该函数会根据定时器编号和通道编号，从硬件映射表中找到对应引脚，
 * 并将该 GPIO 配置为复用推挽输出模式，用于定时器输出功能。
 *
 * @param timer_id 定时器编号
 * @param oc_channel 输出比较通道号
 *
 * @note
 * 函数默认认为通道号合法，且 GPIO 映射存在。
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
 * @brief 初始化通用定时器基础单元和输出比较单元。
 *
 * 该函数负责完成定时器核心初始化流程：
 * - 开启 TIM 外设时钟
 * - 配置计数基础参数
 * - 配置 ARR 和 PSC 预装载
 * - 配置输出比较模式和各项电气参数
 * - 配置 CCR 预装载
 * - 启动定时器
 *
 * @param timer_id 定时器编号
 * @param base_config 基础配置结构体指针
 * @param oc_config 输出比较配置结构体指针
 *
 * @note
 * 1. 函数不检查空指针，调用前应确保参数有效。
 * 2. 函数仅启动定时器，不主动配置中断。
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

    RCC_APB1PeriphClockCmd(hw->rcc_clk, ENABLE);

    /**
     * @brief 配置定时器基础参数。
     *
     * - TIM_Prescaler：预分频值
     * - TIM_CounterMode：计数模式
     * - TIM_Period：自动重装载值
     * - TIM_ClockDivision：时钟分频
     * - TIM_RepetitionCounter：重复计数值
     */
    TIM_TimeBaseInitStruct.TIM_Prescaler = base_config->prescaler;
    TIM_TimeBaseInitStruct.TIM_CounterMode = BSP_TIM_ConvertCounterMode(base_config->counter_mode);
    TIM_TimeBaseInitStruct.TIM_Period = base_config->period;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = BSP_TIM_ConvertClockDivision(base_config->clock_div);
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = base_config->repetition_cnt;
    TIM_TimeBaseInit(hw->timer, &TIM_TimeBaseInitStruct);

    /**
     * @brief 配置预分频器和自动重装载寄存器的预装载功能。
     *
     * 使 PSC 和 ARR 的更新在下一个更新事件时统一生效。
     */
    TIM_PrescalerConfig(hw->timer, base_config->prescaler, TIM_PSCReloadMode_Update);
    TIM_ARRPreloadConfig(hw->timer, ENABLE);

	/**
     * @brief 配置输出比较参数。
     *
     * 包括模式、输出状态、比较值、极性及空闲状态等。
     */
	TIM_OCInitStruct.TIM_OCMode = BSP_TIM_ConvertOCMode(oc_config->oc_mode);
    TIM_OCInitStruct.TIM_OutputState =
        (oc_config->oc_output_state == BSP_TIM_OUTPUT_STATE_ENABLE) ? TIM_OutputState_Enable : TIM_OutputState_Disable;
    TIM_OCInitStruct.TIM_OutputNState =
        (oc_config->oc_output_nstate == BSP_TIM_OUTPUT_NSTATE_ENABLE) ? TIM_OutputNState_Enable : TIM_OutputNState_Disable;
    TIM_OCInitStruct.TIM_Pulse = oc_config->oc_ccr_value;
    TIM_OCInitStruct.TIM_OCPolarity =
        (oc_config->oc_polarity == BSP_TIM_OC_POLARITY_HIGH) ? TIM_OCPolarity_High : TIM_OCPolarity_Low;
    TIM_OCInitStruct.TIM_OCNPolarity =
        (oc_config->oc_npolarity == BSP_TIM_OC_NPOLARITY_HIGH) ? TIM_OCNPolarity_High : TIM_OCNPolarity_Low;
    TIM_OCInitStruct.TIM_OCIdleState =
        (oc_config->oc_idle_state == BSP_TIM_OC_IDLE_STATE_SET) ? TIM_OCIdleState_Set : TIM_OCIdleState_Reset;
    TIM_OCInitStruct.TIM_OCNIdleState =
        (oc_config->oc_nidle_state == BSP_TIM_OC_NIDLE_STATE_SET) ? TIM_OCNIdleState_Set : TIM_OCNIdleState_Reset;

    BSP_TIM_OCxInit(hw->timer, &TIM_OCInitStruct, oc_config->oc_channel);

    /**
     * @brief 使能对应通道 CCR 的预装载功能。
     *
     * 这样在修改比较值时，可避免立即更新造成的输出毛刺。
     */
    BSP_TIM_OCxPreloadConfig(hw->timer, TIM_OCPreload_Enable, oc_config->oc_channel);

    /**
     * @brief 启动定时器计数器。
     */
    TIM_Cmd(hw->timer, ENABLE);
}

/**
 * @brief 配置通用定时器。
 *
 * 本函数是对外统一入口，内部完成 GPIO 初始化和定时器初始化。
 *
 * @param timer_id 定时器编号
 * @param base_config 基础配置结构体指针
 * @param oc_config 输出比较配置结构体指针
 */
void BSP_GeneralTIM_Config(bsp_generaltimer_t timer_id, bsp_generaltimer_config_t *base_config, bsp_tim_oc_config_t *oc_config)
{
    BSP_General_GPIO_Init(timer_id, oc_config->oc_channel);
    BSP_GeneralTIM_Init(timer_id, base_config, oc_config);
}

/**
 * @brief 通用定时器更新中断统一处理函数。
 *
 * 本函数检测指定定时器是否产生更新中断：
 * - 若中断发生，则对应软件计数器加 1
 * - 随后清除更新中断挂起标志
 *
 * @param timer_id 定时器编号
 *
 * @note
 * 本函数通常由具体的中断服务函数调用，例如 TIM2_IRQHandler、
 * TIM3_IRQHandler 等。
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