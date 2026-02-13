#include "bsp_dma.h"

typedef struct{
	DMA_Channel_TypeDef *dma_channel;
	uint32_t rcc_clk;
}bsp_dma_hw_t;

static const bsp_dma_hw_t bsp_dma_hw[BSP_DMA_Channel_MAX] = {
	[BSP_DMA1_Channel1] = {
		.dma_channel = DMA1_Channel1,
		.rcc_clk = RCC_AHBPeriph_DMA1,
	},

	[BSP_DMA1_Channel2] = {
		.dma_channel = DMA1_Channel2,
		.rcc_clk = RCC_AHBPeriph_DMA1,
	},

	[BSP_DMA1_Channel3] = {
		.dma_channel = DMA1_Channel3,
		.rcc_clk = RCC_AHBPeriph_DMA1,
	},

	[BSP_DMA1_Channel4] = {
		.dma_channel = DMA1_Channel4,
		.rcc_clk = RCC_AHBPeriph_DMA1,
	},

	[BSP_DMA1_Channel5] = {
		.dma_channel = DMA1_Channel5,
		.rcc_clk = RCC_AHBPeriph_DMA1,
	},

	[BSP_DMA1_Channel6] = {
		.dma_channel = DMA1_Channel6,
		.rcc_clk = RCC_AHBPeriph_DMA1,
	},

	[BSP_DMA1_Channel7] = {
		.dma_channel = DMA1_Channel7,
		.rcc_clk = RCC_AHBPeriph_DMA1,
	},

	[BSP_DMA2_Channel1] = {
		.dma_channel = DMA2_Channel1,
		.rcc_clk = RCC_AHBPeriph_DMA2,
	},

	[BSP_DMA2_Channel2] = {
		.dma_channel = DMA2_Channel2,
		.rcc_clk = RCC_AHBPeriph_DMA2,
	},

	[BSP_DMA2_Channel3] = {
		.dma_channel = DMA2_Channel3,
		.rcc_clk = RCC_AHBPeriph_DMA2,
	},

	[BSP_DMA2_Channel4] = {
		.dma_channel = DMA2_Channel4,
		.rcc_clk = RCC_AHBPeriph_DMA2,
	},

	[BSP_DMA2_Channel5] = {
		.dma_channel = DMA2_Channel5,
		.rcc_clk = RCC_AHBPeriph_DMA2,
	},
};

void BSP_DMA_Init(bsp_dma_channel_t ch, bsp_dma_config_t *config)
{
	if(ch >= BSP_DMA_Channel_MAX)	return;

	const bsp_dma_hw_t *hw = &bsp_dma_hw[ch];
	DMA_InitTypeDef DMA_InitStruct;

	RCC_AHBPeriphClockCmd(hw->rcc_clk, ENABLE);

	DMA_DeInit(hw->dma_channel);
	DMA_StructInit(&DMA_InitStruct);

	DMA_InitStruct.DMA_PeripheralBaseAddr 	= config->periph_addr;
	DMA_InitStruct.DMA_MemoryBaseAddr		= config->memory_addr;
	DMA_InitStruct.DMA_BufferSize			= config->buffer_size;

	if(config->dir == DIR_Periph_SRC){
		DMA_InitStruct.DMA_DIR 				= DMA_DIR_PeripheralSRC; 
	}else{
		DMA_InitStruct.DMA_DIR              = DMA_DIR_PeripheralDST;
	}

	if(config->periph_inc == PeripheralInc_Enable){
		DMA_InitStruct.DMA_PeripheralInc 	= DMA_PeripheralInc_Enable;	
	}else{
		DMA_InitStruct.DMA_PeripheralInc    = DMA_PeripheralInc_Disable;
	}

	if(config->memory_inc == MemoryInc_Enable){
		DMA_InitStruct.DMA_MemoryInc		= DMA_MemoryInc_Enable;
	}else{
		DMA_InitStruct.DMA_MemoryInc        = DMA_MemoryInc_Disable;
	}

	if(config->periph_data_size == PeripheralDataSize_Byte){
		DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	}else if(config->periph_data_size == PeripheralDataSize_HalfWord){
		DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	}else if(config->periph_data_size == PeripheralDataSize_Word){
		DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	}
	
	if(config->memory_data_size == MemoryDataSize_Byte){
		DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	}else if(config->memory_data_size == MemoryDataSize_HalfWord){
		DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	}else if(config->memory_data_size == MemoryDataSize_Word){
		DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	}

	if(config->mode == DMA_Mode_Nor){
		DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;	
	}else{
		DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	}

	if(config->priority == DMA_Priority_VH){
		DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;	
	}else if(config->priority == DMA_Priority_H){
		DMA_InitStruct.DMA_Priority = DMA_Priority_High;
	}else if(config->priority == DMA_Priority_M){
		DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
	}else if(config->priority == DMA_Priority_L){
		DMA_InitStruct.DMA_Priority = DMA_Priority_Low;
	}

	if(config->m2m == DMA_M2M_ENABLE){
		DMA_InitStruct.DMA_M2M = DMA_M2M_Enable;
	}else{
		DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	}

	DMA_Init(hw->dma_channel, &DMA_InitStruct);
	DMA_Cmd(hw->dma_channel, ENABLE);
}
