#include "bsp_adc.h"

typedef struct{
	ADC_TypeDef *adc;
	uint8_t channel;
	uint32_t rcc_clk;

	bsp_gpio_t adc_gpio;	

	uint8_t 		irq_channel;
	uint8_t 		irq_pre_prio;
	uint8_t			irq_sub_prio;

	bsp_dma_channel_t dma_channel;
}bsp_adc_hw_t;

volatile uint16_t adc_value = 0;

static const bsp_adc_hw_t bsp_adc_hw[BSP_ADC_Channel_MAX] = {

	[BSP_ADC1_Channel0] = {
		.adc = ADC1,
		.channel = ADC_Channel_0,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOA, GPIO_Pin_0, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},	

	[BSP_ADC1_Channel1] = {
		.adc = ADC1,
		.channel = ADC_Channel_1,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOA, GPIO_Pin_1, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel2] = {
		.adc = ADC1,
		.channel = ADC_Channel_2,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOA, GPIO_Pin_2, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel3] = {
		.adc = ADC1,
		.channel = ADC_Channel_3,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOA, GPIO_Pin_3, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel4] = {
		.adc = ADC1,
		.channel = ADC_Channel_4,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOA, GPIO_Pin_4, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel5] = {
		.adc = ADC1,
		.channel = ADC_Channel_5,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOA, GPIO_Pin_5, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel6] = {
		.adc = ADC1,
		.channel = ADC_Channel_6,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOA, GPIO_Pin_6, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel7] = {
		.adc = ADC1,
		.channel = ADC_Channel_7,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOA, GPIO_Pin_7, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel8] = {
		.adc = ADC1,
		.channel = ADC_Channel_8,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOB, GPIO_Pin_0, RCC_APB2Periph_GPIOB},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel9] = {
		.adc = ADC1,
		.channel = ADC_Channel_9,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOB, GPIO_Pin_1, RCC_APB2Periph_GPIOB},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel10] = {
		.adc = ADC1,
		.channel = ADC_Channel_10,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel11] = {
		.adc = ADC1,
		.channel = ADC_Channel_11,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel12] = {
		.adc = ADC1,
		.channel = ADC_Channel_12,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel13] = {
		.adc = ADC1,
		.channel = ADC_Channel_13,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel14] = {
		.adc = ADC1,
		.channel = ADC_Channel_14,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOC, GPIO_Pin_4, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC1_Channel15] = {
		.adc = ADC1,
		.channel = ADC_Channel_15,
		.rcc_clk = RCC_APB2Periph_ADC1,

		.adc_gpio = {GPIOC, GPIO_Pin_5, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA1_Channel1,
	},

	[BSP_ADC2_Channel0] = {
		.adc = ADC2,
		.channel = ADC_Channel_0,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOA, GPIO_Pin_0, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel1] = {
		.adc = ADC2,
		.channel = ADC_Channel_1,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOA, GPIO_Pin_1, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel2] = {
		.adc = ADC2,
		.channel = ADC_Channel_2,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOA, GPIO_Pin_2, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel3] = {
		.adc = ADC2,
		.channel = ADC_Channel_3,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOA, GPIO_Pin_3, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel4] = {
		.adc = ADC2,
		.channel = ADC_Channel_4,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOA, GPIO_Pin_4, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel5] = {
		.adc = ADC2,
		.channel = ADC_Channel_5,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOA, GPIO_Pin_5, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel6] = {
		.adc = ADC2,
		.channel = ADC_Channel_6,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOA, GPIO_Pin_6, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel7] = {
		.adc = ADC2,
		.channel = ADC_Channel_7,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOA, GPIO_Pin_7, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel8] = {
		.adc = ADC2,
		.channel = ADC_Channel_8,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOB, GPIO_Pin_0, RCC_APB2Periph_GPIOB},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel9] = {
		.adc = ADC2,
		.channel = ADC_Channel_9,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOB, GPIO_Pin_1, RCC_APB2Periph_GPIOB},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel10] = {
		.adc = ADC2,
		.channel = ADC_Channel_10,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel11] = {
		.adc = ADC2,
		.channel = ADC_Channel_11,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel12] = {
		.adc = ADC2,
		.channel = ADC_Channel_12,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel13] = {
		.adc = ADC2,
		.channel = ADC_Channel_13,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel14] = {
		.adc = ADC2,
		.channel = ADC_Channel_14,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOC, GPIO_Pin_4, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC2_Channel15] = {
		.adc = ADC2,
		.channel = ADC_Channel_15,
		.rcc_clk = RCC_APB2Periph_ADC2,

		.adc_gpio = {GPIOC, GPIO_Pin_5, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC1_2_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,
	},

	[BSP_ADC3_Channel0] = {
		.adc = ADC3,
		.channel = ADC_Channel_0,
		.rcc_clk = RCC_APB2Periph_ADC3,

		.adc_gpio = {GPIOA, GPIO_Pin_0, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC3_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA2_Channel5,
	},

	[BSP_ADC3_Channel1] = {
		.adc = ADC3,
		.channel = ADC_Channel_1,
		.rcc_clk = RCC_APB2Periph_ADC3,

		.adc_gpio = {GPIOA, GPIO_Pin_1, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC3_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA2_Channel5,
	},

	[BSP_ADC3_Channel2] = {
		.adc = ADC3,
		.channel = ADC_Channel_2,
		.rcc_clk = RCC_APB2Periph_ADC3,

		.adc_gpio = {GPIOA, GPIO_Pin_2, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC3_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA2_Channel5,
	},

	[BSP_ADC3_Channel3] = {
		.adc = ADC3,
		.channel = ADC_Channel_3,
		.rcc_clk = RCC_APB2Periph_ADC3,

		.adc_gpio = {GPIOA, GPIO_Pin_3, RCC_APB2Periph_GPIOA},

		.irq_channel = ADC3_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA2_Channel5,
	},

	[BSP_ADC3_Channel10] = {
		.adc = ADC3,
		.channel = ADC_Channel_10,
		.rcc_clk = RCC_APB2Periph_ADC3,

		.adc_gpio = {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC3_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA2_Channel5,
	},

	[BSP_ADC3_Channel11] = {
		.adc = ADC3,
		.channel = ADC_Channel_11,
		.rcc_clk = RCC_APB2Periph_ADC3,

		.adc_gpio = {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC3_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA2_Channel5,
	},

	[BSP_ADC3_Channel12] = {
		.adc = ADC3,
		.channel = ADC_Channel_12,
		.rcc_clk = RCC_APB2Periph_ADC3,

		.adc_gpio = {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC3_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA2_Channel5,
	},

	[BSP_ADC3_Channel13] = {
		.adc = ADC3,
		.channel = ADC_Channel_13,
		.rcc_clk = RCC_APB2Periph_ADC3,

		.adc_gpio = {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC},

		.irq_channel = ADC3_IRQn,
		.irq_pre_prio = PREEMPT_PRIO,
		.irq_sub_prio = SUB_PRIO,

		.dma_channel = BSP_DMA2_Channel5,
	},
};

void BSP_ADC_Init(bsp_adc_channel_t adc_channel_id, bsp_adc_config_t *config)
{
	if(adc_channel_id >= BSP_ADC_Channel_MAX || config == NULL)	return;	

	const bsp_adc_hw_t *hw = &bsp_adc_hw[adc_channel_id];
	GPIO_InitTypeDef GPIO_InitStruct;
	ADC_InitTypeDef ADC_InitStruct;
	bsp_dma_config_t dma_config;

	/* 初始化结构体(以默认值来填充结构体成员) */
	GPIO_StructInit(&GPIO_InitStruct);
	ADC_StructInit(&ADC_InitStruct);

	/* 开启 GPIO 和 ADC 的时钟 */
	RCC_APB2PeriphClockCmd(hw->adc_gpio.rcc_clk, ENABLE);
	RCC_APB2PeriphClockCmd(hw->rcc_clk, ENABLE);

	/* 初始化 ADC 用到的引脚 */
	GPIO_InitStruct.GPIO_Pin = hw->adc_gpio.gpio_pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(hw->adc_gpio.gpio_port, &GPIO_InitStruct);

	/* 配置 ADC 结构体成员 */
	ADC_InitStruct.ADC_Mode = config->adc_mode;
	ADC_InitStruct.ADC_ScanConvMode = config->scan_mode;
	ADC_InitStruct.ADC_ContinuousConvMode = config->continuous_mode;
	ADC_InitStruct.ADC_ExternalTrigConv = config->trigger_source;
	ADC_InitStruct.ADC_DataAlign = config->align;
	ADC_InitStruct.ADC_NbrOfChannel = config->channel_count;
	ADC_Init(hw->adc, &ADC_InitStruct);

	/* 配置 DMA 结构体成员 */
	dma_config.periph_addr = (uint32_t)&(hw->adc->DR);
	dma_config.memory_addr = (uint32_t)&adc_value;
	dma_config.dir 			= DIR_Periph_SRC;
	dma_config.buffer_size = 1;
	dma_config.periph_inc = PeripheralInc_Disable;
	dma_config.memory_inc = MemoryInc_Disable;
	dma_config.periph_data_size = PeripheralDataSize_HalfWord;
	dma_config.memory_data_size = MemoryDataSize_HalfWord;
	dma_config.mode = DMA_Mode_Cir;
	dma_config.priority = DMA_Priority_M;
	dma_config.m2m = DMA_M2M_DISABLE;

	BSP_DMA_Init(hw->dma_channel, &dma_config);

	/* 配置 ADC时钟 (72M / 8 = 9MHz) */
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);

	/* 配置通道的转换顺序和采样时间 */
	ADC_RegularChannelConfig(hw->adc, hw->channel, 1, config->sample_time);
	
	/* 使能 DMA 搬运数据 */
	ADC_DMACmd(hw->adc, ENABLE);

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

uint16_t BSP_ADC_GetValue(bsp_adc_channel_t adc_channel_id)
{
	if(adc_channel_id >= BSP_ADC_Channel_MAX)	return 0;

	const bsp_adc_hw_t *hw = &bsp_adc_hw[adc_channel_id];

	return ADC_GetConversionValue(hw->adc);
}
