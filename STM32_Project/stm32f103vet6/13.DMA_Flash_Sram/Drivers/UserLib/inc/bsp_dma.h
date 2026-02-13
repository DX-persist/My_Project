#ifndef BSP_DMA_H
#define BSP_DMA_H

#include "stm32f10x.h"

typedef enum{
	BSP_DMA1_Channel1 = 0,
	BSP_DMA1_Channel2,
	BSP_DMA1_Channel3,
	BSP_DMA1_Channel4,
	BSP_DMA1_Channel5,
	BSP_DMA1_Channel6,
	BSP_DMA1_Channel7,

	BSP_DMA2_Channel1,
	BSP_DMA2_Channel2,
	BSP_DMA2_Channel3,
	BSP_DMA2_Channel4,
	BSP_DMA2_Channel5,

	BSP_DMA_Channel_MAX
}bsp_dma_channel_t;

typedef enum{
	DIR_Periph_SRC = 0,
	DIR_Periph_DST
}bsp_dma_dir_t;

typedef enum{
	PeripheralInc_Enable = 0,
	PeripheralInc_Disable
}bsp_dma_per_inc_t;

typedef enum{
	MemoryInc_Enable = 0,
	MemoryInc_Disable
}bsp_dma_mem_inc_t;

typedef enum{
	PeripheralDataSize_Byte = 0,
	PeripheralDataSize_HalfWord,
	PeripheralDataSize_Word,
}bsp_dma_per_datasize_t;

typedef enum{
	MemoryDataSize_Byte = 0,
	MemoryDataSize_HalfWord,
	MemoryDataSize_Word,
}bsp_dma_mem_datasize_t;

typedef enum{
	DMA_Mode_Cir = 0,
	DMA_Mode_Nor
}bsp_dma_mode_t;

typedef enum{
	DMA_Priority_VH = 0,
	DMA_Priority_H,
	DMA_Priority_M,
	DMA_Priority_L
}bsp_dma_prio_t;

typedef enum{
	DMA_M2M_ENABLE = 0,
	DMA_M2M_DISABLE,
}bsp_dma_m2m_t;

typedef struct{
	uint32_t 				periph_addr;
	uint32_t 				memory_addr;
	bsp_dma_dir_t 			dir;
	uint32_t 				buffer_size;
	bsp_dma_per_inc_t 		periph_inc;
	bsp_dma_mem_inc_t 		memory_inc;
	bsp_dma_per_datasize_t 	periph_data_size;
	bsp_dma_mem_datasize_t 	memory_data_size;
	bsp_dma_mode_t 			mode;
	bsp_dma_prio_t 			priority;
	bsp_dma_m2m_t 			m2m;
}bsp_dma_config_t;
	
extern void BSP_DMA_Init(bsp_dma_channel_t ch, bsp_dma_config_t *config);

#endif
