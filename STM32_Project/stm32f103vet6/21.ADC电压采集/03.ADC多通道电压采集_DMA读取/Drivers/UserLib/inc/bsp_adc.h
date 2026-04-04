#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include "bsp_dma.h"

#include <stdlib.h>

/**
 * @file    bsp_adc.h
 * @brief   ADC 分组采样驱动头文件
 * @details
 * 本模块对 STM32F10x 的 ADC 进行分组封装，支持：
 * - ADC1 / ADC2 / ADC3 设备选择
 * - 单通道或多通道规则组采样
 * - DMA 自动搬运采样结果到缓存区
 * - 通过配置结构体一次性描述整组 ADC 输入
 */

/* ===================== 中断优先级 ===================== */
/** @brief ADC 中断抢占优先级 */
#define PREEMPT_PRIO	2

/** @brief ADC 中断响应优先级（子优先级） */
#define SUB_PRIO		2

/**
 * @brief ADC 设备编号
 * @details
 * 用于标识当前使用的是哪一个 ADC 外设。
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
 * 用于描述一个 ADC 输入通道，包括：
 * - ADC 通道号
 * - 对应 GPIO 引脚及端口时钟
 */
typedef struct{
	uint8_t adc_channel;   /**< ADC 通道号，例如 ADC_Channel_10 */
	bsp_gpio_t adc_gpio;   /**< 对应模拟输入引脚信息 */
}bsp_adc_input_t;

/**
 * @brief ADC 分组配置结构体
 * @details
 * 用于描述一组 ADC 采样配置，包括：
 * - 使用哪个 ADC 外设
 * - 要采哪些通道
 * - 通道数
 * - DMA 缓冲区
 * - 采样时间
 * - 触发方式
 * - 数据对齐方式
 * - 是否连续转换
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
 * @brief 初始化一组 ADC 通道
 * @param cfg ADC 分组配置结构体指针
 * @return 无
 * @details
 * 该函数完成：
 * - GPIO 模拟输入初始化
 * - ADC 参数初始化
 * - 规则组通道顺序配置
 * - DMA 初始化
 * - ADC 校准
 * - 软件触发启动转换
 */
void BSP_ADC_InitGroup(const bsp_adc_group_config_t *cfg);

/**
 * @brief 获取指定下标对应的 ADC 采样值
 * @param cfg ADC 分组配置结构体指针
 * @param index 通道下标
 * @return 对应通道当前采样值；若参数非法返回 0
 * @details
 * 返回值来自 DMA 缓冲区 `cfg->buffer[index]`。
 */
uint16_t BSP_ADC_GetValue(const bsp_adc_group_config_t *cfg, uint8_t index);

#endif