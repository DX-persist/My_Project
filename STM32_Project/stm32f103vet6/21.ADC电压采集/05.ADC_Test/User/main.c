#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_adc.h"
#include "stm32f10x_it.h"

#include <stdio.h>
#include <string.h>               

int main(void)
{
	float voltage1[ADC_CHANNEL_COUNT] = {0.0};
	float voltage2[ADC_CHANNEL_COUNT] = {0.0};
	uint32_t last_tick = 0;
	uint16_t adc1_value[ADC_CHANNEL_COUNT] = {0};
	uint16_t adc2_value[ADC_CHANNEL_COUNT] = {0};

	BSP_LED_Init();
	BSP_TimeBase_Init();
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);
	BSP_ADC_Config();	
	
	printf("22.ADC_Test2\r\n");

	while(1){	
		for(int i = 0; i < ADC_CHANNEL_COUNT; i++){
			adc1_value[i] = (uint16_t)(adc_value[i] & 0x0000FFFF);
			adc2_value[i] = (uint16_t)((adc_value[i] & 0xFFFF0000) >> 16);
			voltage1[i] = adc1_value[i] / 4095.0 * 3.3;
			voltage2[i] = adc2_value[i] / 4095.0 * 3.3;
		}
		if((BSP_GetTick() - last_tick) > 1000){
			last_tick = BSP_GetTick();
			for(int i = 0; i < ADC_CHANNEL_COUNT; i++){
				printf("ADC1: [adc_value%d] = %u [voltage1%d] = %.2f\r\n",i, adc1_value[i], i, voltage1[i]);
			}
			for(int i = 0; i < ADC_CHANNEL_COUNT; i++){
				printf("ADC2: [adc_value%d] = %u [voltage1%d] = %.2f\r\n",i, adc2_value[i], i, voltage2[i]);
			}
		}
	}
}
