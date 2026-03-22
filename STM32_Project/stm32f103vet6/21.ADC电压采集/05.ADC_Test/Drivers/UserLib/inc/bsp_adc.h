#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "stm32f10x.h"
#include "bsp_gpio.h"
#include "bsp_dma.h"

#define ADC_CHANNEL_COUNT 3

#define ADC1_CLK			RCC_APB2Periph_ADC1
#define ADC2_CLK			RCC_APB2Periph_ADC2
#define ADC_GPIO_CLK		RCC_APB2Periph_GPIOC
#define ADC_GPIO_PORT		GPIOC

#define DMA1_CLK			RCC_AHBPeriph_DMA1
#define DMA2_CLK			RCC_AHBPeriph_DMA2

#define ADC1_DMA_CHANNEL	DMA1_Channel1
#define ADC3_DMA_CHANNEL	DMA2_Channel5

#define ADC_GPIO0_PIN		GPIO_Pin_0
#define ADC_GPIO0_CHANNEL	ADC_Channel_10

#define ADC_GPIO1_PIN		GPIO_Pin_1
#define ADC_GPIO1_CHANNEL	ADC_Channel_11

#define ADC_GPIO2_PIN		GPIO_Pin_2
#define ADC_GPIO2_CHANNEL	ADC_Channel_12

#define ADC_GPIO3_PIN		GPIO_Pin_3
#define ADC_GPIO3_CHANNEL	ADC_Channel_13

#define ADC_GPIO4_PIN		GPIO_Pin_4
#define ADC_GPIO4_CHANNEL	ADC_Channel_14

#define ADC_GPIO5_PIN		GPIO_Pin_5
#define ADC_GPIO5_CHANNEL	ADC_Channel_15

extern volatile uint32_t adc_value[ADC_CHANNEL_COUNT];
extern void BSP_ADC_Config(void);

#endif
