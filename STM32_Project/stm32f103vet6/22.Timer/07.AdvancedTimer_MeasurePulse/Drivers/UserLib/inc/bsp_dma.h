#ifndef BSP_DMA_H
#define BSP_DMA_H

#include "stm32f10x.h"  // STM32F10x 寄存器和外设定义
#include "bsp_led.h"    // LED 控制（调试用）

/* ===================== DMA 通道枚举 ===================== */
typedef enum{
	// DMA1 通道
	BSP_DMA1_Channel1 = 0,
	BSP_DMA1_Channel2,
	BSP_DMA1_Channel3,
	BSP_DMA1_Channel4,
	BSP_DMA1_Channel5,
	BSP_DMA1_Channel6,
	BSP_DMA1_Channel7,

	// DMA2 通道
	BSP_DMA2_Channel1,
	BSP_DMA2_Channel2,
	BSP_DMA2_Channel3,
	BSP_DMA2_Channel4,
	BSP_DMA2_Channel5,

	BSP_DMA_Channel_MAX,   // DMA 通道数量上限

	BSP_DMA_NONE = 0xFF,  // 无DMA，显式赋值远离有效范围
} bsp_dma_channel_t;

/* ===================== DMA 数据传输方向 ===================== */
typedef enum{
	DIR_Periph_SRC = 0,  // 外设为数据源，内存为目的
	DIR_Periph_DST        // 外设为数据目的，内存为源
} bsp_dma_dir_t;

/* ===================== 外设地址是否自增 ===================== */
typedef enum{
	PeripheralInc_Enable = 0,   // 外设地址递增
	PeripheralInc_Disable        // 外设地址固定
} bsp_dma_per_inc_t;

/* ===================== 内存地址是否自增 ===================== */
typedef enum{
	MemoryInc_Enable = 0,  // 内存地址递增
	MemoryInc_Disable       // 内存地址固定
} bsp_dma_mem_inc_t;

/* ===================== 外设数据宽度 ===================== */
typedef enum{
	PeripheralDataSize_Byte = 0,   // 8位
	PeripheralDataSize_HalfWord,   // 16位
	PeripheralDataSize_Word         // 32位
} bsp_dma_per_datasize_t;

/* ===================== 内存数据宽度 ===================== */
typedef enum{
	MemoryDataSize_Byte = 0,  // 8位
	MemoryDataSize_HalfWord,  // 16位
	MemoryDataSize_Word       // 32位
} bsp_dma_mem_datasize_t;

/* ===================== DMA 工作模式 ===================== */
typedef enum{
	DMA_Mode_Cir = 0,  // 循环模式
	DMA_Mode_Nor       // 普通模式（完成一次传输后停止）
} bsp_dma_mode_t;

/* ===================== DMA 优先级 ===================== */
typedef enum{
	DMA_Priority_VH = 0, // 很高优先级
	DMA_Priority_H,      // 高优先级
	DMA_Priority_M,      // 中优先级
	DMA_Priority_L       // 低优先级
} bsp_dma_prio_t;

/* ===================== 内存到内存模式 ===================== */
typedef enum{
	DMA_M2M_ENABLE = 0,   // 启用内存到内存传输
	DMA_M2M_DISABLE        // 禁用
} bsp_dma_m2m_t;

/* ===================== DMA 配置结构体 ===================== */
typedef struct{
	uint32_t 				periph_addr;       // 外设寄存器地址
	uint32_t 				memory_addr;       // 内存地址
	bsp_dma_dir_t 			dir;               // 数据传输方向
	uint32_t 				buffer_size;       // 传输数据数量
	bsp_dma_per_inc_t 		periph_inc;        // 外设地址是否递增
	bsp_dma_mem_inc_t 		memory_inc;        // 内存地址是否递增
	bsp_dma_per_datasize_t 	periph_data_size;  // 外设数据宽度
	bsp_dma_mem_datasize_t 	memory_data_size;  // 内存数据宽度
	bsp_dma_mode_t 			mode;              // DMA 模式（循环/普通）
	bsp_dma_prio_t 			priority;          // DMA 优先级
	bsp_dma_m2m_t 			m2m;               // 内存到内存模式使能
} bsp_dma_config_t;

/* ===================== 函数接口 ===================== */

/**
 * @brief 初始化指定 DMA 通道
 * @param ch DMA 通道编号
 * @param config DMA 配置结构体指针
 */
extern void BSP_DMA_Init(bsp_dma_channel_t ch, bsp_dma_config_t *config);

#endif
