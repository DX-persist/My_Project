#include "bsp_adc.h"

volatile uint32_t adc_value[ADC_CHANNEL_COUNT] = {0};

void BSP_ADC_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);

	/*open the rcc clock of GPIOC */
	RCC_APB2PeriphClockCmd(ADC_GPIO_CLK, ENABLE);

	/* Initialize ADC pins, including mode, pin, speed*/	
	GPIO_InitStruct.GPIO_Pin = ADC_GPIO0_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = ADC_GPIO1_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = ADC_GPIO2_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = ADC_GPIO3_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = ADC_GPIO4_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = ADC_GPIO5_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC_GPIO_PORT, &GPIO_InitStruct);
}

void BSP_ADC_Init(void)
{
	ADC_InitTypeDef ADC_InitStruct;
	DMA_InitTypeDef DMA_InitStruct;
	ADC_StructInit(&ADC_InitStruct);
	DMA_StructInit(&DMA_InitStruct);

	DMA_DeInit(ADC1_DMA_CHANNEL);			/**< config the dma to the default configure */

	/* open the rcc clock of the ADC1 */
	RCC_AHBPeriphClockCmd(DMA1_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(ADC1_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(ADC2_CLK, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);

	/* Initialize ADC structure, including mode, data align and so on*/
	ADC_InitStruct.ADC_Mode = ADC_Mode_RegSimult;
	ADC_InitStruct.ADC_ScanConvMode = (ADC_CHANNEL_COUNT > 1) ? ENABLE : DISABLE;
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_NbrOfChannel = ADC_CHANNEL_COUNT;
	ADC_Init(ADC1, &ADC_InitStruct);

	ADC_StructInit(&ADC_InitStruct);
	ADC_InitStruct.ADC_Mode = ADC_Mode_RegSimult;
	ADC_InitStruct.ADC_ScanConvMode = (ADC_CHANNEL_COUNT > 1) ? ENABLE : DISABLE;
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_NbrOfChannel = ADC_CHANNEL_COUNT;
	ADC_Init(ADC2, &ADC_InitStruct);

	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)adc_value;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStruct.DMA_BufferSize = ADC_CHANNEL_COUNT;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = (ADC_CHANNEL_COUNT > 1) ? DMA_MemoryInc_Enable : DMA_MemoryInc_Disable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(ADC1_DMA_CHANNEL, &DMA_InitStruct);
	DMA_Cmd(ADC1_DMA_CHANNEL, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);

	ADC_RegularChannelConfig(ADC1, ADC_GPIO0_CHANNEL, 1, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_GPIO1_CHANNEL, 2, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_GPIO2_CHANNEL, 3, ADC_SampleTime_55Cycles5);
	ADC_Cmd(ADC1, ENABLE);

	ADC_RegularChannelConfig(ADC2, ADC_GPIO3_CHANNEL, 1, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC2, ADC_GPIO4_CHANNEL, 2, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC2, ADC_GPIO5_CHANNEL, 3, ADC_SampleTime_55Cycles5);
	ADC_Cmd(ADC2, ENABLE);

	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));

	ADC_ResetCalibration(ADC2);
	while(ADC_GetResetCalibrationStatus(ADC2));
	ADC_StartCalibration(ADC2);
	while(ADC_GetCalibrationStatus(ADC2));

	ADC_ExternalTrigConvCmd(ADC2, ENABLE);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void BSP_ADC_Config(void)
{
	BSP_ADC_GPIO_Init();
	BSP_ADC_Init();
}
