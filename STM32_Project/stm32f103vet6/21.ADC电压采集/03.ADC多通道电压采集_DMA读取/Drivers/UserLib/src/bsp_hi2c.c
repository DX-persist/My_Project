#include "bsp_hi2c.h"

/**
 * @brief I2C硬件配置结构体
 * @note 包含I2C外设、GPIO引脚、时钟等所有硬件相关配置
 */
typedef struct{
	I2C_TypeDef 	*i2c;		// I2C外设寄存器基地址(I2C1/I2C2)
	uint32_t		i2c_clk;	// I2C外设时钟使能位

	uint32_t		clk_speed;	// I2C通信速率(如100kHz、400kHz)
	uint16_t		mode;		// I2C工作模式(标准I2C模式或SMBus模式)
	uint16_t		duty_cycle;	// 快速模式下的占空比(2:1或16:9)
	uint16_t		own_addr;	// 本机作为从机时的地址
	uint16_t		ack;		// 应答使能/禁用
	uint16_t		ack_addr;	// 应答地址长度(7位或10位)

	GPIO_TypeDef 	*scl_port;	// SCL引脚所在GPIO端口
	uint16_t		scl_pin;	// SCL引脚编号
	uint32_t		scl_clk;	// SCL引脚GPIO端口时钟使能位

	GPIO_TypeDef	*sda_port;	// SDA引脚所在GPIO端口
	uint16_t		sda_pin;	// SDA引脚编号
	uint32_t		sda_clk;	// SDA引脚GPIO端口时钟使能位
}bsp_i2c_hw_t;

/**
 * @brief I2C硬件配置表
 * @note 定义了I2C1和I2C2的所有硬件参数
 */
static const bsp_i2c_hw_t bsp_i2c_hw[BSP_I2C_MAX] = {
	// I2C1配置: SCL=PB6, SDA=PB7
	[BSP_I2C1] = {
		.i2c 		= I2C1,					// 使用I2C1外设
		.i2c_clk 	= RCC_APB1Periph_I2C1,	// I2C1挂载在APB1总线上

		.clk_speed 	= I2C_CLK_SPEED,		// 通信速率(由宏定义指定)
		.mode		= I2C_Mode_I2C,			// 标准I2C模式
		.duty_cycle = I2C_DutyCycle_2,		// 占空比2:1
		.own_addr	= I2C_OWNADDR,			// 本机地址
		.ack		= I2C_Ack_Enable,		// 使能应答
		.ack_addr	= I2C_AcknowledgedAddress_7bit,	// 7位地址模式

		.scl_port 	= GPIOB,				// SCL使用GPIOB
		.scl_pin 	= GPIO_Pin_6,			// SCL连接到PB6
		.scl_clk 	= RCC_APB2Periph_GPIOB,	// GPIOB时钟

		.sda_port 	= GPIOB,				// SDA使用GPIOB
		.sda_pin 	= GPIO_Pin_7,			// SDA连接到PB7
		.sda_clk 	= RCC_APB2Periph_GPIOB,	// GPIOB时钟
	},

	// I2C2配置: SCL=PB10, SDA=PB11
	[BSP_I2C2] = {
		.i2c 		= I2C2,					// 使用I2C2外设
		.i2c_clk 	= RCC_APB1Periph_I2C2,	// I2C2挂载在APB1总线上

		.clk_speed 	= I2C_CLK_SPEED,		// 通信速率(由宏定义指定)
		.mode		= I2C_Mode_I2C,			// 标准I2C模式
		.duty_cycle = I2C_DutyCycle_2,		// 占空比2:1
		.own_addr	= I2C_OWNADDR,			// 本机地址
		.ack		= I2C_Ack_Enable,		// 使能应答
		.ack_addr	= I2C_AcknowledgedAddress_7bit,	// 7位地址模式

		.scl_port 	= GPIOB,				// SCL使用GPIOB
		.scl_pin 	= GPIO_Pin_10,			// SCL连接到PB10
		.scl_clk 	= RCC_APB2Periph_GPIOB,	// GPIOB时钟

		.sda_port 	= GPIOB,				// SDA使用GPIOB
		.sda_pin 	= GPIO_Pin_11,			// SDA连接到PB11
		.sda_clk 	= RCC_APB2Periph_GPIOB,	// GPIOB时钟
	},
};

/**
 * @brief I2C总线解锁函数
 * @param GPIO_InitStruct GPIO初始化结构体指针
 * @param hw I2C硬件配置指针
 * @note 通过模拟时钟信号解决I2C总线死锁问题
 */
static void BSP_HI2C_Unlock(GPIO_InitTypeDef *GPIO_InitStruct, const bsp_i2c_hw_t *hw)
{
	/**
	 * 事件：I2C总线死锁
	 * 现象：I2C发送起始信号的时候就检测到BUSY=1表示总线被占用,并且按下复位按键
	 * 并不能够清除占用,BUSY仍然等于1。只有当板子重新上电的时候BUSY才被清空为0
	 * 此时发送起始信号成功
	 * 原因：主从机状态不同步
	 * 场景：单片机 (Master) 正在从 I2C 从机 (Slave) 读取数据。当从机正好输出 
	 * 低电平 (Low)(例如发送数据位 '0' 或者 应答位 ACK)时,你按下了单片机的
	 * 复位键,或者程序发生异常重启。主机 (STM32):复位后,SCL 和 SDA 变成
	 * 高阻态(被上拉电阻拉高)。主机尝试初始化 I2C,检测到 SDA 是低电平,
	 * 认为总线被别人占用了(BUSY 标志置位),于是拒绝启动。从机 (Device):
	 * 因为它没有复位,它还在等待主机给它下一个时钟信号(SCL)来完成那个被中断
	 * 的 '0' 的传输。它一直拉着 SDA 不放。结果主机等从机放手,从机等主机给时钟。
	 * 两人互相等待,直到断电让从机也复位。
	 * 解决方法：增加总线恢复序列(手动模拟时钟信号让从机将剩余的数据发送完毕)
	 * **/	
	
	/* 将scl和sda配置为通用开漏输出,为了能够手动控制高低电平,模拟时钟 */
	GPIO_InitStruct->GPIO_Pin = hw->scl_pin;		// 设置SCL引脚
	GPIO_InitStruct->GPIO_Mode = GPIO_Mode_Out_OD;	// 开漏输出模式
	GPIO_InitStruct->GPIO_Speed = GPIO_Speed_50MHz;	// 输出速度50MHz
	GPIO_Init(hw->scl_port, GPIO_InitStruct);		// 初始化SCL引脚

	GPIO_InitStruct->GPIO_Pin = hw->sda_pin;		// 设置SDA引脚
	GPIO_InitStruct->GPIO_Mode = GPIO_Mode_Out_OD;	// 开漏输出模式
	GPIO_InitStruct->GPIO_Speed = GPIO_Speed_50MHz;	// 输出速度50MHz
	GPIO_Init(hw->sda_port, GPIO_InitStruct);		// 初始化SDA引脚

	// 将SCL和SDA都拉高(空闲状态)
	GPIO_SetBits(hw->scl_port, hw->scl_pin);		// SCL输出高电平
	GPIO_SetBits(hw->sda_port, hw->sda_pin);		// SDA输出高电平
	BSP_Delay_us(5);								// 延时5微秒

	/* 模拟SCL产生9个时钟脉冲,将从机里边的数据引出来 */
	for(int i = 0; i < 9; i++){
		/* 拉低SCL允许SDA线上的数据进行翻转 */
		GPIO_ResetBits(hw->scl_port, hw->scl_pin);	// SCL输出低电平
		BSP_Delay_us(5);							// 延时5微秒
		
		/* 拉高SCL进行数据传输 */
		GPIO_SetBits(hw->scl_port, hw->scl_pin);	// SCL输出高电平
		BSP_Delay_us(5);							// 延时5微秒
	}
	
	/* 此时从机传输完数据并希望收到来自主机的结束信号 */
	/* 结束信号：在SCL高电平期间,SDA线上产生上升沿 */
	GPIO_ResetBits(hw->sda_port, hw->sda_pin);		// SDA先拉低
	BSP_Delay_us(5);								// 延时5微秒
	GPIO_SetBits(hw->scl_port, hw->scl_pin);		// SCL拉高
	BSP_Delay_us(5);								// 延时5微秒
	GPIO_SetBits(hw->scl_port, hw->scl_pin);		// SDA拉高,产生停止信号
}

/**
 * @brief I2C初始化函数
 * @param i2c_id I2C编号(BSP_I2C1或BSP_I2C2)
 * @note 配置GPIO引脚、I2C参数并启用I2C外设
 */
void BSP_HI2C_Init(bsp_hi2c_t i2c_id)
{
	// 参数检查:I2C编号超出范围则返回
	if(i2c_id >= BSP_I2C_MAX)	return;

	const bsp_i2c_hw_t *hw = &bsp_i2c_hw[i2c_id];	// 获取硬件配置
	GPIO_InitTypeDef GPIO_InitStruct;				// GPIO初始化结构体
	I2C_InitTypeDef	 I2C_InitStruct;				// I2C初始化结构体

	GPIO_StructInit(&GPIO_InitStruct);				// 初始化GPIO结构体为默认值
	I2C_StructInit(&I2C_InitStruct);				// 初始化I2C结构体为默认值

	/* 解除总线死锁问题 */
	BSP_HI2C_Unlock(&GPIO_InitStruct, hw);

	/* 开启 I2Cx 的时钟 */
	RCC_APB1PeriphClockCmd(hw->i2c_clk, ENABLE);	// 使能I2C外设时钟

	/* 开启 IIC 引脚所在的 GPIO 端口时钟 */
	RCC_APB2PeriphClockCmd(hw->scl_clk | hw->sda_clk, ENABLE);	// 使能GPIO端口时钟

	/* 配置 IIC 的 scl 引脚的模式、速度 */
	GPIO_InitStruct.GPIO_Pin 	= hw->scl_pin;		// SCL引脚
	GPIO_InitStruct.GPIO_Mode 	= GPIO_Mode_AF_OD;	// 复用开漏输出
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_50MHz;	// 输出速度50MHz
	GPIO_Init(hw->scl_port, &GPIO_InitStruct);		// 初始化SCL引脚

	/* 配置 IIC 的 sda 引脚的模式、速度 */
	GPIO_InitStruct.GPIO_Pin 	= hw->sda_pin;		// SDA引脚
	GPIO_InitStruct.GPIO_Mode 	= GPIO_Mode_AF_OD;	// 复用开漏输出
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_50MHz;	// 输出速度50MHz
	GPIO_Init(hw->sda_port, &GPIO_InitStruct);		// 初始化SDA引脚

	/**
	 * 清空I2C寄存器的所有标志位,表示重新开始,防止之前的标志位的值
	 * 没有清空导致此时的总线还被占用
	*/
	I2C_DeInit(hw->i2c);							// 复位I2C外设寄存器
	
	/* 配置 IIC 的工作模式 */
	I2C_InitStruct.I2C_ClockSpeed 			= hw->clk_speed;	// 时钟频率
	I2C_InitStruct.I2C_Mode					= hw->mode;			// 工作模式
	I2C_InitStruct.I2C_DutyCycle			= hw->duty_cycle;	// 占空比
	I2C_InitStruct.I2C_OwnAddress1			= hw->own_addr;		// 本机地址
	I2C_InitStruct.I2C_Ack					= hw->ack;			// 应答使能
	I2C_InitStruct.I2C_AcknowledgedAddress 	= hw->ack_addr;		// 应答地址模式
	I2C_Init(hw->i2c, &I2C_InitStruct);			// 初始化I2C

	/* 启用 IIC */
	I2C_Cmd(hw->i2c, ENABLE);					// 使能I2C外设
}

/**
 * @brief 发送I2C起始信号
 * @param i2c_id I2C编号
 */
void BSP_HI2C_Start(bsp_hi2c_t i2c_id)
{
	if(i2c_id >= BSP_I2C_MAX)	return;				// 参数检查
	const bsp_i2c_hw_t *hw = &bsp_i2c_hw[i2c_id];	// 获取硬件配置

	I2C_GenerateSTART(hw->i2c, ENABLE);				// 产生起始信号
}

/**
 * @brief 发送I2C停止信号
 * @param i2c_id I2C编号
 */
void BSP_HI2C_Stop(bsp_hi2c_t i2c_id)
{
	if(i2c_id >= BSP_I2C_MAX)	return;				// 参数检查
	const bsp_i2c_hw_t *hw = &bsp_i2c_hw[i2c_id];	// 获取硬件配置

	I2C_GenerateSTOP(hw->i2c, ENABLE);				// 产生停止信号
}

/**
 * @brief 配置I2C应答功能
 * @param i2c_id I2C编号
 * @param NewState 新状态(ENABLE使能/DISABLE禁用)
 */
void BSP_HI2C_AcknowledgeConfig(bsp_hi2c_t i2c_id, FunctionalState NewState)
{
	if(i2c_id >= BSP_I2C_MAX)	return;				// 参数检查
	const bsp_i2c_hw_t *hw = &bsp_i2c_hw[i2c_id];	// 获取硬件配置

	I2C_AcknowledgeConfig(hw->i2c, NewState);		// 配置应答功能
}

/**
 * @brief 通过I2C发送一个字节数据
 * @param i2c_id I2C编号
 * @param Data 要发送的数据
 */
void BSP_HI2C_SendData(bsp_hi2c_t i2c_id, uint8_t Data)
{
	if(i2c_id >= BSP_I2C_MAX)	return;				// 参数检查
	const bsp_i2c_hw_t *hw = &bsp_i2c_hw[i2c_id];	// 获取硬件配置

	I2C_SendData(hw->i2c, Data);					// 发送数据
}

/**
 * @brief 从I2C接收一个字节数据
 * @param i2c_id I2C编号
 * @return 接收到的数据
 */
uint8_t BSP_HI2C_ReceiveData(bsp_hi2c_t i2c_id)
{
	if(i2c_id >= BSP_I2C_MAX)	return 0;			// 参数检查,返回0
	const bsp_i2c_hw_t *hw = &bsp_i2c_hw[i2c_id];	// 获取硬件配置

	return (I2C_ReceiveData(hw->i2c));				// 接收并返回数据
}

/**
 * @brief 发送7位I2C设备地址
 * @param i2c_id I2C编号
 * @param addr 从机设备地址(7位)
 * @param dir 传输方向(发送/接收)
 */
void BSP_HI2C_Send7bitAddress(bsp_hi2c_t i2c_id, uint8_t addr, i2c_dir_t dir)
{
	if(i2c_id >= BSP_I2C_MAX)	return;				// 参数检查
	const bsp_i2c_hw_t *hw = &bsp_i2c_hw[i2c_id];	// 获取硬件配置

	// 根据方向发送地址
	if(dir == BSP_I2C_Dir_Transmitt){
		I2C_Send7bitAddress(hw->i2c, addr, I2C_Direction_Transmitter);	// 发送方向
	}else{
		I2C_Send7bitAddress(hw->i2c, addr, I2C_Direction_Receiver);		// 接收方向
	}
}

/**
 * @brief 获取I2C标志位状态
 * @param i2c_id I2C编号
 * @param flag 标志位(如I2C_FLAG_BUSY、I2C_FLAG_TXE等)
 * @return 标志位状态(SET或RESET)
 */
FlagStatus BSP_HI2C_GetFlagStatus(bsp_hi2c_t i2c_id, uint32_t flag)
{
	if(i2c_id >= BSP_I2C_MAX)	return RESET;		// 参数检查,返回RESET
	const bsp_i2c_hw_t *hw = &bsp_i2c_hw[i2c_id];	// 获取硬件配置

	return (I2C_GetFlagStatus(hw->i2c, flag));		// 返回标志位状态
}

/**
 * @brief 清除I2C标志位
 * @param i2c_id I2C编号
 * @param flag 要清除的标志位
 */
void BSP_HI2C_ClearFlag(bsp_hi2c_t i2c_id, uint32_t flag)
{
	if(i2c_id >= BSP_I2C_MAX)	return;				// 参数检查
	const bsp_i2c_hw_t *hw = &bsp_i2c_hw[i2c_id];	// 获取硬件配置
	
	I2C_ClearFlag(hw->i2c, flag);					// 清除标志位
}

/**
 * @brief 清除I2C地址发送标志位
 * @param i2c_id I2C编号
 * @note 通过读SR1和SR2寄存器来清除ADDR标志位
 */
void BSP_HI2C_ClearFlag_Addr(bsp_hi2c_t i2c_id)
{
	if(i2c_id >= BSP_I2C_MAX)	return;				// 参数检查
	const bsp_i2c_hw_t *hw = &bsp_i2c_hw[i2c_id];	// 获取硬件配置

	// 读SR1和SR2寄存器以清除ADDR标志
	(void)(hw->i2c->SR1);							// 读SR1寄存器
	(void)(hw->i2c->SR2);							// 读SR2寄存器
}

/**
 * @brief 等待I2C事件发生(带超时)
 * @param i2c_id I2C编号
 * @param event 要等待的事件(如I2C_EVENT_MASTER_MODE_SELECT)
 * @param timeout_ms 超时时间(毫秒)
 * @return I2C_OK:成功 I2C_TIMEOUT:超时 I2C_ERROR:错误
 */
i2c_status_t BSP_HI2C_WaitEvent(bsp_hi2c_t i2c_id, uint32_t event, uint32_t timeout_ms)
{
	if(i2c_id >= BSP_I2C_MAX)	return I2C_ERROR;	// 参数检查,返回错误

	const bsp_i2c_hw_t *hw = &bsp_i2c_hw[i2c_id];	// 获取硬件配置
	uint32_t start = BSP_GetTick();					// 记录开始时间

	// 循环等待事件发生
	while(I2C_CheckEvent(hw->i2c, event) == ERROR){
		// 检查是否超时
		if((BSP_GetTick() - start) > timeout_ms){
			return I2C_TIMEOUT;						// 返回超时
		}
	}

	return I2C_OK;									// 事件发生,返回成功
}

/**
 * @brief 检查I2C事件是否发生
 * @param i2c_id I2C编号
 * @param event 要检查的事件
 * @return SUCCESS:事件发生 ERROR:事件未发生
 */
ErrorStatus BSP_HI2C_CheckEvent(bsp_hi2c_t i2c_id, uint32_t event)
{
	if(i2c_id >= BSP_I2C_MAX)	return ERROR;		// 参数检查,返回ERROR
	const bsp_i2c_hw_t *hw = &bsp_i2c_hw[i2c_id];	// 获取硬件配置

	return (I2C_CheckEvent(hw->i2c, event));		// 返回事件检查结果
}

/**
 * @brief 读取I2C寄存器值
 * @param i2c_id I2C编号
 * @param reg 寄存器编号(如I2C_Register_SR1)
 * @return 寄存器的值
 */
uint16_t BSP_HI2C_ReadRegister(bsp_hi2c_t i2c_id, uint8_t reg)
{
	if(i2c_id >= BSP_I2C_MAX)	return 0;			// 参数检查,返回0
	const bsp_i2c_hw_t *hw = &bsp_i2c_hw[i2c_id];	// 获取硬件配置

	return (I2C_ReadRegister(hw->i2c, reg));		// 返回寄存器值
}

/**
 * @brief 打印I2C寄存器信息(调试用)
 * @param i2c_id I2C编号
 * @note 打印I2C_CR1、I2C_OAR1、I2C_SR1、I2C_SR2、I2C_CCR等寄存器的值及关键位状态
 */
void BSP_HI2C_Echo_RegMsg(bsp_hi2c_t i2c_id)
{
	if(i2c_id >= BSP_I2C_MAX)	return;				// 参数检查

	// 读取各个寄存器的值
	uint16_t I2C_CR1 = BSP_HI2C_ReadRegister(i2c_id, I2C_Register_CR1);	// 控制寄存器1
	uint16_t I2C_OAR1 = BSP_HI2C_ReadRegister(i2c_id, I2C_Register_OAR1);	// 本机地址寄存器1
	uint16_t I2C_SR1 = BSP_HI2C_ReadRegister(i2c_id, I2C_Register_SR1);	// 状态寄存器1
	uint16_t I2C_SR2 = BSP_HI2C_ReadRegister(i2c_id, I2C_Register_SR2);	// 状态寄存器2
	uint16_t I2C_CCR = BSP_HI2C_ReadRegister(i2c_id, I2C_Register_CCR);	// 时钟控制寄存器
	//uint16_t I2C_DR = BSP_HI2C_ReadRegister(i2c_id, I2C_Register_DR);	// 数据寄存器(读取会清除标志)

	// 打印寄存器的值
	printf("%s|%s|%d I2C_CR1 = 0x%04x\r\n",__FILE__,__func__, __LINE__,I2C_CR1);
	printf("%s|%s|%d I2C_OAR1 = 0x%04x\r\n",__FILE__,__func__, __LINE__,I2C_OAR1);
	printf("%s|%s|%d I2C_SR1 = 0x%04x I2C_SR2 = 0x%04x\r\n",__FILE__,__func__, __LINE__,I2C_SR1, I2C_SR2);
	printf("%s|%s|%d I2C_CCR = 0x%04x\r\n",__FILE__,__func__, __LINE__,I2C_CCR);
	//printf("%s|%s|%d I2C_DR  = 0x%04x\r\n",__FILE__,__func__, __LINE__,I2C_DR);

	// 提取CR1寄存器中的关键位
	uint8_t CR1_ACK 	= (I2C_CR1 & I2C_CR1_ACK_BIT) ? 1 : 0;		// 应答使能位
	uint8_t CR1_STOP 	= (I2C_CR1 & I2C_CR1_STOP_BIT) ? 1 : 0;		// 停止位
	uint8_t CR1_START 	= (I2C_CR1 & I2C_CR1_START_BIT) ? 1 : 0;	// 起始位
	uint8_t CR1_PE		= (I2C_CR1 & I2C_CR1_PE_BIT) ? 1 : 0;		// 外设使能位
	printf("%s|%s|%d Bit10_ACK = %d Bit9_STOP = %d Bit8_START = %d Bit0_PE = %d\r\n",
								__FILE__, __func__, __LINE__, CR1_ACK, CR1_STOP, CR1_START, CR1_PE);
	
	// 提取SR1寄存器中的关键位
	uint8_t SR1_AF = (I2C_SR1 & I2C_SR1_AF_BIT) ? 1 : 0;			// 应答失败位
	uint8_t SR1_TXE = (I2C_SR1 & I2C_SR1_TXE_BIT) ? 1 : 0;			// 发送数据寄存器空
	uint8_t SR1_RXNE = (I2C_SR1 & I2C_SR1_RXNE_BIT) ? 1 : 0;		// 接收数据寄存器非空
	uint8_t SR1_STOPF = (I2C_SR1 & I2C_SR1_STOPF_BIT) ? 1 : 0;		// 停止检测标志
	uint8_t SR1_BTF = (I2C_SR1 & I2C_SR1_BTF_BIT) ? 1 : 0;			// 字节传输完成
	uint8_t SR1_ADDR = (I2C_SR1 & I2C_SR1_ADDR_BIT) ? 1 : 0;		// 地址已发送/匹配
	uint8_t SR1_SB = (I2C_SR1 & I2C_SR1_SB_BIT) ? 1 : 0;			// 起始位已发送

	// 提取SR2寄存器中的关键位
	uint8_t SR2_BUSY = (I2C_SR2 & I2C_SR2_BUSY_BIT) ? 1 : 0;		// 总线忙
	uint8_t SR2_MSL = (I2C_SR2 & I2C_SR2_MSL_BIT) ? 1 : 0;			// 主/从模式
	uint8_t SR2_TRA = (I2C_SR2 & I2C_SR2_TRA_BIT) ? 1 : 0;			// 发送/接收模式
	
	// 打印SR1和SR2的关键位状态
	printf("%s|%s|%d Bit10_AF = %d Bit7_TXE = %d Bit6_RNXE = %d Bit4_STOPF = %d Bit2_BTF = %d Bit1_ADDR = %d Bit0_SB = %d\r\n",
								__FILE__, __func__, __LINE__, SR1_AF, SR1_TXE, SR1_RXNE, SR1_STOPF, SR1_BTF, SR1_ADDR, SR1_SB);
	printf("%s|%s|%d Bit2_TRA = %d Bit1_BUSY = %d Bit0_MSL = %d\r\n",__FILE__, __func__, __LINE__, SR2_TRA, SR2_BUSY, SR2_MSL);
	printf("==============================================================\r\n");
		
}