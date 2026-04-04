#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_adc.h"

#include <stdio.h>
#include <string.h>

/**
 * @file    main.c
 * @brief   ADC 电压采集示例程序
 * @details
 * 本示例完成以下功能：
 * - 初始化 LED、时基、串口和 ADC；
 * - 使用 ADC3 的通道 11（PC1）进行连续采样；
 * - 通过 ADC 转换完成中断置位标志；
 * - 主循环中读取 ADC 值，并换算为输入电压；
 * - 每隔 1 秒通过串口打印一次采样结果。
 *
 * 电压换算公式：
 * @code
 * voltage = adc_value / 4095.0 * 3.3
 * @endcode
 *
 * @note
 * 该公式默认参考电压为 3.3V，ADC 分辨率为 12 位。
 * 若实际参考电压不为 3.3V，则需要按实际硬件修改换算系数。
 */

/**
 * @brief 主函数
 * @return 程序不会返回
 */
int main(void)
{
    uint16_t adc_value = 0;   /**< 保存 ADC 原始采样值 */
    float voltage = 0.0;      /**< 保存换算后的电压值 */
    uint32_t last_tick = 0;   /**< 上一次打印时间戳 */

    /**
     * @brief ADC 初始化配置参数
     * @details
     * - 独立模式
     * - 单通道采样，不启用扫描
     * - 连续转换模式
     * - 软件触发
     * - 右对齐
     * - 规则组仅 1 个通道
     * - 采样时间为 55.5 个周期
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

    BSP_LED_Init();
    BSP_TimeBase_Init();
    BSP_USART_Config(BSP_USART1);
    BSP_USART_Stdio(BSP_USART1);
    BSP_ADC_PriorityGroupConfig();
    BSP_ADC_Init(BSP_ADC3_Channel11, &config);

    printf("21.ADC采集电压\r\n");

    while(1){
        /**
         * @details
         * 当任意 ADC 转换完成标志被置位时，清除所有标志，
         * 然后读取 ADC3 通道 11 所属 ADC3 的最新转换结果，
         * 并换算为对应电压值。
         *
         * @note
         * 当前工程初始化的是 `BSP_ADC3_Channel11`，
         * 因此真正有效的完成标志应为 `convert_flag[2]`。
         * 这里保留对三个标志的统一判断，不改变原有逻辑。
         */
        if(convert_flag[0] == 1 || convert_flag[1] == 1 || convert_flag[2] == 1){
            convert_flag[0] = 0;
            convert_flag[1] = 0;
            convert_flag[2] = 0;

            adc_value = BSP_ADC_GetValue(BSP_ADC3_Channel11);
            voltage = adc_value / 4095.0 * 3.3;
        }

        /**
         * @details
         * 每隔 1000ms 打印一次采样值和电压值。
         * `BSP_GetTick()` 通常返回系统运行的毫秒节拍计数。
         */
        if(BSP_GetTick() - last_tick >= 1000){
            last_tick = BSP_GetTick();
            printf("adc_value = %d voltage: %.2f\r\n", adc_value, voltage);
        }
    }
}