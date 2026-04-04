#include "bsp_usart.h"
#include "bsp_ringbuffer.h"

typedef struct{
	
	/* 定义串口相关参数 */
	uint32_t 		usart_clk;
	USART_TypeDef 	*usart;
	uint32_t		baudrate;
	uint16_t		wordlength;
	uint16_t		stopbits;
	uint16_t		parity;
	uint16_t		mode;
	uint16_t		hard_flow_ctl;

	/* 定义串口所在端口参数 */
	uint32_t		tx_gpio_clk;
	GPIO_TypeDef	*tx_port;
	uint16_t		tx_pin;
	uint32_t		rx_gpio_clk;
	GPIO_TypeDef	*rx_port;
	uint16_t		rx_pin;

	uint8_t 		irq_channel;
	uint8_t 		irq_pre_prio;
	uint8_t			irq_sub_prio;

	bsp_dma_channel_t dma_channel;
	uint32_t		tx_dma_clk;
	uint16_t		tx_dma_req;

	BSP_BusTypeDef bus;
}bsp_usart_hw_t;

/* 
 * 全局变量：当前 printf/scanf 使用的串口 ID
 * 注意：volatile 只保证可见性，不保证原子性
 * 如果在中断中修改此变量，需要考虑竞态条件
 */
volatile uint8_t g_usart_id = BSP_USART1;

/* 为每个串口分配独立的接收缓冲区 */
static uint8_t usart_rx_rb[BSP_USART_MAX][RING_BUFFER_SIZE];

/* 为每个串口分配独立的环形缓冲区控制结构 */
static RingBuffer_t usart_rb[BSP_USART_MAX];

/*
 * 硬件配置表：使用数组索引初始化方式，确保顺序与枚举一致
 * 这种设计模式的优点：
 * 1. 便于扩展新串口
 * 2. 配置集中管理
 * 3. 避免重复代码
 */
static const bsp_usart_hw_t bsp_usart_hw[BSP_USART_MAX] = {
	[BSP_USART1] = {
		.usart_clk 		= RCC_APB2Periph_USART1,				
		.usart			= USART1,
		.baudrate		= BAUDRATE_DEFAULT,
		.wordlength		= USART_WordLength_8b,
		.stopbits		= USART_StopBits_1,
		.parity			= USART_Parity_No,
		.mode			= USART_Mode_Rx | USART_Mode_Tx,
		.hard_flow_ctl 	= USART_HardwareFlowControl_None,

		.tx_gpio_clk	= RCC_APB2Periph_GPIOA,
		.tx_port		= GPIOA,
		.tx_pin			= GPIO_Pin_9,
		.rx_gpio_clk	= RCC_APB2Periph_GPIOA,
		.rx_port		= GPIOA,
		.rx_pin			= GPIO_Pin_10,

		.irq_channel	= USART1_IRQn,
		.irq_pre_prio 	= PREEMPT_PRIO,
		.irq_sub_prio	= SUB_PRIO,
		
		.dma_channel	= BSP_DMA1_Channel4,
		.tx_dma_clk		= RCC_AHBPeriph_DMA1,
		.tx_dma_req		= USART_DMAReq_Tx,

		.bus			= BSP_BUS_APB2,  // USART1 挂载在 APB2 总线
	},

	[BSP_USART2] = {
		.usart_clk 		= RCC_APB1Periph_USART2,				
		.usart			= USART2,
		.baudrate		= BAUDRATE_DEFAULT,
		.wordlength		= USART_WordLength_8b,
		.stopbits		= USART_StopBits_1,
		.parity			= USART_Parity_No,
		.mode			= USART_Mode_Rx | USART_Mode_Tx,
		.hard_flow_ctl 	= USART_HardwareFlowControl_None,

		.tx_gpio_clk	= RCC_APB2Periph_GPIOA,
		.tx_port		= GPIOA,
		.tx_pin			= GPIO_Pin_2,
		.rx_gpio_clk	= RCC_APB2Periph_GPIOA,
		.rx_port		= GPIOA,
		.rx_pin			= GPIO_Pin_3,

		.irq_channel	= USART2_IRQn,
		.irq_pre_prio 	= PREEMPT_PRIO,
		.irq_sub_prio	= SUB_PRIO,

		.dma_channel	= BSP_DMA1_Channel7,
		.tx_dma_clk		= RCC_AHBPeriph_DMA1,
		.tx_dma_req		= USART_DMAReq_Tx,

		.bus			= BSP_BUS_APB1,  // USART2 挂载在 APB1 总线
	},

	[BSP_USART3] = {
		.usart_clk 		= RCC_APB1Periph_USART3,				
		.usart			= USART3,
		.baudrate		= BAUDRATE_DEFAULT,
		.wordlength		= USART_WordLength_8b,
		.stopbits		= USART_StopBits_1,
		.parity			= USART_Parity_No,
		.mode			= USART_Mode_Rx | USART_Mode_Tx,
		.hard_flow_ctl 	= USART_HardwareFlowControl_None,

		.tx_gpio_clk	= RCC_APB2Periph_GPIOB,
		.tx_port		= GPIOB,
		.tx_pin			= GPIO_Pin_10,
		.rx_gpio_clk	= RCC_APB2Periph_GPIOB,
		.rx_port		= GPIOB,
		.rx_pin			= GPIO_Pin_11,

		.irq_channel	= USART3_IRQn,
		.irq_pre_prio 	= PREEMPT_PRIO,
		.irq_sub_prio	= SUB_PRIO,

		.dma_channel	= BSP_DMA1_Channel2,
		.tx_dma_clk		= RCC_AHBPeriph_DMA1,
		.tx_dma_req		= USART_DMAReq_Tx,

		.bus			= BSP_BUS_APB1,
	},

	[BSP_UART4] = {
		.usart_clk 		= RCC_APB1Periph_UART4,				
		.usart			= UART4,
		.baudrate		= BAUDRATE_DEFAULT,
		.wordlength		= USART_WordLength_8b,
		.stopbits		= USART_StopBits_1,
		.parity			= USART_Parity_No,
		.mode			= USART_Mode_Rx | USART_Mode_Tx,
		.hard_flow_ctl 	= USART_HardwareFlowControl_None,

		.tx_gpio_clk	= RCC_APB2Periph_GPIOC,
		.tx_port		= GPIOC,
		.tx_pin			= GPIO_Pin_10,
		.rx_gpio_clk	= RCC_APB2Periph_GPIOC,
		.rx_port		= GPIOC,
		.rx_pin			= GPIO_Pin_11,

		.irq_channel	= UART4_IRQn,
		.irq_pre_prio 	= PREEMPT_PRIO,
		.irq_sub_prio	= SUB_PRIO,

		.dma_channel	= BSP_DMA2_Channel5,
		.tx_dma_clk		= RCC_AHBPeriph_DMA2,
		.tx_dma_req		= USART_DMAReq_Tx,

		.bus			= BSP_BUS_APB1,
	},

	[BSP_UART5] = {
		.usart_clk 		= RCC_APB1Periph_UART5,				
		.usart			= UART5,
		.baudrate		= BAUDRATE_DEFAULT,
		.wordlength		= USART_WordLength_8b,
		.stopbits		= USART_StopBits_1,
		.parity			= USART_Parity_No,
		.mode			= USART_Mode_Rx | USART_Mode_Tx,
		.hard_flow_ctl 	= USART_HardwareFlowControl_None,

		.tx_gpio_clk	= RCC_APB2Periph_GPIOC,
		.tx_port		= GPIOC,
		.tx_pin			= GPIO_Pin_12,
		.rx_gpio_clk	= RCC_APB2Periph_GPIOD,
		.rx_port		= GPIOD,
		.rx_pin			= GPIO_Pin_2,

		.irq_channel	= UART5_IRQn,
		.irq_pre_prio 	= PREEMPT_PRIO,
		.irq_sub_prio	= SUB_PRIO,

		.bus			= BSP_BUS_APB1,
	},
};

/*
 * @brief  初始化串口 GPIO 引脚
 * @param  usart_id  串口ID
 * @note   
 *  - TX 配置为复用推挽输出
 *  - RX 配置为浮空输入（由外部上拉）
 */
static void BSP_USART_GPIO_Init(bsp_usart_t usart_id)
{
	if(usart_id >= BSP_USART_MAX)	return;

	const bsp_usart_hw_t *hw = &bsp_usart_hw[usart_id];
	
	/* 使能 GPIO 时钟（支持 TX/RX 在不同 GPIO 组的情况）*/
	RCC_APB2PeriphClockCmd(hw->tx_gpio_clk | hw->rx_gpio_clk, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);

	/* 配置 TX 引脚：复用推挽输出，50MHz */
	GPIO_InitStruct.GPIO_Pin = hw->tx_pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(hw->tx_port, &GPIO_InitStruct);

	/* 配置 RX 引脚：浮空输入 */
	GPIO_InitStruct.GPIO_Pin = hw->rx_pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(hw->rx_port, &GPIO_InitStruct);
}	

/*
 * @brief  初始化串口外设
 * @param  usart_id  串口ID
 * @note   
 *  - 根据总线类型使能对应时钟
 *  - 使能接收中断（RXNE）
 */
static void BSP_USART_Init(bsp_usart_t usart_id)
{
	if(usart_id >= BSP_USART_MAX)	return;

	const bsp_usart_hw_t *hw = &bsp_usart_hw[usart_id];

	/* 根据总线类型使能串口时钟 */
	if(hw->bus == BSP_BUS_APB1){
		RCC_APB1PeriphClockCmd(hw->usart_clk, ENABLE);
	}else{
		RCC_APB2PeriphClockCmd(hw->usart_clk, ENABLE);
	}

	/* 配置串口参数 */
	USART_InitTypeDef USART_InitStruct;
	USART_StructInit(&USART_InitStruct);

	USART_InitStruct.USART_BaudRate 			= hw->baudrate;
	USART_InitStruct.USART_WordLength 			= hw->wordlength;
	USART_InitStruct.USART_StopBits				= hw->stopbits;
	USART_InitStruct.USART_Parity 				= hw->parity;
	USART_InitStruct.USART_Mode 				= hw->mode;
	USART_InitStruct.USART_HardwareFlowControl 	= hw->hard_flow_ctl;
	
	USART_Init(hw->usart, &USART_InitStruct);

	/* 使能接收中断：每接收一个字节触发一次 */
	USART_ITConfig(hw->usart, USART_IT_RXNE, ENABLE);

	USART_Cmd(hw->usart, ENABLE);
}

/*
 * @brief  配置 NVIC 中断优先级分组
 * @note   只需调用一次，通常在 main 函数开始处
 *         Group 2 表示 2 位抢占优先级，2 位子优先级
 */
void BSP_NVIC_Priority_GroupConfig(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

/*
 * @brief  配置串口中断优先级
 * @param  usart_id  串口ID
 */
static void BSP_USART_NVIC_Config(bsp_usart_t usart_id)
{
	if(usart_id >= BSP_USART_MAX)	return;

	const bsp_usart_hw_t *hw = &bsp_usart_hw[usart_id];

	NVIC_InitTypeDef NVIC_InitStruct;

	NVIC_InitStruct.NVIC_IRQChannel = hw->irq_channel;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = hw->irq_pre_prio;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = hw->irq_sub_prio;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStruct);
}

/*
 * @brief  串口完整配置（外部调用接口）
 * @param  usart_id  串口ID
 * @note   依次完成：环形缓冲区清零 -> GPIO 初始化 -> NVIC 配置 -> 串口初始化
 */
void BSP_USART_Config(bsp_usart_t usart_id)
{
	if(usart_id >= BSP_USART_MAX)	return;

	/* 清零环形缓冲区控制结构 */
	memset(&usart_rb[usart_id], 0, sizeof(RingBuffer_t));
	
	BSP_USART_GPIO_Init(usart_id);
	BSP_USART_NVIC_Config(usart_id);
	BSP_USART_Init(usart_id);	
}

/*
 * @brief  发送单个字节（轮询方式）
 * @param  usart_id   串口ID
 * @param  data_byte  要发送的字节
 * @note   
 *  - 优点：实现简单
 *  - 缺点：阻塞等待，效率较低
 *  - 改进方向：可考虑使用 DMA 或中断驱动的发送缓冲区
 */
void BSP_USART_SendByte(bsp_usart_t usart_id, uint8_t data_byte)
{
	if(usart_id >= BSP_USART_MAX)	return;

	const bsp_usart_hw_t *hw = &bsp_usart_hw[usart_id];

	/* 等待发送数据寄存器为空 */
	while(USART_GetFlagStatus(hw->usart, USART_FLAG_TXE) == RESET);

	USART_SendData(hw->usart, (uint16_t)data_byte);

	/* 等待传输完成（防止过快发送下一个字节导致数据覆盖）*/
	while(USART_GetFlagStatus(hw->usart, USART_FLAG_TC) == RESET);
}

/*
 * @brief  发送字节数组
 * @param  usart_id  串口ID
 * @param  arr       数据数组
 * @param  size      数组长度
 */
void BSP_USART_SendArray(bsp_usart_t usart_id, uint8_t *arr, uint8_t size)
{
	for(int i = 0; i < size; i++){
		BSP_USART_SendByte(usart_id, arr[i]);
	}
}

/*
 * @brief  发送字符串
 * @param  usart_id  串口ID
 * @param  str       以 '\0' 结尾的字符串
 */
void BSP_USART_SendString(bsp_usart_t usart_id, char *str)
{
	while(*str != '\0'){
		BSP_USART_SendByte(usart_id, (uint8_t)*str++);
	}
}

/*
 * @brief  初始化接收缓冲区
 * @param  usart_id  串口ID
 * @note   应在使用串口接收前调用
 */
void BSP_USART_Init_RxBuffer(bsp_usart_t usart_id)
{
	if(usart_id >= BSP_USART_MAX)	return;
	RingBuffer_t *rb = &usart_rb[usart_id];

	RingBuffer_Init(rb, (uint8_t*)&usart_rx_rb[usart_id], RING_BUFFER_SIZE);
}

/*
 * @brief  清空接收缓冲区
 * @param  usart_id  串口ID
 */
void BSP_USART_Clear_RxBuffer(bsp_usart_t usart_id)
{
	if(usart_id >= BSP_USART_MAX)	return;
	RingBuffer_t *rb = &usart_rb[usart_id];

	RingBuffer_Clear(rb);
}

/*
 * @brief  获取接收缓冲区中的数据量
 * @param  usart_id  串口ID
 * @retval 缓冲区中的字节数
 */
uint8_t BSP_USART_GetRxCount(bsp_usart_t usart_id)
{
	if(usart_id >= BSP_USART_MAX)	return 0;
	RingBuffer_t *rb = &usart_rb[usart_id];

	return RingBuffer_GetCount(rb);
}

/*
 * @brief  从接收缓冲区读取一个字节
 * @param  usart_id  串口ID
 * @param  data      用于存放读取数据的指针
 * @retval 1  读取成功
 * @retval 0  缓冲区为空
 */
uint8_t BSP_USART_ReceiveByte(bsp_usart_t usart_id, uint8_t *data)
{
	if(usart_id >= BSP_USART_MAX)	return 0;
	RingBuffer_t *rb = &usart_rb[usart_id];

	return (RingBuffer_Pop(rb, data));
}

/*
 * @brief  串口接收中断处理函数
 * @param  usart_id  串口ID
 * @note   
 *  - 应在对应的 USARTx_IRQHandler 中调用
 *  - 读取 DR 寄存器会自动清除 RXNE 标志位
 */
void BSP_USART_IRQHandler(bsp_usart_t usart_id)
{
	if(usart_id >= BSP_USART_MAX)	return;

	const bsp_usart_hw_t *hw = &bsp_usart_hw[usart_id];
	RingBuffer_t *rb = &usart_rb[usart_id];

	if(USART_GetITStatus(hw->usart, USART_IT_RXNE) != RESET){
		uint8_t data_byte = (uint8_t)USART_ReceiveData(hw->usart);

		/* 将接收到的数据压入环形缓冲区 */
		RingBuffer_Push(rb, data_byte);
		
		/* 读取 DR 寄存器会自动清空 RXNE 标志位，无需手动清除 */
		//USART_ClearITPendingBit(hw->usart, USART_IT_RXNE);
	}
}

/**
 * @brief 使用 DMA 方式配置 USART 发送（TX）
 *
 * @param usart_id BSP 层定义的串口编号
 * @param buffer   待发送的数据缓冲区（内存 → 外设）
 * @param size     发送的数据长度（单位：字节）
 *
 * 说明：
 *  - 本函数只负责一次 DMA 发送配置
 *  - 不包含中断/完成回调
 *  - 适用于 USART1/2/3、UART4（不适用于 UART5）
 */
void BSP_USART_DMA_Tx_Config(bsp_usart_t usart_id, uint8_t *buffer, uint16_t size)
{
	/* 参数合法性检查，防止访问非法串口 */
	if(usart_id >= BSP_USART_MAX) return;

	/* 根据串口 ID 获取对应的硬件资源描述 */
	const bsp_usart_hw_t *hw = &bsp_usart_hw[usart_id];

	/* DMA 配置结构体（由 BSP_DMA_Init 统一解析） */
	bsp_dma_config_t config;

	/* ================= DMA 地址配置 ================= */

	/* 外设地址：USART 数据寄存器 DR
	 * TX 场景下，DMA 会不断把内存数据写入 DR
	 */
	config.periph_addr = (uint32_t)&(hw->usart->DR);

	/* 内存地址：待发送的数据缓冲区 */
	config.memory_addr = (uint32_t)buffer;

	/* ================= 传输方向 ================= */

	/* 内存 → 外设（TX 方向） */
	config.dir = DIR_Periph_DST;

	/* 发送的数据长度（单位：字节） */
	config.buffer_size = size;

	/* ================= 地址递增方式 ================= */

	/* 外设地址固定（DR 寄存器不能递增） */
	config.periph_inc = PeripheralInc_Disable;

	/* 内存地址递增（依次发送 buffer 中的数据） */
	config.memory_inc = MemoryInc_Enable;

	/* ================= 数据宽度 ================= */

	/* USART 按字节发送 */
	config.periph_data_size = PeripheralDataSize_Byte;
	config.memory_data_size = MemoryDataSize_Byte;

	/* ================= DMA 工作模式 ================= */

	/* 普通模式：一次发送完成后停止 */
	config.mode = DMA_Mode_Nor;

	/* 中等优先级，避免长期占用 DMA */
	config.priority = DMA_Priority_M;

	/* 非内存到内存模式 */
	config.m2m = DMA_M2M_DISABLE;

	/* ================= 启用 DMA 请求 ================= */

	/* 开启 USART 的 DMA 发送请求
	 * 没有这一步，DMA 不会响应串口 TX 请求
	 */
	USART_DMACmd(hw->usart, hw->tx_dma_req, ENABLE);

	/* ================= 启动 DMA 通道 ================= */

	/* 根据串口对应的 DMA 通道启动 DMA 传输 */
	BSP_DMA_Init(hw->dma_channel, &config);
}


/*
 * @brief  设置 printf/scanf 使用的串口
 * @param  usart_id  串口ID
 * @note   修改全局变量，影响后续所有的 printf/scanf 调用
 */
void BSP_USART_Stdio(bsp_usart_t usart_id)
{
	if(usart_id >= BSP_USART_MAX)	return;

	g_usart_id = usart_id;
}

/*
 * @brief  重定向 printf 输出（newlib 接口）
 * @param  fd   文件描述符（未使用）
 * @param  ptr  数据缓冲区
 * @param  len  数据长度
 * @retval 实际写入的字节数
 * @note   
 *  - 所有 printf 调用最终会调用 _write
 *  - 通过 g_usart_id 决定输出到哪个串口
 */
int _write(int fd, char *ptr, int len)
{
	(void)fd;  /* 防止编译器 unused 警告 */

	for(int i = 0; i < len; i++){
		BSP_USART_SendByte(g_usart_id, (uint8_t)*(ptr+i));
	}
	return len;
}

/*
 * @brief  重定向 scanf 输入（newlib 接口）
 * @param  fd   文件描述符（未使用）
 * @param  ptr  数据缓冲区
 * @param  len  最大读取长度
 * @retval 实际读取的字节数
 * @note   
 *  - 所有 scanf 调用最终会调用 _read
 *  - 遇到 '\r' 或 '\n' 时返回（模拟行缓冲）
 *  - 阻塞等待数据（适用于简单调试，生产环境应考虑超时机制）
 */
int _read(int fd, char *ptr, int len)
{
	(void)fd;
	RingBuffer_t *rb = &usart_rb[g_usart_id];

	for(int i = 0; i < len; i++){
		/* 阻塞等待数据（潜在风险：可能永久阻塞）*/
		while(!RingBuffer_Pop(rb, (uint8_t*)(ptr + i)));

		/* 遇到回车或换行符时结束输入 */
		if(*(ptr + i) == '\r' || *(ptr + i) == '\n'){
			*(ptr + i) = '\n';  /* 统一转换为 '\n' */
			return i + 1;
		}
	}
	return len;
}

void BSP_USART_ControlCmd(char *cmd)
{
	printf("cmd = [%s]\r\n",cmd);
	if(strstr(cmd, BSP_LED_GREEN_ON)){
		BSP_LED_On(LED_GREEN);
		BSP_LED_Off(LED_BLUE);
		BSP_LED_Off(LED_RED);
	}else if(strstr(cmd, BSP_LED_GREEN_OFF)){
		BSP_LED_Off(LED_GREEN);
		BSP_LED_Off(LED_BLUE);
		BSP_LED_Off(LED_RED);
	}else if(strstr(cmd, BSP_LED_BLUE_ON)){
		BSP_LED_On(LED_BLUE);
		BSP_LED_Off(LED_GREEN);
		BSP_LED_Off(LED_RED);
	}else if(strstr(cmd, BSP_LED_BLUE_OFF)){
		BSP_LED_Off(LED_BLUE);
		BSP_LED_Off(LED_GREEN);
		BSP_LED_Off(LED_RED);
	}else if(strstr(cmd, BSP_LED_RED_ON)){
		BSP_LED_On(LED_RED);
		BSP_LED_Off(LED_BLUE);
		BSP_LED_Off(LED_GREEN);
	}else if(strstr(cmd, BSP_LED_RED_OFF)){
		BSP_LED_Off(LED_RED);
		BSP_LED_Off(LED_BLUE);
		BSP_LED_Off(LED_GREEN);
	}else if(strstr(cmd, BSP_LED_ALL_ON)){
		BSP_LED_On(LED_GREEN);
		BSP_LED_On(LED_BLUE);
		BSP_LED_On(LED_RED);
	}else if(strstr(cmd, BSP_LED_ALL_OFF)){
		BSP_LED_Off(LED_RED);
		BSP_LED_Off(LED_BLUE);
		BSP_LED_Off(LED_GREEN);
	}
}
