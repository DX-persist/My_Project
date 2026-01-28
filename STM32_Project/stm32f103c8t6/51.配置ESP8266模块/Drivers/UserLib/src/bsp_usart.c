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

volatile uint8_t g_usart_id = BSP_USART1;
volatile uint16_t recv_buf_len = 0;
volatile uint8_t recv_buf[RECV_BUF_MAX] = {0};
volatile uint8_t recv_done = 0;

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
	
	/* 开启 USARTx 的引脚所在端口的时钟 */
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
	if(id >= BSP_USART_MAX)	return;

	const bsp_usart_hw_t *hw = &bsp_usart_hw[id];

	while(*str != '\0'){
		while(USART_GetFlagStatus(hw->usart, USART_FLAG_TXE) == RESET);
		BSP_USART_SendByte(id, (uint8_t)*str++);
	}
	while(USART_GetFlagStatus(hw->usart, USART_FLAG_TC) == RESET);
}

void BSP_USART_SendArray(bsp_usart_t id, uint8_t *array, uint8_t size)
{
	if(id >= BSP_USART_MAX)	return;

	const bsp_usart_hw_t *hw = &bsp_usart_hw[id];

	for(int i = 0; i < size; i++){
		while(USART_GetFlagStatus(hw->usart, USART_FLAG_TXE) == RESET);
		USART_SendData(hw->usart, (uint16_t)array[i]);
	}
	while(USART_GetFlagStatus(hw->usart, USART_FLAG_TC) == RESET);
}

void BSP_USART_Stdio(bsp_usart_t id)
{
	/* 由于_write函数的原型是已经固定好的,所以使用全局变量
	 * 在main函数中调用该函数改变g_usart_id的值,然后传给
	 * _write函数使用,这样子它就能变成一个通用函数,可以被
	 * 多个串口调用,而非是写死的一个串口
	 * 若使用printf函数必须在main函数中初始化该接口！！！
	 * */

	if(id >= BSP_USART_MAX)	return ;

	/* 通过外部传参改变该变量的值供 _write函数使用 */
	g_usart_id = id;
}	

/* 重写_write函数方便在发送数据时使用 printf */

int _write(int fd, char *buf, int nbyte)
{
	(void)fd;		/* 防止编译器报 unused 的警告 */

	const bsp_usart_hw_t *hw = &bsp_usart_hw[g_usart_id];
	for(int i = 0; i < nbyte; i++){

		/* 判断发送数据寄存器是否为空,若为空则往发送数据寄存器中填入数据*/
		while(USART_GetFlagStatus(hw->usart, USART_FLAG_TXE) == RESET);
		BSP_USART_SendByte(g_usart_id, *(uint8_t *)(buf + i));
	}
	/* 判断数据是否已经发送完毕 */
	while(USART_GetFlagStatus(hw->usart, USART_FLAG_TXE) == RESET);

	return nbyte;
}

/* 重写_read函数方便在发送数据时使用 scanf 
 * 需要注意的一点是若想要scanf函数获取来自RXD
 * 的消息时，需要禁用RXNE中断,若不禁用,RXNE会
 * 进入中断，而重定向的_read函数是阻塞式获取RDR
 * 寄存器中的值，并非是通过中断获取，所以RXNE标
 * 志位并不会清空
*/

int _read(int fd, char *buf, int nbyte)
{
	(void)fd;

	const bsp_usart_hw_t *hw = &bsp_usart_hw[g_usart_id];
	for(int i = 0; i < nbyte; i++){
		
		/* 判断接受数据寄存器不为空则说明有数据,读取数据后RXNE标志位自动清零 */
		while(USART_GetFlagStatus(hw->usart, USART_FLAG_RXNE) == RESET);
		buf[i] = (char)USART_ReceiveData(hw->usart);

		if(buf[i] == '\n' || buf[i] == '\r'){
			buf[i++] = '\n';
			return i;
		}
	}
	return nbyte;
}

static void BSP_USART_InputByte(uint8_t recv_byte)
{
	/*
	 *  判断当前接收到的字节是否已经超出接收缓冲区的最大接受字节数
	 *	若是，则退出，将最后一个字节置为'\0'
	 *	反之，则继续读取  
	 *	每进入串口中断一次(RXNE触发)，就表示接收到了一个字节的数据
	 *	然后将数据都存在recv_buf中，直到接收到'\n'的换行符的时候表
	 *	明数据接收完毕，此时recv_done = 1，main函数判断到recv_done
	 *	被置为1后，就将数据重新发送到串口
	 */
	if(recv_buf_len < RECV_BUF_MAX - 1){
		if(recv_byte != '\n'){
			recv_buf[recv_buf_len++] = recv_byte;
		}else{
			recv_buf[recv_buf_len] = '\0';
			recv_done = 1;
		}
	}else{
		/*缓冲区满,强制关闭并把最后一位置为字符结束标志'\0'*/
		recv_buf[RECV_BUF_MAX - 1] = '\0';
		recv_done = 1;
	}
}

void BSP_USART_CommandHandler(bsp_usart_t id)
{
	if(id >= BSP_USART_MAX)	return;

	uint8_t recv_byte = 0;
	const bsp_usart_hw_t *hw = &bsp_usart_hw[id];

	/* 判断 RXNE 标志位是否被置位,置位说明接收到数据 */
	if(USART_GetITStatus(hw->usart, USART_IT_RXNE) != RESET){
		recv_byte = USART_ReceiveData(hw->usart);
		BSP_USART_InputByte(recv_byte);
		USART_ClearITPendingBit(hw->usart, USART_IT_RXNE);
	}
}

void BSP_Set_USARTIT_RXNE_State(bsp_usart_t id, FunctionalState NewState)
{
	if(id >= BSP_USART_MAX)	return;

	const bsp_usart_hw_t *hw = &bsp_usart_hw[id];

	/* 传入要开启/关闭的指定的中断标志位 */
	USART_ITConfig(hw->usart, USART_IT_RXNE, NewState);
}
