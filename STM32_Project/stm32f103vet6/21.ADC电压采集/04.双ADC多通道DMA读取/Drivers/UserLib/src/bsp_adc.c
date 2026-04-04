#include "bsp_adc.h"

/**
 * @file    bsp_adc.c
 * @brief   ADC 驱动源文件
 * @details
 * 本文件实现：
 * - 单 ADC 分组采样初始化
 * - 双 ADC 同步采样初始化
 * - ADC 硬件资源映射
 * - DMA 数据搬运配置
 * - ADC 校准与启动
 */

/**
 * @brief ADC 硬件资源映射结构体
 * @details
 * 描述一个 ADC 外设实例所对应的底层硬件信息，包括：
 * - ADC 实例地址
 * - RCC 时钟使能位
 * - 中断信息
 * - DMA 通道
 */
typedef struct{
	ADC_TypeDef 	*adc;           /**< ADC 外设实例 */
	uint32_t 		rcc_clk;        /**< ADC 时钟使能位 */
	uint8_t 		irq_channel;    /**< 中断通道 */
	uint8_t 		irq_pre_prio;   /**< 抢占优先级 */
	uint8_t			irq_sub_prio;   /**< 子优先级 */
	bsp_dma_channel_t dma_channel; /**< DMA 通道 */
}bsp_adc_hw_t;

/**
 * @brief 保留的 ADC 采样值变量
 * @note
 * 当前这版实现中未实际使用该变量。
 * 单 ADC 模式使用 `cfg->buffer`，
 * 双 ADC 模式使用 `uint32_t` 缓冲区。
 */
volatile uint16_t adc_value = 0;

/**
 * @brief ADC 硬件资源映射表
 * @details
 * 根据 @ref bsp_adc_dev_id_t 将逻辑 ADC 设备号映射到底层硬件资源。
 *
 * 当前配置：
 * - ADC1 -> DMA1_Channel1
 * - ADC2 -> 无 DMA
 * - ADC3 -> DMA2_Channel5
 */
static const bsp_adc_hw_t bsp_adc_hw[BSP_ADC_DEV_MAX] = {
	[BSP_ADC_DEV1] = {
		.adc = ADC1,
		.rcc_clk = RCC_APB2Periph_ADC1,
		
		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC_DEV2] = {
		.adc = ADC2,
		.rcc_clk = RCC_APB2Periph_ADC2,
		
		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA_NONE,
	},

	[BSP_ADC_DEV3] = {
		.adc = ADC3,
		.rcc_clk = RCC_APB2Periph_ADC3,
		
		.irq_channel = ADC3_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA2_Channel5,
	}
};

/**
 * @brief 初始化单 ADC 分组采样
 * @param cfg 单 ADC 分组配置结构体指针
 * @return 无
 * @details
 * 初始化流程如下：
 * 1. 检查参数合法性
 * 2. 开启 ADC 时钟
 * 3. 初始化所有输入引脚为模拟输入
 * 4. 配置 ADC 基本参数
 * 5. 配置 ADC 时钟分频
 * 6. 配置规则组通道顺序和采样时间
 * 7. 初始化 DMA 并使能 ADC DMA 请求
 * 8. 使能 ADC
 * 9. 进行 ADC 校准
 * 10. 软件触发启动转换
 *
 * @note
 * 当 `channel_count > 1` 时，自动开启扫描模式。
 */
void BSP_ADC_InitGroup(const bsp_adc_group_config_t *cfg)
{
	if(cfg == NULL || cfg->dev_id >= BSP_ADC_DEV_MAX || cfg->input_list == NULL || 
						cfg->buffer == NULL || cfg->channel_count == 0)		return;

	const bsp_adc_hw_t *hw = &bsp_adc_hw[cfg->dev_id];
	GPIO_InitTypeDef GPIO_InitStruct;
	ADC_InitTypeDef ADC_InitStruct;
	bsp_dma_config_t dma_config;

	GPIO_StructInit(&GPIO_InitStruct);
	ADC_StructInit(&ADC_InitStruct);

	/* 开启 ADC 时钟 */
	RCC_APB2PeriphClockCmd(hw->rcc_clk, ENABLE);

	/* 初始化所有 ADC 输入引脚 */
	for(int i = 0; i < cfg->channel_count; i++){
		const bsp_adc_input_t *input = &cfg->input_list[i];

		RCC_APB2PeriphClockCmd(input->adc_gpio.rcc_clk, ENABLE);

		GPIO_InitStruct.GPIO_Pin = input->adc_gpio.gpio_pin;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(input->adc_gpio.gpio_port, &GPIO_InitStruct);
	}

	/* 配置 ADC */
	ADC_InitStruct.ADC_Mode = cfg->adc_mode;
	ADC_InitStruct.ADC_ScanConvMode = (cfg->channel_count > 1) ? ENABLE : DISABLE;
	ADC_InitStruct.ADC_ContinuousConvMode = cfg->continuous_mode;
	ADC_InitStruct.ADC_ExternalTrigConv = cfg->trigger_source;
	ADC_InitStruct.ADC_DataAlign = cfg->align;
	ADC_InitStruct.ADC_NbrOfChannel = cfg->channel_count;
	ADC_Init(hw->adc, &ADC_InitStruct);

	/* 配置 ADC 时钟 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);

	/* 配置规则组转换顺序和采样时间 */
	for(int i = 0; i < cfg->channel_count; i++){
		ADC_RegularChannelConfig(hw->adc, cfg->input_list[i].adc_channel, i + 1, cfg->sample_time);
	}

	/* 配置 DMA */
	if(hw->dma_channel != BSP_DMA_NONE){
		dma_config.periph_addr = (uint32_t)&(hw->adc->DR);
		dma_config.memory_addr = (uint32_t)(cfg->buffer);
		dma_config.dir 			= DIR_Periph_SRC;
		dma_config.buffer_size = cfg->channel_count;
		dma_config.periph_inc = PeripheralInc_Disable;
		dma_config.memory_inc = (cfg->channel_count > 1) ? MemoryInc_Enable : MemoryInc_Disable;
		dma_config.periph_data_size = PeripheralDataSize_HalfWord;
		dma_config.memory_data_size = MemoryDataSize_HalfWord;
		dma_config.mode = DMA_Mode_Cir;
		dma_config.priority = DMA_Priority_M;
		dma_config.m2m = DMA_M2M_DISABLE;

		BSP_DMA_Init(hw->dma_channel, &dma_config);

		/* 使能 DMA 请求 */
		ADC_DMACmd(hw->adc, ENABLE);
	}

	/* 使能 ADC */
	ADC_Cmd(hw->adc, ENABLE);

	/* 复位并校准 ADC */
	ADC_ResetCalibration(hw->adc);
	while(ADC_GetResetCalibrationStatus(hw->adc));		/**< 等待复位完成 */

	ADC_StartCalibration(hw->adc);
	while(ADC_GetCalibrationStatus(hw->adc));			/**< 等待校准完成 */

	/* 软件触发 ADC 转换 */
	ADC_SoftwareStartConvCmd(hw->adc, ENABLE);
}

/**
 * @brief 初始化双 ADC 同步采样
 * @param cfg 双 ADC 同步配置结构体指针
 * @return 无
 * @details
 * 本函数实现 ADC1 + ADC2 规则组同步采样，使用：
 * - ADC1 作为主 ADC
 * - ADC2 作为从 ADC
 * - DMA 从 ADC1->DR 读取 32 位结果
 *
 * 在同步模式下，DMA 每次搬运得到一个 32 位结果：
 * - 低 16 位：ADC1 结果
 * - 高 16 位：ADC2 结果
 *
 * @warning
 * 当前要求 ADC1 和 ADC2 的规则组通道数量必须相等，
 * 否则无法按“成对数据”正确存储到 32 位缓冲区中。
 */
void BSP_DualADC_InitGroup(const bsp_dual_adc_config_t *cfg)
{
    if(cfg == NULL || cfg->adc1_inputs == NULL || cfg->adc2_inputs == NULL ||
       cfg->buffer == NULL || cfg->adc1_channel_count == 0 ||
       cfg->adc1_channel_count != cfg->adc2_channel_count) return;

    GPIO_InitTypeDef GPIO_InitStruct;
    ADC_InitTypeDef  ADC_InitStruct;
    bsp_dma_config_t dma_config;

    uint8_t ch_count = cfg->adc1_channel_count;

    /* ---- GPIO 初始化（ADC1 和 ADC2 的引脚都配置为模拟输入）---- */
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;

    for(int i = 0; i < ch_count; i++){
        const bsp_adc_input_t *p = &cfg->adc1_inputs[i];
        RCC_APB2PeriphClockCmd(p->adc_gpio.rcc_clk, ENABLE);
        GPIO_InitStruct.GPIO_Pin = p->adc_gpio.gpio_pin;
        GPIO_Init(p->adc_gpio.gpio_port, &GPIO_InitStruct);
    }

    for(int i = 0; i < ch_count; i++){
        const bsp_adc_input_t *p = &cfg->adc2_inputs[i];
        RCC_APB2PeriphClockCmd(p->adc_gpio.rcc_clk, ENABLE);
        GPIO_InitStruct.GPIO_Pin = p->adc_gpio.gpio_pin;
        GPIO_Init(p->adc_gpio.gpio_port, &GPIO_InitStruct);
    }

    /* ---- 开启 ADC1 和 ADC2 时钟 ---- */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    /* ---- ADC1：主 ADC ---- */
    ADC_StructInit(&ADC_InitStruct);
    ADC_InitStruct.ADC_Mode               = ADC_Mode_RegSimult; // 同步规则通道模式
    ADC_InitStruct.ADC_ScanConvMode       = (ch_count > 1) ? ENABLE : DISABLE;
    ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStruct.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
    ADC_InitStruct.ADC_DataAlign          = ADC_DataAlign_Right;
    ADC_InitStruct.ADC_NbrOfChannel       = ch_count;
    ADC_Init(ADC1, &ADC_InitStruct);

    for(int i = 0; i < ch_count; i++){
        ADC_RegularChannelConfig(ADC1, cfg->adc1_inputs[i].adc_channel,
                                 i + 1, cfg->sample_time);
    }

    /* ---- ADC2：从 ADC，规则组顺序需与 ADC1 对齐 ---- */
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_Init(ADC2, &ADC_InitStruct);

    for(int i = 0; i < ch_count; i++){
        ADC_RegularChannelConfig(ADC2, cfg->adc2_inputs[i].adc_channel,
                                 i + 1, cfg->sample_time);
    }

    /* 允许 ADC2 跟随 ADC1 触发 */
    ADC_ExternalTrigConvCmd(ADC2, ENABLE);

    /* ---- DMA：仅挂在 ADC1 上，搬运 32 位合并结果 ---- */
    dma_config.periph_addr      = (uint32_t)&ADC1->DR;
    dma_config.memory_addr      = (uint32_t)(cfg->buffer);
    dma_config.dir              = DIR_Periph_SRC;
    dma_config.buffer_size      = ch_count;
    dma_config.periph_inc       = PeripheralInc_Disable;
    dma_config.memory_inc       = (ch_count > 1) ? MemoryInc_Enable : MemoryInc_Disable;
    dma_config.periph_data_size = PeripheralDataSize_Word;
    dma_config.memory_data_size = MemoryDataSize_Word;
    dma_config.mode             = DMA_Mode_Cir;
    dma_config.priority         = DMA_Priority_H;
    dma_config.m2m              = DMA_M2M_DISABLE;

    BSP_DMA_Init(BSP_DMA1_Channel1, &dma_config);
    ADC_DMACmd(ADC1, ENABLE);

    /* ---- 使能并校准 ADC1 / ADC2 ---- */
    ADC_Cmd(ADC1, ENABLE);
    ADC_Cmd(ADC2, ENABLE);

    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

    ADC_ResetCalibration(ADC2);
    while(ADC_GetResetCalibrationStatus(ADC2));
    ADC_StartCalibration(ADC2);
    while(ADC_GetCalibrationStatus(ADC2));

    /* ADC1 软件触发开始转换，ADC2 自动同步 */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/**
 * @brief 读取单 ADC 缓冲区中的采样值
 * @param cfg 单 ADC 分组配置结构体指针
 * @param index 通道下标
 * @return 当前采样值；若参数非法则返回 0
 */
uint16_t BSP_ADC_GetValue(const bsp_adc_group_config_t *cfg, uint8_t index)
{
	if(cfg == NULL || index >= cfg->channel_count)	return 0;

	return cfg->buffer[index];
}