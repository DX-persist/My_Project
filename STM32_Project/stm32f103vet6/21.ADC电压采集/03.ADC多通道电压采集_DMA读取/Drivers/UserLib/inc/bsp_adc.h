#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include "bsp_dma.h"

#include <stdlib.h>

/* ===================== 中断优先级 ===================== */
#define PREEMPT_PRIO	2   // 抢占优先级
#define SUB_PRIO		2   // 响应优先级（子优先级）

typedef enum{
	BSP_ADC_DEV1 = 0,
	BSP_ADC_DEV2,
	BSP_ADC_DEV3,
	BSP_ADC_DEV_MAX
}bsp_adc_dev_id_t;

typedef struct{
	uint8_t adc_channel;
	bsp_gpio_t adc_gpio;
}bsp_adc_input_t;

typedef struct{
	bsp_adc_dev_id_t dev_id;
	const bsp_adc_input_t *input_list;
	uint32_t adc_mode;
	uint8_t channel_count;
	uint16_t *buffer;
	uint32_t sample_time;
	uint32_t trigger_source;
	uint32_t align;
	FunctionalState continuous_mode;
}bsp_adc_group_config_t;

void BSP_ADC_InitGroup(const bsp_adc_group_config_t *cfg);
uint16_t BSP_ADC_GetValue(const bsp_adc_group_config_t *cfg, uint8_t index);

#endif
