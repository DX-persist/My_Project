#include "bsp_dma.h"

/**
 * @brief DMA 硬件资源描述结构体
 * 
 * 把「DMA 通道指针」和「对应 RCC 时钟」绑在一起，
 * 用于 BSP 层做硬件抽象，避免上层直接接触寄存器。
 */
typedef struct{
	DMA_Channel_TypeDef *dma_channel;   // DMAx_Channely 寄存器基地址
	uint32_t rcc_clk;                   // 对应 DMA 控制器的 RCC 时钟
} bsp_dma_hw_t;

/**
 * @brief DMA 硬件映射表（软件枚举 → 实际硬件）
 * 
 * 作用：
 *  - 用 BSP_DMA_Channel 枚举作为索引
 *  - 屏蔽 DMA1 / DMA2 的差异
 *  - 初始化时无需关心是 DMA1 还是 DMA2
 */
static const bsp_dma_hw_t bsp_dma_hw[BSP_DMA_Channel_MAX] = {

	/* ================= DMA1 ================= */

	[BSP_DMA1_Channel1] = {
		.dma_channel = DMA1_Channel1,
		.rcc_clk     = RCC_AHBPeriph_DMA1,
	},

	[BSP_DMA1_Channel2] = {
		.dma_channel = DMA1_Channel2,
		.rcc_clk     = RCC_AHBPeriph_DMA1,
	},

	[BSP_DMA1_Channel3] = {
		.dma_channel = DMA1_Channel3,
		.rcc_clk     = RCC_AHBPeriph_DMA1,
	},

	[BSP_DMA1_Channel4] = {
		.dma_channel = DMA1_Channel4,
		.rcc_clk     = RCC_AHBPeriph_DMA1,
	},

	[BSP_DMA1_Channel5] = {
		.dma_channel = DMA1_Channel5,
		.rcc_clk     = RCC_AHBPeriph_DMA1,
	},

	[BSP_DMA1_Channel6] = {
		.dma_channel = DMA1_Channel6,
		.rcc_clk     = RCC_AHBPeriph_DMA1,
	},

	[BSP_DMA1_Channel7] = {
		.dma_channel = DMA1_Channel7,
		.rcc_clk     = RCC_AHBPeriph_DMA1,
	},

	/* ================= DMA2（大容量芯片才有） ================= */

	[BSP_DMA2_Channel1] = {
		.dma_channel = DMA2_Channel1,
		.rcc_clk     = RCC_AHBPeriph_DMA2,
	},

	[BSP_DMA2_Channel2] = {
		.dma_channel = DMA2_Channel2,
		.rcc_clk     = RCC_AHBPeriph_DMA2,
	},

	[BSP_DMA2_Channel3] = {
		.dma_channel = DMA2_Channel3,
		.rcc_clk     = RCC_AHBPeriph_DMA2,
	},

	[BSP_DMA2_Channel4] = {
		.dma_channel = DMA2_Channel4,
		.rcc_clk     = RCC_AHBPeriph_DMA2,
	},

	[BSP_DMA2_Channel5] = {
		.dma_channel = DMA2_Channel5,
		.rcc_clk     = RCC_AHBPeriph_DMA2,
	},
};

/**
 * @brief DMA 初始化接口（BSP 层统一入口）
 * 
 * @param ch     BSP 层定义的 DMA 通道枚举
 * @param config DMA 配置参数（方向、数据宽度、模式等）
 */
void BSP_DMA_Init(bsp_dma_channel_t ch, bsp_dma_config_t *config)
{
	/* 参数合法性检查，防止数组越界 */
	if(ch >= BSP_DMA_Channel_MAX) return;

	/* 根据枚举索引获取对应的硬件资源 */
	const bsp_dma_hw_t *hw = &bsp_dma_hw[ch];

	DMA_InitTypeDef DMA_InitStruct;

	/* 使能 DMA 控制器时钟（DMA1 或 DMA2） */
	RCC_AHBPeriphClockCmd(hw->rcc_clk, ENABLE);

	/* 复位 DMA 通道，防止残留配置 */
	DMA_DeInit(hw->dma_channel);

	/* 使用库函数填充默认值 */
	DMA_StructInit(&DMA_InitStruct);

	/* ================= 基础地址与传输长度 ================= */

	/* 外设地址（或源地址） */
	DMA_InitStruct.DMA_PeripheralBaseAddr = config->periph_addr;

	/* 内存地址（或目的地址） */
	DMA_InitStruct.DMA_MemoryBaseAddr     = config->memory_addr;

	/* 传输数据单元数量（不是字节数） */
	DMA_InitStruct.DMA_BufferSize         = config->buffer_size;

	/* ================= 传输方向 ================= */

	/* 外设 → 内存 / 内存 → 外设 / M2M */
	if(config->dir == DIR_Periph_SRC){
		DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC; 
	}else{
		DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
	}

	/* ================= 地址自增控制 ================= */

	/* 外设地址是否递增（通常外设寄存器不递增） */
	DMA_InitStruct.DMA_PeripheralInc =
		(config->periph_inc == PeripheralInc_Enable) ?
		DMA_PeripheralInc_Enable : DMA_PeripheralInc_Disable;

	/* 内存地址是否递增（缓冲区一般需要递增） */
	DMA_InitStruct.DMA_MemoryInc =
		(config->memory_inc == MemoryInc_Enable) ?
		DMA_MemoryInc_Enable : DMA_MemoryInc_Disable;

	/* ================= 数据宽度 ================= */

	/* 外设数据宽度 */
	if(config->periph_data_size == PeripheralDataSize_Byte){
		DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	}else if(config->periph_data_size == PeripheralDataSize_HalfWord){
		DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	}else{
		DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	}

	/* 内存数据宽度 */
	if(config->memory_data_size == MemoryDataSize_Byte){
		DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	}else if(config->memory_data_size == MemoryDataSize_HalfWord){
		DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	}else{
		DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	}

	/* ================= 传输模式 ================= */

	/* 普通模式 / 循环模式 */
	DMA_InitStruct.DMA_Mode =
		(config->mode == DMA_Mode_Nor) ?
		DMA_Mode_Normal : DMA_Mode_Circular;

	/* ================= 优先级 ================= */

	if(config->priority == DMA_Priority_VH){
		DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;
	}else if(config->priority == DMA_Priority_H){
		DMA_InitStruct.DMA_Priority = DMA_Priority_High;
	}else if(config->priority == DMA_Priority_M){
		DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
	}else{
		DMA_InitStruct.DMA_Priority = DMA_Priority_Low;
	}

	/* ================= M2M 控制 ================= */

	/* 内存到内存模式（Flash → SRAM 测试必须开启） */
	DMA_InitStruct.DMA_M2M =
		(config->m2m == DMA_M2M_ENABLE) ?
		DMA_M2M_Enable : DMA_M2M_Disable;

	/* 应用 DMA 配置 */
	DMA_Init(hw->dma_channel, &DMA_InitStruct);

	/* 使能 DMA 通道 */
	DMA_Cmd(hw->dma_channel, ENABLE);
}
