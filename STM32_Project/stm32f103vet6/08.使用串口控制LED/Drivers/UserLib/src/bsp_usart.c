#include "bsp_usart.h"

typedef struct {
    /* USART外设配置 */
    USART_TypeDef*  usartx;              
    uint32_t        baud_rate;           
    uint16_t        word_length;      
    uint16_t        stop_bits;  
    uint16_t        parity; 
    uint16_t        mode;
    uint16_t        flow_control;        

    /* GPIO配置 */
    GPIO_TypeDef*   tx_port;             
    uint16_t        tx_pin;
    GPIO_TypeDef*   rx_port;
    uint16_t        rx_pin;

    /* 时钟配置 */
    uint32_t        usart_clk;
    uint32_t        gpio_clk;
    bsp_usart_bus_t bus;

    /* 串口中断配置 */
    IRQn_Type       irq_n;      /* 中断源 */
    uint16_t        irq_it;     /* 哪个标志位产生了中断 */
    uint8_t         irq_preempt_prio;
    uint8_t         irq_sub_prio;

} bsp_usart_hw_t;

volatile uint8_t g_rx_flag = 0;
volatile uint8_t recv_data = 0;

/* 给标准C库函数用 */
bsp_usart_t g_stdio_usart = BSP_USART1;


static const bsp_usart_hw_t bsp_usart_hw[BSP_USART_MAX] = {
    [BSP_USART1] = {
        /* USART1 配置 */
        .usartx             = USART1,
        .baud_rate          = BAUDRATE_DEFAULT,
        .word_length        = USART_WordLength_8b,
        .stop_bits          = USART_StopBits_1,
        .parity             = USART_Parity_No,
        .mode               = USART_Mode_Rx | USART_Mode_Tx,
        .flow_control       = USART_HardwareFlowControl_None,

        /* GPIO 配置：TX(PA9) RX(PA10) */
        .tx_port            = GPIOA,
        .tx_pin             = GPIO_Pin_9,
        .rx_port            = GPIOA,
        .rx_pin             = GPIO_Pin_10,
        
        /* 时钟配置：USART1 使用 APB2 时钟，剩余串口使用 APB1 时钟 */
        .usart_clk          = RCC_APB2Periph_USART1,
        .gpio_clk           = RCC_APB2Periph_GPIOA,
        .bus                = USART_BUS_APB2,

        /* 中断相关配置 */
        .irq_n              = USART1_IRQn,              /* 串口1的中断源 */
        .irq_it             = USART_IT_RXNE,            /* 判断串口1的 RXNE 标志位是否产生了中断 */
        .irq_preempt_prio   = BSP_USART_PREEMPT_PRIO,   /* 抢占优先级 */
        .irq_sub_prio       = BSP_USART_SUB_PRIO,       /* 响应优先级 */

    },
    [BSP_USART2] = {
        /* USART2 配置 */
        .usartx             = USART2,
        .baud_rate          = BAUDRATE_DEFAULT,
        .word_length        = USART_WordLength_8b,
        .stop_bits          = USART_StopBits_1,
        .parity             = USART_Parity_No,
        .mode               = USART_Mode_Rx | USART_Mode_Tx,
        .flow_control       = USART_HardwareFlowControl_None,

        /* GPIO 配置：TX(PA2) RX(PA3) */
        .tx_port            = GPIOA,
        .tx_pin             = GPIO_Pin_2,
        .rx_port            = GPIOA,
        .rx_pin             = GPIO_Pin_3,
        
        /* 时钟配置 */
        .usart_clk          = RCC_APB1Periph_USART2,
        .gpio_clk           = RCC_APB2Periph_GPIOA,
        .bus                = USART_BUS_APB1,

        /* 中断相关配置 */
        .irq_n              = USART2_IRQn,              /* 串口2的中断源 */
        .irq_it             = USART_IT_RXNE,            /* 判断串口2的 RXNE 标志位是否产生了中断 */
        .irq_preempt_prio   = BSP_USART_PREEMPT_PRIO,   /* 抢占优先级 */
        .irq_sub_prio       = BSP_USART_SUB_PRIO,       /* 响应优先级 */
    },
    [BSP_USART3] = {
        /* USART3 配置 */
        .usartx             = USART3,
        .baud_rate          = BAUDRATE_DEFAULT,
        .word_length        = USART_WordLength_8b,
        .stop_bits          = USART_StopBits_1,
        .parity             = USART_Parity_No,
        .mode               = USART_Mode_Rx | USART_Mode_Tx,
        .flow_control       = USART_HardwareFlowControl_None,

        /* GPIO 配置：TX(PB10) RX(PB11) */
        .tx_port            = GPIOB,
        .tx_pin             = GPIO_Pin_10,
        .rx_port            = GPIOB,
        .rx_pin             = GPIO_Pin_11,
        
        /* 时钟配置 */
        .usart_clk          = RCC_APB1Periph_USART3,
        .gpio_clk           = RCC_APB2Periph_GPIOB,
        .bus                = USART_BUS_APB1,

        /* 中断相关配置 */
        .irq_n              = USART3_IRQn,              /* 串口3的中断源 */
        .irq_it             = USART_IT_RXNE,            /* 判断串口3的 RXNE 标志位是否产生了中断 */
        .irq_preempt_prio   = BSP_USART_PREEMPT_PRIO,   /* 抢占优先级 */
        .irq_sub_prio       = BSP_USART_SUB_PRIO,       /* 响应优先级 */
    },
    [BSP_UART4] = {
        /* UART4 配置*/
        .usartx             = UART4,
        .baud_rate          = BAUDRATE_DEFAULT,
        .word_length        = USART_WordLength_8b,
        .stop_bits          = USART_StopBits_1,
        .parity             = USART_Parity_No,
        .mode               = USART_Mode_Rx | USART_Mode_Tx,
        .flow_control       = USART_HardwareFlowControl_None,

        /* GPIO 配置：TX(PC10) RX(PC11) */
        .tx_port            = GPIOC,
        .tx_pin             = GPIO_Pin_10,
        .rx_port            = GPIOC,
        .rx_pin             = GPIO_Pin_11,
        
        /* 时钟配置 */
        .usart_clk          = RCC_APB1Periph_UART4,
        .gpio_clk           = RCC_APB2Periph_GPIOC,
        .bus                = USART_BUS_APB1,

        /* 中断相关配置 */
        .irq_n              = UART4_IRQn,               /* 串口4的中断源 */
        .irq_it             = USART_IT_RXNE,            /* 判断串口4的 RXNE 标志位是否产生了中断 */
        .irq_preempt_prio   = BSP_USART_PREEMPT_PRIO,   /* 抢占优先级 */
        .irq_sub_prio       = BSP_USART_SUB_PRIO,       /* 响应优先级 */

    },
    [BSP_UART5] = {
        /* UART5 配置*/
        .usartx             = UART5,
        .baud_rate          = BAUDRATE_DEFAULT,
        .word_length        = USART_WordLength_8b,
        .stop_bits          = USART_StopBits_1,
        .parity             = USART_Parity_No,
        .mode               = USART_Mode_Rx | USART_Mode_Tx,
        .flow_control       = USART_HardwareFlowControl_None,

        /* GPIO 配置：TX(PC12) RX(PD2) */
        .tx_port            = GPIOC,
        .tx_pin             = GPIO_Pin_12,
        .rx_port            = GPIOD,
        .rx_pin             = GPIO_Pin_2,
        
        /* 时钟配置 */
        .usart_clk          = RCC_APB1Periph_UART5,
        .gpio_clk           = RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD,
        .bus                = USART_BUS_APB1,

        /* 中断相关配置 */
        .irq_n              = UART5_IRQn,              /* 串口5的中断源 */
        .irq_it             = USART_IT_RXNE,            /* 判断串口5的 RXNE 标志位是否产生了中断 */
        .irq_preempt_prio   = BSP_USART_PREEMPT_PRIO,   /* 抢占优先级 */
        .irq_sub_prio       = BSP_USART_SUB_PRIO,       /* 响应优先级 */
    },
};

static void BSP_USART_NVIC_Config(bsp_usart_t id)
{
    if(id >= BSP_USART_MAX) return;
    const bsp_usart_hw_t *hw = &bsp_usart_hw[id];
    NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannel = hw->irq_n;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = hw->irq_preempt_prio;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = hw->irq_sub_prio;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStruct);
}

void BSP_USART_Init(bsp_usart_t id)
{
    if(id >= BSP_USART_MAX)  return;

    const bsp_usart_hw_t *hw = &bsp_usart_hw[id];
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;

    /* 清空结构体 */
    GPIO_StructInit(&GPIO_InitStruct);
    USART_StructInit(&USART_InitStruct);

    /* 初始化时钟*/
    if(hw->bus == USART_BUS_APB2){
        RCC_APB2PeriphClockCmd(hw->usart_clk | hw->gpio_clk, ENABLE);
    }else{
        RCC_APB1PeriphClockCmd(hw->usart_clk, ENABLE);
        RCC_APB2PeriphClockCmd(hw->gpio_clk, ENABLE);
    }

    /* 初始化用到的 GPIO */
    /* 配置 tx 引脚 */
    GPIO_InitStruct.GPIO_Pin    = hw->tx_pin;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(hw->tx_port, &GPIO_InitStruct);

    /* 配置 rx 引脚 */
    GPIO_InitStruct.GPIO_Pin    = hw->rx_pin;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN_FLOATING;
    GPIO_Init(hw->rx_port, &GPIO_InitStruct);

    /* 初始化串口 */
    USART_InitStruct.USART_BaudRate             = hw->baud_rate;
    USART_InitStruct.USART_WordLength           = hw->word_length;
    USART_InitStruct.USART_StopBits             = hw->stop_bits;
    USART_InitStruct.USART_Parity               = hw->parity;
    USART_InitStruct.USART_Mode                 = hw->mode;
    USART_InitStruct.USART_HardwareFlowControl  = hw->flow_control;

    USART_Init(hw->usartx, &USART_InitStruct);      /* 初始化串口 */

    USART_ITConfig(hw->usartx, hw->irq_it, ENABLE);     /* 启用串口接收中断 */
    BSP_USART_NVIC_Config(id);                          /* 配置中断优先级 */
    USART_Cmd(hw->usartx, ENABLE);                      /* 其用串口 */
}

void BSP_USART_SendByte(bsp_usart_t id, uint8_t byte)
{
    if(id >= BSP_USART_MAX)  return;

    const bsp_usart_hw_t *hw = &bsp_usart_hw[id];
    USART_TypeDef *usart = hw->usartx;

    /* 判断发送数据寄存器是否为空，若为空则填入数据发送*/
    while(USART_GetFlagStatus(usart, USART_FLAG_TXE) == RESET);
    USART_SendData(usart, (uint16_t)byte);

    /* 判断数据是否发送完成 */
    while(USART_GetFlagStatus(usart, USART_FLAG_TC) == RESET);
}

void BSP_USART_SendHalfWord(bsp_usart_t id, uint16_t data)
{
    if(id >= BSP_USART_MAX) return;

    const bsp_usart_hw_t *hw = &bsp_usart_hw[id];
    USART_TypeDef *usart = hw->usartx;
    uint8_t high_byte = (data & 0xFF00) >> 8;
    uint8_t low_byte = data & 0x00FF;

    /* 判断发送数据寄存器是否为空，若为空则填入数据发送*/
    while(USART_GetFlagStatus(usart, USART_FLAG_TXE) == RESET);
    USART_SendData(usart, (uint16_t)high_byte);

    /* 判断发送数据寄存器是否为空，若为空则填入数据发送*/
    while(USART_GetFlagStatus(usart, USART_FLAG_TXE) == RESET);
    USART_SendData(usart, (uint16_t)low_byte);

    /* 判断数据是否发送完成 */
    while(USART_GetFlagStatus(usart, USART_FLAG_TC) == RESET);
}

void BSP_USART_SendArray(bsp_usart_t id, uint8_t *array, uint16_t size)
{
    if(id >= BSP_USART_MAX) return;

    const bsp_usart_hw_t *hw = &bsp_usart_hw[id];
    USART_TypeDef *usart = hw->usartx;

    for(int i = 0; i < size; i++){

        /* 判断发送数据寄存器是否为空,若为空则表示可以存放数据到 TDR 中 */
        while(USART_GetFlagStatus(usart, USART_FLAG_TXE) == RESET);
        USART_SendData(usart, (uint16_t)*(array + i));
    }
    /* 判断数据是否发送完成 */
    while(USART_GetFlagStatus(usart, USART_FLAG_TC) == RESET);
}

void BSP_USART_SendString(bsp_usart_t id, uint8_t *str)
{
    if(id >= BSP_USART_MAX) return;

    const bsp_usart_hw_t *hw = &bsp_usart_hw[id];
    USART_TypeDef *usart = hw->usartx;

    while(*str){
        /* 判断发送数据寄存器是否为空,若为空则表示可以存放数据到 TDR 中 */
        while(USART_GetFlagStatus(usart, USART_FLAG_TXE) == RESET);
        USART_SendData(usart, (uint16_t)*str++);
    }
    /* 判断数据是否发送完成 */
    while(USART_GetFlagStatus(usart, USART_FLAG_TC) == RESET);
}

void BSP_USART_Setdio(bsp_usart_t id)
{
    if(id >= BSP_USART_MAX) 
        return;
    else
        g_stdio_usart = id;
}

int _write(int fd, char *ptr, int len)
{
    if(g_stdio_usart >= BSP_USART_MAX)  return -1;
    (void)fd; // 防止未使用警告

    /* 根据全局变量获取当前的 USART 外设*/
    USART_TypeDef *usart = bsp_usart_hw[g_stdio_usart].usartx;

    for(int i = 0; i < len; i++){
        /* 判断发送数据寄存器是否为空,若为空则表示可以存放数据到 TDR 中 */
        while(USART_GetFlagStatus(usart, USART_FLAG_TXE) == RESET);
        USART_SendData(usart, (uint16_t) *(ptr+i));
    }
    /* 判断数据是否发送完成 */
    while(USART_GetFlagStatus(usart, USART_FLAG_TC) == RESET);

    return len;
}

/* 阻塞式读取一个字节 */
uint8_t BSP_USART_ReceiveByte(bsp_usart_t id)
{
    if(id >= BSP_USART_MAX) return -1;

    const bsp_usart_hw_t *hw = &bsp_usart_hw[id];
    uint8_t recv_data = 0;

    while(USART_GetFlagStatus(hw->usartx, USART_FLAG_RXNE) == RESET);
    if(USART_GetFlagStatus(hw->usartx, USART_FLAG_RXNE) == SET){
        recv_data = USART_ReceiveData(hw->usartx);
    }
    (void)recv_data;

    return recv_data;
}

void BSP_NVICGroup_Config(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

uint8_t BSP_USART_ReceiveString(bsp_usart_t id, uint8_t *buff, uint8_t len)
{
    if(id >= BSP_USART_MAX) return -1;

    const bsp_usart_hw_t *hw = &bsp_usart_hw[id];
    
    for(int i = 0; i < len; i++){
        /* 判断数据寄存器是否为空，若不为空将数据读取出来 */
        while(USART_GetFlagStatus(hw->usartx, USART_FLAG_RXNE) == RESET);
        buff[i] = (uint8_t)USART_ReceiveData(hw->usartx);
    }
    return len;
}

void BSP_USART_IRQHandler(bsp_usart_t id)
{
    if(id >= BSP_USART_MAX) return;
    const bsp_usart_hw_t *hw = &bsp_usart_hw[id];

    /* 检测 RXNE 标志位是否为1 */
    if(USART_GetITStatus(hw->usartx, hw->irq_it) != RESET){
        recv_data = USART_ReceiveData(hw->usartx);
        g_rx_flag = 1;
    }
}