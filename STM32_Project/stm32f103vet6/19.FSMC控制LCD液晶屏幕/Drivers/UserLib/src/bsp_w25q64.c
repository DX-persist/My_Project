#include "bsp_w25q64.h"

/**
 * @brief  初始化 W25Q64 
 * @note   包含所使用的 SPI 外设初始化以及 CS 片选引脚的初始化。
 *         初始化完成后会将 CS 引脚默认拉高（取消选中）。
 * @retval 无
 */
void BSP_W25Q64_Init(void)
{
	/* 初始化W25Q64 所用到的SPI外设 */
	BSP_HSPI_Init(BSP_W25Q64_SPI_ID);
	
	/* 开启 CS 片选引脚的时钟 */
	RCC_APB2PeriphClockCmd(W25Q64_CS_CLK, ENABLE);	

	/* 初始化CS 片选引脚 */
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = W25Q64_CS_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(W25Q64_CS_PORT, &GPIO_InitStruct);	

	/* 默认CS 片选信号为高电平，表示未选中 */
	W25Q64_CS_DISABLE();
}	

/**
 * @brief  通过 SPI 总线同时发送和接收一个字节
 * @param  write_byte 要发送给丛机的字节数据
 * @param  read_byte  用于接收丛机返回数据的指针（可传入 NULL 忽略接收）
 * @retval 函数返回值返回接收到的那一个字节数据
 */
static uint8_t BSP_W25Q64_ReadWriteByte(uint8_t write_byte, uint8_t *read_byte)
{
	return (BSP_HSPI_ReadWriteByte(BSP_W25Q64_SPI_ID, write_byte, read_byte));
}

/**
 * @brief  读取 W25Q64 状态寄存器 1 
 * @retval 状态寄存器 1 的当前值
 */
static uint8_t BSP_W25Q64_ReadRegister1(void)
{
	uint8_t reg_status = 0;

	W25Q64_CS_ENABLE();
	
	/* 在 CS 为低电平时发送读取W25Q64寄存器指令 */
	BSP_W25Q64_ReadWriteByte(W25Q64_READ_REGISTER1_CMD, NULL);

	/* 发送伪指令接收从机返回的寄存器状态 */
	BSP_W25Q64_ReadWriteByte(W25Q64_DUMMY_CMD, &reg_status);

	W25Q64_CS_DISABLE();

	return reg_status;
}

/**
 * @brief  等待 Flash 内部写入或擦除操作完成 (轮询 BUSY 标志位)
 * @param  timeout_ms 最大等待超时时间（毫秒）
 * @param  err_code   超时发生时用于 LOG 打印定位的错误识别码
 * @retval 无
 */
static void BSP_W25Q64_WaitBusy(uint32_t timeout_ms, uint8_t err_code)
{
	uint32_t start = BSP_GetTick();

	while((BSP_W25Q64_ReadRegister1() & W25Q64_REGISTER_BUSY) == 1)
    {
        /* 每次只等 1ms */
        BSP_Delay_ms(1); 
        
        /* 检查是否总时间超标 */
        if((BSP_GetTick() - start) > timeout_ms)
        {
            W25Q64_LOG("W25Q64 操作超时: err_code = %d\r\n", err_code);
            return;
        }
    }
}

/**
 * @brief  获取 W25Q64 制造商 ID 与设备 ID (JEDEC ID)
 * @retval 组合后的 24 位 JEDEC ID (例如：0xEF4017)
 */
uint32_t BSP_W25Q64_ReadID(void)
{
	uint32_t JEDEC_ID = 0;
	uint8_t temp_byte = 0;
	uint8_t manu_id = 0;			/* 制造商ID */
	uint16_t dev_id = 0;			/* 设备ID */

	/* 拉低 CS 片选信号选中W25Q64 */
	W25Q64_CS_ENABLE();

	/* 发送检测 JEDEC ID 的指令 */
	BSP_W25Q64_ReadWriteByte(W25Q64_READ_JEDEC_ID_CMD, NULL);

	/* 从机接收到指令后准备返回长度为3个字节的JEDEC ID 
	 * 由于 SPI 是同步传输，所以这里需要发送三个伪指令
	 * 来驱动 SCK 时钟，此时可以从 MISO 引脚上接收到3
	 * 个字节的数据
	 * */
	BSP_W25Q64_ReadWriteByte(W25Q64_DUMMY_CMD, &temp_byte);
	JEDEC_ID |= (uint32_t)temp_byte << 16;

	BSP_W25Q64_ReadWriteByte(W25Q64_DUMMY_CMD, &temp_byte);
	JEDEC_ID |= (uint32_t)temp_byte << 8;

	BSP_W25Q64_ReadWriteByte(W25Q64_DUMMY_CMD, &temp_byte);
	JEDEC_ID |= (uint32_t)temp_byte;

	/* 获取制造商ID */
	manu_id = (JEDEC_ID >> 16) & 0xFF;

	/* 获取设备ID */
	dev_id = JEDEC_ID & 0xFFFF;

	W25Q64_LOG("%s|%s|%d JEDEC_ID: 0x%08lx\r\n",__FILE__, __func__, __LINE__, JEDEC_ID);
	W25Q64_LOG("%s|%s|%d Manu_ID: 0x%x Dev_ID: 0x%04x\r\n",__FILE__, __func__, __LINE__, manu_id, dev_id);

	/* 拉高 CS 片选信号取消选中W25Q64 */
	W25Q64_CS_DISABLE();

	return JEDEC_ID;
}

/**
 * @brief  向 Flash 发送写使能指令 (Write Enable)
 * @note   在对 Flash 发送擦除或任何写数据指令前，均需调用本函数。
 * @retval 无
 */
static void BSP_W25Q64_WriteEnable(void)
{
	W25Q64_CS_ENABLE();

	/* 在 CS 为低电平时发送写入使能指令 */
	BSP_W25Q64_ReadWriteByte(W25Q64_WRITE_ENABLE_CMD, NULL);

	W25Q64_CS_DISABLE();
}

/**
 * @brief  擦除 W25Q64 指定的 4KB 扇区
 * @param  erase_addr 想擦除的扇区地址（必须为 4096 的整数倍对应边界）
 * @retval 无
 */
void BSP_W25Q64_Sector_Erase(uint32_t erase_addr)
{
	/* 在擦除前先写入使能，因为实际上擦除也是写入操作，
	 * 该操作会往要擦除的扇区内写入1
	 * */
	BSP_W25Q64_WriteEnable();
	
	W25Q64_CS_ENABLE();

	/* 写入扇区擦除指令，并附带24位的擦除地址*/
	BSP_W25Q64_ReadWriteByte(W25Q64_SECTOR_ERASE_CMD, NULL);
	/* 写入24位地址 */
	BSP_W25Q64_ReadWriteByte((erase_addr >> 16) & 0xFF, NULL);
	BSP_W25Q64_ReadWriteByte((erase_addr >> 8) & 0xFF, NULL);
	BSP_W25Q64_ReadWriteByte(erase_addr & 0xFF, NULL);

	W25Q64_CS_DISABLE();

	/* 这里需要先将 CS 拉高才能判断内部的 BUSY 标志位是否为0，
	 * 因为只有当 CS 拉高的时候，SPI 才会认为此次通信结束。此
	 * 时内部会锁存数据并启动内部写入，只有当写入完成后，BUSY
	 * 标志位才会变为0，并且这里需要注意的是每次操作结束后WEL
	 * 标志位也会变为0，所以每次对 Flash 进行写入的时候都要重
	 * 新进行写入使能
	 * */
	BSP_W25Q64_WaitBusy(W25Q64_TIMEOUT_SECTOR_ERASE, 1);		
}

/**
 * @brief  在一页内 (不超过256字节) 写入数据
 * @note   严禁跨越页边界调用此函数，超出边界的数据会回卷覆盖当前页起始内容。
 * @param  write_buffer 待写入的数据指针
 * @param  size         需要写入的字节数 (max: 256)
 * @param  write_addr   在 Flash 中的起始写入地址
 * @retval 无
 */
static void BSP_W25Q64_PageWrite(uint8_t *write_buffer, uint16_t size, uint32_t write_addr)
{
	if(size > W25Q64_PAGE_WRITE_MAX){
		W25Q64_LOG("%s|%s|%d W25Q64 页写入最多写入256个字节!!!\r\n",__FILE__, __func__, __LINE__);
		return;
	}

	/* 写入使能 */
	BSP_W25Q64_WriteEnable();

	W25Q64_CS_ENABLE();

	/* 发送页写入指令 + 24位写入地址*/
	BSP_W25Q64_ReadWriteByte(W25Q64_PAGE_WRITE_CMD, NULL);
	BSP_W25Q64_ReadWriteByte((write_addr >> 16) & 0xFF, NULL);
	BSP_W25Q64_ReadWriteByte((write_addr >> 8) & 0xFF, NULL);
	BSP_W25Q64_ReadWriteByte(write_addr & 0xFF, NULL);
	
	for(int i = 0; i < size; i++){
		BSP_W25Q64_ReadWriteByte(write_buffer[i], NULL);
	}

	W25Q64_CS_DISABLE();	

	BSP_W25Q64_WaitBusy(W25Q64_TIMEOUT_PAGE_PROGRAM, 2);
}

/**
 * @brief  写入大量不定长数据至 W25Q64 (自动管理页偏移)
 * @note   在写操作前保证传入地址及其所在扇区已擦除完毕。本接口不负责擦除。
 * @param  write_buffer 待写入的总数据指针
 * @param  size         待写入的数据总长度（单位：字节）
 * @param  write_addr   Flash 中目标起始地址
 * @retval 无
 */
void BSP_W25Q64_BufferWrite(uint8_t *write_buffer, uint16_t size, uint32_t write_addr)
{
	while(size > 0){
		/* 求出当前页还可以写入多少字节数据 */
		/* 因为 flash 内部写入后地址会自动偏移，
		 * 所以利用这一特性可知当前页还有多少字节可以写入 
		 * */
		uint16_t remain = W25Q64_PAGE_WRITE_MAX - (write_addr % W25Q64_PAGE_WRITE_MAX);
		/* 判断若要写入的字节数是否小于当前页所剩余的字节数 
		 * 若小于，则直接写入size
		 * 若size > remain，那么就先写满本页
		 * */
		uint16_t len = (size < remain) ? size : remain;
		BSP_W25Q64_PageWrite(write_buffer, len, write_addr);

		/* 下一个循环进来，写入字节偏移 + 地址偏移 + 带写入字节数 */
		write_buffer	+= len;
		write_addr		+= len;
		size 			-= len;
	}
}

/**
 * @brief  从指定地址读取多字节连续数据
 * @param  read_buffer 存放读出数据的指针缓存
 * @param  size        想要读出的连续字符数长度（单位：字节）
 * @param  read_addr   读取的目标所在 Flash 物理地址
 * @retval 无
 */
void BSP_W25Q64_BufferRead(uint8_t *read_buffer, uint16_t size, uint32_t read_addr)
{
	W25Q64_CS_ENABLE();

	/* 发送读取数据指令 + 24位写入地址*/
	BSP_W25Q64_ReadWriteByte(W25Q64_READ_DATA_CMD, NULL);
	BSP_W25Q64_ReadWriteByte((read_addr >> 16) & 0xFF, NULL);
	BSP_W25Q64_ReadWriteByte((read_addr >> 8) & 0xFF, NULL);
	BSP_W25Q64_ReadWriteByte(read_addr & 0xFF, NULL);
	
	for(int i = 0; i < size; i++){
		BSP_W25Q64_ReadWriteByte(W25Q64_DUMMY_CMD, &read_buffer[i]);
	}

	W25Q64_CS_DISABLE();
}

/**
 * @brief  完整的 W25Q64 测试入口函数 
 * @note   依次执行初始化、ID读取鉴别、单页读写对比测试、多页(跨页)对读测试、
 *         浮点数据存取对比校验，以及字符串数据存取等多种边界类型的测试流程。
 *         配合板载 LED 及宏向终端输出测试报告。
 * @retval 无
 */
void BSP_W25Q64_Test(void)
{
	uint32_t w25q64_JEDECID = 0;
	uint8_t write_buffer[W25Q64_PAGE_WRITE_MAX];
	uint8_t read_buffer[W25Q64_PAGE_WRITE_MAX];
	uint16_t len = sizeof(write_buffer) / sizeof(write_buffer[0]);
	uint8_t err_flag = 0;

	W25Q64_LOG("======================== W25Q64 Flash 测试开始 ======================\r\n");

	/* 初始化 W25Q64 Flash */
	BSP_W25Q64_Init();

	/* 读JEDEC ID */
	w25q64_JEDECID = BSP_W25Q64_ReadID();
	if(w25q64_JEDECID == W25Q64_JEDEC_ID){
		W25Q64_LOG("%s|%s|%d 检测到 W25Q64 芯片\r\n",__FILE__, __func__, __LINE__);
	}

	/* 给write_buffer写入数据 */
	for(int i = 0; i < len; i++){
		write_buffer[i] = i;
	}

	/*======================= 测试1：1页写入 + 1页读取 ======================= */
	W25Q64_LOG("======================= 测试1: 1页写入 + 1页读取 =======================\r\n");
	BSP_W25Q64_Sector_Erase(W25Q64_ERASE_ADDR);
	BSP_W25Q64_PageWrite(write_buffer, len, W25Q64_WRITE_ADDR);
	BSP_W25Q64_BufferRead(read_buffer, len, W25Q64_READ_ADDR);
	for(int i = 0; i < len; i++){
		W25Q64_LOG("%d ",read_buffer[i]);
		if((i + 1) % 20 == 0){
			W25Q64_LOG("\r\n");
		}
	}
	W25Q64_LOG("\r\n");

	for(int i = 0; i < len; i++){
		if(write_buffer[i] != read_buffer[i]){
			err_flag = 1;
			break;
		}
	}
	if(err_flag == 1){
		W25Q64_LOG("================= 测试1: 1页写入 + 1页读取测试失败 ==============\r\n");
		for(int i = 0; i < 3; i++){
			BSP_LED_On(LED_RED);
			BSP_Delay_ms(500);
			BSP_LED_Off(LED_RED);
			BSP_Delay_ms(500);
		}
	}else if(err_flag == 0){
		W25Q64_LOG("================= 测试1: 1页写入 + 1页读取测试成功 ==============\r\n");
		for(int i = 0; i < 3; i++){
			BSP_LED_On(LED_GREEN);
			BSP_Delay_ms(500);
			BSP_LED_Off(LED_GREEN);
			BSP_Delay_ms(500);
		}
	}

	/*======================= 测试2: 2页写入 + 2页读取 ======================= */
	W25Q64_LOG("======================= 测试2: 2页写入 + 2页读取 =======================\r\n");
	uint16_t write_buffer2[512];
	uint16_t read_buffer2[512];
	len = sizeof(write_buffer2) / sizeof(write_buffer2[0]);

	for(int i = 0; i < len; i++){
		write_buffer2[i] = i;
	}

	/* 这里写入两页,即 512Bytes,但是还是属于第一扇区
	 * 因为一个扇区的大小为 4k = 4096Bytes
	**/
	BSP_W25Q64_Sector_Erase(W25Q64_ERASE_ADDR);
	/* 这里有一点需要注意，因为 write_buffer2 的类型是 uint16_t 的
	 * 所以它这里每一个成员占用两个字节的数据，写入的时候就应该写入
	 * len * 2，如果只写入长度为 len 的数据，那么就是只写入了 write_buffer2
	 * 里边的前512个字节的数据，即只写入了256个数。同样的，读取的时候也是同理
	*/
	BSP_W25Q64_BufferWrite((uint8_t *)write_buffer2, len * 2, W25Q64_WRITE_ADDR);
	BSP_W25Q64_BufferRead((uint8_t *)read_buffer2, len * 2, W25Q64_READ_ADDR);
	for(int i = 0; i < len; i++){
		W25Q64_LOG("%d ",read_buffer2[i]);
		if((i + 1) % 20 == 0){
			W25Q64_LOG("\r\n");
		}
	}
	W25Q64_LOG("\r\n");

	for(int i = 0; i < len; i++){
		if(write_buffer2[i] != read_buffer2[i]){
			err_flag = 1;
			break;
		}
	}
	if(err_flag == 1){
		W25Q64_LOG("================= 测试2: 2页写入 + 2页读取测试失败 ==============\r\n");
		for(int i = 0; i < 3; i++){
			BSP_LED_On(LED_RED);
			BSP_Delay_ms(500);
			BSP_LED_Off(LED_RED);
			BSP_Delay_ms(500);
		}
	}else if(err_flag == 0){
		W25Q64_LOG("================= 测试2: 2页写入 + 2页读取测试成功 ==============\r\n");
		for(int i = 0; i < 3; i++){
			BSP_LED_On(LED_GREEN);
			BSP_Delay_ms(500);
			BSP_LED_Off(LED_GREEN);
			BSP_Delay_ms(500);
		}
	}

	/*======================= 测试3: 写入小数 + 读取小数 ======================= */
	W25Q64_LOG("======================= 测试3: 写入小数 + 读取小数 =======================\r\n");
	float write_float_data = 3.14;
	float read_float_data = 0;

	BSP_W25Q64_Sector_Erase(W25Q64_ERASE_ADDR);
	
	BSP_W25Q64_BufferWrite((uint8_t *)&write_float_data, sizeof(float), W25Q64_WRITE_ADDR);
	BSP_W25Q64_BufferRead((uint8_t *)&read_float_data, sizeof(float), W25Q64_READ_ADDR);
	
	W25Q64_LOG("写入的小数为: %f\r\n",write_float_data);
	W25Q64_LOG("读取到的小数为: %f\r\n",read_float_data);

	if(fabs(write_float_data - read_float_data) < 1e-9){
		err_flag = 0;
	}else{
		err_flag = 1;
	}
	if(err_flag == 1){
		W25Q64_LOG("================= 测试3: 写入小数 + 读取小数测试失败 ==============\r\n");
		for(int i = 0; i < 3; i++){
			BSP_LED_On(LED_RED);
			BSP_Delay_ms(500);
			BSP_LED_Off(LED_RED);
			BSP_Delay_ms(500);
		}
	}else if(err_flag == 0){
		W25Q64_LOG("================= 测试3: 写入小数 + 读取小数测试成功 ==============\r\n");
		for(int i = 0; i < 3; i++){
			BSP_LED_On(LED_GREEN);
			BSP_Delay_ms(500);
			BSP_LED_Off(LED_GREEN);
			BSP_Delay_ms(500);
		}
	}

	/*======================= 测试4: 写入字符串 + 读取字符串 ======================= */
	W25Q64_LOG("======================= 测试4: 写入字符串 + 读取字符串 =======================\r\n");
	char write_char_data[32] = {'\0'};
	char read_char_data[32] = {'\0'};

	strncpy(write_char_data, "W25Q64 读取字符串测试", 29);

	BSP_W25Q64_Sector_Erase(W25Q64_ERASE_ADDR);
	
	BSP_W25Q64_BufferWrite((uint8_t *)write_char_data, strlen(write_char_data), W25Q64_WRITE_ADDR);
	BSP_W25Q64_BufferRead((uint8_t *)read_char_data, strlen(write_char_data), W25Q64_READ_ADDR);
	
	W25Q64_LOG("写入的字符串为: %s\r\n",write_char_data);
	W25Q64_LOG("读取到的字符串为: %s\r\n",read_char_data);

	if(!strncmp(write_char_data, read_char_data, strlen(read_char_data))){
		err_flag = 0;
	}else{
		err_flag = 1;
	}
	if(err_flag == 1){
		W25Q64_LOG("================= 测试4: 写入字符串 + 读取字符串测试失败 ==============\r\n");
		for(int i = 0; i < 3; i++){
			BSP_LED_On(LED_RED);
			BSP_Delay_ms(500);
			BSP_LED_Off(LED_RED);
			BSP_Delay_ms(500);
		}
	}else if(err_flag == 0){
		W25Q64_LOG("================= 测试4: 写入字符串 + 读取字符串测试成功 ==============\r\n");
		for(int i = 0; i < 3; i++){
			BSP_LED_On(LED_GREEN);
			BSP_Delay_ms(500);
			BSP_LED_Off(LED_GREEN);
			BSP_Delay_ms(500);
		}
	}

	W25Q64_LOG("======================== W25Q64 Flash 测试结束 ======================\r\n");
}
