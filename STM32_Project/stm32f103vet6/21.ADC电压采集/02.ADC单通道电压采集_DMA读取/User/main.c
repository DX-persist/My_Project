#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_adc.h"

#include <stdio.h>
#include <string.h>

/**
 * @file    main.c
 * @brief   ADC + DMA 电压采集示例主程序
 * @details
 * 本示例使用 STM32F10x 的 ADC3 通道 11（PC1）进行模拟电压采样，
 * 并结合 DMA 将采样结果自动搬运到全局变量 `adc_value` 中。
 *
 * 主循环中周期性读取 `adc_value`，并按参考电压 3.3V 将其换算成实际电压值，
 * 最后通过 USART1 串口每隔 1 秒打印一次结果。
 */

/**
 * @brief 主函数
 * @return 程序不会返回
 */
int main(void)
{
    float voltage = 0.0;      /**< 保存换算后的电压值 */
    uint32_t last_tick = 0;   /**< 记录上一次串口打印时间 */

    /**
     * @brief ADC 初始化配置参数
     * @details
     * 当前配置为：
     * - 独立模式
     * - 单通道采样
     * - 连续转换
     * - 软件触发
     * - 右对齐
     * - 规则组通道数为 1
     * - 采样时间为 55.5 周期
     */
    bsp_adc_config_t config = {
        .adc_mode = ADC_Mode_Independent,              /**< 独立模式 */
        .scan_mode = DISABLE,                          /**< 单通道采样 */
        .continuous_mode = ENABLE,                     /**< 连续采样 */
        .trigger_source = ADC_ExternalTrigConv_None,   /**< 软件触发 */
        .align = ADC_DataAlign_Right,                  /**< 右对齐 */
        .channel_count = 1,                            /**< 只有一个通道需要采样 */
        .sample_time = ADC_SampleTime_55Cycles5,       /**< 采样时间 */
    };

    /* 外设初始化 */
    BSP_LED_Init();
    BSP_TimeBase_Init();
    BSP_USART_Config(BSP_USART1);
    BSP_USART_Stdio(BSP_USART1);

    /* 初始化 ADC3 通道 11（PC1） */
    BSP_ADC_Init(BSP_ADC3_Channel11, &config);

    printf("21.ADC采集电压\r\n");

    while(1){

        /**
         * @details
         * `adc_value` 由 DMA 自动更新，这里直接读取其值并换算为电压。
         *
         * 电压换算公式：
         * @code
         * voltage = adc_value / 4095.0 * 3.3;
         * @endcode
         *
         * 其中：
         * - 4095 为 12 位 ADC 的满量程；
         * - 3.3 为参考电压。
         */
        voltage = adc_value / 4095.0 * 3.3;

        /**
         * @details
         * 每隔 1000ms 通过串口输出一次 ADC 原始值和换算后的电压值。
         */
        if(BSP_GetTick() - last_tick >= 1000){
            last_tick = BSP_GetTick();
            printf("adc_value = %d voltage: %.2f\r\n", adc_value, voltage);
        }
    }
}