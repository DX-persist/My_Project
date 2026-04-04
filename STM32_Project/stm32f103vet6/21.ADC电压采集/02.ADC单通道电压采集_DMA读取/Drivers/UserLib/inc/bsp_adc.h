#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include "bsp_dma.h"

#include <stdlib.h>

/**
 * @file    bsp_adc.h
 * @brief   ADC 驱动模块头文件
 * @details
 * 本模块对 STM32F10x 标准外设库中的 ADC 功能进行了简单封装，
 * 支持 ADC1、ADC2、ADC3 的多个输入通道，并结合 DMA 实现采样结果
 * 自动搬运到内存变量中。
 *
 * 当前设计特点：
 * - 使用统一枚举管理 ADC1/ADC2/ADC3 的各个通道；
 * - 通过硬件映射表完成“逻辑通道”到“实际硬件资源”的绑定；
 * - 支持单通道连续采样；
 * - 支持 ADC + DMA 循环搬运；
 * - 主循环直接读取最新采样值，无需等待中断标志。
 */

/* ===================== 中断优先级 ===================== */
/** @brief ADC 中断抢占优先级 */
#define PREEMPT_PRIO    2

/** @brief ADC 中断响应优先级（子优先级） */
#define SUB_PRIO        2

/**
 * @brief ADC 通道编号枚举
 * @details
 * 该枚举将 ADC 外设实例与具体通道号组合成统一编号，
 * 便于上层通过同一个接口选择不同 ADC 输入通道。
 */
typedef enum{
    BSP_ADC1_Channel0 = 0,   /**< ADC1 通道 0，对应 PA0 */
    BSP_ADC1_Channel1,       /**< ADC1 通道 1，对应 PA1 */
    BSP_ADC1_Channel2,       /**< ADC1 通道 2，对应 PA2 */
    BSP_ADC1_Channel3,       /**< ADC1 通道 3，对应 PA3 */
    BSP_ADC1_Channel4,       /**< ADC1 通道 4，对应 PA4 */
    BSP_ADC1_Channel5,       /**< ADC1 通道 5，对应 PA5 */
    BSP_ADC1_Channel6,       /**< ADC1 通道 6，对应 PA6 */
    BSP_ADC1_Channel7,       /**< ADC1 通道 7，对应 PA7 */
    BSP_ADC1_Channel8,       /**< ADC1 通道 8，对应 PB0 */
    BSP_ADC1_Channel9,       /**< ADC1 通道 9，对应 PB1 */
    BSP_ADC1_Channel10,      /**< ADC1 通道 10，对应 PC0 */
    BSP_ADC1_Channel11,      /**< ADC1 通道 11，对应 PC1 */
    BSP_ADC1_Channel12,      /**< ADC1 通道 12，对应 PC2 */
    BSP_ADC1_Channel13,      /**< ADC1 通道 13，对应 PC3 */
    BSP_ADC1_Channel14,      /**< ADC1 通道 14，对应 PC4 */
    BSP_ADC1_Channel15,      /**< ADC1 通道 15，对应 PC5 */

    BSP_ADC2_Channel0,       /**< ADC2 通道 0，对应 PA0 */
    BSP_ADC2_Channel1,       /**< ADC2 通道 1，对应 PA1 */
    BSP_ADC2_Channel2,       /**< ADC2 通道 2，对应 PA2 */
    BSP_ADC2_Channel3,       /**< ADC2 通道 3，对应 PA3 */
    BSP_ADC2_Channel4,       /**< ADC2 通道 4，对应 PA4 */
    BSP_ADC2_Channel5,       /**< ADC2 通道 5，对应 PA5 */
    BSP_ADC2_Channel6,       /**< ADC2 通道 6，对应 PA6 */
    BSP_ADC2_Channel7,       /**< ADC2 通道 7，对应 PA7 */
    BSP_ADC2_Channel8,       /**< ADC2 通道 8，对应 PB0 */
    BSP_ADC2_Channel9,       /**< ADC2 通道 9，对应 PB1 */
    BSP_ADC2_Channel10,      /**< ADC2 通道 10，对应 PC0 */
    BSP_ADC2_Channel11,      /**< ADC2 通道 11，对应 PC1 */
    BSP_ADC2_Channel12,      /**< ADC2 通道 12，对应 PC2 */
    BSP_ADC2_Channel13,      /**< ADC2 通道 13，对应 PC3 */
    BSP_ADC2_Channel14,      /**< ADC2 通道 14，对应 PC4 */
    BSP_ADC2_Channel15,      /**< ADC2 通道 15，对应 PC5 */

    BSP_ADC3_Channel0,       /**< ADC3 通道 0，对应 PA0 */
    BSP_ADC3_Channel1,       /**< ADC3 通道 1，对应 PA1 */
    BSP_ADC3_Channel2,       /**< ADC3 通道 2，对应 PA2 */
    BSP_ADC3_Channel3,       /**< ADC3 通道 3，对应 PA3 */
    BSP_ADC3_Channel10,      /**< ADC3 通道 10，对应 PC0 */
    BSP_ADC3_Channel11,      /**< ADC3 通道 11，对应 PC1 */
    BSP_ADC3_Channel12,      /**< ADC3 通道 12，对应 PC2 */
    BSP_ADC3_Channel13,      /**< ADC3 通道 13，对应 PC3 */

    BSP_ADC_Channel_MAX      /**< ADC 通道枚举上限 */
}bsp_adc_channel_t;

/**
 * @brief ADC 初始化配置结构体
 * @details
 * 该结构体用于描述 ADC 的基本工作模式、是否扫描、是否连续转换、
 * 触发方式、数据对齐方式、规则组通道数以及采样时间。
 */
typedef struct{
    uint32_t adc_mode;               /**< ADC 模式，如独立模式/双 ADC 模式 */
    FunctionalState scan_mode;       /**< 是否启用扫描模式 */
    FunctionalState continuous_mode; /**< 是否启用连续转换模式 */
    uint32_t trigger_source;         /**< 外部触发源选择 */
    uint32_t align;                  /**< 数据对齐方式：左对齐或右对齐 */
    uint8_t channel_count;           /**< 规则组采样通道数量 */
    uint32_t sample_time;            /**< 采样时间 */
} bsp_adc_config_t;

/**
 * @brief ADC 最新采样值
 * @details
 * 在当前 ADC + DMA 设计中，DMA 会把 ADC 数据寄存器中的结果
 * 自动搬运到该变量中，因此主循环可以直接读取它。
 *
 * @note
 * 由于该变量可能在 DMA 硬件搬运过程中被更新，因此使用 `volatile` 修饰。
 */
extern volatile uint16_t adc_value;

/**
 * @brief 初始化指定 ADC 通道
 * @param adc_channel_id ADC 通道编号，取值见 @ref bsp_adc_channel_t
 * @param config ADC 配置结构体指针
 * @return 无
 * @details
 * 本函数完成以下工作：
 * - 开启 GPIO 和 ADC 时钟；
 * - 配置 ADC 输入引脚为模拟输入；
 * - 初始化 ADC 工作参数；
 * - 初始化 DMA 参数；
 * - 配置规则组通道顺序和采样时间；
 * - 使能 ADC 的 DMA 功能；
 * - 使能 ADC 并完成校准；
 * - 启动软件触发转换。
 */
extern void BSP_ADC_Init(bsp_adc_channel_t adc_channel_id, bsp_adc_config_t *config);

/**
 * @brief 配置 ADC 中断优先级分组
 * @return 无
 * @note
 * 当前头文件中保留该接口声明，但在你提供的当前实现文件中未看到对应定义。
 */
extern void BSP_ADC_PriorityGroupConfig(void);

/**
 * @brief 获取指定 ADC 的当前转换值
 * @param adc_channel_id ADC 通道编号
 * @return ADC 转换结果；若参数非法则返回 0
 * @details
 * 该函数直接读取对应 ADC 外设的数据寄存器中的转换值。
 *
 * @note
 * 当前主程序主要通过 DMA 更新的 @ref adc_value 读取结果，
 * 本函数属于额外保留的主动读取接口。
 */
extern uint16_t BSP_ADC_GetValue(bsp_adc_channel_t adc_channel_id);

/**
 * @brief ADC 通用中断处理函数
 * @param adcx ADC 外设指针
 * @return 无
 * @note
 * 当前头文件中保留该接口声明，但在你提供的当前实现文件中未看到对应定义。
 */
extern void BSP_ADC_IRQHandler(ADC_TypeDef *adcx);

#endif