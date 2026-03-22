#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include <stdlib.h>

/* ===================== 中断优先级 ===================== */
#define PREEMPT_PRIO	2   // 抢占优先级
#define SUB_PRIO		2   // 响应优先级（子优先级）


typedef enum{
	BSP_ADC1_Channel0 = 0,
	BSP_ADC1_Channel1,
	BSP_ADC1_Channel2,
	BSP_ADC1_Channel3,
	BSP_ADC1_Channel4,
	BSP_ADC1_Channel5,
	BSP_ADC1_Channel6,
	BSP_ADC1_Channel7,
	BSP_ADC1_Channel8,
	BSP_ADC1_Channel9,
	BSP_ADC1_Channel10,
	BSP_ADC1_Channel11,
	BSP_ADC1_Channel12,
	BSP_ADC1_Channel13,
	BSP_ADC1_Channel14,
	BSP_ADC1_Channel15,

	BSP_ADC2_Channel0,
	BSP_ADC2_Channel1,
	BSP_ADC2_Channel2,
	BSP_ADC2_Channel3,
	BSP_ADC2_Channel4,
	BSP_ADC2_Channel5,
	BSP_ADC2_Channel6,
	BSP_ADC2_Channel7,
	BSP_ADC2_Channel8,
	BSP_ADC2_Channel9,
	BSP_ADC2_Channel10,
	BSP_ADC2_Channel11,
	BSP_ADC2_Channel12,
	BSP_ADC2_Channel13,
	BSP_ADC2_Channel14,
	BSP_ADC2_Channel15,

	BSP_ADC3_Channel0,
	BSP_ADC3_Channel1,
	BSP_ADC3_Channel2,
	BSP_ADC3_Channel3,
	BSP_ADC3_Channel10,
	BSP_ADC3_Channel11,
	BSP_ADC3_Channel12,
	BSP_ADC3_Channel13,

	BSP_ADC_Channel_MAX
}bsp_adc_channel_t;

typedef struct{
    uint32_t adc_mode;             // 单/双ADC模式
    FunctionalState scan_mode;     // 是否扫描多个通道
    FunctionalState continuous_mode; // 是否连续采样
    uint32_t trigger_source;       // 触发方式
    uint32_t align;                // 左/右对齐
    uint8_t channel_count;         // 采样通道数
	uint32_t sample_time;			//采样时间
} bsp_adc_config_t;

extern volatile uint8_t convert_flag[3];

extern void BSP_ADC_Init(bsp_adc_channel_t adc_channel_id, bsp_adc_config_t *config);
extern void BSP_ADC_PriorityGroupConfig(void);
extern uint16_t BSP_ADC_GetValue(bsp_adc_channel_t adc_channel_id);
extern void BSP_ADC_IRQHandler(ADC_TypeDef *adcx);

#endif
