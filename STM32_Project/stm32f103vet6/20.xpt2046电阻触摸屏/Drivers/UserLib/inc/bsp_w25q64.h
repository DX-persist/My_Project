#ifndef BSP_W25Q64_H
#define BSP_W25Q64_H

#include "bsp_hspi.h"
#include "bsp_usart.h"
#include "bsp_delay.h"
#include "stm32f10x.h"
#include <math.h>

/**
 * @brief 调试日志宏，开启 W25Q64_DEBUG_ON 后输出调试信息
 */
//#define W25Q64_DEBUG_ON
#ifdef W25Q64_DEBUG_ON
    #include <stdio.h>
    #define W25Q64_LOG(format, ...) printf(format, ##__VA_ARGS__)
#else
    #define W25Q64_LOG(format, ...)
#endif

/** @brief W25Q64 使用的 SPI 总线编号 */
#define BSP_W25Q64_SPI_ID           BSP_HSPI1

/** @brief W25Q64 片选引脚时钟 */
#define W25Q64_CS_CLK               RCC_APB2Periph_GPIOC
/** @brief W25Q64 片选引脚端口 */
#define W25Q64_CS_PORT              GPIOC
/** @brief W25Q64 片选引脚 */
#define W25Q64_CS_PIN               GPIO_Pin_0

/** @brief W25Q64 的 JEDEC ID，用于验证芯片型号是否正确 */
#define W25Q64_JEDEC_ID             0xEF4017

/** @defgroup W25Q64_Commands W25Q64 指令集
 * @{
 */
#define W25Q64_READ_JEDEC_ID_CMD    0x9F    /**< 读取 JEDEC ID 指令 */
#define W25Q64_WRITE_ENABLE_CMD     0x06    /**< 写使能指令，写入/擦除前必须先发送 */
#define W25Q64_READ_REGISTER1_CMD   0x05    /**< 读取状态寄存器1指令 */
#define W25Q64_SECTOR_ERASE_CMD     0x20    /**< 扇区擦除指令，每次擦除4KB */
#define W25Q64_PAGE_WRITE_CMD       0x02    /**< 页写入指令，每页最多256字节 */
#define W25Q64_READ_DATA_CMD        0x03    /**< 读取数据指令 */
#define W25Q64_DUMMY_CMD            0x00    /**< 伪指令，用于驱动 SCK 时钟以接收从机数据 */
/** @} */

/** @brief 每页最大写入字节数 */
#define W25Q64_PAGE_WRITE_MAX       256

/** @brief 每个扇区大小(4k = 4096Bytes) */
#define W25Q64_SECTOR_SIZE			4096

/** @brief 总共有多少扇区(一共有128个块，每个块有16个扇区) */
#define W25Q64_SECTOR_COUNT			2048

/** @brief 擦除的最小单位(最小单位是扇区) */
#define W25Q64_ERASE_BLOCK_SIZE		1

/** @brief 状态寄存器1 BUSY 标志位掩码，为1表示内部正在写入 */
#define W25Q64_REGISTER_BUSY        0x01

/** @defgroup W25Q64_TestAddr 测试用地址
 * @{
 */
#define W25Q64_ERASE_ADDR           0x000000    /**< 测试擦除地址 */
#define W25Q64_WRITE_ADDR           W25Q64_ERASE_ADDR   /**< 测试写入地址 */
#define W25Q64_READ_ADDR            W25Q64_ERASE_ADDR   /**< 测试读取地址 */
/** @} */

/** @brief 扇区擦除超时时间（ms），W25Q64 典型擦除时间约 150ms，最大 400ms */
#define W25Q64_TIMEOUT_SECTOR_ERASE 500
/** @brief 页写入超时时间（ms），W25Q64 典型页写入时间约 0.7ms，最大 3ms */
#define W25Q64_TIMEOUT_PAGE_PROGRAM 5

/**
 * @brief  控制 GPIO 引脚电平
 * @note   直接操作 BSRR 寄存器，效率高于库函数
 * @param  gpio_port     GPIO 端口
 * @param  gpio_pin_mask GPIO 引脚掩码
 * @param  level         电平，SET 为高电平，RESET 为低电平
 * @retval 无
 */
static inline void BSP_W25Q64_WritePin(GPIO_TypeDef *gpio_port, uint16_t gpio_pin_mask, uint8_t level)
{
    if (level) {
        gpio_port->BSRR = ((uint32_t)gpio_pin_mask);
    } else {
        gpio_port->BSRR = (((uint32_t)gpio_pin_mask) << 16);
    }
}

/** @brief 拉低 CS 片选信号，选中 W25Q64 */
#define W25Q64_CS_ENABLE()          BSP_W25Q64_WritePin(W25Q64_CS_PORT, W25Q64_CS_PIN, RESET)
/** @brief 拉高 CS 片选信号，取消选中 W25Q64 */
#define W25Q64_CS_DISABLE()         BSP_W25Q64_WritePin(W25Q64_CS_PORT, W25Q64_CS_PIN, SET)


/**
 * @brief  初始化 W25Q64 及相应的 SPI 硬件总线
 */
extern void     BSP_W25Q64_Init(void);

/**
 * @brief  读取 W25Q64 芯片的 JEDEC ID (厂商及设备ID)
 * @retval 24位 ID 组合 (高 8 位厂商 ID，低 16 位设备 ID)
 */
extern uint32_t BSP_W25Q64_ReadID(void);

/**
 * @brief  擦除 W25Q64 指定的扇区 (4KB 大小)
 * @param  erase_addr 要擦除的地址，建议对齐到 4096 字节
 */
extern void     BSP_W25Q64_Sector_Erase(uint32_t erase_addr);

/**
 * @brief  向 W25Q64 写入大量不定长数据
 * @param  write_buffer 指向要写入数据的缓冲区的指针
 * @param  size         需要写入的连续字节数
 * @param  write_addr   写入的目标起始地址
 */
extern void     BSP_W25Q64_BufferWrite(uint8_t *write_buffer, uint16_t size, uint32_t write_addr);

/**
 * @brief  从 W25Q64 指定位置读取连续数据
 * @param  read_buffer  指向存储读取数据的缓冲区的指针
 * @param  size         需要读取的字节数
 * @param  read_addr    读取的起始地址
 */
extern void     BSP_W25Q64_BufferRead(uint8_t *read_buffer, uint16_t size, uint32_t read_addr);

/**
 * @brief  执行 W25Q64 全面的读写测试
 */
extern void     BSP_W25Q64_Test(void);

#endif /* BSP_W25Q64_H */
