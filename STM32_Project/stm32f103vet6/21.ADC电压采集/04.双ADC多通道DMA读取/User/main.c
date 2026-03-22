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

    // ADC1 采集前三个通道
    static const bsp_adc_input_t adc1_inputs[] = {
        {ADC_Channel_10, {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC}},
        {ADC_Channel_11, {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC}},
        {ADC_Channel_12, {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC}},
    };

    // ADC2 采集后三个通道
    static const bsp_adc_input_t adc2_inputs[] = {
        {ADC_Channel_13, {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC}},
        {ADC_Channel_14, {GPIOC, GPIO_Pin_4, RCC_APB2Periph_GPIOC}},
        {ADC_Channel_15, {GPIOC, GPIO_Pin_5, RCC_APB2Periph_GPIOC}},
    };

    // DMA 缓冲区，uint32_t：低16位=ADC1结果，高16位=ADC2结果
    static uint32_t dual_adc_buf[3];

    static const bsp_dual_adc_config_t dual_cfg = {
        .adc1_inputs        = adc1_inputs,
        .adc1_channel_count = 3,
        .adc2_inputs        = adc2_inputs,
        .adc2_channel_count = 3,
        .sample_time        = ADC_SampleTime_55Cycles5,
        .buffer             = dual_adc_buf,
    };

    BSP_LED_Init();
    BSP_TimeBase_Init();
    BSP_USART_Config(BSP_USART1);
    BSP_USART_Stdio(BSP_USART1);

    // 初始化双ADC
    BSP_DualADC_InitGroup(&dual_cfg);

    printf("22.双ADC同步采集电压\r\n");

    while(1){

        // 拆包：低16位是ADC1，高16位是ADC2
        for(int i = 0; i < 3; i++){
            uint16_t adc1_val = (uint16_t)(dual_adc_buf[i] & 0xFFFF);
            uint16_t adc2_val = (uint16_t)(dual_adc_buf[i] >> 16);
            voltage[i]     = adc1_val / 4095.0f * 3.3f;   // ADC1：PC0~PC2
            voltage[i + 3] = adc2_val / 4095.0f * 3.3f;   // ADC2：PC3~PC5
        }

        if(BSP_GetTick() - last_tick >= 2000){
            last_tick = BSP_GetTick();
            for(int i = 0; i < 3; i++){
                uint16_t adc1_raw = (uint16_t)(dual_adc_buf[i] & 0xFFFF);
                uint16_t adc2_raw = (uint16_t)(dual_adc_buf[i] >> 16);
                printf("PC%d: adc_value = %4d  voltage = %.2fV  |  "
                       "PC%d: adc_value = %4d  voltage = %.2fV\r\n",
                       i,     adc1_raw, voltage[i],
                       i + 3, adc2_raw, voltage[i + 3]);
            }
        }
    }
}