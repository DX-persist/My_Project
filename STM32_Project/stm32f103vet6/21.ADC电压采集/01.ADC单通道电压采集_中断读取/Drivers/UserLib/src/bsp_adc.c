#include "bsp_adc.h"

/**
 * @file    bsp_adc.c
 * @brief   ADC Board Support Package 源文件
 * @details
 * 本文件实现 ADC 模块的底层硬件映射、初始化、数据读取及中断处理。
 *
 * 设计思路：
 * - 使用 `bsp_adc_hw_t` 维护“逻辑通道编号”到“实际硬件资源”的映射；
 * - 上层只需传入 `bsp_adc_channel_t` 即可完成 ADC 初始化与读取；
 * - 中断服务函数只负责置位标志，不做复杂处理，便于主循环安全读取。
 */

/**
 * @brief ADC 硬件资源映射结构体
 * @details
 * 每个逻辑 ADC 通道对应一个硬件描述项，用于记录：
 * - ADC 外设实例
 * - ADC 通道号
 * - ADC 时钟使能位
 * - 对应 GPIO 端口/引脚
 * - 中断通道及优先级参数
 */
typedef struct{
    ADC_TypeDef *adc;      /**< ADC 外设实例 */
    uint8_t channel;       /**< ADC 通道号 */
    uint32_t rcc_clk;      /**< ADC 对应 RCC 时钟使能位 */
    bsp_gpio_t adc_gpio;   /**< ADC 输入引脚配置 */
    uint8_t irq_channel;   /**< NVIC 中断通道 */
    uint8_t irq_pre_prio;  /**< 抢占优先级 */
    uint8_t irq_sub_prio;  /**< 子优先级 */
}bsp_adc_hw_t;

/**
 * @brief ADC 转换完成标志
 * @details
 * - `convert_flag[0]`：ADC1 转换完成标志
 * - `convert_flag[1]`：ADC2 转换完成标志
 * - `convert_flag[2]`：ADC3 转换完成标志
 *
 * @note
 * 该变量同时被主循环和中断访问，因此必须使用 `volatile` 修饰。
 */
volatile uint8_t convert_flag[3] = {0};

/**
 * @brief ADC 硬件映射表
 * @details
 * 该表按照 @ref bsp_adc_channel_t 枚举顺序建立索引，
 * 将逻辑 ADC 通道编号映射到具体的外设、引脚、时钟和中断配置。
 *
 * @note
 * 通过这种方式可以避免大量 `switch-case`，提高代码可维护性。
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
    },

    [BSP_ADC1_Channel1] = {
        .adc = ADC1,
        .channel = ADC_Channel_1,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOA, GPIO_Pin_1, RCC_APB2Periph_GPIOA},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel2] = {
        .adc = ADC1,
        .channel = ADC_Channel_2,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOA, GPIO_Pin_2, RCC_APB2Periph_GPIOA},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel3] = {
        .adc = ADC1,
        .channel = ADC_Channel_3,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOA, GPIO_Pin_3, RCC_APB2Periph_GPIOA},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel4] = {
        .adc = ADC1,
        .channel = ADC_Channel_4,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOA, GPIO_Pin_4, RCC_APB2Periph_GPIOA},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel5] = {
        .adc = ADC1,
        .channel = ADC_Channel_5,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOA, GPIO_Pin_5, RCC_APB2Periph_GPIOA},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel6] = {
        .adc = ADC1,
        .channel = ADC_Channel_6,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOA, GPIO_Pin_6, RCC_APB2Periph_GPIOA},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel7] = {
        .adc = ADC1,
        .channel = ADC_Channel_7,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOA, GPIO_Pin_7, RCC_APB2Periph_GPIOA},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel8] = {
        .adc = ADC1,
        .channel = ADC_Channel_8,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOB, GPIO_Pin_0, RCC_APB2Periph_GPIOB},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel9] = {
        .adc = ADC1,
        .channel = ADC_Channel_9,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOB, GPIO_Pin_1, RCC_APB2Periph_GPIOB},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel10] = {
        .adc = ADC1,
        .channel = ADC_Channel_10,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel11] = {
        .adc = ADC1,
        .channel = ADC_Channel_11,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel12] = {
        .adc = ADC1,
        .channel = ADC_Channel_12,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel13] = {
        .adc = ADC1,
        .channel = ADC_Channel_13,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel14] = {
        .adc = ADC1,
        .channel = ADC_Channel_14,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOC, GPIO_Pin_4, RCC_APB2Periph_GPIOC},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC1_Channel15] = {
        .adc = ADC1,
        .channel = ADC_Channel_15,
        .rcc_clk = RCC_APB2Periph_ADC1,
        .adc_gpio = {GPIOC, GPIO_Pin_5, RCC_APB2Periph_GPIOC},
        .irq_channel = ADC1_2_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
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
    },

    [BSP_ADC3_Channel1] = {
        .adc = ADC3,
        .channel = ADC_Channel_1,
        .rcc_clk = RCC_APB2Periph_ADC3,
        .adc_gpio = {GPIOA, GPIO_Pin_1, RCC_APB2Periph_GPIOA},
        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC3_Channel2] = {
        .adc = ADC3,
        .channel = ADC_Channel_2,
        .rcc_clk = RCC_APB2Periph_ADC3,
        .adc_gpio = {GPIOA, GPIO_Pin_2, RCC_APB2Periph_GPIOA},
        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC3_Channel3] = {
        .adc = ADC3,
        .channel = ADC_Channel_3,
        .rcc_clk = RCC_APB2Periph_ADC3,
        .adc_gpio = {GPIOA, GPIO_Pin_3, RCC_APB2Periph_GPIOA},
        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC3_Channel10] = {
        .adc = ADC3,
        .channel = ADC_Channel_10,
        .rcc_clk = RCC_APB2Periph_ADC3,
        .adc_gpio = {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC},
        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC3_Channel11] = {
        .adc = ADC3,
        .channel = ADC_Channel_11,
        .rcc_clk = RCC_APB2Periph_ADC3,
        .adc_gpio = {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC},
        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC3_Channel12] = {
        .adc = ADC3,
        .channel = ADC_Channel_12,
        .rcc_clk = RCC_APB2Periph_ADC3,
        .adc_gpio = {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC},
        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },

    [BSP_ADC3_Channel13] = {
        .adc = ADC3,
        .channel = ADC_Channel_13,
        .rcc_clk = RCC_APB2Periph_ADC3,
        .adc_gpio = {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC},
        .irq_channel = ADC3_IRQn,
        .irq_pre_prio = PREEMPT_PRIO,
        .irq_sub_prio = SUB_PRIO,
    },
};

/**
 * @brief 配置 ADC 中断优先级分组
 * @return 无
 */
void BSP_ADC_PriorityGroupConfig(void)
{
    /* 配置中断优先级分组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

/**
 * @brief 初始化 ADC 对应的 NVIC 中断
 * @param adc_channel_id ADC 通道编号
 * @return 无
 * @note
 * ADC1 和 ADC2 共用 `ADC1_2_IRQn`，
 * ADC3 使用独立的 `ADC3_IRQn`。
 */
static void BSP_ADC_NVIC_Init(bsp_adc_channel_t adc_channel_id)
{
    if(adc_channel_id >= BSP_ADC_Channel_MAX) return;

    const bsp_adc_hw_t *hw = &bsp_adc_hw[adc_channel_id];
    NVIC_InitTypeDef NVIC_InitStruct;

    /* 配置 ADC 中断通道 */
    NVIC_InitStruct.NVIC_IRQChannel = hw->irq_channel;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = hw->irq_pre_prio;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = hw->irq_sub_prio;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStruct);
}

/**
 * @brief 初始化指定 ADC 通道
 * @param adc_channel_id ADC 通道编号
 * @param config ADC 配置参数指针
 * @return 无
 * @details
 * 初始化过程如下：
 * 1. 校验参数合法性；
 * 2. 开启 ADC 与 GPIO 时钟；
 * 3. 将输入引脚配置为模拟输入；
 * 4. 初始化 ADC 基本参数；
 * 5. 设置 ADC 时钟分频；
 * 6. 配置规则组通道与采样时间；
 * 7. 使能 EOC 中断并初始化 NVIC；
 * 8. 使能 ADC；
 * 9. 执行复位校准和启动校准；
 * 10. 启动软件触发转换。
 *
 * @note
 * 当前函数固定使用规则组的第 1 个序列位置。
 *
 * @note
 * 当前 ADC 时钟配置为 `PCLK2 / 8`。
 * 若 APB2 = 72MHz，则 ADC 时钟为 9MHz，满足 STM32F10x ADC 时钟限制。
 */
void BSP_ADC_Init(bsp_adc_channel_t adc_channel_id, bsp_adc_config_t *config)
{
    if(adc_channel_id >= BSP_ADC_Channel_MAX || config == NULL) return;

    const bsp_adc_hw_t *hw = &bsp_adc_hw[adc_channel_id];
    GPIO_InitTypeDef GPIO_InitStruct;
    ADC_InitTypeDef ADC_InitStruct;

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

    /* 配置 ADC时钟 (72M / 8 = 9MHz) */
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    /* 配置通道的转换顺序和采样时间 */
    ADC_RegularChannelConfig(hw->adc, hw->channel, 1, config->sample_time);

    /* 使能 ADC 转化完成中断，配置中断优先级 */
    ADC_ITConfig(hw->adc, ADC_IT_EOC, ENABLE);
    BSP_ADC_NVIC_Init(adc_channel_id);

    /* 使能 ADC */
    ADC_Cmd(hw->adc, ENABLE);

    /* 复位并校准 ADC */
    ADC_ResetCalibration(hw->adc);
    while(ADC_GetResetCalibrationStatus(hw->adc));  /**< 等待复位完成 */

    ADC_StartCalibration(hw->adc);
    while(ADC_GetCalibrationStatus(hw->adc));       /**< 等待校准完成 */

    /* 软件触发 ADC 转换 */
    ADC_SoftwareStartConvCmd(hw->adc, ENABLE);
}

/**
 * @brief 获取指定 ADC 对应的最近一次转换值
 * @param adc_channel_id ADC 通道编号
 * @return ADC 转换结果；若参数非法则返回 0
 * @note
 * 返回值范围通常为 0~4095（12 位 ADC，右对齐时）。
 */
uint16_t BSP_ADC_GetValue(bsp_adc_channel_t adc_channel_id)
{
    if(adc_channel_id >= BSP_ADC_Channel_MAX) return 0;

    const bsp_adc_hw_t *hw = &bsp_adc_hw[adc_channel_id];

    return ADC_GetConversionValue(hw->adc);
}

/**
 * @brief ADC 通用中断处理函数
 * @param adcx ADC 外设指针
 * @return 无
 * @details
 * 当检测到转换完成中断（EOC）后：
 * - 若为 ADC1，则设置 `convert_flag[0] = 1`
 * - 若为 ADC2，则设置 `convert_flag[1] = 1`
 * - 若为 ADC3，则设置 `convert_flag[2] = 1`
 *
 * 随后清除对应 ADC 的中断挂起标志。
 *
 * @note
 * 本函数只做最小化中断处理，不在中断中直接读取/打印数据，
 * 以减少中断处理时间。
 */
void BSP_ADC_IRQHandler(ADC_TypeDef *adcx)
{
    if(ADC_GetITStatus(adcx, ADC_IT_EOC) == SET){
        if(adcx == ADC1){
            convert_flag[0] = 1;
        }else if(adcx == ADC2){
            convert_flag[1] = 1;
        }else if(adcx == ADC3){
            convert_flag[2] = 1;
        }
        ADC_ClearITPendingBit(adcx, ADC_IT_EOC);
    }
}