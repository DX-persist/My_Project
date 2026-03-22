#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_adc.h"

#include <stdio.h>
#include <string.h>               

int main(void)
{
	uint16_t adc_value = 0;
	float voltage = 0.0;
	uint32_t last_tick = 0;
	bsp_adc_config_t config = {
		.adc_mode = ADC_Mode_Independent,				/**< 独立模式 */
		.scan_mode = DISABLE,							/**< 单通道采样 */
		.continuous_mode = ENABLE,						/**< 连续采样 */
		.trigger_source	= ADC_ExternalTrigConv_None,	/**< 软件触发 */
		.align = ADC_DataAlign_Right,					/**< 右对齐 */
		.channel_count = 1,								/**< 只有一个通道需要采样 */
		.sample_time = ADC_SampleTime_55Cycles5,		/**< 采样时间 */
	};
	
	BSP_LED_Init();
	BSP_TimeBase_Init();
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);
	BSP_ADC_PriorityGroupConfig();
	BSP_ADC_Init(BSP_ADC3_Channel11, &config);
	
	
	printf("21.ADC采集电压\r\n");
	
	while(1){	
		if(convert_flag[0] == 1 || convert_flag[1] == 1 || convert_flag[2] == 1){
			convert_flag[0] = 0;
			convert_flag[1] = 0;
			convert_flag[2] = 0;
			adc_value = BSP_ADC_GetValue(BSP_ADC3_Channel11);
			voltage = adc_value / 4095.0 * 3.3;
		}
		
		if(BSP_GetTick() - last_tick >= 1000){
				last_tick = BSP_GetTick();
				printf("adc_value = %d voltage: %.2f\r\n", adc_value, voltage);
		}
	}
}
