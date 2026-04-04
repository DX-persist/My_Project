/**
 * @file    bsp_sspi.c
 * @brief   软件 SPI 驱动实现
 * @details
 * 本文件基于 GPIO 模拟实现 SPI 通信时序，提供以下功能：
 * - 软件 SPI 引脚配置
 * - 软件 SPI 初始化
 * - 片选控制
 * - 单字节收发
 *
 * 默认工作模式为 SPI Mode 0：
 * - CPOL = 0
 * - CPHA = 0
 */

#include "bsp_sspi.h"

/**
 * @brief  软件 SPI 引脚配置结构体
 * @details
 * 用于保存一个软件 SPI 实例所需的 GPIO 引脚信息，包括：
 * - 片选引脚 CS
 * - 主机输出从机输入 MOSI
 * - 主机输入从机输出 MISO
 * - 时钟引脚 CLK
 */
typedef struct
{
    bsp_gpio_t sspi_cs;    /**< 片选引脚 */
    bsp_gpio_t sspi_mosi;  /**< 主机输出从机输入引脚 */
    bsp_gpio_t sspi_miso;  /**< 主机输入从机输出引脚 */
    bsp_gpio_t sspi_clk;   /**< SPI 时钟引脚 */
} bsp_spi_sw_t;


/**
 * @brief  软件 SPI 引脚配置表
 * @details
 * 每个软件 SPI 实例对应一组固定的 GPIO 引脚配置。
 */
static const bsp_spi_sw_t bsp_spi_sw[BSP_SSPI_MAX] =
{
    [BSP_SSPI1] =
    {
        .sspi_cs   = {GPIOD, GPIO_Pin_13, RCC_APB2Periph_GPIOD},
        .sspi_mosi = {GPIOE, GPIO_Pin_2,  RCC_APB2Periph_GPIOE},
        .sspi_miso = {GPIOE, GPIO_Pin_3,  RCC_APB2Periph_GPIOE},
        .sspi_clk  = {GPIOE, GPIO_Pin_0,  RCC_APB2Periph_GPIOE},
    },
};


/**
 * @brief  软件 SPI 时序延时
 * @details
 * 在软件 SPI 通信过程中插入适当延时，用于控制时钟翻转速度，
 * 进而控制 SPI 通信速率。
 *
 * @note
 * 可根据 MCU 主频和外设时序要求调整循环次数。
 *
 * @param  None
 * @retval None
 */
static inline void BSP_SSPI_Delay(void)
{
    for(volatile uint8_t i = 0; i < 10; i++){
        __NOP();
    }
}


/**
 * @brief  初始化软件 SPI
 * @details
 * 初始化指定软件 SPI 实例所需的 GPIO，包括：
 * - CS 配置为推挽输出
 * - MOSI 配置为推挽输出
 * - CLK 配置为推挽输出
 * - MISO 配置为浮空输入
 *
 * 同时设置默认引脚电平：
 * - CS 默认拉高，表示未选中从设备
 * - CLK 默认拉低，符合 SPI Mode 0 空闲电平要求
 * - MOSI 默认拉低
 *
 * @param  sspi_id 软件 SPI 编号
 * @retval None
 */
void BSP_SSPI_Init(bsp_sspi_t sspi_id)
{
    if(sspi_id >= BSP_SSPI_MAX) return;

    const bsp_spi_sw_t *sw = &bsp_spi_sw[sspi_id];
    uint32_t enable_clk = 0;
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_StructInit(&GPIO_InitStruct);

    /* 开启 GPIO 时钟 */
    enable_clk = sw->sspi_cs.rcc_clk | sw->sspi_mosi.rcc_clk |
                 sw->sspi_miso.rcc_clk | sw->sspi_clk.rcc_clk;
    RCC_APB2PeriphClockCmd(enable_clk, ENABLE);

    /* 初始化 GPIO 默认电平 */
    BSP_GPIO_WritePin(&sw->sspi_cs, Bit_SET);      /* 默认不选中 */
    BSP_GPIO_WritePin(&sw->sspi_clk, Bit_RESET);   /* SPI Mode 0 时钟空闲低电平 */
    BSP_GPIO_WritePin(&sw->sspi_mosi, Bit_RESET);  /* MOSI 默认输出低电平 */

    /* 配置 CS 为推挽输出 */
    GPIO_InitStruct.GPIO_Pin = sw->sspi_cs.gpio_pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(sw->sspi_cs.gpio_port, &GPIO_InitStruct);

    /* 配置 MOSI 为推挽输出 */
    GPIO_InitStruct.GPIO_Pin = sw->sspi_mosi.gpio_pin;
    GPIO_Init(sw->sspi_mosi.gpio_port, &GPIO_InitStruct);

    /* 配置 CLK 为推挽输出 */
    GPIO_InitStruct.GPIO_Pin = sw->sspi_clk.gpio_pin;
    GPIO_Init(sw->sspi_clk.gpio_port, &GPIO_InitStruct);

    /* 配置 MISO 为浮空输入 */
    GPIO_InitStruct.GPIO_Pin = sw->sspi_miso.gpio_pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(sw->sspi_miso.gpio_port, &GPIO_InitStruct);
}


/**
 * @brief  控制 SPI 片选信号
 * @details
 * 根据输入参数设置指定软件 SPI 实例的片选引脚电平，
 * 用于选中或释放 SPI 从设备。
 *
 * @param  sspi_id 软件 SPI 编号
 * @param  option  片选状态
 *         - CHIP_SELECTED：选中从设备
 *         - CHIP_NON_SELECTED：取消选中从设备
 *
 * @retval None
 */
void BSP_SSPI_CtlChip(bsp_sspi_t sspi_id, uint8_t option)
{
    if(sspi_id >= BSP_SSPI_MAX) return;

    const bsp_spi_sw_t *sw = &bsp_spi_sw[sspi_id];

    BSP_GPIO_WritePin(&sw->sspi_cs, option);
}


/**
 * @brief  软件 SPI 读写一个字节
 * @details
 * 通过 GPIO 模拟 SPI 时序，实现 1 字节数据的发送与接收。
 *
 * SPI 为全双工通信方式，因此在发送 1 字节数据的同时，
 * 也会从从设备接收 1 字节数据。
 *
 * 默认采用 SPI Mode 0：
 * - CPOL = 0：时钟空闲电平为低
 * - CPHA = 0：数据在时钟上升沿采样
 *
 * 通信时序如下：
 * 1. 主机在 MOSI 上输出当前数据位
 * 2. 产生时钟上升沿
 * 3. 在上升沿后读取 MISO 电平
 * 4. 拉低时钟，准备下一位传输
 *
 * 数据传输顺序为高位优先（MSB First）。
 *
 * @param  sspi_id   软件 SPI 编号
 * @param  writebyte 待发送的 1 字节数据
 * @param  readbyte  接收数据存储地址
 *         - 非 NULL：保存接收到的 1 字节数据
 *         - NULL：忽略接收数据
 *
 * @retval None
 *
 * @warning
 * 调用本函数前，应确保目标从设备已被正确选中。
 */
void BSP_SSPI_ReadWriteByte(bsp_sspi_t sspi_id, uint8_t writebyte, uint8_t *readbyte)
{
    if(sspi_id >= BSP_SSPI_MAX) return;

    const bsp_spi_sw_t *sw = &bsp_spi_sw[sspi_id];
    uint8_t temp_data = 0;

    for(int i = 0; i < 8; i++){

        /* 发送当前最高位 */
        if(writebyte & 0x80){
            BSP_GPIO_WritePin(&sw->sspi_mosi, Bit_SET);
        }else{
            BSP_GPIO_WritePin(&sw->sspi_mosi, Bit_RESET);
        }

        writebyte <<= 1;

        BSP_SSPI_Delay();

        /* 时钟上升沿 */
        BSP_GPIO_WritePin(&sw->sspi_clk, Bit_SET);

        BSP_SSPI_Delay();

        /* 读取 MISO */
        temp_data <<= 1;
        if(BSP_GPIO_ReadPin(&sw->sspi_miso)){
            temp_data |= 0x01;
        }

        /* 时钟下降沿 */
        BSP_GPIO_WritePin(&sw->sspi_clk, Bit_RESET);

        BSP_SSPI_Delay();
    }

    /* 返回接收到的数据 */
    if(readbyte != NULL){
        *readbyte = temp_data;
    }
}