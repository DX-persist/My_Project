#include "bsp_usart.h"

typedef struct{
	
	USART_TypeDef	*usart;

	GPIO_TypeDef 	*tx_port;
	uint16_t		tx_pin;

	GPIO_TypeDef	*rx_port;
	uint16_t		rx_pin;

	uint32_t		tx_clk;
	uint32_t		rx_clk;
	uint32_t		usart_clk;

	uint32_t		baud_rate;
	uint16_t		word_length;
	uint16_t		stop_bits;
	uint16_t		parity;
	uint16_t		mode;
	uint16_t		hard_flow_ctl;

	uint16_t		irq_src_rxne;
	uint16_t		irq_src_txe;
	uint16_t		irq_src_tc;
	
	uint8_t			irq_channel;
	uint8_t			irq_preempt_prio;
	uint8_t			irq_sub_prio;
	
	bsp_usart_bus_t bus;
}bsp_usart_hw_t;

uint8_t g_usart_id = BSP_USART1;

/* 给每个串口都创一个环形缓冲区用来接收数据 */
static RingBuffer_t usart_rx_rb[BSP_USART_MAX];

static const bsp_usart_hw_t bsp_usart_hw[BSP_USART_MAX] = {
	[BSP_USART1] = {
		
		.usart 				= USART1,							/* USART1 */

		.tx_port 			= GPIOA,							/* USART1 TX 所在端口 */
		.tx_pin 			= GPIO_Pin_9,						/* USART1 TX 所在引脚 */

		.rx_port 			= GPIOA,							/* USART1 RX 所在端口 */
		.rx_pin 			= GPIO_Pin_10,						/* USART1 RX 所在引脚 */

		.tx_clk 			= RCC_APB2Periph_GPIOA,				/* USART1 TX 所在端口的时钟 */
		.rx_clk 			= RCC_APB2Periph_GPIOA,				/* USART1 RX 所在端口的时钟 */
		.usart_clk 			= RCC_APB2Periph_USART1,			/* USART1 所在的时钟 */

		.baud_rate 			= BAUDRATE_DEFAULT,					/* 使用默认波特率：115200*/
		.word_length 		= USART_WordLength_8b,				/* 数据位长度：8位 */
		.stop_bits 			= USART_StopBits_1,					/* 停止位长度：1位 */
		.parity 			= USART_Parity_No,					/* 校验位：无校验位 */
		.mode 				= USART_Mode_Rx | USART_Mode_Tx,	/* 模式：既发送又接收 */
		.hard_flow_ctl 		= USART_HardwareFlowControl_None,	/* 硬件流控：无 */

		.irq_src_rxne 		= USART_IT_RXNE,					/* 接收数据寄存器非空(中断) */
		.irq_src_txe 		= USART_IT_TXE,						/* 发送数据寄存器空(中断) */
		.irq_src_tc 		= USART_IT_TC,						/* 发送数据完成(中断) */
	
		.irq_channel 		= USART1_IRQn,						/* USART1 对应的中断号 */
		.irq_preempt_prio 	= USART_PREEMPT_PRIO,				/* USART1 的抢占优先级：2 */
		.irq_sub_prio 		= USART_SUB_PRIO,					/* USART1 的响应优先级：2 */

		.bus 				= USART_APB2_BUS,					/* USART1 挂载到 APB2 总线上 */

	},

	[BSP_USART2] = {

		.usart 				= USART2,							/* USART2 */		

		.tx_port 			= GPIOA,							/* USART2 TX 所在端口 */
		.tx_pin 			= GPIO_Pin_2,						/* USART2 TX 所在引脚 */

		.rx_port 			= GPIOA,							/* USART2 RX 所在端口 */
		.rx_pin 			= GPIO_Pin_3,						/* USART2 RX 所在引脚 */

		.tx_clk				= RCC_APB2Periph_GPIOA,				/* USART2 TX 所在端口的时钟*/
		.rx_clk 			= RCC_APB2Periph_GPIOA,				/* USART2 RX 所在端口的时钟 */
		.usart_clk 			= RCC_APB1Periph_USART2,			/* USART2 所在的时钟 */

		.baud_rate 			= BAUDRATE_DEFAULT,					/* 使用默认波特率：115200*/
		.word_length 		= USART_WordLength_8b,				/* 数据位长度：8位 */
		.stop_bits 			= USART_StopBits_1,					/* 停止位长度：1位 */
		.parity 			= USART_Parity_No,					/* 校验位：无校验位 */
		.mode 				= USART_Mode_Rx | USART_Mode_Tx,	/* 模式：既发送又接收 */
		.hard_flow_ctl 		= USART_HardwareFlowControl_None,	/* 硬件流控：无 */

		.irq_src_rxne 		= USART_IT_RXNE,					/* 接收数据寄存器非空(中断) */
		.irq_src_txe 		= USART_IT_TXE,						/* 发送数据寄存器空(中断) */
		.irq_src_tc 		= USART_IT_TC,						/* 发送数据完成(中断) */

		.irq_channel 		= USART2_IRQn,						/* USART2 对应的中断号 */
		.irq_preempt_prio 	= USART_PREEMPT_PRIO,				/* USART2 的抢占优先级：2 */
		.irq_sub_prio 		= USART_SUB_PRIO,					/* USART2 的响应优先级：2 */

		.bus 				= USART_APB1_BUS,					/* USART2 挂载到 APB1 总线上 */

	},

	[BSP_USART3] = {
		
		.usart 				= USART3,							/* USART3 */

		.tx_port 			= GPIOB,							/* USART3 TX 所在端口 */
		.tx_pin 			= GPIO_Pin_10,						/* USART3 TX 所在引脚 */

		.rx_port 			= GPIOB,							/* USART3 RX 所在端口 */
		.rx_pin 			= GPIO_Pin_11,						/* USART3 RX 所在引脚 */

		.tx_clk 			= RCC_APB2Periph_GPIOB,				/* USART3 TX 所在端口的时钟 */
		.rx_clk 			= RCC_APB2Periph_GPIOB,				/* USART3 RX 所在端口的时钟 */
		.usart_clk 			= RCC_APB1Periph_USART3,			/* USART3 所在的时钟 */

		.baud_rate 			= BAUDRATE_DEFAULT,					/* 使用默认波特率：115200*/
		.word_length 		= USART_WordLength_8b,				/* 数据位长度：8位 */
		.stop_bits 			= USART_StopBits_1,					/* 停止位长度：1位 */
		.parity 			= USART_Parity_No,					/* 校验位：无校验位 */
		.mode 				= USART_Mode_Rx | USART_Mode_Tx,	/* 模式：既发送又接收 */
		.hard_flow_ctl 		= USART_HardwareFlowControl_None,	/* 硬件流控：无 */

		.irq_src_rxne 		= USART_IT_RXNE,					/* 接收数据寄存器非空(中断) */
		.irq_src_txe 		= USART_IT_TXE,						/* 发送数据寄存器空(中断) */
		.irq_src_tc 		= USART_IT_TC,						/* 发送数据完成(中断) */

		.irq_channel 		= USART3_IRQn,						/* USART3 对应的中断号 */
		.irq_preempt_prio 	= USART_PREEMPT_PRIO,				/* USART3 的抢占优先级：2 */
		.irq_sub_prio 		= USART_SUB_PRIO,					/* USART3 的响应优先级：2 */

		.bus 				= USART_APB1_BUS,					/* USART3 挂载到 APB1 总线上 */

	}
};

static void BSP_USART_GPIO_Init(bsp_usart_t id)
{
	if(id >= BSP_USART_MAX)	return;

	GPIO_InitTypeDef GPIO_InitStruct;
	const bsp_usart_hw_t *hw = &bsp_usart_hw[id];

	/* 清空 GPIO_InitStruct 结构体 */
	GPIO_StructInit(&GPIO_InitStruct);
	
	/* 开启 USARTx 所在引脚的时钟 */
	RCC_APB2PeriphClockCmd(hw->tx_clk | hw->rx_clk, ENABLE);
	
	if(hw->bus == USART_APB1_BUS){
		/*开启 USARTx 的时钟,例如USART2、USART3 */
		RCC_APB1PeriphClockCmd(hw->usart_clk, ENABLE);
	}else{
		/*开启 USARTx 的时钟,例如USART1 */
		RCC_APB2PeriphClockCmd(hw->usart_clk, ENABLE);
	}

	/* 初始化 USARTx 的 TX 引脚 */
	GPIO_InitStruct.GPIO_Pin = hw->tx_pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(hw->tx_port, &GPIO_InitStruct);

	/* 初始化 USARTx 的 RX 引脚 */
	GPIO_InitStruct.GPIO_Pin = hw->rx_pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;

	GPIO_Init(hw->rx_port, &GPIO_InitStruct);
}

static void BSP_USART_PriorityGroupConfig(void)
{
	/* 配置中断优先级分组为2 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

static void BSP_USART_NVIC_Config(bsp_usart_t id)
{
	if(id >= BSP_USART_MAX)	return;

	const bsp_usart_hw_t *hw = &bsp_usart_hw[id];
	NVIC_InitTypeDef NVIC_InitStruct;

	NVIC_InitStruct.NVIC_IRQChannel = hw->irq_channel;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = hw->irq_preempt_prio;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = hw->irq_sub_prio;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

	/* 配置串口中断的抢占优先级和响应优先级 */
	NVIC_Init(&NVIC_InitStruct);
}

static void BSP_USART_Init(bsp_usart_t id)
{
	if(id >= BSP_USART_MAX)	return;

	USART_InitTypeDef USART_InitStruct;
	const bsp_usart_hw_t *hw = &bsp_usart_hw[id];

	/* 清空 USART_InitStruct 结构体 */
	USART_StructInit(&USART_InitStruct);

	USART_InitStruct.USART_BaudRate 			= hw->baud_rate;
	USART_InitStruct.USART_WordLength 			= hw->word_length;
	USART_InitStruct.USART_StopBits 			= hw->stop_bits;
	USART_InitStruct.USART_Parity 				= hw->parity;
	USART_InitStruct.USART_Mode 				= hw->mode;
	USART_InitStruct.USART_HardwareFlowControl 	= hw->hard_flow_ctl;

	/* 配置串口的参数 */
	USART_Init(hw->usart, &USART_InitStruct);

	/* 使能 RXNE 串口中断 */
	USART_ITConfig(hw->usart, hw->irq_src_rxne, ENABLE);
	
	/* 使能 TXE 串口中断 */
	//USART_IT_Config(hw->usart, hw->irq_src_txe, ENABLE);

	/* 使能 TC 串口中断*/
	//USART_ITConfig(hw->usart, hw->irq_src_tc, ENABLE);

	/* 启用串口 */
	USART_Cmd(hw->usart, ENABLE);
}

void BSP_USART_Config(bsp_usart_t id)
{
	if(id >= BSP_USART_MAX)	return;
	
	memset(&usart_rx_rb[id], 0, sizeof(RingBuffer_t));

	BSP_USART_GPIO_Init(id);
	BSP_USART_PriorityGroupConfig();
	BSP_USART_NVIC_Config(id);
	BSP_USART_Init(id);
}

void BSP_USART_SendByte(bsp_usart_t id, uint8_t byte)
{
	if(id >= BSP_USART_MAX)	return;

	const bsp_usart_hw_t *hw = &bsp_usart_hw[id];

	/* 判断发送寄存器是否为空 */
	while(USART_GetFlagStatus(hw->usart, USART_FLAG_TXE) == RESET);
	USART_SendData(hw->usart, (uint16_t)byte);
	/* 判断数据是否发送成功 */
	while(USART_GetFlagStatus(hw->usart, USART_FLAG_TC) == RESET);
}

void BSP_USART_SendString(bsp_usart_t id, char *str)
{
	/* 判断是否读到了字符结束标志'\0' */
	while(*str != '\0'){
		BSP_USART_SendByte(id, (uint8_t)*str++);
	}
}

void BSP_USART_SendArray(bsp_usart_t id, uint8_t *array, uint8_t size)
{
	for(int i = 0; i < size; i++){
		BSP_USART_SendByte(id, array[i]);
	}
}

uint8_t BSP_USART_ReadByte(bsp_usart_t id)
{
	if(id >= BSP_USART_MAX)	return 0;

	RingBuffer_t *rb = &usart_rx_rb[id];
	
	/* 判断环形缓冲区里边有没有数据 */
	if(rb->head == rb->tail){
		return -1;
	}

	uint8_t data = rb->buffer[rb->tail];
	rb->tail = (rb->tail + 1) % RING_BUF_SIZE;

	return data;
}

void BSP_USART_Stdio(bsp_usart_t id)
{
	/* 由于_write函数的原型是已经固定好的,所以使用全局变量
	 * 在main函数中调用该函数改变g_usart_id的值,然后传给
	 * _write函数使用,这样子它就能变成一个通用函数,可以被
	 * 多个串口调用,而非是写死的一个串口
	 * 若使用printf函数必须在main函数中初始化该接口！！！
	 * */

	if(id >= BSP_USART_MAX)	return;

	/* 通过外部传参改变该变量的值供 _write函数使用 */
	g_usart_id = id;
}

int _write(int fd, char *ptr, int len)
{
	(void)fd;			/* 防止编译器报 unsed 警告 */
	
	for(int i = 0; i < len; i++){
		BSP_USART_SendByte(g_usart_id, *(ptr+i));
		return i + 1;				/* 留一个字节存放结束标志 */
	}
	return len;
}

int _read(int fd, char *ptr, int len)
{
	(void)fd;
	RingBuffer_t *rb = &usart_rx_rb[g_usart_id];
		
	for(int i = 0; i < len; i++){
		/* 如果没有数据就一直在这里卡着 */
		while(rb->head == rb->tail);

		/* 如果有数据就调用tail下标进行读取 */
		*(ptr+i) = rb->buffer[rb->tail];
		rb->tail = (rb->tail + 1) % RING_BUF_SIZE;

		/* 如果收到'\r'或者'\n'就结束本次的输入 */
		if(*(ptr + i) == '\r' || *(ptr + i) == '\n'){
			*(ptr + i) = '\n';
			return i + 1;
		}
	}	

	return len;
}

uint8_t BSP_USART_GetRxCount(bsp_usart_t id)
{
	if(id >= BSP_USART_MAX)	return 0;

	RingBuffer_t *rb = &usart_rx_rb[id];
	/*
	 * 若head >= tail，那么缓冲区内的数据个数为head - tail
	 * 若head < tail，就表明head已经存完一轮又转回来了，那么
	 * 此时缓冲区内的数据个数为RING_BUF_SIZE - tail + head;
	 * 两个统一公式为(head - tail + RING_BUF_SIZE) % RING_BUF_SIZE
	 *
	 * */
	return ((rb->head - rb->tail + RING_BUF_SIZE) % RING_BUF_SIZE);
}

void BSP_USART_Clear_RingBuffer(bsp_usart_t id)
{
	if(id >= BSP_USART_MAX)	return;

	RingBuffer_t *rb = &usart_rx_rb[id];

	//memset(rb->buffer, '\0', sizeof(rb->buffer));
	/* 直接将头和为指向一起就可以清空缓冲区 */
	rb->head = rb->head;	
}

void BSP_USART_Set_ITRXNE_State(bsp_usart_t id, FunctionalState NewState)
{
	if(id >= BSP_USART_MAX)	return;

	const bsp_usart_hw_t *hw = &bsp_usart_hw[id];

	/* 传入要开启/关闭的指定的中断标志位 */
	USART_ITConfig(hw->usart, USART_IT_RXNE, NewState);
}

void BSP_USART_CommandHandler(bsp_usart_t id)
{
	if(id >= BSP_USART_MAX)	return;

	uint8_t data = 0;
	const bsp_usart_hw_t *hw = &bsp_usart_hw[id];
	RingBuffer_t *rb = &usart_rx_rb[id];

	/* 判断 RXNE 标志位是否被置位,置位说明接收到数据 */
	if(USART_GetITStatus(hw->usart, USART_IT_RXNE) != RESET){
		data = (uint8_t)USART_ReceiveData(hw->usart);

		/* 
		 * next_head 用来确定head和tail位置是否重合
		 * 若重合则说明缓冲区满，不能再存放数据
		 * 反之，继续存放
		 * 
		 * */
		uint16_t next_head = (rb->head + 1) % RING_BUF_SIZE;
		if(next_head != rb->tail){
			/* 缓冲区未满，写入数据 */
			rb->buffer[rb->head] = data;
			rb->head = next_head;
		}else{
			/* 缓冲区满直接丢弃 */
		}
		USART_ClearITPendingBit(hw->usart, USART_IT_RXNE);
	}
}

void BSP_RecvCommand_Analysis(char *cmd_buf)
{
    printf("收到控制指令: [%s]\r\n",cmd_buf);
    if(!strncmp((char *)cmd_buf, "blue_on", 7)){
        BSP_LED_On(LED_BLUE);
        BSP_LED_Off(LED_ORANGE);
    }else if(!strncmp((char *)cmd_buf, "orange_on", 9)){
        BSP_LED_On(LED_ORANGE);
        BSP_LED_Off(LED_BLUE);
    }else if(!strncmp((char *)cmd_buf, "blue_off", 8)){
        BSP_LED_Off(LED_BLUE);
    }else if(!strncmp((char *)cmd_buf, "orange_off", 10)){
        BSP_LED_Off(LED_ORANGE);
    }else if(!strncmp((char *)cmd_buf, "all_on", 6)){
        BSP_LED_On(LED_BLUE);
        BSP_LED_On(LED_ORANGE);
    }else if(!strncmp((char *)cmd_buf, "all_off", 7)){
        BSP_LED_Off(LED_BLUE);
        BSP_LED_Off(LED_ORANGE);
    }
}


