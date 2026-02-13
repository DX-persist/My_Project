#include "bsp_eeprom.h"

void BSP_EEPROM_Init(void)
{
	/* 由于板载 EEPROM 接到了I2C1，所以初始化I2C1 */
	BSP_HI2C_Init(BSP_EEPROM_I2C_ID);
}

static void BSP_I2C_DiagnosticCheck(void)
{
    printf("\r\n========== I2C诊断检查 ==========\r\n");

    /* 1. 检查时钟 */
    printf("1. 时钟检查:\r\n");
    if(RCC->APB1ENR & RCC_APB1ENR_I2C1EN)
        printf("   ✓ I2C1时钟已开启\r\n");
    else
    {
        printf("   ✗ I2C1时钟未开启！正在开启...\r\n");
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    }

    if(RCC->APB2ENR & RCC_APB2ENR_IOPBEN)
        printf("   ✓ GPIOB时钟已开启\r\n");
    else
    {
        printf("   ✗ GPIOB时钟未开启！正在开启...\r\n");
        RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    }

    /* 2. 检查I2C寄存器 */
    printf("\n2. I2C寄存器状态:\r\n");
    printf("   CR1   = 0x%04X\r\n", I2C1->CR1);
    printf("   CR2   = 0x%04X\r\n", I2C1->CR2);
    printf("   OAR1  = 0x%04X\r\n", I2C1->OAR1);
    printf("   SR1   = 0x%04X\r\n", I2C1->SR1);
    printf("   SR2   = 0x%04X\r\n", I2C1->SR2);
    printf("   CCR   = 0x%04X\r\n", I2C1->CCR);
    printf("   TRISE = 0x%04X\r\n", I2C1->TRISE);

    /* 3. 检查关键位 */
    printf("\n3. 关键位状态:\r\n");
    printf("   PE (使能位)      = %d %s\r\n",
           (I2C1->CR1 & I2C_CR1_PE) ? 1 : 0,
           (I2C1->CR1 & I2C_CR1_PE) ? "✓" : "✗ 问题！");

    printf("   SWRST (复位位)   = %d %s\r\n",
           (I2C1->CR1 & I2C_CR1_SWRST) ? 1 : 0,
           (I2C1->CR1 & I2C_CR1_SWRST) ? "✗ 问题！" : "✓");

    printf("   FREQ (频率配置)  = %d MHz %s\r\n",
           I2C1->CR2 & 0x3F,
           ((I2C1->CR2 & 0x3F) >= 2) ? "✓" : "✗ 问题！");

    /* 4. 检查GPIO配置 */
    printf("\n4. GPIO配置:\r\n");
    printf("   PB6 (SCL) CRL = 0x%08X\r\n", GPIOB->CRL);
    printf("   PB7 (SDA) CRL = 0x%08X\r\n", GPIOB->CRL);

    /* 5. 修复问题 */
    printf("\n5. 尝试修复:\r\n");

    if((I2C1->CR1 & I2C_CR1_SWRST) != 0)
    {
        printf("   清除复位状态...\r\n");
        I2C1->CR1 &= ~I2C_CR1_SWRST;
    }

    if((I2C1->CR1 & I2C_CR1_PE) == 0)
    {
        printf("   使能I2C外设...\r\n");
        I2C1->CR1 |= I2C_CR1_PE;

        // 等待PE生效
        for(volatile int i = 0; i < 1000; i++);

        printf("   使能后 CR1 = 0x%04X\r\n", I2C1->CR1);
    }

    printf("========== 诊断完成 ==========\r\n\r\n");
}

static void BSP_EEPROM_Polling(void)
{
    printf("\r\n========== 开始EEPROM轮询 ==========\r\n");

 	BSP_I2C_DiagnosticCheck();   

    while(1)
    {
        uint32_t timeout;
        
        /* 1. 等待总线空闲 */
        printf("步骤1: 等待总线空闲...\r\n");
        timeout = 100000;
        while(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_BUSY) == SET)
        {
            if((timeout--) == 0)
            {
                printf("ERROR: 等待总线空闲超时\r\n");
                return;
            }
        }
        printf("总线空闲，BUSY=0\r\n");
        
        /* 2. 发送起始信号 */
        printf("步骤2: 发送起始信号...\r\n");
        BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
        
        /* 3. 等待SB标志位 */
        printf("步骤3: 等待SB标志位...\r\n");
        timeout = 100000;
        while(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_SB) == RESET)
        {
            if((timeout--) == 0)
            {
                printf("ERROR: SB标志位超时\r\n");
                printf("SR1=0x%04X, SR2=0x%04X\r\n", I2C1->SR1, I2C1->SR2);
                return;
            }
        }
        printf("SB=1, 起始信号已发送\r\n");
        printf("发送前 SR1=0x%04X, SR2=0x%04X\r\n", I2C1->SR1, I2C1->SR2);
        
        /* 4. 发送器件地址 */
        printf("步骤4: 发送地址 0x%02X...\r\n", BSP_EEPROM_WRITE_ADDR);
        BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
        printf("地址已写入DR寄存器\r\n");
        printf("发送后 SR1=0x%04X, SR2=0x%04X\r\n", I2C1->SR1, I2C1->SR2);
        
        /* 5. 等待 ADDR 或 AF */
        printf("步骤5: 等待ADDR或AF标志位...\r\n");
        timeout = 100000;
        uint32_t check_count = 0;
        
        while(1)
        {
            check_count++;
            
            // 每1000次检查打印一次状态
            if(check_count % 1000 == 0)
            {
                uint16_t sr1 = I2C1->SR1;
                uint16_t sr2 = I2C1->SR2;
                printf("检查次数:%lu, SR1=0x%04X, SR2=0x%04X, ADDR=%d, AF=%d, TxE=%d, BTF=%d\r\n",
                       check_count, sr1, sr2,
                       (sr1 & I2C_SR1_ADDR) ? 1 : 0,
                       (sr1 & I2C_SR1_AF) ? 1 : 0,
                       (sr1 & I2C_SR1_TXE) ? 1 : 0,
                       (sr1 & I2C_SR1_BTF) ? 1 : 0);
            }
            
            if((timeout--) == 0)
            {
                printf("ERROR: 等待ADDR/AF超时！\r\n");
                printf("最终 SR1=0x%04X, SR2=0x%04X\r\n", I2C1->SR1, I2C1->SR2);
                BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
                return;
            }
            
            /* 检查ADDR */
            if(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_ADDR) == SET)
            {
                printf(">>> ADDR=1，地址已应答，EEPROM写入完成！\r\n");
                BSP_HI2C_ClearFlag_Addr(BSP_EEPROM_I2C_ID);
                BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
                printf("========== EEPROM轮询结束 ==========\r\n\r\n");
                return;
            }
            
            /* 检查AF */
            if(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_AF) == SET)
            {
                printf(">>> AF=1，无应答，EEPROM仍在写周期中\r\n");
                BSP_HI2C_ClearFlag(BSP_EEPROM_I2C_ID, I2C_FLAG_AF);
                BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
                
                // 延时后重试
                printf("延时后重试...\r\n");
                for(volatile uint32_t i = 0; i < 10000; i++);
                break;
            }
        }
    }
}
#if 0
static void BSP_EEPROM_Polling(void)
{
	/* 在写入数据的那一轮中发送完停止信号，EEPROM 就开始进入到写周期
	 *
	 * 注意这里不是想要和 EEPROM 进行数据通信，而是想要检测 EEPWOM
	 * 是否完成了写入数据这一操作。在写入数据期间，EEPROM 输入无效，
	 * 此时启动应答查询：发送起始条件和器件地址(读/写)。只有当内部
	 * 写周期完成，EEPROM 才会应答为0
	 * */
	while(1){
		while(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_BUSY) == SET);
		/* 发送起始信号 */
		BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
		/* 
		 * SB 标志位置为1的条件：
		 * (1) 起始条件已经发送到总线上；
		 * (2) MSL 置为1(发送起始条件的I2C器件被设为主模式)；
		 * (3) BUSY 置为1(总线忙，表示此时总线已经被占用开始通信)；
		 * (4) SB 置为1
		 * */
		while(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_SB) == RESET);

		/* 发送器件地址 + W */
		BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);

		/* 等待 ADD / AF 标志位 */
		while(1){
			
			/* ADD 标志位置为1的条件：地址阶段完成且收到从机ACK */
			if(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_ADDR) == SET){
				/* 清除ADD 标志位的值 */
				BSP_HI2C_ClearFlag_Addr(BSP_EEPROM_I2C_ID);
				/* 发送停止信号(释放总线) */
				BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);

				return;
			}

			/* AF 标志位置为1的条件：ACK 应答失败(无应答)
			 * 注意：此时应该清除 AF 标志位，若不清除该标志位，发送方会一直认为应答失败，由于 AF 标志位
			 * 有着高优先级，若在 AF = 1 时尝试写入STOP / STAT位硬件会直接忽略这个请求。所以就会导致死循环。
			 *
			 * 在清除标志位后还需要发送停止信号释放总线，等待下一次判断通信(判断 EEPROM 数据是否写入完成)
			 * */
			if(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_AF) == SET){
				/* 清除 AF 标志位 */
				BSP_HI2C_ClearFlag(BSP_EEPROM_I2C_ID, I2C_FLAG_AF);
				/* 发送停止信号(释放总线) */
				BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
				break;
			}
		}
	}
}
#endif

void BSP_EEPROM_WriteByte(uint8_t write_addr, uint8_t data, uint32_t timeout_ms)
{
	printf("%s|%s|%d write addr: %x write data = %d\r\n",__FILE__, __func__, __LINE__,write_addr, data);
	printf("%s|%s|%d write addr: %x write data = %d\r\n",__FILE__, __func__, __LINE__,write_addr, data);
	/* 发送起始信号 */	
	BSP_HI2C_Start(BSP_EEPROM_I2C_ID);

	/**
	 * 事件：I2C_EVENT_MASTE_MODE_SELECT
	 * 何时产生：I2C_GenerateSTAT() 之后
	 * 事件含义：STAT 条件已经成功发送，STM32 已进入主机模式
	 * */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, timeout_ms);
	printf("%s|%s|%d write addr: %x write data = %d\r\n",__FILE__, __func__, __LINE__,write_addr, data);

	/* 发送从机地址(主机此时是发送方) */
	BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
	/**
	 * 事件：I2C_EVENT_MASTE_TWANSMITTER_MODE_SELECTED
	 * 何时产生：发送 从机地址 + 写方向, 从机 ACK
	 * 事件含义：地址阶段完成，当前处于“主机 + 发送模式”
	 * */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, timeout_ms);
	printf("%s|%s|%d write addr: %x write data = %d\r\n",__FILE__, __func__, __LINE__,write_addr, data);

	/* 发送存储地址(数据往 EEPROM 的哪个地址写) */
	BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, write_addr);

	/**
	 * 事件：I2C_EVENT_MASTE_BYTE_TWANSMITTED
	 * 何时产生：写入 D 的一个字节(字地址)被完整发送, 从机 ACK
	 * 事件含义：上一个字节已经成功发送并被从机接收
	 * */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, timeout_ms);
	printf("%s|%s|%d write addr: %x write data = %d\r\n",__FILE__, __func__, __LINE__,write_addr, data);

	/* 发送存储数据(数据写入到指定的地址) */
	BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, data);

	/**
	 * 事件：I2C_EVENT_MASTE_BYTE_TWANSMITTED
	 * 何时产生：写入 D 的一个字节(数据)被完整发送, 从机 ACK
	 * 事件含义：上一个字节已经成功发送并被从机接收
	 * */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, timeout_ms);
	printf("%s|%s|%d write addr: %x write data = %d\r\n",__FILE__, __func__, __LINE__,write_addr, data);

	/* 发送停止信号 */
	BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
	printf("%s|%s|%d write addr: %x write data = %d\r\n",__FILE__, __func__, __LINE__,write_addr, data);

	/* 等待数据写入到 EEPROM 中 */
	BSP_EEPROM_Polling();
	printf("%s|%s|%d write addr: %x write data = %d\r\n",__FILE__, __func__, __LINE__,write_addr, data);
}

void BSP_EEPROM_WritePage(uint8_t write_addr, uint8_t *buffer, uint8_t size, uint32_t timeout_ms)
{
	if(size >= BSP_EEPROM_WRITE_PAGE_MAX)	return;

	/* 主机发送起始条件表明此时进入主模式 */
	BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
	
	/* 判断是否产生了事件5：I2C_EVENT_MASTE_MODE_SELECT */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, timeout_ms);

	/* 主机发送从机地址 + W */
	BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
	/* 检测是否产生事件6：I2C_EVENT_MASTE_TWANSMITTER_MODE_SELECTED */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, timeout_ms);
	
	/* 发送数据（字地址）*/
	BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, write_addr);
	/* 判断是否产生事件8_2: I2C_EVENT_MASTE_BYTE_TWANSMITTED */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, timeout_ms);

	for(int i = 0; i < size; i++){
		/* 发送数据（要写入的数据）*/
		BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, buffer[i]);
		/* 判断是否产生事件8_2: I2C_EVENT_MASTE_BYTE_TWANSMITTED */
		BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, timeout_ms);
	}
	/* 发送停止信号 */
	BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);

	/* 等待数据写入到 EEPROM 中 */
	BSP_EEPROM_Polling();
}

void BSP_EEPROM_ReadRandom(uint8_t read_addr, uint8_t *data, uint32_t timeout_ms)
{
	printf("%s|%s|%d 开始随即接收数据\r\n",__FILE__,__func__,__LINE__);
	/* 主机发送起始条件表明此时进入主模式 */
	BSP_HI2C_Start(BSP_EEPROM_I2C_ID);	
	/* 判断是否产生了事件5：I2C_EVENT_MASTE_MODE_SELECT */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, timeout_ms);

	/* 主机发送从机地址 + W */
	BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
	/* 检测是否产生事件6：I2C_EVENT_MASTE_TWANSMITTER_MODE_SELECTED */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, timeout_ms);

	/* 发送数据（读地址）*/
	BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, read_addr);
	/* 判断是否产生事件8_2: I2C_EVENT_MASTE_BYTE_TWANSMITTED */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, timeout_ms);

	 /* 重新发送起始条件 */
    BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, timeout_ms);
    /* 发送从机地址 + R */
    // 注意：这里手动调用库函数，以便控制清除 ADDR 的时机
    I2C_Send7bitAddress(I2C1, BSP_EEPROM_READ_ADDR, I2C_Direction_Receiver);

    // 等待 ADDR 标志位设置 (单纯检测标志位，不清除)
    // 注意：不要用 I2C_CheckEvent 检测 MASTER_RECEIVER_MODE_SELECTED，因为它会读 SR1+SR2 自动清除 ADDR
    uint32_t timeout = timeout_ms * 0x1000;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR) == RESET)
    {
        if((timeout--) == 0) return; // 超时退出
    }
    // --- 关键修改：读取单个字节的核心步骤 ---
    // 1. 禁用 ACK
    BSP_HI2C_AcknowledgeConfig(BSP_EEPROM_I2C_ID, DISABLE);
    // 2. 发送 STOP
    BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);

    // 3. 清除 ADDR 标志位 (读 SR1 后 读 SR2)
    BSP_HI2C_ClearFlag_Addr(BSP_EEPROM_I2C_ID);
    // 4. 等待 RXNE (数据接收完成)
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_RECEIVED, timeout_ms);

    // 5. 读取数据
    *data = BSP_HI2C_ReceiveData(BSP_EEPROM_I2C_ID);
    // 6. 重新使能 ACK (为下一次通信做准备)
    BSP_HI2C_AcknowledgeConfig(BSP_EEPROM_I2C_ID, ENABLE);
#if 0
	/* 重新发送起始条件 */
	BSP_HI2C_Start(BSP_EEPROM_I2C_ID);	
	/* 判断是否产生了事件5：I2C_EVENT_MASTE_MODE_SELECT */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, timeout_ms);

	/* 发送从机地址 + R */
	BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_READ_ADDR, BSP_I2C_Dir_Receive);
	/* 判断是否产生事件6：I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, timeout_ms);

	/**
	 * 事件：I2C_EVENT_MASTER_BYTE_RECEIVED
	 * 何时产生：
	 * (1) I2C 工作在主机模式（MSL = 1） 
	 * (2) 总线忙（BUSY = 1）
	 * (3) 成功接收到从机发送的 1 个字节
	 * (4) 接收数据已被硬件写入 DR 寄存器（RXNE = 1）
	 * 事件含义：硬件已经接收到一个字节的数据，并且将数据存放到 DR 寄存器中了,等待读取
	 * */
	/* 判断是否产生事件7：I2C_EVENT_MASTER_BYTE_RECEIVED */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_RECEIVED, timeout_ms);
	/* 从 DR 寄存器中读取数据 */
	*data = BSP_HI2C_ReceiveData(BSP_EEPROM_I2C_ID);

	printf("%s|%s|%d read addr:%x read data = %d timeout_ms = %d\r\n",__FILE__,__func__,__LINE__,read_addr, *data,timeout_ms);
	
	/* 主机发送NACK信号表示只接受一个字节 */
	BSP_HI2C_AcknowledgeConfig(BSP_EEPROM_I2C_ID, DISABLE);
	/* 主机发送停止信号结束此次通信 */
	BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
#endif
}

void BSP_EEPROM_ReadSequential(uint8_t read_addr, uint8_t *buffer, uint8_t size, uint32_t timeout_ms)
{
	//if(read_addr % 8 != 0)	return;
	
	/* 主机发送起始条件表明此时进入主模式 */
	BSP_HI2C_Start(BSP_EEPROM_I2C_ID);	
	/* 判断是否产生了事件5：I2C_EVENT_MASTE_MODE_SELECT */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, timeout_ms);

	/* 主机发送从机地址 + W */
	BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
	/* 检测是否产生事件6：I2C_EVENT_MASTE_TWANSMITTER_MODE_SELECTED */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, timeout_ms);

	/* 发送数据（读地址）*/
	BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, read_addr);
	/* 判断是否产生事件8_2: I2C_EVENT_MASTE_BYTE_TWANSMITTED */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, timeout_ms);

	/* 重新发送起始条件 */
	BSP_HI2C_Start(BSP_EEPROM_I2C_ID);	
	/* 判断是否产生了事件5：I2C_EVENT_MASTE_MODE_SELECT */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, timeout_ms);

	/* 发送从机地址 + R */
	BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_READ_ADDR, BSP_I2C_Dir_Receive);
	/* 判断是否产生事件6：I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED */
	BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, timeout_ms);

	for(int i = 0; i < size; i++){
		/* 判断是否产生事件7：I2C_EVENT_MASTER_BYTE_RECEIVED */
		BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_RECEIVED, timeout_ms);
		/* 从 DR 寄存器中读取数据 */
		buffer[i] = BSP_HI2C_ReceiveData(BSP_EEPROM_I2C_ID);
	}
	
	/* 主机发送NACK信号表示接收完毕 */
	BSP_HI2C_AcknowledgeConfig(BSP_EEPROM_I2C_ID, DISABLE);
	/* 主机发送停止信号结束此次通信 */
	BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
	
}

