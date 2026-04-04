/**
 * @file bsp_eeprom_hi2c.c
 * @brief EEPROM(AT24C02)驱动实现文件
 * @note 基于硬件I2C接口实现EEPROM的字节写入/读取、页写入、顺序读取等功能
 *       支持写周期轮询检测,确保数据可靠写入
 */
#include "bsp_eeprom_hi2c.h"

/**
 * @brief 打印EEPROM操作状态信息(调试用)
 * @param status I2C操作状态(I2C_OK/I2C_ERROR/I2C_TIMEOUT)
 * @param msg 操作描述信息
 * @note 用于调试时打印详细的操作状态和寄存器信息
 *       当操作出错或超时时,会调用BSP_HI2C_Echo_RegMsg打印I2C寄存器详情
 */
static void BSP_EEPROM_Echo_Statusmsg(i2c_status_t status, char *msg)
{
	printf("=================================================================\r\n");
	if(status == I2C_ERROR){
		printf("%s|%s|%d EEPROM %s出错\r\n",__FILE__, __func__, __LINE__, msg);
		BSP_HI2C_Echo_RegMsg(BSP_EEPROM_I2C_ID);		// 打印I2C寄存器详细信息
	}else if(status == I2C_TIMEOUT){
		printf("%s|%s|%d EEPROM %s超时\r\n",__FILE__, __func__, __LINE__, msg);
		BSP_HI2C_Echo_RegMsg(BSP_EEPROM_I2C_ID);		// 打印I2C寄存器详细信息
	}else{
		printf("%s|%s|%d EEPROM %s成功\r\n",__FILE__, __func__, __LINE__, msg);
	}
}

/**
 * @brief EEPROM写完成轮询函数
 * @note EEPROM内部写周期约5ms,在此期间不响应外部命令
 *       通过应答查询方式检测写入是否完成:
 *       
 *       工作原理:
 *       - 在写入数据后,EEPROM进入内部写周期
 *       - 写周期期间EEPROM不响应外部命令
 *       - 通过不断发送起始信号+器件地址来轮询
 *       - 若收到ACK(ADDR=1): 说明写入完成,退出轮询
 *       - 若收到NACK(AF=1): 说明仍在写周期中,清除标志后继续轮询
 *       
 *       工作流程:
 *       1. 等待总线空闲(BUSY=0)
 *       2. 进入轮询循环:
 *          a. 发送起始信号
 *          b. 等待SB标志位(起始信号已发送)
 *          c. 发送EEPROM器件地址
 *          d. 进入子循环等待ADDR或AF:
 *             - ADDR=1: 写入完成,清除标志,发送STOP,退出
 *             - AF=1: 仍在写周期,清除标志,发送STOP,跳出子循环继续轮询
 *       
 *       注意事项:
 *       - 不是进行数据通信,而是检测写入是否完成
 *       - AF标志位必须清除,否则会阻止后续STOP/START位写入导致死循环
 *       - 每次轮询后都要发送STOP信号释放总线
 *       - 超时保护避免无限等待
 */
static void BSP_EEPROM_Polling(void)
{
	/* 检测总线是否空闲(是否已经完成发送停止信号这一操作并释放总线) */	
	uint32_t timeout = 10000;			// 总线空闲超时时间
	i2c_status_t status;

	// 等待总线空闲
	while(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_BUSY) == SET){
		if((timeout--) == 0){
			printf("%s|%s|%d 总线仍然被占用且已超时，检查停止信号是否发送\r\n",__FILE__, __func__, __LINE__);
			return;
		}
	}
	//printf("%s|%s|%d 总线已空闲\r\n",__FILE__, __func__, __LINE__);

	/* 不断的发送起始信号和 EEPROM 的地址检测它是否已经将数据写入到存储区中 */
	timeout = 0xFFFF;					// 轮询超时时间
	while(timeout--){
		/* 发送起始信号 */
		BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
		
		/**
		 * 检测事件：I2C_EVENT_MASTER_MODE_SELECT (EV5)
		 * SB标志位置1条件:
		 * (1) 起始条件已经发送到总线上
		 * (2) MSL置为1(发送起始条件的I2C器件被设为主模式)
		 * (3) BUSY置为1(总线忙,表示此时总线已经被占用开始通信)
		 * (4) SB置为1
		 * */
		status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, 100);
		//BSP_EEPROM_Echo_Statusmsg(status, "发送起始信号");
		
		/* 若这里没有收到起始信号(SB == 1 && MSL == 1 && BUSY == 1)，则后边操作无效*/
		if(status != I2C_OK){
			printf("%s|%s|%d 发送起始信号失败\r\n",__FILE__, __func__,__LINE__);
			continue;		// 跳过本次循环,继续下一次轮询
		}

		/* 发送EEPROM地址 + R/W读写位 */
		BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
		
		/* 等待ADDR或AF标志位 */
		/* AF 标志位置为1的条件：ACK 应答失败(无应答)
		 * 注意：此时应该清除 AF 标志位，若不清除该标志位，发送方会一直认为应答失败，由于 AF 标志位
		 * 有着高优先级，若在 AF = 1 时尝试写入STOP / STAT位硬件会直接忽略这个请求。所以就会导致死循环。
		 *
		 * 在清除标志位后还需要发送停止信号释放总线，等待下一次判断通信(判断 EEPROM 数据是否写入完成)
		 * */
		uint32_t sub_timeout = 1000;	// 子循环超时时间
		while(sub_timeout--){
			/* 检查ADDR标志位(地址应答成功) */
			/* ADDR标志位置1条件: 地址阶段完成且收到从机ACK */
			if(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_ADDR) == SET){
				BSP_HI2C_ClearFlag_Addr(BSP_EEPROM_I2C_ID);		// 清除ADDR标志位
				BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);				// 发送停止信号
				//printf("%s|%s|%d EEPROM 写入完成\r\n",__FILE__, __func__,__LINE__);
				return;		// EEPROM写入完成,退出轮询
			}

			/* 检查AF标志位(应答失败,EEPROM仍在写周期中) */
			/**
			 * AF标志位置1条件: ACK应答失败(无应答)
			 * 注意: AF标志位必须清除,否则:
			 * - 发送方会一直认为应答失败
			 * - AF标志位有高优先级
			 * - AF=1时尝试写入STOP/START位,硬件会直接忽略这个请求
			 * - 导致死循环
			 * 
			 * 在清除标志位后还需要发送停止信号释放总线,
			 * 等待下一次轮询判断(判断EEPROM数据是否写入完成)
			 * */
			if(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_AF) == SET){
				BSP_HI2C_ClearFlag(BSP_EEPROM_I2C_ID, I2C_FLAG_AF);	// 清除AF标志位
				BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);					// 发送停止信号释放总线
				break;		// 跳出子循环,继续外层轮询
			}
		}
	}
	
	// 轮询超时,写入失败
	printf("%s|%s|%d EEPROM 写入超时\r\n",__FILE__, __func__, __LINE__);
}

/**
 * @brief EEPROM初始化函数
 * @note 初始化EEPROM所使用的I2C接口(I2C1)
 *       板载EEPROM连接到I2C1,所以只需初始化I2C1即可
 */
void BSP_EEPROM_Init(void)
{
	/* EEPROM 接的是I2C1,初始化I2C1接口 */
	BSP_HI2C_Init(BSP_EEPROM_I2C_ID);
}

/**
 * @brief 向EEPROM写入单个字节
 * @param write_addr EEPROM内部存储地址(0-255)
 * @param write_data 要写入的数据
 * 
 * @note 写入流程:
 *       1. 发送起始信号 → 等待EV5(MASTER_MODE_SELECT)
 *       2. 发送器件地址+写方向 → 等待EV6(MASTER_TRANSMITTER_MODE_SELECTED)
 *       3. 发送存储地址(字地址) → 等待EV8(MASTER_BYTE_TRANSMITTING)
 *       4. 发送数据 → 等待EV8_2(MASTER_BYTE_TRANSMITTED)
 *       5. 发送停止信号
 *       6. 等待EEPROM内部写入完成(约5ms)
 *       
 *       事件说明:
 *       - EV5 (MASTER_MODE_SELECT): START条件已发送,进入主机模式
 *         标志位: SB=1, MSL=1, BUSY=1
 *       - EV6 (MASTER_TRANSMITTER_MODE_SELECTED): 地址阶段完成,主机发送模式
 *         标志位: ADDR=1, TXE=1, MSL=1, BUSY=1, TRA=1
 *       - EV8 (MASTER_BYTE_TRANSMITTING): DR空,可以写下一字节
 *         标志位: TXE=1
 *       - EV8_2 (MASTER_BYTE_TRANSMITTED): 字节传输完成,DR和移位寄存器均空
 *         标志位: TXE=1, BTF=1
 *       
 *       重要提示:
 *       - 最后一个字节必须等待EV8_2,确保数据完全发送
 *       - 如果在移位寄存器未空时发送STOP,可能导致数据丢失和总线占用
 */
static void BSP_EEPROM_WriteByte(uint8_t write_addr, uint8_t write_data)
{
	i2c_status_t status;
	
	/* 发送起始信号 */
	BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
	
	/**
	 * 检测事件：I2C_EVENT_MASTER_MODE_SELECT (EV5)
	 * 何时产生：I2C_GenerateSTART()之后
	 * 事件含义：START条件已经成功发送,STM32已进入主机模式
	 * 标志位状态: SB=1, MSL=1, BUSY=1
	 * */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送起始信号");

	/* 发送EEPROM地址 + R/W读写位 */
	BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
	
	/**
	 * 检测事件：I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED (EV6)
	 * 何时产生：发送从机地址+写方向,从机ACK
	 * 事件含义：地址阶段完成,当前处于"主机+发送模式"
	 * 标志位状态: ADDR=1, TXE=1, MSL=1, BUSY=1, TRA=1
	 * */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送 EEPROM 地址");

	//printf("%s|%s|%d write_addr: %d write_data:%d\r\n",__FILE__,__func__, __LINE__, write_addr, write_data);
	
	/* 发送 EEPROM 字地址(写入到哪里) */
	BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, write_addr);
	
	/**
	 * 检测事件：I2C_EVENT_MASTER_BYTE_TRANSMITTING (EV8)
	 * 何时产生：数据从DR转移到移位寄存器或已发送
	 * 事件含义：TXE为空,表明数据已经从DR转移到移位寄存器或者是已经发送出去了,
	 *          所以只要DR寄存器为空就可以写入下一个字节了
	 * 标志位状态: TXE=1
	 * 用途：检测非最后一个字节的发送状态
	 * */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTING, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送 EEPROM 字地址");
	
	/* 发送 EEPROM 数据(写什么到 EEPROM 中) */
	BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, write_data);
	
	/**
	 * 检测事件：I2C_EVENT_MASTER_BYTE_TRANSMITTED (EV8_2)
	 * 何时产生：写入DR的一个字节被完整发送,从机ACK
	 * 事件含义：上一个字节已经成功发送并被从机接收
	 * 标志位状态: TXE=1, BTF=1 (数据寄存器空且字节传输完成)
	 * 
	 * 重要性：这里必须检测EV8_2这个事件来判断最后一个字节是否发送完成
	 * 1. EV8只表示DR已空,可以写下一个字节,但移位寄存器可能仍在发送中
	 * 2. 如果在移位寄存器数据未发送完毕时就发送STOP,可能导致数据未完整发出,
	 *    SDA总线仍被占用,从而导致下一次I2C通信失败
 	 * 因此,最后一个字节发送完成必须等待EV8_2(TXE=1且BTF=1),确保DR和移位寄存器均空
 	 * */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送 EEPROM 写入数据");

	/* 发送停止信号 */
	BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);

	/* 等待数据写入到EEPROM中(等待内部写周期完成,约5ms) */
	BSP_EEPROM_Polling();
	printf("%s|%s|%d 数据成功写入\r\n",__FILE__, __func__, __LINE__);
}

/**
 * @brief 向EEPROM写入一页数据
 * @param write_addr EEPROM内部起始地址
 * @param buffer 要写入的数据缓冲区指针
 * @param size 要写入的数据大小(最大8字节,AT24C02的页大小)
 * 
 * @note 页写入可以一次性写入多个字节(最多8字节),提高写入效率
 *       写入流程:
 *       1. 发送起始信号 → 等待EV5
 *       2. 发送器件地址+写方向 → 等待EV6
 *       3. 发送存储起始地址 → 等待EV8
 *       4. 循环发送数据:
 *          - 前size-1个字节: 发送数据 → 等待EV8 (TXE=1)
 *          - 最后1个字节: 发送数据 → 等待EV8_2 (TXE=1 && BTF=1)
 *       5. 发送停止信号
 *       6. 等待EEPROM内部写入完成
 *       
 *       注意事项:
 *       - AT24C02每页8字节,不能跨页写入
 *       - 最后一个字节必须等待EV8_2确保完全发送
 *       - 写入后需等待约5ms的内部写周期
 */
static void BSP_EEPROM_WritePage(uint8_t write_addr, uint8_t *buffer, uint8_t size)
{
	/* AT24C02 页写入一次性最多写入8字节数据 */
	if(size > BSP_EEPROM_WRITE_PAGE_MAX)	return;

	i2c_status_t status;
	
	/* 发送起始信号 */
	BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
	/* 等待事件EV5：I2C_EVENT_MASTER_MODE_SELECT */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送起始地址");

	/* 发送设备地址+W */
	BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
	/* 等待事件EV6：I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送设备地址");

	/* 发送字地址(存储起始地址) */
	BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, write_addr);
	/* 等待事件EV8：I2C_EVENT_MASTER_BYTE_TRANSMITTING */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTING, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送字地址");

	/* 循环发送数据 */
	for(int i = 0; i < size; i++){
		/* 发送数据 */
		BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, buffer[i]);
		
		if(i < size - 1){
			/* 前size-1个字节检测事件EV8 */
			/* 等待事件EV8：I2C_EVENT_MASTER_BYTE_TRANSMITTING */
			/* EV8表示DR已空,可以写入下一字节,但移位寄存器可能还在发送 */
			status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTING, 100);
		}else if(i == size - 1){
			/* 最后一个字节检测EV8_2: I2C_EVENT_MASTER_BYTE_TRANSMITTED */
			/* EV8_2表示DR和移位寄存器均空,字节完全发送完成 */
			status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, 100);
		}
		//BSP_EEPROM_Echo_Statusmsg(status, "发送数据");
	}
	
	/* 发送停止信号 */
	BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);

	/* 检测数据是否已经写入成功(等待内部写周期完成) */
	BSP_EEPROM_Polling();
	//printf("%s|%s|%d 数据写入成功\r\n",__FILE__,__func__, __LINE__);
}

/**
 * @brief 连续向EEPROM写入多字节数据(支持跨页写入)
 * @param write_addr EEPROM内部起始写入地址(0-255)
 * @param buffer 要写入的数据缓冲区指针
 * @param size 要写入的数据总字节数
 * 
 * @note 自动处理跨页写入的问题:
 *       AT24C02等EEPROM按页写入(通常8字节/页),不能直接跨越页边界连续写入。
 *       本函数会自动计算当前页的剩余空间,分次调用页写函数,实现任意长度数据的连续写入。
 *       写入流程:
 *       1. 计算当前页剩余可写字节数
 *       2. 比较剩余字节数和待写总字节数,取较小值作为本次写入长度
 *       3. 调用页写操作写入数据
 *       4. 更新地址、缓冲区指针和剩余字节数,循环直到写完全部数据
 */
void BSP_EEPROM_WriteBuffer(uint8_t write_addr, uint8_t *buffer, uint16_t size)
{
	while(size > 0){
		/* 查看当前地址所在页还可以写入多少字节数据 */
		uint8_t remain = BSP_EEPROM_WRITE_PAGE_MAX - (write_addr % BSP_EEPROM_WRITE_PAGE_MAX);
		
		/* 判断要写入字节数是否小于当前页剩余字节数
		 * 若小于则直接写入实际大小
		 * 若大于则先写满本页剩余空间后再进入下一次循环写入
		 * */
		uint8_t len = (size < remain) ? size : remain;
		BSP_EEPROM_WritePage(write_addr, buffer, len);

		/* 下一个循环来的时候，数据偏移 + 地址偏移 + 减去已经写入的字节数 */
		buffer += len;          
		write_addr += len;
		size -= len;
	}
}

/**
 * @brief 从EEPROM随机读取单个字节
 * @param read_addr EEPROM内部存储地址(0-255)
 * @param read_data 读取数据的存储指针
 * 
 * @note 随机读取:可以从任意地址开始读取
 *       
 *       读取流程(虚拟写+真实读):
 *       第一阶段-虚拟写(设置读地址):
 *       1. 等待总线空闲(BUSY=0)
 *       2. 发送起始信号 → 等待EV5
 *       3. 发送器件地址+写方向 → 等待EV6
 *       4. 发送读地址(字地址) → 等待EV8_2
 *       
 *       第二阶段-真实读:
 *       5. 重新发送起始信号(重复起始) → 等待EV5
 *       6. 发送器件地址+读方向 → 等待EV6
 *       7. 禁用ACK(单字节读取不需要应答)
 *       8. 发送停止信号(提前设置,等待NACK后发送)
 *       9. 等待RXNE → 读取数据
 *       10. 重新使能ACK(为下次通信准备)
 *       
 *       关键点:
 *       - 读取单字节时需提前设置NACK和STOP,时序要求严格
 *       - 如果当接收到一个字节后再去设置NACK,时序上根本来不及,
 *         主机可能早就应答了从机设备,然后从机会进入发送下一字节的过程
 *       - 此时仅仅只是设置停止信号不发送,等待从机响应NACK后才发送停止信号
 */
static void BSP_EEPROM_ReadRandom(uint8_t read_addr, uint8_t *read_data)
{
	i2c_status_t status;

	/* 检测总线是否空闲(是否已经完成发送停止信号这一操作并释放总线) */	
	uint32_t timeout = 10000;
	while(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_BUSY) == SET){
		if((timeout--) == 0){
			printf("%s|%s|%d 总线仍然被占用且已超时，检查停止信号是否发送\r\n",__FILE__, __func__, __LINE__);
			return;
		}
	}
	//printf("%s|%s|%d 总线已空闲\r\n",__FILE__, __func__, __LINE__);

	/* ========== 第一阶段: 虚拟写操作(设置读地址) ========== */
	
	/* 发送起始信号 */
	BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
	/* 检测事件EV5：I2C_EVENT_MASTER_MODE_SELECT */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送起始信号");

	/* 发送 EEPROM 写地址 */
	BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
	/* 检测事件EV6：I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送EEPROM 写地址");
	
	/* 发送EEPROM 字地址(用来读取数据的地址) */
	BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, read_addr);
	/**
	 * 检测事件EV8_2: I2C_EVENT_MASTER_BYTE_TRANSMITTED
	 * 由于下一步要重新发送起始信号，所以必须确保上一字节发送完成
	 * 否则可能导致时序混乱
	 * */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送字地址(读取数据地址)");

	/* ========== 第二阶段: 真实读操作 ========== */
	
	/* 重新发送起始信号(重复起始条件) */
	BSP_HI2C_Start(BSP_EEPROM_I2C_ID);	
	/* 检测事件EV5：I2C_EVENT_MASTER_MODE_SELECT */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送起始信号");

	/* 发送 EEPROM 读地址 */
	BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_READ_ADDR, BSP_I2C_Dir_Receive);
	/* 检测事件EV6：I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送EEPROM 读地址");
	
	/**
	 * 禁用ACK
	 * 原因: 等主机接收到一个字节数据后不回复ACK,此时从机认为不再需要发送数据,
	 *      等待主机发送停止信号
	 * 注意: 这里需提前设置NACK,因为如果当接收到一个字节后再去设置NACK,
	 *      时序上根本来不及,主机可能早就应答了从机设备,然后从机会进入
	 *      发送下一字节的过程
	 * */
	BSP_HI2C_AcknowledgeConfig(BSP_EEPROM_I2C_ID, DISABLE);
	
	/**
	 * 发送停止信号 
	 * 注意: 此时仅仅只是设置停止信号不发送,等待从机响应NACK后才真正发送停止信号
	 * */
	BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
	
	/**
	 * 检测事件EV7：I2C_EVENT_MASTER_BYTE_RECEIVED
	 * 事件含义:
	 * (1) I2C工作在主机模式(MSL=1)
	 * (2) 总线忙(BUSY=1)
	 * (3) 成功接收到从机发送的1个字节
	 * (4) 接收数据已被硬件写入DR寄存器(RXNE=1)
	 * 检测RXNE标志位为1,表示接收数据寄存器非空可以读取数据
	 * */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_RECEIVED, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "接受数据");
	
	/* 从DR寄存器读取数据 */
	*read_data = BSP_HI2C_ReceiveData(BSP_EEPROM_I2C_ID);
	
	/* 重新使能ACK方便下一次通信 */
	/* 注意: 这里应该是ENABLE而不是DISABLE */
	BSP_HI2C_AcknowledgeConfig(BSP_EEPROM_I2C_ID, DISABLE);
}

/**
 * @brief 从EEPROM顺序读取多个字节
 * @param read_addr EEPROM内部起始地址
 * @param buffer 读取数据的缓冲区指针
 * @param size 要读取的数据大小
 * 
 * @note 顺序读取:从指定地址开始连续读取多个字节
 *       
 *       读取流程(虚拟写+真实读):
 *       第一阶段-虚拟写(设置读地址):
 *       1. 等待总线空闲(BUSY=0)
 *       2. 发送起始信号 → 等待EV5
 *       3. 发送器件地址+写方向 → 等待EV6
 *       4. 发送读起始地址 → 等待EV8_2
 *       
 *       第二阶段-真实读:
 *       5. 重新发送起始信号 → 等待EV5
 *       6. 发送器件地址+读方向 → 等待EV6
 *       7. 使能ACK(多字节读取需要应答)
 *       8. 循环读取数据:
 *          - 对于前size-1个字节: 等待RXNE → 读取 → 自动发送ACK
 *          - 对于最后1个字节: 禁用ACK → 发送STOP → 等待RXNE → 读取
 *       9. 重新使能ACK(为下次通信准备)
 *       
 *       关键点:
 *       - 读取多字节时,除最后一个字节外都要发送ACK
 *       - 最后一个字节要发送NACK,告诉从机停止发送
 *       - NACK和STOP的时序很重要,必须在读取最后一个字节之前设置
 */
static void BSP_EEPROM_ReadSequential(uint8_t read_addr, uint8_t *buffer, uint8_t size)
{
	i2c_status_t status;

	/* 检测总线是否空闲(是否已经完成发送停止信号这一操作并释放总线) */	
	uint32_t timeout = 10000;
	while(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_BUSY) == SET){
		if((timeout--) == 0){
			//printf("%s|%s|%d 总线仍然被占用且已超时，检查停止信号是否发送\r\n",__FILE__, __func__, __LINE__);
			return;
		}
	}
	//printf("%s|%s|%d 总线已空闲\r\n",__FILE__, __func__, __LINE__);

	/* ========== 第一阶段: 虚拟写操作(设置读地址) ========== */
	
	/* 发送起始信号 */
	BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
	/* 检测事件EV5：I2C_EVENT_MASTER_MODE_SELECT */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送起始地址");

	/* 发送 EEPROM 写地址 */
	BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
	/* 检测事件EV6：I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送EEPROM 写地址");
	
	/* 发送EEPROM 字地址(用来读取数据的起始地址) */
	BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, read_addr);
	/**
	 * 检测事件EV8_2: I2C_EVENT_MASTER_BYTE_TRANSMITTED
	 * 由于下一步要重新发送起始信号，所以必须确保上一字节发送完成
	 * */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送字地址(读取数据地址)");

	/* ========== 第二阶段: 真实读操作 ========== */
	
	/* 重新发送起始信号(重复起始条件) */
	BSP_HI2C_Start(BSP_EEPROM_I2C_ID);	
	/* 检测事件EV5：I2C_EVENT_MASTER_MODE_SELECT */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送起始地址");

	/* 发送 EEPROM 读地址 */
	BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_READ_ADDR, BSP_I2C_Dir_Receive);
	/* 检测事件EV6：I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED */
	status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, 100);
	//BSP_EEPROM_Echo_Statusmsg(status, "发送EEPROM 读地址");
	
	/* 使能ACK回复发送方(多字节读取需要应答) */
	BSP_HI2C_AcknowledgeConfig(BSP_EEPROM_I2C_ID, ENABLE);

	/* 循环读取数据 */
	for(int i = 0; i < size; i++){
		/**
		 * 由于是读若干个字节，所以需要在接收最后一个字节后发送NACK
		 * 告诉从机停止发送数据
		 * */
		if(i == size - 1){
			/* 最后一个字节不发送ACK表示接收字节结束 */
			BSP_HI2C_AcknowledgeConfig(BSP_EEPROM_I2C_ID, DISABLE);
			/* 发送停止信号 */
			BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
		}
		
		/**
		 * 检测事件EV7：I2C_EVENT_MASTER_BYTE_RECEIVED 
		 * 检测RXNE标志位为1,表示接收数据寄存器非空可以读取数据
		 * */
		status = BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_RECEIVED, 100);
		//BSP_EEPROM_Echo_Statusmsg(status, "接收数据");
		
		/* 从DR寄存器读取数据 */
		buffer[i] = BSP_HI2C_ReceiveData(BSP_EEPROM_I2C_ID);
	}

	/* 重新使能ACK方便下一次通信 */
	BSP_HI2C_AcknowledgeConfig(BSP_EEPROM_I2C_ID, ENABLE);
}

/**
 * @brief 从EEPROM读取任意长度数据 (对外提供的高层统一接口)
 * @param read_addr EEPROM内部起始地址
 * @param buffer 读取数据的缓冲区指针
 * @param size 要读取的数据大小
 * @note 内部调用了连续读取函数来实现
 */
void BSP_EEPROM_ReadBuffer(uint8_t read_addr, uint8_t *buffer, uint16_t size)
{
    /* 既然底层的 ReadSequential 已经支持任意长度（包括1个字节），直接调用即可 */
    BSP_EEPROM_ReadSequential(read_addr, buffer, size);
}

/**
 * @brief EEPROM功能测试函数
 * @note 测试EEPROM的读写功能是否正常,包括:
 *       1. 单字节写入+单字节读取测试
 *       2. 页写入(8字节)+顺序读取(8字节)测试
 *       
 *       测试流程:
 *       - 初始化EEPROM(I2C接口)
 *       - 测试1: 写入单字节15到地址15,读回并比较
 *       - 测试2: 写入8字节数组到地址0,读回并比较
 *       - 根据测试结果LED指示:
 *         成功: 绿灯闪烁3次
 *         失败: 红灯闪烁3次
 */
/**
 * @brief EEPROM功能测试函数
 * @note 测试EEPROM的读写功能是否正常,包括:
 *       1. 单字节写入+单字节读取测试
 *       2. 多字节连续写入(跨页测试)+顺序读取测试
 *       
 *       测试流程:
 *       - 初始化EEPROM(I2C接口)
 *       - 测试1: 写入单字节15到地址15,读回并比较
 *       - 测试2: 跨页写入12字节数组到地址6,读回并比较
 *       - 根据测试结果LED指示:
 *         成功: 绿灯闪烁3次
 *         失败: 红灯闪烁3次
 */
void BSP_EEPROM_Test(void)
{
	uint8_t write_data = 15;						// 单字节测试数据
	uint8_t read_data = 0;							// 单字节读取缓冲
	
	/* 测试跨越页边界条件：起始地址6，写入12字节。
	 * 第一页只能写2个字节(地址6,7),剩下10个字节会分两次写入(第二页8字节,第三页2字节)
	 */
	uint8_t handler_addr = 6;						// 跨页写入起始地址
	uint8_t write_buffer[12] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC};	// 跨页测试数据
	uint8_t recv_buf[12] = {0};						// 读取缓冲

	printf("============EEPROM=========\r\n");
	BSP_EEPROM_Init();

	/* ========== 测试1: EEPROM 写入一个字节 + 读取一个字节 ========== */
	printf("%s|%s|%d EEPROM 单字节写入 + 读取单字节测试......\r\n", __FILE__, __func__, __LINE__);
	BSP_EEPROM_WriteByte(15, write_data);			// 写入数据15到地址15
	printf("%s|%s|%d 写入数据：%d\r\n", __FILE__, __func__, __LINE__, write_data);
	BSP_EEPROM_ReadRandom(15, &read_data);			// 从地址15读取数据
	printf("%s|%s|%d 读取到的数据：%d\r\n", __FILE__, __func__, __LINE__, read_data);
	
	/* 比较写入和读取的数据 */
	if(write_data == read_data){
		printf("%s|%s|%d 单字节写入 + 读取测试成功\r\n", __FILE__, __func__, __LINE__);
		// 成功: 绿灯闪烁3次
		for(int i = 0; i < 3; i++){
			BSP_LED_On(LED_GREEN);
			BSP_Delay_ms(500);
			BSP_LED_Off(LED_GREEN);
			BSP_Delay_ms(500);
		}
	}else{
		printf("%s|%s|%d 单字节写入 + 读取测试失败\r\n", __FILE__, __func__, __LINE__);
		// 失败: 红灯闪烁3次
		for(int i = 0; i < 3; i++){
			BSP_LED_On(LED_RED);
			BSP_Delay_ms(500);
			BSP_LED_Off(LED_RED);
			BSP_Delay_ms(500);
		}
	}
	
	/* ========== 测试2: EEPROM 跨页多字节连续写入 + 顺序读取 ========== */
	printf("%s|%s|%d EEPROM 跨页多字节连续写入 + 顺序读取测试 ......\r\n", __FILE__, __func__, __LINE__);
	BSP_EEPROM_WriteBuffer(handler_addr, write_buffer, 12);	// 调用写入一串数据
	printf("%s|%s|%d 写入数据: \r\n", __FILE__, __func__, __LINE__);
	for(int i = 0; i < 12; i++){
		printf("%s|%s|%d buffer[%d] = 0x%02X\r\n",__FILE__, __func__, __LINE__,i, write_buffer[i]);
	}

	BSP_EEPROM_ReadSequential(handler_addr, recv_buf, 12);	// 顺序读取刚写入的数据
	printf("%s|%s|%d 读取数据: \r\n", __FILE__, __func__, __LINE__);
	for(int i = 0; i < 12; i++){
		printf("%s|%s|%d buffer[%d] = 0x%02X\r\n",__FILE__, __func__, __LINE__,i, recv_buf[i]);
	}

	/* 比较写入和读取的数据 */
	uint8_t flag = 1;
	for(int i = 0; i < 12; i++){
		if(write_buffer[i] != recv_buf[i]){
			flag = 0;		// 数据不匹配
			break;
		}
	}

	/* 根据比较结果显示LED */
	if(flag == 0){
		printf("%s|%s|%d EEPROM 跨页多字节写入 + 顺序读取测试失败\r\n", __FILE__, __func__, __LINE__);
		// 失败: 红灯闪烁3次
		for(int i = 0; i < 3; i++){
			BSP_LED_On(LED_RED);
			BSP_Delay_ms(500);
			BSP_LED_Off(LED_RED);
			BSP_Delay_ms(500);
			return; // 失败直接退出
		}
	}else if(flag == 1){
		printf("%s|%s|%d EEPROM 跨页多字节写入 + 顺序读取测试成功\r\n", __FILE__, __func__, __LINE__);
		// 成功: 绿灯闪烁3次
		for(int i = 0; i < 3; i++){
			BSP_LED_On(LED_GREEN);
			BSP_Delay_ms(500);
			BSP_LED_Off(LED_GREEN);
			BSP_Delay_ms(500);
		}
	}

	printf("%s|%s|%d EEPROM 测试完成\r\n",__FILE__, __func__, __LINE__);
}
