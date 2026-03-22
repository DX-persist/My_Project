#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_adc.h"

#include <stdio.h>
#include <string.h>               

int main(void)
{
	float voltage[6] = {0.0};
	uint32_t last_tick = 0;
	// 描述用到的通道（只改这里就能加减通道）
	static const bsp_adc_input_t my_adc_inputs[] = {
		{ADC_Channel_10, {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC}},
		{ADC_Channel_11, {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC}},
		{ADC_Channel_12, {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC}},
		{ADC_Channel_13, {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC}},
		{ADC_Channel_14, {GPIOC, GPIO_Pin_4, RCC_APB2Periph_GPIOC}},
		{ADC_Channel_15, {GPIOC, GPIO_Pin_5, RCC_APB2Periph_GPIOC}},
	};	

	// DMA 缓冲区，长度和通道数一致
	static uint16_t adc_value[6];

	static const bsp_adc_group_config_t adc_cfg = {
		.dev_id         = BSP_ADC_DEV1,
		.input_list     = my_adc_inputs,
		.adc_mode 		= ADC_Mode_Independent,
		.channel_count  = 6,
		.buffer         = adc_value,
		.sample_time    = ADC_SampleTime_55Cycles5,
		.trigger_source = ADC_ExternalTrigConv_None,
		.align          = ADC_DataAlign_Right,
		.continuous_mode = ENABLE,
	};


	BSP_LED_Init();
	BSP_TimeBase_Init();
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);
	
	// 初始化
	BSP_ADC_InitGroup(&adc_cfg);	
	
	printf("21.ADC采集电压\r\n");
	
	while(1){	

		for(int i = 0; i < adc_cfg.channel_count; i++){
			voltage[i] = adc_value[i] / 4095.0 * 3.3;
		}

		if(BSP_GetTick() - last_tick >= 2000){
				last_tick = BSP_GetTick();
				for(int i = 0; i < adc_cfg.channel_count; i++){
					printf("PC%d: adc_value = %d voltage: %.2f\r\n",i, adc_value[i], voltage[i]);
				}
		}
	}
}
