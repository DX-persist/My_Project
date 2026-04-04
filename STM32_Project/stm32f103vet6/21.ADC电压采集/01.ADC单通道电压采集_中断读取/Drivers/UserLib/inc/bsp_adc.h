#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include <stdlib.h>

/**
 * @file    bsp_adc.h
 * @brief   ADC Board Support Package 头文件
 * @details
 * 本模块对 STM32F10x 标准外设库中的 ADC 功能进行了简单封装，
 * 提供 ADC 通道选择、初始化、数据读取以及中断处理接口。
 *
 * 当前设计特点：
 * - 通过枚举值统一管理 ADC1 / ADC2 / ADC3 的各个通道；
 * - 每个枚举项对应一个固定硬件通道和 GPIO 引脚；
 * - 使用规则组进行单通道采样配置；
 * - 通过转换完成中断（EOC）置位 `convert_flag[]`，供主循环轮询处理。
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
 * 便于上层使用同一个接口初始化不同 ADC 输入通道。
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
 * 该结构体用于描述 ADC 的工作模式、扫描方式、触发方式、数据对齐方式、
 * 采样通道数量以及采样时间等参数。
 */
typedef struct{
    uint32_t adc_mode;               /**< ADC 工作模式，如独立模式、双重模式等 */
    FunctionalState scan_mode;       /**< 是否启用扫描模式 */
    FunctionalState continuous_mode; /**< 是否启用连续转换模式 */
    uint32_t trigger_source;         /**< 外部触发源配置 */
    uint32_t align;                  /**< 转换结果对齐方式：左对齐或右对齐 */
    uint8_t channel_count;           /**< 规则组通道数量 */
    uint32_t sample_time;            /**< 通道采样时间 */
} bsp_adc_config_t;

/**
 * @brief ADC 转换完成标志数组
 * @details
 * - `convert_flag[0]` 对应 ADC1
 * - `convert_flag[1]` 对应 ADC2
 * - `convert_flag[2]` 对应 ADC3
 *
 * 中断服务函数在对应 ADC 完成一次转换后将标志位置 1，
 * 主循环可通过轮询该标志判断是否有新数据可读。
 *
 * @note
 * 该数组被中断和主循环共同访问，因此声明为 `volatile`。
 */
extern volatile uint8_t convert_flag[3];

/**
 * @brief 初始化指定 ADC 通道
 * @param adc_channel_id ADC 通道编号，取值见 @ref bsp_adc_channel_t
 * @param config ADC 配置结构体指针
 * @return 无
 * @details
 * 该函数完成以下操作：
 * - 开启 GPIO 与 ADC 时钟；
 * - 将对应引脚配置为模拟输入模式；
 * - 按配置参数初始化 ADC；
 * - 配置 ADC 时钟分频；
 * - 配置规则组通道顺序与采样时间；
 * - 使能 EOC 中断并初始化 NVIC；
 * - 进行 ADC 复位校准与启动校准；
 * - 最后启动软件触发转换。
 *
 * @note
 * 当前实现固定将所选通道配置为规则组第 1 个转换通道。
 */
extern void BSP_ADC_Init(bsp_adc_channel_t adc_channel_id, bsp_adc_config_t *config);

/**
 * @brief 配置 NVIC 中断优先级分组
 * @return 无
 * @details
 * 当前配置为 `NVIC_PriorityGroup_2`，即：
 * - 2 位抢占优先级
 * - 2 位响应优先级
 */
extern void BSP_ADC_PriorityGroupConfig(void);

/**
 * @brief 读取指定 ADC 外设最近一次转换结果
 * @param adc_channel_id ADC 通道编号
 * @return 12 位 ADC 转换结果
 * @details
 * 本函数根据传入的通道编号定位到对应 ADC 外设，
 * 然后读取其数据寄存器中的转换值。
 *
 * @note
 * 如果多个枚举值属于同一个 ADC 外设，则读取的是该 ADC 当前最近一次转换结果。
 */
extern uint16_t BSP_ADC_GetValue(bsp_adc_channel_t adc_channel_id);

/**
 * @brief ADC 通用中断处理函数
 * @param adcx ADC 外设指针，可为 ADC1 / ADC2 / ADC3
 * @return 无
 * @details
 * 该函数主要用于：
 * - 检查 ADC 转换完成中断标志（EOC）；
 * - 根据 ADC 实例设置对应的 `convert_flag[]`；
 * - 清除 ADC 中断挂起标志。
 */
extern void BSP_ADC_IRQHandler(ADC_TypeDef *adcx);

#endif