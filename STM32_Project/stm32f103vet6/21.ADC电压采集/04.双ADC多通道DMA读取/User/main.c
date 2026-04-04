#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_adc.h"

#include <stdio.h>
#include <string.h>

/**
 * @file    main.c
 * @brief   双 ADC 同步采样电压示例
 * @details
 * 本示例中：
 * - ADC1 采集 PC0 ~ PC2
 * - ADC2 采集 PC3 ~ PC5
 * - 使用双 ADC 规则同步模式
 * - 使用 DMA 搬运同步结果到 `dual_adc_buf[]`
 *
 * 每个 DMA 单元是一个 32 位数据：
 * - 低 16 位：ADC1 当前结果
 * - 高 16 位：ADC2 当前结果
 *
 * 主循环中拆包、换算电压，并通过串口周期性打印。
 */

/**
 * @brief 主函数
 * @return 程序不会返回
 */
int main(void)
{
    float voltage[6] = {0.0};   /**< 保存 6 路输入换算后的电压值 */
    uint32_t last_tick = 0;     /**< 记录上一次打印时间 */

    /**
     * @brief ADC1 输入列表
     * @details
     * ADC1 采集前三个通道：
     * - PC0 -> ADC_Channel_10
     * - PC1 -> ADC_Channel_11
     * - PC2 -> ADC_Channel_12
     */
    static const bsp_adc_input_t adc1_inputs[] = {
        {ADC_Channel_10, {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC}},
        {ADC_Channel_11, {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC}},
        {ADC_Channel_12, {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC}},
    };

    /**
     * @brief ADC2 输入列表
     * @details
     * ADC2 采集后三个通道：
     * - PC3 -> ADC_Channel_13
     * - PC4 -> ADC_Channel_14
     * - PC5 -> ADC_Channel_15
     */
    static const bsp_adc_input_t adc2_inputs[] = {
        {ADC_Channel_13, {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC}},
        {ADC_Channel_14, {GPIOC, GPIO_Pin_4, RCC_APB2Periph_GPIOC}},
        {ADC_Channel_15, {GPIOC, GPIO_Pin_5, RCC_APB2Periph_GPIOC}},
    };

    /**
     * @brief 双 ADC DMA 缓冲区
     * @details
     * 每个元素是一个 32 位数据：
     * - 低 16 位为 ADC1 结果
     * - 高 16 位为 ADC2 结果
     */
    static uint32_t dual_adc_buf[3];

    /**
     * @brief 双 ADC 配置结构体
     * @details
     * 当前配置：
     * - ADC1 采集 3 路
     * - ADC2 采集 3 路
     * - 双 ADC 同步采样
     * - 采样时间 55.5 周期
     * - DMA 结果缓冲区为 dual_adc_buf[]
     */
    static const bsp_dual_adc_config_t dual_cfg = {
        .adc1_inputs        = adc1_inputs,
        .adc1_channel_count = 3,
        .adc2_inputs        = adc2_inputs,
        .adc2_channel_count = 3,
        .sample_time        = ADC_SampleTime_55Cycles5,
        .buffer             = dual_adc_buf,
    };

    /* 初始化基础外设 */
    BSP_LED_Init();
    BSP_TimeBase_Init();
    BSP_USART_Config(BSP_USART1);
    BSP_USART_Stdio(BSP_USART1);

    /* 初始化双 ADC 同步采样 */
    BSP_DualADC_InitGroup(&dual_cfg);

    printf("22.双ADC同步采集电压\r\n");

    while(1){

        /**
         * @details
         * 从 dual_adc_buf 中拆包：
         * - 低 16 位是 ADC1 结果，对应 PC0 ~ PC2
         * - 高 16 位是 ADC2 结果，对应 PC3 ~ PC5
         */
        for(int i = 0; i < 3; i++){
            uint16_t adc1_val = (uint16_t)(dual_adc_buf[i] & 0xFFFF);
            uint16_t adc2_val = (uint16_t)(dual_adc_buf[i] >> 16);

            voltage[i]     = adc1_val / 4095.0f * 3.3f;
            voltage[i + 3] = adc2_val / 4095.0f * 3.3f;
        }

        /* 每隔 2 秒打印一次双 ADC 采样结果 */
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