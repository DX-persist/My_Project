#include "bsp_adc.h"

typedef struct{
	ADC_TypeDef 	*adc;
	uint32_t 		rcc_clk;
	uint8_t 		irq_channel;
	uint8_t 		irq_pre_prio;
	uint8_t			irq_sub_prio;
	bsp_dma_channel_t dma_channel;
}bsp_adc_hw_t;

volatile uint16_t adc_value = 0;

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

	/* 配置 ADC 通道转换顺序和采样时间 */
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
	while(ADC_GetResetCalibrationStatus(hw->adc));		/**< 等待复为完成 */

	ADC_StartCalibration(hw->adc);
	while(ADC_GetCalibrationStatus(hw->adc));			/**< 等待校准完成 */

	/* 软件触发ADC转化 */
	ADC_SoftwareStartConvCmd(hw->adc, ENABLE);
}

uint16_t BSP_ADC_GetValue(const bsp_adc_group_config_t *cfg, uint8_t index)
{
	if(cfg == NULL || index >= cfg->channel_count)	return 0;

	return cfg->buffer[index];
}
