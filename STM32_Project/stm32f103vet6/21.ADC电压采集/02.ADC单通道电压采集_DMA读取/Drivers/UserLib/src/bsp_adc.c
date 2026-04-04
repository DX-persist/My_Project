#include "bsp_adc.h"

/**
 * @file    bsp_adc.c
 * @brief   ADC 驱动模块源文件
 * @details
 * 本文件实现 ADC 硬件资源映射、GPIO 初始化、ADC 参数配置、
 * DMA 参数配置以及 ADC 启动和采样值读取等功能。
 *
 * 当前实现采用：
 * - ADC 连续转换模式
 * - DMA 循环搬运模式
 *
 * ADC 每完成一次转换，DMA 都会自动把 ADC 数据寄存器中的最新值
 * 写入全局变量 `adc_value`。
 */

/**
 * @brief ADC 硬件映射结构体
 * @details
 * 用于描述某个逻辑 ADC 通道对应的实际硬件资源，包括：
 * - ADC 外设实例
 * - ADC 通道号
 * - ADC 时钟使能位
 * - ADC 输入 GPIO
 * - 中断通道及优先级
 * - DMA 通道
 */
typedef struct{
    ADC_TypeDef *adc;              /**< ADC 外设实例 */
    uint8_t channel;               /**< ADC 通道号 */
    uint32_t rcc_clk;              /**< ADC 对应 RCC 时钟 */

    bsp_gpio_t adc_gpio;           /**< ADC 输入引脚信息 */

    uint8_t irq_channel;           /**< NVIC 中断通道 */
    uint8_t irq_pre_prio;          /**< 抢占优先级 */
    uint8_t irq_sub_prio;          /**< 子优先级 */

    bsp_dma_channel_t dma_channel; /**< ADC 对应 DMA 通道 */
}bsp_adc_hw_t;

/**
 * @brief ADC 最新采样值
 * @details
 * 当前单通道 DMA 采样中，DMA 会不断将最新的 ADC 转换值写入该变量。
 *
 * @note
 * - 该变量为全局变量；
 * - 数据宽度为 16 位；
 * - 对于 STM32F10x 的 12 位 ADC，实际有效值通常为 0~4095。
 */
volatile uint16_t adc_value = 0;

/**
 * @brief ADC 硬件资源映射表
 * @details
 * 该表按照 @ref bsp_adc_channel_t 枚举顺序索引，
 * 用于把逻辑 ADC 通道编号映射到实际硬件资源。
 *
 * 对于当前设计：
 * - ADC1 使用 DMA1_Channel1
 * - ADC3 使用 DMA2_Channel5
 * - ADC2 条目当前未完整填写 DMA 通道信息，后续若启用 ADC2 需补全
 */
static const bsp_adc_hw_t bsp_adc_hw[BSP_ADC_Channel_MAX] = {

    [BSP_ADC1_Channel0] = {
        .adc = ADC1,
        .channel = ADC_Channel_0,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOA, GPIO_Pin_0, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel1] = {
        .adc = ADC1,
        .channel = ADC_Channel_1,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOA, GPIO_Pin_1, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel2] = {
        .adc = ADC1,
        .channel = ADC_Channel_2,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOA, GPIO_Pin_2, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel3] = {
        .adc = ADC1,
        .channel = ADC_Channel_3,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOA, GPIO_Pin_3, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel4] = {
        .adc = ADC1,
        .channel = ADC_Channel_4,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOA, GPIO_Pin_4, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel5] = {
        .adc = ADC1,
        .channel = ADC_Channel_5,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOA, GPIO_Pin_5, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel6] = {
        .adc = ADC1,
        .channel = ADC_Channel_6,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOA, GPIO_Pin_6, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel7] = {
        .adc = ADC1,
        .channel = ADC_Channel_7,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOA, GPIO_Pin_7, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel8] = {
        .adc = ADC1,
        .channel = ADC_Channel_8,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOB, GPIO_Pin_0, RCC_APB2Periph_GPIOB},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel9] = {
        .adc = ADC1,
        .channel = ADC_Channel_9,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOB, GPIO_Pin_1, RCC_APB2Periph_GPIOB},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel10] = {
        .adc = ADC1,
        .channel = ADC_Channel_10,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel11] = {
        .adc = ADC1,
        .channel = ADC_Channel_11,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel12] = {
        .adc = ADC1,
        .channel = ADC_Channel_12,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel13] = {
        .adc = ADC1,
        .channel = ADC_Channel_13,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel14] = {
        .adc = ADC1,
        .channel = ADC_Channel_14,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOC, GPIO_Pin_4, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC1_Channel15] = {
        .adc = ADC1,
        .channel = ADC_Channel_15,
        .rcc_clk = RCC_APB2Periph_ADC1,

        .adc_gpio = {GPIOC, GPIO_Pin_5, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA1_Channel1,
    },

    [BSP_ADC2_Channel0] = {
        .adc = ADC2,
        .channel = ADC_Channel_0,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOA, GPIO_Pin_0, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel1] = {
        .adc = ADC2,
        .channel = ADC_Channel_1,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOA, GPIO_Pin_1, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel2] = {
        .adc = ADC2,
        .channel = ADC_Channel_2,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOA, GPIO_Pin_2, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel3] = {
        .adc = ADC2,
        .channel = ADC_Channel_3,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOA, GPIO_Pin_3, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel4] = {
        .adc = ADC2,
        .channel = ADC_Channel_4,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOA, GPIO_Pin_4, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel5] = {
        .adc = ADC2,
        .channel = ADC_Channel_5,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOA, GPIO_Pin_5, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel6] = {
        .adc = ADC2,
        .channel = ADC_Channel_6,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOA, GPIO_Pin_6, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel7] = {
        .adc = ADC2,
        .channel = ADC_Channel_7,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOA, GPIO_Pin_7, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel8] = {
        .adc = ADC2,
        .channel = ADC_Channel_8,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOB, GPIO_Pin_0, RCC_APB2Periph_GPIOB},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel9] = {
        .adc = ADC2,
        .channel = ADC_Channel_9,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOB, GPIO_Pin_1, RCC_APB2Periph_GPIOB},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel10] = {
        .adc = ADC2,
        .channel = ADC_Channel_10,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel11] = {
        .adc = ADC2,
        .channel = ADC_Channel_11,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel12] = {
        .adc = ADC2,
        .channel = ADC_Channel_12,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel13] = {
        .adc = ADC2,
        .channel = ADC_Channel_13,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel14] = {
        .adc = ADC2,
        .channel = ADC_Channel_14,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOC, GPIO_Pin_4, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC2_Channel15] = {
        .adc = ADC2,
        .channel = ADC_Channel_15,
        .rcc_clk = RCC_APB2Periph_ADC2,

        .adc_gpio = {GPIOC, GPIO_Pin_5, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC3_Channel0] = {
        .adc = ADC3,
        .channel = ADC_Channel_0,
        .rcc_clk = RCC_APB2Periph_ADC3,

        .adc_gpio = {GPIOA, GPIO_Pin_0, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA2_Channel5,
    },

    [BSP_ADC3_Channel1] = {
        .adc = ADC3,
        .channel = ADC_Channel_1,
        .rcc_clk = RCC_APB2Periph_ADC3,

        .adc_gpio = {GPIOA, GPIO_Pin_1, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA2_Channel5,
    },

    [BSP_ADC3_Channel2] = {
        .adc = ADC3,
        .channel = ADC_Channel_2,
        .rcc_clk = RCC_APB2Periph_ADC3,

        .adc_gpio = {GPIOA, GPIO_Pin_2, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA2_Channel5,
    },

    [BSP_ADC3_Channel3] = {
        .adc = ADC3,
        .channel = ADC_Channel_3,
        .rcc_clk = RCC_APB2Periph_ADC3,

        .adc_gpio = {GPIOA, GPIO_Pin_3, RCC_APB2Periph_GPIOA},

        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA2_Channel5,
    },

    [BSP_ADC3_Channel10] = {
        .adc = ADC3,
        .channel = ADC_Channel_10,
        .rcc_clk = RCC_APB2Periph_ADC3,

        .adc_gpio = {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA2_Channel5,
    },

    [BSP_ADC3_Channel11] = {
        .adc = ADC3,
        .channel = ADC_Channel_11,
        .rcc_clk = RCC_APB2Periph_ADC3,

        .adc_gpio = {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA2_Channel5,
    },

    [BSP_ADC3_Channel12] = {
        .adc = ADC3,
        .channel = ADC_Channel_12,
        .rcc_clk = RCC_APB2Periph_ADC3,

        .adc_gpio = {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA2_Channel5,
    },

    [BSP_ADC3_Channel13] = {
        .adc = ADC3,
        .channel = ADC_Channel_13,
        .rcc_clk = RCC_APB2Periph_ADC3,

        .adc_gpio = {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC},

        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,

        .dma_channel = BSP_DMA2_Channel5,
    },
};

/**
 * @brief 初始化指定 ADC 通道
 * @param adc_channel_id ADC 通道编号
 * @param config ADC 配置结构体指针
 * @return 无
 * @details
 * 本函数用于初始化指定 ADC 通道及其对应 DMA 通道。
 * 主要步骤包括：
 * - 参数检查；
 * - 初始化 GPIO 和 ADC 时钟；
 * - 设置输入引脚为模拟输入；
 * - 初始化 ADC 参数；
 * - 初始化 DMA 参数；
 * - 配置 ADC 时钟分频；
 * - 配置规则组采样通道；
 * - 开启 ADC 的 DMA 功能；
 * - 使能 ADC；
 * - 执行 ADC 校准；
 * - 启动软件触发转换。
 *
 * @note
 * 当前实现适合单通道连续采样场景。
 *
 * @warning
 * 若选择 ADC2，对应映射表中的 DMA 通道未完整初始化，使用前建议补全。
 */
void BSP_ADC_Init(bsp_adc_channel_t adc_channel_id, bsp_adc_config_t *config)
{
    if(adc_channel_id >= BSP_ADC_Channel_MAX || config == NULL) return;

    const bsp_adc_hw_t *hw = &bsp_adc_hw[adc_channel_id];
    GPIO_InitTypeDef GPIO_InitStruct;
    ADC_InitTypeDef ADC_InitStruct;
    bsp_dma_config_t dma_config;

    /* 初始化结构体(以默认值来填充结构体成员) */
    GPIO_StructInit(&GPIO_InitStruct);
    ADC_StructInit(&ADC_InitStruct);

    /* 开启 GPIO 和 ADC 的时钟 */
    RCC_APB2PeriphClockCmd(hw->adc_gpio.rcc_clk, ENABLE);
    RCC_APB2PeriphClockCmd(hw->rcc_clk, ENABLE);

    /* 初始化 ADC 用到的引脚 */
    GPIO_InitStruct.GPIO_Pin = hw->adc_gpio.gpio_pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(hw->adc_gpio.gpio_port, &GPIO_InitStruct);

    /* 配置 ADC 结构体成员 */
    ADC_InitStruct.ADC_Mode = config->adc_mode;
    ADC_InitStruct.ADC_ScanConvMode = config->scan_mode;
    ADC_InitStruct.ADC_ContinuousConvMode = config->continuous_mode;
    ADC_InitStruct.ADC_ExternalTrigConv = config->trigger_source;
    ADC_InitStruct.ADC_DataAlign = config->align;
    ADC_InitStruct.ADC_NbrOfChannel = config->channel_count;
    ADC_Init(hw->adc, &ADC_InitStruct);

    /* 配置 DMA 结构体成员 */
    dma_config.periph_addr = (uint32_t)&(hw->adc->DR);
    dma_config.memory_addr = (uint32_t)&adc_value;
    dma_config.dir = DIR_Periph_SRC;
    dma_config.buffer_size = 1;
    dma_config.periph_inc = PeripheralInc_Disable;
    dma_config.memory_inc = MemoryInc_Disable;
    dma_config.periph_data_size = PeripheralDataSize_HalfWord;
    dma_config.memory_data_size = MemoryDataSize_HalfWord;
    dma_config.mode = DMA_Mode_Cir;
    dma_config.priority = DMA_Priority_M;
    dma_config.m2m = DMA_M2M_DISABLE;

    /* 初始化 DMA */
    BSP_DMA_Init(hw->dma_channel, &dma_config);

    /* 配置 ADC 时钟 (72M / 8 = 9MHz) */
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    /* 配置通道的转换顺序和采样时间 */
    ADC_RegularChannelConfig(hw->adc, hw->channel, 1, config->sample_time);

    /* 使能 DMA 搬运数据 */
    ADC_DMACmd(hw->adc, ENABLE);

    /* 使能 ADC */
    ADC_Cmd(hw->adc, ENABLE);

    /* 复位并校准 ADC */
    ADC_ResetCalibration(hw->adc);
    while(ADC_GetResetCalibrationStatus(hw->adc));   /**< 等待复位完成 */

    ADC_StartCalibration(hw->adc);
    while(ADC_GetCalibrationStatus(hw->adc));        /**< 等待校准完成 */

    /* 软件触发 ADC 转换 */
    ADC_SoftwareStartConvCmd(hw->adc, ENABLE);
}

/**
 * @brief 获取指定 ADC 当前的转换值
 * @param adc_channel_id ADC 通道编号
 * @return 当前 ADC 转换结果；若参数非法返回 0
 * @details
 * 本函数直接从 ADC 数据寄存器中读取当前转换结果。
 *
 * @note
 * 在当前 ADC + DMA 模式下，更常见的做法是直接读取全局变量 `adc_value`，
 * 因为 DMA 已经自动完成数据搬运。
 */
uint16_t BSP_ADC_GetValue(bsp_adc_channel_t adc_channel_id)
{
    if(adc_channel_id >= BSP_ADC_Channel_MAX) return 0;

    const bsp_adc_hw_t *hw = &bsp_adc_hw[adc_channel_id];

    return ADC_GetConversionValue(hw->adc);
}