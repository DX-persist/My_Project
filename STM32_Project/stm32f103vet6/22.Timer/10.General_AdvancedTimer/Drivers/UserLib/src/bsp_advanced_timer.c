/**
 * @file bsp_advanced_timer.c
 * @brief STM32F10x 高级定时器 BSP 驱动源文件
 * @author
 * @date
 * @version 1.0
 *
 * @details
 * 本文件实现了高级定时器 TIM1 和 TIM8 的 PWM 输入捕获功能，
 * 主要包括：
 * - GPIO 输入引脚初始化
 * - 定时器基础计数参数初始化
 * - PWM Input 模式配置
 * - 捕获比较中断配置
 * - PWM 周期、脉宽、占空比、频率测量
 *
 * 本实现基于 STM32 标准外设库的 TIM_PWMIConfig() 接口，
 * 适用于对外部 PWM 信号进行测量分析。
 */

#include "bsp_advanced_timer.h"

/**
 * @struct bsp_advancedtimer_hw_t
 * @brief 高级定时器硬件资源描述结构体。
 *
 * 用于保存某个高级定时器实例的硬件资源信息，包括：
 * - TIM 外设指针
 * - RCC 时钟使能位
 * - 捕获比较中断信息
 * - CH1 / CH2 对应的 GPIO 引脚信息
 */
typedef struct{
	/**< TIM 外设寄存器基地址 */
	TIM_TypeDef *timer;

    /**< TIM 外设 RCC 时钟使能位 */
	uint32_t rcc_clk;

    /**< 捕获比较中断号 */
	uint8_t irq_cc_channel;

    /**< 捕获比较中断抢占优先级 */
	uint8_t irq_cc_pre_prio;

    /**< 捕获比较中断子优先级 */
	uint8_t irq_cc_sub_prio;

    /**< 通道 1 对应 GPIO */
	bsp_gpio_t ch1_gpio;

    /**< 通道 2 对应 GPIO */
    bsp_gpio_t ch2_gpio;
}bsp_advancedtimer_hw_t;

/**
 * @brief PWM 输入捕获结果数组定义。
 *
 * 每个高级定时器实例各自维护一份捕获测量结果。
 */
bsp_tim_ic_result_t result[BSP_ADVANCED_TIMER_MAX] = {0};

/**
 * @brief 记录每个高级定时器当前选择的 PWM 输入通道。
 *
 * 该数组用于中断处理函数中判断当前应读取哪个捕获寄存器。
 */
static bsp_tim_pwm_input_channel_t s_pwm_channel[BSP_ADVANCED_TIMER_MAX];

/**
 * @brief 高级定时器硬件资源映射表。
 *
 * 建立逻辑高级定时器编号到具体硬件资源的映射关系。
 */
static const bsp_advancedtimer_hw_t bsp_advancedtimer_hw[BSP_ADVANCED_TIMER_MAX] = {
	[BSP_ADVANCED_TIMER1] = {
		.timer = TIM1,
		.rcc_clk = RCC_APB2Periph_TIM1,

		.irq_cc_channel 	= TIM1_CC_IRQn,
		.irq_cc_pre_prio	= CC_PREEPT_PRIO,
		.irq_cc_sub_prio	= CC_SUB_PRIO, 

		.ch1_gpio  = {GPIOA, GPIO_Pin_8, RCC_APB2Periph_GPIOA},
        .ch2_gpio  = {GPIOA, GPIO_Pin_9, RCC_APB2Periph_GPIOA},
	},

	[BSP_ADVANCED_TIMER8] = {
		.timer = TIM8,
		.rcc_clk = RCC_APB2Periph_TIM8,

		.irq_cc_channel = TIM8_CC_IRQn,
		.irq_cc_pre_prio	= CC_PREEPT_PRIO,
		.irq_cc_sub_prio	= CC_SUB_PRIO, 

		.ch1_gpio  = {GPIOC, GPIO_Pin_6, RCC_APB2Periph_GPIOC},
        .ch2_gpio  = {GPIOC, GPIO_Pin_7, RCC_APB2Periph_GPIOC},
	}
};

/**
 * @brief 获取指定 PWM 输入通道对应的 GPIO 描述信息。
 *
 * @param hw 高级定时器硬件资源描述结构体指针
 * @param pwm_channel PWM 输入通道
 * @return 对应 GPIO 描述指针；若通道非法则返回 NULL
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
 * @brief 初始化高级定时器 PWM 输入对应的 GPIO。
 *
 * 本函数将选定通道的引脚初始化为浮空输入模式。
 *
 * @param timer_id 高级定时器编号
 * @param pwm_channel PWM 输入通道
 *
 * @note
 * 1. 当前使用 GPIO_Mode_IN_FLOATING 模式。
 * 2. 若外部信号源驱动能力较弱或对抗干扰要求较高，
 *    可根据实际场景考虑改为上拉/下拉输入。
 */
static void BSP_TIM_GPIO_Init(bsp_advancedtimer_t timer_id, bsp_tim_pwm_input_channel_t pwm_channel)
{
	if(timer_id >= BSP_ADVANCED_TIMER_MAX)	return;

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
 * @brief 将 BSP 抽象层计数模式转换为 STM32 标准库计数模式宏。
 *
 * @param counter_mode BSP 抽象层计数模式
 * @return 对应 TIM_CounterMode_xxx 宏值；若参数非法返回 0
 */
static uint16_t BSP_TIM_ConvertCounterMode(bsp_tim_counter_mode_t counter_mode)
{
	switch(counter_mode){
		case BSP_TIM_COUNTER_MODE_UP: return TIM_CounterMode_Up;
		case BSP_TIM_COUNTER_MODE_DOWN:	return TIM_CounterMode_Down;
		case BSP_TIM_COUNTER_MODE_CENTER_ALIGNED1:	return TIM_CounterMode_CenterAligned1;
		case BSP_TIM_COUNTER_MODE_CENTER_ALIGNED2:	return TIM_CounterMode_CenterAligned2;
		case BSP_TIM_COUNTER_MODE_CENTER_ALIGNED3:	return TIM_CounterMode_CenterAligned3;
		default:	return 0;
	}
}

/**
 * @brief 将 BSP 抽象层时钟分频配置转换为 STM32 标准库宏。
 *
 * @param clock_div BSP 抽象层时钟分频枚举
 * @return 对应 TIM_CKD_DIVx 宏值；若参数非法返回 0
 */
static uint16_t BSP_TIM_ConvertClockDivision(bsp_tim_clock_div_t clock_div)
{
	switch(clock_div){
		case BSP_TIM_CLOCK_DIV_1: 	return TIM_CKD_DIV1;
		case BSP_TIM_CLOCK_DIV_2:	return TIM_CKD_DIV2;
		case BSP_TIM_CLOCK_DIV_4:	return TIM_CKD_DIV4;
		default:	return 0;
	}
}

/**
 * @brief 将 PWM 输入通道枚举转换为 STM32 标准库输入捕获通道宏。
 *
 * @param pwm_channel PWM 输入通道
 * @return 对应 TIM_Channel_x 宏值；若参数非法返回 0
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
 * @brief 将 PWM 输入通道枚举转换为对应的捕获比较中断标志。
 *
 * @param pwm_channel PWM 输入通道
 * @return 对应 TIM_IT_CCx 宏值；若参数非法返回 0
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
 * @brief 将输入捕获极性枚举转换为 STM32 标准库极性宏。
 *
 * @param ic_polarity 输入捕获极性
 * @return 对应 TIM_ICPolarity_xxx 宏值；若参数非法返回 0
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
 * @brief 将输入捕获预分频枚举转换为 STM32 标准库输入捕获预分频宏。
 *
 * @param ic_prescaler_div 输入捕获预分频
 * @return 对应 TIM_ICPSC_DIVx 宏值；若参数非法返回 0
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
 * @brief 配置高级定时器捕获比较中断对应的 NVIC。
 *
 * @param timer_id 高级定时器编号
 *
 * @note
 * 本函数仅配置捕获比较中断，不涉及更新中断、刹车中断等其他中断源。
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
 * @brief 初始化高级定时器 PWM 输入捕获功能。
 *
 * 该函数完成以下配置：
 * - 开启定时器外设时钟
 * - 初始化定时器基础计数单元
 * - 配置 PWM 输入模式
 * - 设置从模式为复位模式
 * - 选择内部触发源
 * - 清除中断标志
 * - 使能捕获比较中断
 * - 配置 NVIC
 * - 启动定时器
 *
 * @param timer_id 高级定时器编号
 * @param base_config 基础参数配置结构体指针
 * @param pwm_config PWM 输入配置结构体指针
 */
static void BSP_AdvancedTIM_Init(bsp_advancedtimer_t timer_id, 
			bsp_advancedtimer_config_t *base_config, 
			bsp_tim_pwm_config_t *pwm_config)
{
	if(timer_id >= BSP_ADVANCED_TIMER_MAX)	return;

	const bsp_advancedtimer_hw_t *hw = &bsp_advancedtimer_hw[timer_id];
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_ICInitTypeDef TIM_ICInitStruct;

	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	TIM_ICStructInit(&TIM_ICInitStruct);
	
	RCC_APB2PeriphClockCmd(hw->rcc_clk, ENABLE);	/**< 开启 TIMx 时钟 */

	/* 初始化定时器基础参数 */
	TIM_TimeBaseInitStruct.TIM_Prescaler = base_config->prescaler;
	TIM_TimeBaseInitStruct.TIM_CounterMode = BSP_TIM_ConvertCounterMode(base_config->counter_mode);
	TIM_TimeBaseInitStruct.TIM_Period = base_config->period;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = BSP_TIM_ConvertClockDivision(base_config->clock_div);
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = base_config->repetition_cnt;
	TIM_TimeBaseInit(hw->timer, &TIM_TimeBaseInitStruct);
	
	/* 启用 Prescaler 和 ARR 预装载功能 */
	TIM_PrescalerConfig(hw->timer, base_config->prescaler, TIM_PSCReloadMode_Update);
	TIM_ARRPreloadConfig(hw->timer, ENABLE);

    /* 记录当前定时器所使用的 PWM 输入通道，供中断处理使用 */
	s_pwm_channel[timer_id] = pwm_config->pwm_channel;

    /* 配置 PWM 输入模式参数 */
	TIM_ICInitStruct.TIM_Channel = BSP_TIM_ConvertICChannel(pwm_config->pwm_channel);
	TIM_ICInitStruct.TIM_ICPrescaler = BSP_TIM_ConvertICPrescaler(pwm_config->ic_prescaler_div);
	TIM_ICInitStruct.TIM_ICPolarity = BSP_TIM_ConvertICPolarity(pwm_config->ic_polarity);
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStruct.TIM_ICFilter = pwm_config->ic_filter;
	TIM_PWMIConfig(hw->timer, &TIM_ICInitStruct);

	/**
     * @brief 设置定时器从模式为 Reset 模式。
     *
     * 在 PWM 输入模式下，常使用复位模式使计数器在触发沿到来时清零，
     * 从而方便测量信号周期。
     */
	TIM_SelectSlaveMode(hw->timer, TIM_SlaveMode_Reset);

	/**
     * @brief 选择触发源。
     *
     * 根据输入源选择 TI1FP1 或 TI2FP2 作为内部触发源。
     */
	if(pwm_config->pwm_channel == BSP_TIM_PWM_INPUT_CHANNEL1){
		TIM_SelectInputTrigger(hw->timer, TIM_TS_TI1FP1);
	}else if(pwm_config->pwm_channel == BSP_TIM_PWM_INPUT_CHANNEL2){
		TIM_SelectInputTrigger(hw->timer, TIM_TS_TI2FP2);
	}
	
    /* 清除可能存在的捕获比较中断标志 */
	TIM_ClearFlag(hw->timer, BSP_TIM_ConvertICITFlag(pwm_config->pwm_channel));

	/* 配置捕获比较中断及 NVIC */
	TIM_ITConfig(hw->timer, BSP_TIM_ConvertICITFlag(pwm_config->pwm_channel), ENABLE);
	BSP_TIM_NVIC_Config(timer_id);

    /* 启动定时器 */
	TIM_Cmd(hw->timer, ENABLE);
}

/**
 * @brief 配置高级定时器 PWM 输入捕获功能。
 *
 * 本函数为模块对外统一入口，依次完成：
 * - GPIO 初始化
 * - 高级定时器 PWM 输入初始化
 *
 * @param timer_id 高级定时器编号
 * @param base_config 基础参数配置结构体指针
 * @param pwm_config PWM 输入配置结构体指针
 */
void BSP_AdvancedTimer_Config(bsp_advancedtimer_t timer_id, 
			bsp_advancedtimer_config_t *base_config, 
			bsp_tim_pwm_config_t *pwm_config)
{
	BSP_TIM_GPIO_Init(timer_id, pwm_config->pwm_channel);
	BSP_AdvancedTIM_Init(timer_id, base_config, pwm_config);
}

/**
 * @brief 高级定时器捕获比较中断统一处理函数。
 *
 * 本函数在捕获比较中断发生时执行以下流程：
 * - 根据当前 PWM 输入通道读取对应的捕获寄存器
 * - 获取周期和高电平脉宽计数值
 * - 计算占空比与频率
 * - 置位数据更新标志
 * - 清除中断挂起位
 *
 * @param timer_id 高级定时器编号
 *
 * @note
 * 1. CH1 模式下：
 *    - Capture1 通常表示周期
 *    - Capture2 通常表示脉宽
 * 2. CH2 模式下：
 *    - Capture2 通常表示周期
 *    - Capture1 通常表示脉宽
 * 3. 若测得周期为 0，则占空比和频率均置 0，以避免除零。
 */
void BSP_TIM_CC_IRQHandler(bsp_advancedtimer_t timer_id)
{
	if(timer_id >= BSP_ADVANCED_TIMER_MAX)	return;
	
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
			res->g_pwm_duty = 0.0f;
			res->g_pwm_freq = 0;
		}

        /* 标记有新数据 */
		res->g_pwm_update = 1;

        /* 清除中断挂起位 */
		TIM_ClearITPendingBit(hw->timer, it_flag);
	}
}