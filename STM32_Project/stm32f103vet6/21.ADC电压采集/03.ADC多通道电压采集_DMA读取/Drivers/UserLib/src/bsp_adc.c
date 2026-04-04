#include "bsp_adc.h"

/**
 * @file    bsp_adc.c
 * @brief   ADC 分组采样驱动源文件
 * @details
 * 本文件实现 ADC 分组初始化和数据读取功能，支持：
 * - 多通道规则组采样
 * - DMA 循环搬运
 * - ADC1 / ADC2 / ADC3 统一硬件映射
 */

/**
 * @brief ADC 硬件资源映射结构体
 * @details
 * 每个 ADC 设备对应一组固定硬件资源，包括：
 * - ADC 实例
 * - RCC 时钟
 * - 中断信息
 * - DMA 通道
 */
typedef struct{
	ADC_TypeDef 	*adc;           /**< ADC 外设实例 */
	uint32_t 		rcc_clk;        /**< ADC 时钟 */
	uint8_t 		irq_channel;    /**< 中断通道 */
	uint8_t 		irq_pre_prio;   /**< 抢占优先级 */
	uint8_t			irq_sub_prio;   /**< 子优先级 */
	bsp_dma_channel_t dma_channel; /**< 对应 DMA 通道 */
}bsp_adc_hw_t;

/**
 * @brief ADC 硬件资源表
 * @details
 * 根据 ADC 设备号索引对应硬件资源。
 *
 * @note
 * 当前配置中：
 * - ADC1 对应 DMA1_Channel1
 * - ADC2 未配置 DMA，使用 BSP_DMA_NONE
 * - ADC3 对应 DMA2_Channel5
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
 * @brief 初始化一组 ADC 通道
 * @param cfg ADC 分组配置结构体指针
 * @return 无
 * @details
 * 本函数支持单通道或多通道 ADC 规则组采样。
 *
 * 初始化流程如下：
 * 1. 检查配置参数合法性
 * 2. 开启 ADC 时钟
 * 3. 初始化所有输入引脚为模拟输入
 * 4. 配置 ADC 参数
 * 5. 配置 ADC 时钟分频
 * 6. 配置规则组通道顺序
 * 7. 配置 DMA（若当前 ADC 支持 DMA）
 * 8. 使能 ADC
 * 9. 复位校准并启动校准
 * 10. 软件触发开始转换
 *
 * @note
 * 当 `channel_count > 1` 时，会自动开启扫描模式。
 *
 * @warning
 * 当前若选择 ADC2，由于其 DMA 通道配置为 `BSP_DMA_NONE`，
 * 则不会启用 DMA 自动搬运。
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

	/* 初始化所有 ADC 输入引脚为模拟输入 */
	for(int i = 0; i < cfg->channel_count; i++){
		const bsp_adc_input_t *input = &cfg->input_list[i];

		RCC_APB2PeriphClockCmd(input->adc_gpio.rcc_clk, ENABLE);

		GPIO_InitStruct.GPIO_Pin = input->adc_gpio.gpio_pin;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(input->adc_gpio.gpio_port, &GPIO_InitStruct);
	}

	/* 配置 ADC 基本参数 */
	ADC_InitStruct.ADC_Mode = cfg->adc_mode;
	ADC_InitStruct.ADC_ScanConvMode = (cfg->channel_count > 1) ? ENABLE : DISABLE;
	ADC_InitStruct.ADC_ContinuousConvMode = cfg->continuous_mode;
	ADC_InitStruct.ADC_ExternalTrigConv = cfg->trigger_source;
	ADC_InitStruct.ADC_DataAlign = cfg->align;
	ADC_InitStruct.ADC_NbrOfChannel = cfg->channel_count;
	ADC_Init(hw->adc, &ADC_InitStruct);

	/* 配置 ADC 时钟：72MHz / 8 = 9MHz */
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);

	/* 配置 ADC 规则组的通道顺序和采样时间 */
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

		/* 使能 ADC DMA 请求 */
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
 * @brief 获取指定通道下标对应的 ADC 采样值
 * @param cfg ADC 分组配置结构体指针
 * @param index 通道下标
 * @return 当前采样值；若参数非法返回 0
 * @details
 * 该函数本质上是从 DMA 缓冲区中取值。
 */
uint16_t BSP_ADC_GetValue(const bsp_adc_group_config_t *cfg, uint8_t index)
{
	if(cfg == NULL || index >= cfg->channel_count)	return 0;

	return cfg->buffer[index];
}