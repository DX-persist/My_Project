/**
 * @file    bsp_si2c.c
 * @author  Antigravity
 * @brief   软件模拟I2C驱动实现文件
 * @version V1.0
 * @date    2026-02-15
 * @note    本文件实现了I2C协议的底层时序模拟，包括起始信号、停止信号、字节收发等。
 *          通过直接操作寄存器(BSRR/IDR)实现高效的引脚控制。
 */

#include "bsp_si2c.h"

/**
 * @brief 软件I2C引脚定义结构体
 */
typedef struct{
    GPIO_TypeDef *scl_port;     /*!< SCL端口 */
    uint16_t scl_pin;           /*!< SCL引脚 */
    uint32_t scl_clk;           /*!< SCL时钟 */

    GPIO_TypeDef *sda_port;     /*!< SDA端口 */
    uint16_t sda_pin;           /*!< SDA引脚 */
    uint32_t sda_clk;           /*!< SDA时钟 */
} bsp_i2c_sw_t;

/** @brief I2C硬件引脚配置表 */
static const bsp_i2c_sw_t bsp_i2c_sw[BSP_SI2C_MAX] = {
    [BSP_SI2C1] = {
        .scl_port = GPIOB,
        .scl_pin = GPIO_Pin_6,
        .scl_clk = RCC_APB2Periph_GPIOB,
        .sda_port = GPIOB,
        .sda_pin = GPIO_Pin_7,
        .sda_clk = RCC_APB2Periph_GPIOB
    },

    [BSP_SI2C2] = {
        .scl_port = GPIOC,
        .scl_pin = GPIO_Pin_6,
        .scl_clk = RCC_APB2Periph_GPIOC,
        .sda_port = GPIOC,
        .sda_pin = GPIO_Pin_7,
        .sda_clk = RCC_APB2Periph_GPIOC
    }
};

/**
 * @brief  I2C GPIO初始化函数
 * @param  i2c_id I2C通道ID
 * @retval 无
 * @note   初始化SCL和SDA引脚为开漏输出模式(GPIO_Mode_Out_OD)。
 *         这里在初始化之前操作BSRR寄存器的目的是为了防止产生毛刺。
 *         因为STM32默认上电后大部分引脚为浮空输入，但是由于I2C
 *         总线上都会有上拉电阻，所以此时总线电平为1。
 *         如果在GPIO_Init函数之后再配置scl和sda引脚为1，势必会出现 1 --> 0 --> 1 的过程。
 *         如果在 SCL 上产生一个这样的脉冲，可能会被认为是一个时钟周期，导致数据错位。
 *         所以先将 ODR 寄存器的值设为1，最后调用GPIO_Init函数后它默认就为高电平，
 *         从而消除了干扰信号。
 */
void BSP_SI2C_Init(bsp_si2c_t i2c_id)
{
    if(i2c_id >= BSP_SI2C_MAX) return;

    const bsp_i2c_sw_t *sw = &bsp_i2c_sw[i2c_id];
    GPIO_InitTypeDef GPIO_InitStruct;
    
    GPIO_StructInit(&GPIO_InitStruct);

    /* 开启软件I2C引脚所在的端口时钟 */
    RCC_APB2PeriphClockCmd(sw->scl_clk | sw->sda_clk, ENABLE);

    /* 将 SCL 和 SDA 全部配置为高电平表示总线空闲 */
    sw->scl_port->BSRR = (uint32_t)sw->scl_pin;
    sw->sda_port->BSRR = (uint32_t)sw->sda_pin;

    /* 配置软件I2C引脚的模式、速度 */
    GPIO_InitStruct.GPIO_Pin = sw->scl_pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(sw->scl_port, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = sw->sda_pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(sw->sda_port, &GPIO_InitStruct);
}

/**
 * @brief  写入引脚电平 (内联函数)
 * @param  gpio_port     GPIO 端口
 * @param  gpio_pin_mask 引脚掩码
 * @param  level         电平状态 (1:高, 0:低)
 * @retval 无
 * @note   inline关键字建议编译器将函数代码直接“粘贴”到调用处，作用等同于宏定义。
 *         直接操作寄存器是为了防止函数调用过程中的上下文切换浪费时间。
 *         BSRR 寄存器高16位用于复位(0)，低16位用于置位(1)。
 */
static inline void BSP_SI2C_WritePin(GPIO_TypeDef *gpio_port, uint16_t gpio_pin_mask, uint8_t level)
{
    if(level){
        gpio_port->BSRR = (uint32_t)gpio_pin_mask;
    }else{
        gpio_port->BSRR = (((uint32_t)gpio_pin_mask) << 16);
    }
}

/**
 * @brief  读取引脚电平 (内联函数)
 * @param  gpio_port     GPIO 端口
 * @param  gpio_pin_mask 引脚掩码
 * @retval 1:高电平, 0:低电平
 * @note   直接读取IDR寄存器
 */
static inline uint8_t BSP_SI2C_ReadPin(GPIO_TypeDef *gpio_port, uint16_t gpio_pin_mask)
{
    /* 检测 SDA 线上的数据是否为高电平 */
    return ((gpio_port->IDR & gpio_pin_mask) ? 1 : 0);
}

/**
 * @brief  产生Start信号
 * @param  i2c_id I2C通道ID
 * @retval 无
 * @note   SCL=High时，SDA由High跳变到Low。
 *         将 SCL 拉低的作用是钳住总线，准备发送数据。
 *         I2C 协议规定只有当 SCL 为低电平的时候才允许 SDA 数据翻转。
 */
void BSP_SI2C_Start(bsp_si2c_t i2c_id)
{
    if(i2c_id >= BSP_SI2C_MAX) return;

    const bsp_i2c_sw_t *sw = &bsp_i2c_sw[i2c_id];    

    /* 确保 SDA 和 SCL 处于高电平(空闲状态) */
    BSP_SI2C_WritePin(sw->sda_port, sw->sda_pin, SET);
    BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, SET);
    BSP_Delay_us(5);

    /* 起始信号：当 SCL 为高电平时 SDA 产生下降沿 */
    BSP_SI2C_WritePin(sw->sda_port, sw->sda_pin, RESET);
    BSP_Delay_us(5);

    /* SCL拉低钳住总线，以便后续数据电平变化 */
    BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, RESET);
    BSP_Delay_us(5);
}

/**
 * @brief  产生Stop信号
 * @param  i2c_id I2C通道ID
 * @retval 无
 * @note   SCL=High时，SDA由Low跳变到High
 */
void BSP_SI2C_Stop(bsp_si2c_t i2c_id)
{
    if(i2c_id >= BSP_SI2C_MAX) return;

    const bsp_i2c_sw_t *sw = &bsp_i2c_sw[i2c_id];

    /* 确保 SCL 和 SDA 都为低电平 (准备产生上升沿) */
    BSP_SI2C_WritePin(sw->sda_port, sw->sda_pin, RESET);
    BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, RESET);
    BSP_Delay_us(5);

    /* 将 SCL 拉高 */
    BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, SET);
    BSP_Delay_us(5);
    
    /* 停止信号：在 SCL为高电平期间 SDA 产生上升沿 */
    BSP_SI2C_WritePin(sw->sda_port, sw->sda_pin, SET);
    BSP_Delay_us(5);
}

/**
 * @brief  向总线发送一个字节
 * @param  i2c_id I2C通道ID
 * @param  byte   要发送的数据
 * @retval 无
 */
void BSP_SI2C_SendByte(bsp_si2c_t i2c_id, uint8_t byte)
{
    if(i2c_id >= BSP_SI2C_MAX) return;

    const bsp_i2c_sw_t *sw = &bsp_i2c_sw[i2c_id];
    for(int i = 0; i < 8; i++){
        /* 获取要写入数据最高位的值, MSB先行 */
        if(byte & 0x80){
            BSP_SI2C_WritePin(sw->sda_port, sw->sda_pin, SET);
        }else{
            BSP_SI2C_WritePin(sw->sda_port, sw->sda_pin, RESET);
        }
        BSP_Delay_us(5);    /* 给 SDA 线上一些时间用来建立数据 */    

        /* 将 SCL 拉高表示此时数据有效，从机读取 */
        BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, SET);
        BSP_Delay_us(5);
        
        /* 将 SCL 拉低表示允许 SDA 线上的数据进行翻转(准备下一位) */
        BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, RESET);
        BSP_Delay_us(5);
        
        byte <<= 1; /* 移出最高位 */
    }
}

/**
 * @brief  等待从机的应答信号 (ACK)
 * @param  i2c_id I2C通道ID
 * @retval ACK: 成功接收到应答 (从机拉低SDA)
 * @retval NACK: 未接收到应答 (从机未拉低SDA)
 */
si2c_ack_status BSP_SI2C_WaitAck(bsp_si2c_t i2c_id)
{
    if(i2c_id >= BSP_SI2C_MAX) return NACK;
    const bsp_i2c_sw_t *sw = &bsp_i2c_sw[i2c_id];
    si2c_ack_status ack_status = NACK;

    /* 主机释放 SDA 线(拉高)，将控制权交给从机 */
    BSP_SI2C_WritePin(sw->sda_port, sw->sda_pin, SET);
    BSP_Delay_us(5);

    /* 主机拉高 SCL，读取从机状态 */
    BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, SET);
    BSP_Delay_us(5);        
    
    /* 读取 SDA: 0=ACK, 1=NACK */
    if(!BSP_SI2C_ReadPin(sw->sda_port, sw->sda_pin)){
        ack_status = ACK;
    }else{
        ack_status = NACK;
    }
    
    /* 将 SCL 拉回低电平，完成一个时钟周期 */
    BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, RESET);
    BSP_Delay_us(5);

    return ack_status;
}

/**
 * @brief  主机主动发送应答信号 (ACK)
 * @param  i2c_id I2C通道ID
 * @retval 无
 */
void BSP_SI2C_Ack(bsp_si2c_t i2c_id)
{
    if(i2c_id >= BSP_SI2C_MAX) return;
    const bsp_i2c_sw_t *sw = &bsp_i2c_sw[i2c_id];

    /* 主机拉低 SDA 表示应答 */
    BSP_SI2C_WritePin(sw->sda_port, sw->sda_pin, RESET);
    BSP_Delay_us(5);

    /* 产生时钟脉冲 */
    BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, SET);
    BSP_Delay_us(5);

    BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, RESET);
    BSP_Delay_us(5);

    /* 释放 SDA 线 */
    BSP_SI2C_WritePin(sw->sda_port, sw->sda_pin, SET);
    BSP_Delay_us(5);
}

/**
 * @brief  主机主动发送非应答信号 (NACK)
 * @param  i2c_id I2C通道ID
 * @retval 无
 */
void BSP_SI2C_NAck(bsp_si2c_t i2c_id)
{
    if(i2c_id >= BSP_SI2C_MAX) return;
    const bsp_i2c_sw_t *sw = &bsp_i2c_sw[i2c_id];

    /* 主机拉高 SDA 表示非应答 */
    BSP_SI2C_WritePin(sw->sda_port, sw->sda_pin, SET);
    BSP_Delay_us(5);

    /* 产生时钟脉冲 */
    BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, SET);
    BSP_Delay_us(5);

    BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, RESET);
    BSP_Delay_us(5);

    /* 释放 SDA 线 */
    BSP_SI2C_WritePin(sw->sda_port, sw->sda_pin, SET);
    BSP_Delay_us(5);
}

/**
 * @brief  从总线读取一个字节
 * @param  i2c_id I2C通道ID
 * @retval 读取到的 8 位数据
 */
uint8_t BSP_SI2C_ReadByte(bsp_si2c_t i2c_id)
{
    if(i2c_id >= BSP_SI2C_MAX) return 0;
    const bsp_i2c_sw_t *sw = &bsp_i2c_sw[i2c_id];
    bool bit = 1;
    uint8_t data = 0;

    /* 主机释放SDA线，设为输入模式 */
    BSP_SI2C_WritePin(sw->sda_port, sw->sda_pin, SET);
    BSP_Delay_us(5);

    for(int i = 0; i < 8; i++){
        /* 主机拉高 SCL 通知从机数据有效 */
        BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, SET);
        BSP_Delay_us(5);

        /* 读取数据位 */
        if(BSP_SI2C_ReadPin(sw->sda_port, sw->sda_pin)){
            bit = 1;
        }else{
            bit = 0;
        }
        data <<= 1;
        data |= bit;

        /* 拉低SCL准备下一位 */
        BSP_SI2C_WritePin(sw->scl_port, sw->scl_pin, RESET);
        BSP_Delay_us(5);
    }
    return data;
}

/**
 * @brief  向从设备的指定内部寄存器写入单字节数据
 * @param  i2c_id       I2C通道ID
 * @param  device_addr  从设备物理地址 (最低位会在函数中被强制清0，表示写模式)
 * @param  reg_addr     内部寄存器地址 / 内存地址
 * @param  data         要写入的单字节数据
 * @retval 0 表示写入成功，1 表示通信过程发生未响应错误
 * @note   通信序列: Start -> 设备地址(W) -> 寄存器地址 -> 数据 -> Stop
 */
uint8_t BSP_SI2C_WriteReg(bsp_si2c_t i2c_id, uint8_t device_addr, uint8_t reg_addr, uint8_t data)
{
    if(i2c_id >= BSP_SI2C_MAX) return 1;

    /* 发送起始信号 */
    BSP_SI2C_Start(i2c_id);
    
    /* 发送设备地址(写) */
    BSP_SI2C_SendByte(i2c_id, device_addr & 0xFE);
    if(BSP_SI2C_WaitAck(i2c_id) != ACK){
        BSP_SI2C_Stop(i2c_id);
        return 1;
    }
    
    /* 发送寄存器地址 */
    BSP_SI2C_SendByte(i2c_id, reg_addr);
    if(BSP_SI2C_WaitAck(i2c_id) != ACK){
        BSP_SI2C_Stop(i2c_id);
        return 1;
    }
    
    /* 发送数据 */
    BSP_SI2C_SendByte(i2c_id, data);
    if(BSP_SI2C_WaitAck(i2c_id) != ACK){
        BSP_SI2C_Stop(i2c_id);
        return 1;
    }
    
    /* 发送结束信号 */
    BSP_SI2C_Stop(i2c_id);

    return 0;
}

/**
 * @brief  从从设备的指定内部寄存器读取单字节数据
 * @param  i2c_id       I2C通道ID
 * @param  device_addr  从设备物理地址
 * @param  reg_addr     要读取的内部寄存器地址 / 内存地址
 * @param  data         用于保存读取结果的数据指针
 * @retval 0 表示读取成功，1 表示通信过程发生未响应错误
 * @note   通信序列: Start -> 设备地址(W) -> 寄存器地址 -> 重新Start -> 设备地址(R) -> 读取数据 -> NACK -> Stop
 */
uint8_t BSP_SI2C_ReadReg(bsp_si2c_t i2c_id, uint8_t device_addr, uint8_t reg_addr, uint8_t *data)
{
    if(i2c_id >= BSP_SI2C_MAX) return 1;

    /* 1. 发送 Start & 写地址 */
    BSP_SI2C_Start(i2c_id);
    BSP_SI2C_SendByte(i2c_id, device_addr & 0xFE);
    if(BSP_SI2C_WaitAck(i2c_id) != ACK){
        BSP_SI2C_Stop(i2c_id);
        return 1;
    }
    
    /* 2. 发送寄存器地址 */
    BSP_SI2C_SendByte(i2c_id, reg_addr);
    if(BSP_SI2C_WaitAck(i2c_id) != ACK){
        BSP_SI2C_Stop(i2c_id);
        return 1;
    }

    /* 3. 重复 Start & 读地址 */
    BSP_SI2C_Start(i2c_id);
    BSP_SI2C_SendByte(i2c_id, device_addr | 0x01);
    if(BSP_SI2C_WaitAck(i2c_id) != ACK){
        BSP_SI2C_Stop(i2c_id);
        return 1;
    }
    
    /* 4. 读取数据 */
    *data = BSP_SI2C_ReadByte(i2c_id);

    /* 5. 发送 NACK & Stop */
    BSP_SI2C_NAck(i2c_id);
    BSP_SI2C_Stop(i2c_id);

    return 0;
}

/**
 * @brief  向从设备寄存器连续写入多字节数据 (例如 EEPROM 页写操作)
 * @param  i2c_id       I2C通道ID
 * @param  device_addr  从设备物理地址
 * @param  reg_addr     内部起始寄存器地址
 * @param  data         传入待写入数据的连续缓冲数组指针
 * @param  size         需要写入的连续字节总数
 * @retval 0 表示成功，1 表示写过程发生无响应错误
 */
uint8_t BSP_SI2C_WriteBuffer(bsp_si2c_t i2c_id, uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint16_t size)
{
    if(i2c_id >= BSP_SI2C_MAX) return 1;

    BSP_SI2C_Start(i2c_id);
    
    BSP_SI2C_SendByte(i2c_id, device_addr & 0xFE);
    if(BSP_SI2C_WaitAck(i2c_id) != ACK){
        BSP_SI2C_Stop(i2c_id);
        return 1;
    }
    
    BSP_SI2C_SendByte(i2c_id, reg_addr);
    if(BSP_SI2C_WaitAck(i2c_id) != ACK){
        BSP_SI2C_Stop(i2c_id);
        return 1;
    }

    /* 循环发送 buffer 数据 */
    for(int i = 0; i < size; i++){
        BSP_SI2C_SendByte(i2c_id, data[i]);
        if(BSP_SI2C_WaitAck(i2c_id) != ACK){
            BSP_SI2C_Stop(i2c_id);
            return 1;
        }
    }
    
    BSP_SI2C_Stop(i2c_id);

    return 0;
}

/**
 * @brief  自从设备寄存器中连续读取多字节数据
 * @param  i2c_id       I2C通道ID
 * @param  device_addr  从设备物理地址
 * @param  reg_addr     内部起始寄存器地址
 * @param  data         存取读取数据的缓冲数组指针
 * @param  size         需要读取的连续字节总数
 * @retval 0 表示成功，1 表示通信过程发生未响应错误
 * @note   在接收过程中，接收完前(size-1)个字节应全回复 ACK，收到最后一个字后节必须回复 NACK
 */
uint8_t BSP_SI2C_ReadBuffer(bsp_si2c_t i2c_id, uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint16_t size)
{
    if(i2c_id >= BSP_SI2C_MAX) return 1;

    BSP_SI2C_Start(i2c_id);
    
    BSP_SI2C_SendByte(i2c_id, device_addr & 0xFE);
    if(BSP_SI2C_WaitAck(i2c_id) != ACK){
        BSP_SI2C_Stop(i2c_id);
        return 1;
    }

    BSP_SI2C_SendByte(i2c_id, reg_addr);
    if(BSP_SI2C_WaitAck(i2c_id) != ACK){
        BSP_SI2C_Stop(i2c_id);
        return 1;
    }

    BSP_SI2C_Start(i2c_id);
    BSP_SI2C_SendByte(i2c_id, device_addr | 0x01);
    if(BSP_SI2C_WaitAck(i2c_id) != ACK){
        BSP_SI2C_Stop(i2c_id);
        return 1;
    }

    /* 循环读取 buffer 数据 */
    for(int i = 0; i < size; i++){
        data[i] = BSP_SI2C_ReadByte(i2c_id);
        if(i == size - 1){
            BSP_SI2C_NAck(i2c_id);  /* 最后一个字节必须发 NACK */
        }else{
            BSP_SI2C_Ack(i2c_id);   /* 中间字节发 ACK */
        }
    }
    BSP_SI2C_Stop(i2c_id);

    return 0;
}
