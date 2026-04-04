#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include "bsp_dma.h"

#include <stdlib.h>

/**
 * @file    bsp_adc.h
 * @brief   ADC 驱动头文件
 * @details
 * 本模块对 STM32F10x 的 ADC 功能进行了统一封装，支持：
 * - 单 ADC 分组采样
 * - 双 ADC 同步规则组采样
 * - DMA 自动搬运采样结果
 *
 * 当前模块主要提供三类能力：
 * 1. 单 ADC 多通道扫描 + DMA
 * 2. 双 ADC 规则同步采样
 * 3. 从 DMA 缓冲区读取采样值
 */

/* ===================== 中断优先级 ===================== */
/** @brief ADC 中断抢占优先级 */
#define PREEMPT_PRIO	2

/** @brief ADC 中断响应优先级（子优先级） */
#define SUB_PRIO		2

/** @brief 当前双 ADC 示例中每个 ADC 的采样通道数 */
#define ADC_CHANNEL_COUNT 3

/**
 * @brief ADC 设备编号枚举
 * @details
 * 用于标识当前使用的 ADC 外设实例。
 */
typedef enum{
	BSP_ADC_DEV1 = 0,   /**< ADC1 */
	BSP_ADC_DEV2,       /**< ADC2 */
	BSP_ADC_DEV3,       /**< ADC3 */
	BSP_ADC_DEV_MAX     /**< ADC 设备数量上限 */
}bsp_adc_dev_id_t;

/**
 * @brief ADC 输入通道描述结构体
 * @details
 * 每一个 ADC 输入由以下信息描述：
 * - ADC 通道号
 * - 对应 GPIO 引脚及其端口时钟
 */
typedef struct{
	uint8_t adc_channel;   /**< ADC 通道号，如 ADC_Channel_10 */
	bsp_gpio_t adc_gpio;   /**< 模拟输入引脚信息 */
}bsp_adc_input_t;

/**
 * @brief 单 ADC 分组配置结构体
 * @details
 * 用于描述一组由同一个 ADC 完成的采样配置，包括：
 * - ADC 外设编号
 * - 输入通道列表
 * - 通道数量
 * - DMA 缓冲区地址
 * - 采样时间
 * - 触发源
 * - 数据对齐方式
 * - 是否连续采样
 */
typedef struct{
	bsp_adc_dev_id_t dev_id;             /**< ADC 设备号 */
	const bsp_adc_input_t *input_list;   /**< 输入通道列表 */
	uint32_t adc_mode;                   /**< ADC 工作模式 */
	uint8_t channel_count;               /**< 通道数量 */
	uint16_t *buffer;                    /**< DMA 结果缓冲区 */
	uint32_t sample_time;                /**< 采样时间 */
	uint32_t trigger_source;             /**< 触发源 */
	uint32_t align;                      /**< 数据对齐方式 */
	FunctionalState continuous_mode;     /**< 是否连续转换 */
}bsp_adc_group_config_t;

/**
 * @brief 双 ADC 同步采样配置结构体
 * @details
 * 用于描述 ADC1 和 ADC2 的同步采样配置。
 *
 * 当前设计中：
 * - ADC1 作为主 ADC
 * - ADC2 作为从 ADC
 * - 两边规则组通道数必须相同
 * - DMA 结果缓冲区使用 uint32_t 数组
 *
 * 对于每个 32 位数据：
 * - 低 16 位为 ADC1 结果
 * - 高 16 位为 ADC2 结果
 */
typedef struct{
	const bsp_adc_input_t *adc1_inputs;  /**< ADC1 输入通道列表 */
	uint8_t adc1_channel_count;          /**< ADC1 通道数 */
	const bsp_adc_input_t *adc2_inputs;  /**< ADC2 输入通道列表 */
	uint8_t adc2_channel_count;          /**< ADC2 通道数 */
	uint32_t sample_time;                /**< 采样时间 */
	uint32_t *buffer;                    /**< 双 ADC DMA 缓冲区 */
}bsp_dual_adc_config_t;

/**
 * @brief 初始化单 ADC 分组采样
 * @param cfg 单 ADC 分组配置结构体指针
 * @return 无
 * @details
 * 该函数完成：
 * - GPIO 模拟输入初始化
 * - ADC 参数配置
 * - 规则组通道顺序配置
 * - DMA 初始化
 * - ADC 校准
 * - 软件触发启动采样
 */
extern void BSP_ADC_InitGroup(const bsp_adc_group_config_t *cfg);

/**
 * @brief 读取单 ADC 分组缓冲区中的指定通道值
 * @param cfg 单 ADC 分组配置结构体指针
 * @param index 通道下标
 * @return 指定通道当前采样值；若参数非法则返回 0
 * @details
 * 该函数本质上直接返回 DMA 缓冲区中的 `buffer[index]`。
 */
extern uint16_t BSP_ADC_GetValue(const bsp_adc_group_config_t *cfg, uint8_t index);

/**
 * @brief 初始化双 ADC 同步采样
 * @param cfg 双 ADC 同步配置结构体指针
 * @return 无
 * @details
 * 该函数用于初始化 ADC1 + ADC2 双规则组同步采样，
 * 并通过 DMA 把 ADC1 和 ADC2 的同步结果成对搬运到 `uint32_t` 缓冲区中。
 */
extern void BSP_DualADC_InitGroup(const bsp_dual_adc_config_t *cfg);

#endif