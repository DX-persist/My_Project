/**
 * @file    bsp_advanced_timer.c
 * @brief   STM32F10x 高级定时器 PWM 输入捕获 BSP 实现文件
 * @details
 * 本文件实现了高级定时器 TIM1 / TIM8 的 PWM 输入测量功能。
 *
 * 核心实现要点：
 * - 通过 GPIO 配置输入引脚
 * - 配置高级定时器基本计数参数
 * - 使用 `TIM_PWMIConfig()` 进入 PWM 输入模式
 * - 通过从模式复位（Reset Mode）保证每个周期重新计数
 * - 在捕获比较中断中读取捕获寄存器并计算频率与占空比
 *
 * PWM 输入模式说明：
 * - 当选择 CH1 作为 PWM 输入时，定时器内部会自动使用 CH1/CH2 配合测量
 * - 当选择 CH2 作为 PWM 输入时，内部同样会配合另一个通道工作
 * - 因此，用户配置“PWM 输入通道”并不意味着只使用单一通道寄存器
 *
 * @note
 * 1. `TIM_GetCapture1()` / `TIM_GetCapture2()` 读取到的是当前锁存的计数器值，不是直接时间值。
 * 2. 频率计算依赖 `CLK_FREQ` 与 PSC 配置，若系统时钟树变化，应同步检查计算公式。
 * 3. 本文件默认使用捕获比较中断，而不是更新中断。
 */

#include "bsp_advanced_timer.h"

/**
 * @brief 高级定时器硬件资源映射表结构体
 * @details
 * 用于描述某个高级定时器实例的：
 * - 定时器寄存器基地址
 * - RCC 时钟
 * - 中断号和优先级
 * - PWM 输入相关 GPIO 引脚
 */
typedef struct{
    TIM_TypeDef *timer;       /**< 定时器寄存器实例 */
    uint32_t rcc_clk;         /**< 定时器 RCC 时钟使能位 */

    uint8_t irq_cc_channel;   /**< 捕获比较中断号 */
    uint8_t irq_cc_pre_prio;  /**< 捕获比较中断抢占优先级 */
    uint8_t irq_cc_sub_prio;  /**< 捕获比较中断子优先级 */

    bsp_gpio_t ch1_gpio;      /**< 通道 1 对应 GPIO */
    bsp_gpio_t ch2_gpio;      /**< 通道 2 对应 GPIO */
}bsp_advancedtimer_hw_t;

/**
 * @brief 全局 PWM 测量结果数组
 * @details
 * 每个高级定时器对应一个结果槽位，在中断中实时更新。
 */
bsp_tim_ic_result_t result[BSP_ADVANCED_TIMER_MAX] = {0};

/**
 * @brief 记录每个高级定时器当前选择的 PWM 输入源通道
 * @details
 * 中断处理函数需要知道最初是从 CH1 还是 CH2 作为输入源，
 * 因为这将决定 Capture1 / Capture2 分别代表周期还是高电平时间。
 */
static bsp_tim_pwm_input_channel_t s_pwm_channel[BSP_ADVANCED_TIMER_MAX];

/**
 * @brief 高级定时器硬件资源静态映射表
 * @details
 * 建立逻辑定时器编号与底层硬件资源之间的关联。
 */
static const bsp_advancedtimer_hw_t bsp_advancedtimer_hw[BSP_ADVANCED_TIMER_MAX] = {
    [BSP_ADVANCED_TIMER1] = {
        .timer = TIM1,
        .rcc_clk = RCC_APB2Periph_TIM1,

        .irq_cc_channel    = TIM1_CC_IRQn,
        .irq_cc_pre_prio   = CC_PREEPT_PRIO,
        .irq_cc_sub_prio   = CC_SUB_PRIO,

        .ch1_gpio  = {GPIOA, GPIO_Pin_8, RCC_APB2Periph_GPIOA},
        .ch2_gpio  = {GPIOA, GPIO_Pin_9, RCC_APB2Periph_GPIOA},
    },

    [BSP_ADVANCED_TIMER8] = {
        .timer = TIM8,
        .rcc_clk = RCC_APB2Periph_TIM8,

        .irq_cc_channel    = TIM8_CC_IRQn,
        .irq_cc_pre_prio   = CC_PREEPT_PRIO,
        .irq_cc_sub_prio   = CC_SUB_PRIO,

        .ch1_gpio  = {GPIOC, GPIO_Pin_6, RCC_APB2Periph_GPIOC},
        .ch2_gpio  = {GPIOC, GPIO_Pin_7, RCC_APB2Periph_GPIOC},
    }
};

/**
 * @brief 根据 PWM 输入通道获取对应 GPIO 描述信息
 * @param[in] hw
 * 指向某个高级定时器硬件资源描述结构体。
 *
 * @param[in] pwm_channel
 * PWM 输入通道选择。
 *
 * @return
 * - 成功：返回对应 GPIO 描述结构体指针
 * - 失败：返回 `NULL`
 *
 * @note
 * 该函数只负责“输入源通道对应的引脚选择”，不涉及 PWM 输入模式下另一个辅助通道的配置细节。
 */
static const bsp_gpio_t *BSP_TIM_GetCHGPIO(const bsp_advancedtimer_hw_t *hw, bsp_tim_pwm_input_channel_t pwm_channel)
{
    switch(pwm_channel){
        case BSP_TIM_PWM_INPUT_CHANNEL1: return &hw->ch1_gpio;
        case BSP_TIM_PWM_INPUT_CHANNEL2: return &hw->ch2_gpio;
        default: return NULL;
    }
}

/**
 * @brief 初始化 PWM 输入通道对应 GPIO
 * @details
 * 将选定的定时器输入引脚配置为浮空输入模式。
 *
 * @param[in] timer_id
 * 高级定时器编号。
 *
 * @param[in] pwm_channel
 * PWM 输入源通道。
 *
 * @return
 * 无返回值。
 *
 * @note
 * 这里使用 `GPIO_Mode_IN_FLOATING`。
 * 在某些硬件设计中，如果输入信号源为开漏或弱驱动，可能更适合上拉/下拉输入模式，
 * 需根据实际电路情况调整。
 */
static void BSP_TIM_GPIO_Init(bsp_advancedtimer_t timer_id, bsp_tim_pwm_input_channel_t pwm_channel)
{
    if(timer_id >= BSP_ADVANCED_TIMER_MAX) return;

    const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];
    const bsp_gpio_t *ch_gpio = BSP_TIM_GetCHGPIO(hw, pwm_channel);
    uint32_t enable_clk = ch_gpio->rcc_clk;
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(enable_clk, ENABLE);

    GPIO_InitStruct.GPIO_Pin = ch_gpio->gpio_pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(ch_gpio->gpio_port, &GPIO_InitStruct);
}

/**
 * @brief 将 BSP 计数模式枚举转换为标准库宏定义
 * @param[in] counter_mode
 * BSP 层计数模式枚举值。
 *
 * @return
 * 返回标准外设库对应的 `TIM_CounterMode_xxx` 宏值；
 * 若参数非法，返回 0。
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
 * @brief 将 BSP 时钟分频枚举转换为标准库 CKD 宏定义
 * @param[in] clock_div
 * BSP 层时钟分频枚举值。
 *
 * @return
 * 返回标准外设库对应的 `TIM_CKD_DIVx` 宏值；
 * 若参数非法，返回 0。
 *
 * @note
 * 该分频并不等同于计数器预分频 PSC，容易混淆。
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
 * @brief 将 PWM 输入通道转换为标准库输入捕获通道宏
 * @param[in] pwm_channel
 * PWM 输入通道。
 *
 * @return
 * - `TIM_Channel_1`
 * - `TIM_Channel_2`
 * - 参数非法时返回 0
 */
static uint16_t BSP_TIM_ConvertICChannel(bsp_tim_pwm_input_channel_t pwm_channel)
{
    switch(pwm_channel){
        case BSP_TIM_PWM_INPUT_CHANNEL1: return TIM_Channel_1;
        case BSP_TIM_PWM_INPUT_CHANNEL2: return TIM_Channel_2;
        default: return 0;
    }
}

/**
 * @brief 将 PWM 输入通道转换为对应的捕获比较中断使能位
 * @param[in] pwm_channel
 * PWM 输入通道。
 *
 * @return
 * - `TIM_IT_CC1`
 * - `TIM_IT_CC2`
 * - 参数非法时返回 0
 *
 * @note
 * 这里只使能“主输入源通道”的 CC 中断，配合 PWM 输入模式完成测量。
 */
static uint16_t BSP_TIM_ConvertICITFlag(bsp_tim_pwm_input_channel_t pwm_channel)
{
    switch(pwm_channel){
        case BSP_TIM_PWM_INPUT_CHANNEL1: return TIM_IT_CC1;
        case BSP_TIM_PWM_INPUT_CHANNEL2: return TIM_IT_CC2;
        default: return 0;
    }
}

/**
 * @brief 将 BSP 输入捕获极性枚举转换为标准库宏
 * @param[in] ic_polarity
 * 输入捕获极性。
 *
 * @return
 * 标准库 `TIM_ICPolarity_xxx` 宏值，非法参数返回 0。
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
 * @param[in] ic_prescaler_div
 * 输入捕获预分频设置。
 *
 * @return
 * 标准库 `TIM_ICPSC_DIVx` 宏值，非法参数返回 0。
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
 * @brief 配置定时器捕获比较中断对应的 NVIC
 * @param[in] timer_id
 * 高级定时器编号。
 *
 * @return
 * 无返回值。
 */
static void BSP_TIM_NVIC_Config(bsp_advancedtimer_t timer_id)
{
    NVIC_InitTypeDef NVIC_InitStruct;
    const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];

    NVIC_InitStruct.NVIC_IRQChannel = hw->irq_cc_channel;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = hw->irq_cc_pre_prio;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = hw->irq_cc_sub_prio;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
 * @brief 初始化高级定时器 PWM 输入测量功能
 * @details
 * 该函数完成高级定时器的核心初始化，包括：
 * - 定时器基础参数配置
 * - PWM 输入模式配置
 * - 从模式复位配置
 * - 触发源配置
 * - 中断配置
 * - 启动定时器
 *
 * @param[in] timer_id
 * 高级定时器编号。
 *
 * @param[in] base_config
 * 基础定时器配置。
 *
 * @param[in] pwm_config
 * PWM 输入配置。
 *
 * @return
 * 无返回值。
 *
 * @note
 * `TIM_PWMIConfig()` 会自动配置成配对输入捕获模式。
 * 因此后续读取 Capture1 / Capture2 时，含义与输入源通道有关。
 */
static void BSP_AdvancedTIM_Init(bsp_advancedtimer_t timer_id,
            bsp_advancedtimer_config_t *base_config,
            bsp_tim_pwm_config_t *pwm_config)
{
    if(timer_id >= BSP_ADVANCED_TIMER_MAX) return;

    const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_ICInitTypeDef TIM_ICInitStruct;
    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
    TIM_ICStructInit(&TIM_ICInitStruct);

    RCC_APB2PeriphClockCmd(hw->rcc_clk, ENABLE); /**< 开启 TIMx 时钟 */

    /* 配置定时器基础参数 */
    TIM_TimeBaseInitStruct.TIM_Prescaler = base_config->prescaler;
    TIM_TimeBaseInitStruct.TIM_CounterMode = BSP_TIM_ConvertCounterMode(base_config->counter_mode);
    TIM_TimeBaseInitStruct.TIM_Period = base_config->period;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = BSP_TIM_ConvertClockDivision(base_config->clock_div);
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = base_config->repetition_cnt;
    TIM_TimeBaseInit(hw->timer, &TIM_TimeBaseInitStruct);

    /* 启用 PSC 和 ARR 预装载 */
    TIM_PrescalerConfig(hw->timer, base_config->prescaler, TIM_PSCReloadMode_Update);
    TIM_ARRPreloadConfig(hw->timer, ENABLE);

    s_pwm_channel[timer_id] = pwm_config->pwm_channel;

    /* 配置 PWM 输入模式 */
    TIM_ICInitStruct.TIM_Channel = BSP_TIM_ConvertICChannel(pwm_config->pwm_channel);
    TIM_ICInitStruct.TIM_ICPrescaler = BSP_TIM_ConvertICPrescaler(pwm_config->ic_prescaler_div);
    TIM_ICInitStruct.TIM_ICPolarity = BSP_TIM_ConvertICPolarity(pwm_config->ic_polarity);
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStruct.TIM_ICFilter = pwm_config->ic_filter;
    TIM_PWMIConfig(hw->timer, &TIM_ICInitStruct);

    /**
     * @note
     * 使用从模式复位（Reset Mode）后，每次触发输入事件都会将计数器清零，
     * 这样一个周期内计数器累加值即可直接表示周期长度，便于测量 PWM 频率。
     */
    TIM_SelectSlaveMode(hw->timer, TIM_SlaveMode_Reset);

    /**
     * @note
     * 这里的触发源选择要与 PWM 输入源通道匹配：
     * - 选择 CH1 输入时，触发源设为 TI1FP1
     * - 选择 CH2 输入时，触发源设为 TI2FP2
     *
     * 若这里配置错误，周期测量结果往往会异常。
     */
    if(pwm_config->pwm_channel == BSP_TIM_PWM_INPUT_CHANNEL1){
        TIM_SelectInputTrigger(hw->timer, TIM_TS_TI1FP1);
    }else if(pwm_config->pwm_channel == BSP_TIM_PWM_INPUT_CHANNEL2){
        TIM_SelectInputTrigger(hw->timer, TIM_TS_TI2FP2);
    }

    TIM_ClearFlag(hw->timer, BSP_TIM_ConvertICITFlag(pwm_config->pwm_channel));

    /* 配置捕获比较中断并设置 NVIC */
    TIM_ITConfig(hw->timer, BSP_TIM_ConvertICITFlag(pwm_config->pwm_channel), ENABLE);
    BSP_TIM_NVIC_Config(timer_id);

    /* 启动定时器 */
    TIM_Cmd(hw->timer, ENABLE);
}

/**
 * @brief 对外提供的高级定时器 PWM 输入配置接口
 * @param[in] timer_id
 * 高级定时器编号。
 *
 * @param[in] base_config
 * 基础参数配置结构体指针。
 *
 * @param[in] pwm_config
 * PWM 输入参数配置结构体指针。
 *
 * @return
 * 无返回值。
 *
 * @details
 * 本函数是模块初始化入口，内部依次执行：
 * 1. GPIO 初始化
 * 2. 定时器 PWM 输入模式初始化
 */
void BSP_AdvancedTimer_Config(bsp_advancedtimer_t timer_id,
            bsp_advancedtimer_config_t *base_config,
            bsp_tim_pwm_config_t *pwm_config)
{
    BSP_TIM_GPIO_Init(timer_id, pwm_config->pwm_channel);
    BSP_AdvancedTIM_Init(timer_id, base_config, pwm_config);
}

/**
 * @brief 高级定时器捕获比较中断公共处理函数
 * @details
 * 当检测到对应捕获比较中断后：
 * - 读取周期值和高电平脉宽值
 * - 计算占空比
 * - 计算频率
 * - 置位更新标志
 * - 清除中断挂起位
 *
 * @param[in] timer_id
 * 高级定时器编号。
 *
 * @return
 * 无返回值。
 *
 * @note
 * 关于 Capture1 / Capture2 的含义：
 * - 当输入源为 CH1 时：
 *   - CCR1 通常对应周期
 *   - CCR2 通常对应高电平宽度
 * - 当输入源为 CH2 时：
 *   - CCR2 通常对应周期
 *   - CCR1 通常对应高电平宽度
 *
 * 这一点很容易误解，因此这里根据 `s_pwm_channel[timer_id]` 做了区分。
 */
void BSP_TIM_CC_IRQHandler(bsp_advancedtimer_t timer_id)
{
    if(timer_id >= BSP_ADVANCED_TIMER_MAX) return;

    const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];
    bsp_tim_pwm_input_channel_t pwm_channel = s_pwm_channel[timer_id];
    uint16_t it_flag = BSP_TIM_ConvertICITFlag(pwm_channel);
    bsp_tim_ic_result_t *res = &result[timer_id];

    if(TIM_GetITStatus(hw->timer, it_flag) != RESET){
        if(pwm_channel == BSP_TIM_PWM_INPUT_CHANNEL1){
            res->g_pwm_period = TIM_GetCapture1(hw->timer);
            res->g_pwm_pulse_width = TIM_GetCapture2(hw->timer);
        }else if(pwm_channel == BSP_TIM_PWM_INPUT_CHANNEL2){
            res->g_pwm_period = TIM_GetCapture2(hw->timer);
            res->g_pwm_pulse_width = TIM_GetCapture1(hw->timer);
        }

        /* 计算占空比和频率 */
        if(res->g_pwm_period != 0){
            res->g_pwm_duty = (float)res->g_pwm_pulse_width / res->g_pwm_period * 100.f;
            res->g_pwm_freq = CLK_FREQ / (hw->timer->PSC + 1) / res->g_pwm_period;
        }else{
            /**
             * @note
             * 理论上正常 PWM 输入下周期不应为 0。
             * 这里做保护处理，避免除 0 错误。
             */
            res->g_pwm_duty = 0.0f;
            res->g_pwm_freq = 0;
        }
        res->g_pwm_update = 1;

        TIM_ClearITPendingBit(hw->timer, it_flag);
    }
}