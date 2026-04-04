#include "bsp_led.h"
#include "bsp_delay.h"
#include "bsp_usart.h"
#include "bsp_adc.h"

#include <stdio.h>
#include <string.h>

/**
 * @file    main.c
 * @brief   多通道 ADC + DMA 电压采集示例
 * @details
 * 本示例使用 ADC1 对 6 路模拟输入进行连续采样，
 * 通过 DMA 将采样结果自动搬运到数组缓冲区中，
 * 并每隔 2 秒通过 USART1 输出一次各通道的原始值和电压值。
 */

/**
 * @brief 主函数
 * @return 程序不会返回
 */
int main(void)
{
	float voltage[6] = {0.0};   /**< 保存各通道换算后的电压值 */
	uint32_t last_tick = 0;     /**< 记录上一次打印时间 */

	/**
	 * @brief ADC 输入通道列表
	 * @details
	 * 这里只需要维护通道列表，就能灵活增减采样通道。
	 *
	 * 当前配置：
	 * - PC0 -> ADC_Channel_10
	 * - PC1 -> ADC_Channel_11
	 * - PC2 -> ADC_Channel_12
	 * - PC3 -> ADC_Channel_13
	 * - PC4 -> ADC_Channel_14
	 * - PC5 -> ADC_Channel_15
	 */
	static const bsp_adc_input_t my_adc_inputs[] = {
		{ADC_Channel_10, {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC}},
		{ADC_Channel_11, {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC}},
		{ADC_Channel_12, {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC}},
		{ADC_Channel_13, {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC}},
		{ADC_Channel_14, {GPIOC, GPIO_Pin_4, RCC_APB2Periph_GPIOC}},
		{ADC_Channel_15, {GPIOC, GPIO_Pin_5, RCC_APB2Periph_GPIOC}},
	};

	/**
	 * @brief DMA 接收缓冲区
	 * @details
	 * 长度必须与 ADC 通道数一致。
	 * DMA 会将每一轮规则组转换的结果按顺序写入该数组。
	 */
	static uint16_t adc_value[6];

	/**
	 * @brief ADC 分组配置
	 * @details
	 * 当前配置为：
	 * - ADC1
	 * - 独立模式
	 * - 6 通道扫描
	 * - DMA 缓冲区为 adc_value[]
	 * - 软件触发
	 * - 连续采样
	 * - 右对齐
	 * - 采样时间 55.5 周期
	 */
	static const bsp_adc_group_config_t adc_cfg = {
		.dev_id          = BSP_ADC_DEV1,
		.input_list      = my_adc_inputs,
		.adc_mode        = ADC_Mode_Independent,
		.channel_count   = 6,
		.buffer          = adc_value,
		.sample_time     = ADC_SampleTime_55Cycles5,
		.trigger_source  = ADC_ExternalTrigConv_None,
		.align           = ADC_DataAlign_Right,
		.continuous_mode = ENABLE,
	};

	/* 初始化基础外设 */
	BSP_LED_Init();
	BSP_TimeBase_Init();
	BSP_USART_Config(BSP_USART1);
	BSP_USART_Stdio(BSP_USART1);

	/* 初始化 ADC 分组采样 */
	BSP_ADC_InitGroup(&adc_cfg);

	printf("21.ADC采集电压\r\n");

	while(1){

		/* 将 ADC 原始值换算成电压值 */
		for(int i = 0; i < adc_cfg.channel_count; i++){
			voltage[i] = adc_value[i] / 4095.0 * 3.3;
		}

		/* 每隔 2 秒打印一次所有通道的采样结果 */
		if(BSP_GetTick() - last_tick >= 2000){
			last_tick = BSP_GetTick();

			for(int i = 0; i < adc_cfg.channel_count; i++){
				printf("PC%d: adc_value = %d voltage: %.2f\r\n", i, adc_value[i], voltage[i]);
			}
		}
	}
}