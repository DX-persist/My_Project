/**
 * @file    bsp_eeprom_si2c.c
 * @brief   EEPROM(AT24C02)驱动实现文件
 * @note    基于软件模拟I2C接口实现EEPROM的字节写入/读取、页写入、顺序读取等功能
 *          支持写周期轮询检测,确保数据可靠写入
 */
#include "bsp_eeprom_si2c.h"

/**
 * @brief EEPROM初始化函数
 * @note 初始化EEPROM所使用的I2C接口
 *       板载EEPROM连接到I2C1,所以只需初始化I2C1即可
 */
void BSP_EEPROM_Init(void)
{
    /* EEPROM 接的是软件I2C1接口 */
    BSP_SI2C_Init(BSP_EEPROM_I2C_ID);
}

/**
 * @brief EEPROM写完成轮询函数
 * @note  通过应答查询方式检测写入是否完成 (Acknowledge Polling):
 *        EEPROM内部写周期约5ms,在此期间不响应外部命令。
 *        通过不断发送起始信号+器件地址(写方向)来轮询:
 *        - 若收到ACK: 说明写入完成,退出轮询
 *        - 若收到NACK: 说明仍在写周期中,继续轮询
 */
static uint8_t BSP_EEPROM_Polling(void)
{
    uint32_t timeout = EEPROM_WAIT_TIMEOUT_MAX;
    while(timeout--){
        /* 发送起始信号 */
        BSP_SI2C_Start(BSP_EEPROM_I2C_ID);
        
        /* 发送期间地址 */
        BSP_SI2C_SendByte(BSP_EEPROM_I2C_ID, EEPROM_DEV_ADDR & 0xFE);
        
        /* 检查是否会返回ACK，若返回ACK则说明写入完成，反之说明还在写入 */
        if(BSP_SI2C_WaitAck(BSP_EEPROM_I2C_ID) == ACK){
            BSP_SI2C_Stop(BSP_EEPROM_I2C_ID);
            return 0; // 成功
        }else{
            /* 继续循环发送起始信号和器件地址直到超时/应答成功 */
            /* 注意：即使收到NACK也要发送Stop以复位总线状态 */
            BSP_SI2C_Stop(BSP_EEPROM_I2C_ID);
            // continue; // 隐式continue
        }
    }
    return 1; // 超时
}

/**
 * @brief  向EEPROM写入单个字节
 * @param  reg_addr: EEPROM内部存储地址(0-255)
 * @param  data:     要写入的数据
 * 
 * @note   写入流程:
 *         1. Start -> DevAddr(W) -> ACK
 *         2. RegAddr -> ACK
 *         3. Data -> ACK
 *         4. Stop
 *         5. Polling等待写入完成
 */
uint8_t BSP_EEPROM_WriteByte(uint8_t reg_addr, uint8_t data)
{
    uint8_t retval = 1;

    /* 调用底层I2C写寄存器函数 */
    retval = BSP_SI2C_WriteReg(BSP_EEPROM_I2C_ID, EEPROM_DEV_ADDR, reg_addr, data);
    
    /* 等待EEPROM内部写入完成 */
    if(BSP_EEPROM_Polling() == 1){
        printf("%s|%s|%d 写入超时\r\n",__FILE__, __func__,__LINE__);
        return 1;
    }
    return retval;
}

/**
 * @brief  向EEPROM物理页内写入数据 (内部辅助函数)
 * @param  reg_addr: EEPROM内部起始地址
 * @param  data:     要写入的数据缓冲区指针
 * @param  size:     要写入的数据大小 (必须保证不跨越物理页边界)
 * @return 0:成功, 1:失败
 */
static uint8_t BSP_EEPROM_WritePage(uint8_t reg_addr, uint8_t *data, uint8_t size)
{
    uint8_t retval;
    /* 调用底层I2C Buffer写入 */
    retval = BSP_SI2C_WriteBuffer(BSP_EEPROM_I2C_ID, EEPROM_DEV_ADDR, reg_addr, data, size);
    if(retval != 0) return retval; // I2C错误
    /* 等待内部写入完成 */
    if(BSP_EEPROM_Polling() == 1){
        printf("%s|%s|%d 按页写入超时\r\n",__FILE__, __func__,__LINE__);
        return 1;
    }
    return 0;
}

/**
 * @brief  向EEPROM写入任意长度数据 (支持跨页自动分块)
 * @param  reg_addr: EEPROM内部起始地址
 * @param  data:     要写入的数据缓冲区指针
 * @param  size:     要写入的数据大小
 * 
 * @note   核心逻辑：
 *         I2C EEPROM有页大小限制(AT24C02为8字节)，不能一次性跨页写入，否则地址会回卷。
 *         本函数自动计算当前页剩余空间，将大数据块拆分成多次单页写入调用。
 */
uint8_t BSP_EEPROM_WriteBuffer(uint8_t reg_addr, uint8_t *data, uint16_t size)
{
    uint8_t retval = 0; // 默认为成功
    while(size > 0){
        /* 计算当前物理页还剩多少字节空间 (AT24C02每页8字节) */
        uint8_t remain = BSP_EEPROM_WRITE_PAGE_MAX - (reg_addr % BSP_EEPROM_WRITE_PAGE_MAX);
        /* 若总剩余大小 < 当前页剩余空间，直接写size。否则写满当前页剩余空间 */
        uint8_t len = (size < remain) ? (uint8_t)size : remain;
        /* 调用内部封装的页写函数 (它包含了发送与轮询等待) */
        retval = BSP_EEPROM_WritePage(reg_addr, data, len);
        if(retval != 0) return retval; // 如果发送出现错误则退出
        /* 更新地址及指针，准备写下一个跨页的块 */
        reg_addr += len;
        data     += len;
        size     -= len;
    }
    return retval;
}

/**
 * @brief  从EEPROM随机读取单个字节
 * @param  reg_addr: EEPROM内部存储地址(0-255)
 * @param  data:     读取数据的存储指针
 * 
 * @note   读取流程:
 *         1. Start -> DevAddr(W) -> ACK -> RegAddr -> ACK
 *         2. RepeatedStart -> DevAddr(R) -> ACK
 *         3. ReadByte -> NACK -> Stop
 */
uint8_t BSP_EEPROM_ReadRandom(uint8_t reg_addr, uint8_t *data)
{
    /* 直接调用底层I2C读寄存器函数 */
    return BSP_SI2C_ReadReg(BSP_EEPROM_I2C_ID, EEPROM_DEV_ADDR, reg_addr, data);
}

/**
 * @brief  从EEPROM顺序读取多个字节
 * @param  reg_addr: EEPROM内部起始地址
 * @param  data:     读取数据的缓冲区指针
 * @param  size:     要读取的数据大小
 * 
 * @note   顺序读取:
 *         EEPROM支持内部地址自动递增，无需考虑页边界，可以直接连续读取。
 *         流程：
 *         1. Start -> DevAddr(W) -> ACK -> RegAddr -> ACK
 *         2. RepeatedStart -> DevAddr(R) -> ACK
 *         3. ReadByte -> ACK -> ReadByte -> ACK ...
 *         4. ReadByte(Last) -> NACK -> Stop
 */
uint8_t BSP_EEPROM_ReadSequential(uint8_t reg_addr, uint8_t *data, uint16_t size)
{
    /* 直接调用底层I2C读Buffer函数 */
    return BSP_SI2C_ReadBuffer(BSP_EEPROM_I2C_ID, EEPROM_DEV_ADDR, reg_addr, data, size);
}


/**
 * @brief  从EEPROM读取任意长度数据
 * @param  reg_addr: EEPROM内部起始地址
 * @param  data:     读取数据的缓冲区指针
 * @param  size:     要读取的数据大小
 * 
 * @note   读取无需考虑页边界，EEPROM支持内部地址自动递增连续读取。
 */
uint8_t BSP_EEPROM_ReadBuffer(uint8_t reg_addr, uint8_t *data, uint16_t size)
{
    return BSP_EEPROM_ReadSequential(reg_addr, data, size);
}

/**
 * @brief EEPROM功能测试函数
 * @note 测试EEPROM的读写功能是否正常
 */
void BSP_EEPROM_Test(void)
{
    printf("============EEPROM 测试开始=========\r\n");
    BSP_EEPROM_Init();
    /* ========== 测试1: 单字节写入 + 单字节读取 ========== */
    uint8_t write_data = 15;
    uint8_t read_data = 0;
    
    printf("%s|%s|%d EEPROM 写入一个字节 + 读取一个字节测试......\r\n", __FILE__, __func__, __LINE__);
    BSP_EEPROM_WriteBuffer(15, &write_data, 1);    // 内部其实自动走了 WritePage 1字节长度  
    printf("%s|%s|%d 写入数据：%d\r\n", __FILE__, __func__, __LINE__, write_data);
    BSP_EEPROM_ReadBuffer(15, &read_data, 1);
    printf("%s|%s|%d 读取到的数据：%d\r\n", __FILE__, __func__, __LINE__, read_data);
    
    if(write_data == read_data){
        printf("%s|%s|%d 测试成功\r\n", __FILE__, __func__, __LINE__);
        for(int i = 0; i < 3; i++){ BSP_LED_On(LED_GREEN); BSP_Delay_ms(500); BSP_LED_Off(LED_GREEN); BSP_Delay_ms(500); }
    }else{
        printf("%s|%s|%d 测试失败\r\n", __FILE__, __func__, __LINE__);
        for(int i = 0; i < 3; i++){ BSP_LED_On(LED_RED); BSP_Delay_ms(500); BSP_LED_Off(LED_RED); BSP_Delay_ms(500); }
    }
    
    /* ========== 测试2: 多字节写入 + 多字节读取 ========== */
    uint8_t handler_addr = 0;
    uint8_t write_array[8] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80};
    uint8_t recv_array[8] = {0};
    printf("%s|%s|%d EEPROM 多字节写入 + 多字节读取测试 ......\r\n", __FILE__, __func__, __LINE__);
    BSP_EEPROM_WriteBuffer(handler_addr, write_array, 8);
    printf("%s|%s|%d 写入数据: \r\n", __FILE__, __func__, __LINE__);
    for(int i = 0; i < 8; i++){ printf("%s|%s|%d buffer[%d] = %d\r\n",__FILE__, __func__, __LINE__,i, write_array[i]); }
    BSP_EEPROM_ReadBuffer(handler_addr, recv_array, 8);
    printf("%s|%s|%d 读取数据: \r\n", __FILE__, __func__, __LINE__);
    for(int i = 0; i < 8; i++){ printf("%s|%s|%d buffer[%d] = %d\r\n",__FILE__, __func__, __LINE__,i, recv_array[i]); }
    uint8_t flag = 1;
    for(int i = 0; i < 8; i++){
        if(write_array[i] != recv_array[i]){ flag = 0; break; }
    }
    if(flag == 0){
        printf("%s|%s|%d 测试失败\r\n", __FILE__, __func__, __LINE__);
        for(int i = 0; i < 3; i++){ BSP_LED_On(LED_RED); BSP_Delay_ms(500); BSP_LED_Off(LED_RED); BSP_Delay_ms(500); return; }
    }else{
        printf("%s|%s|%d 测试成功\r\n", __FILE__, __func__, __LINE__);
        for(int i = 0; i < 3; i++){ BSP_LED_On(LED_GREEN); BSP_Delay_ms(500); BSP_LED_Off(LED_GREEN); BSP_Delay_ms(500); }
    }
    /* ========== 测试3: 写入/读取一个整数 ========== */
    int int_data = 525;
    int recv_int_data = 0;
    uint8_t int_addr = 20;
    printf("%s|%s|%d EEPROM 写入一个整数并读取 ......\r\n", __FILE__, __func__, __LINE__);
    BSP_EEPROM_WriteBuffer(int_addr, (uint8_t *)&int_data, sizeof(int_data));
    printf("%s|%s|%d 写入整数 data = %d\r\n",__FILE__, __func__, __LINE__, int_data);
    BSP_EEPROM_ReadBuffer(int_addr, (uint8_t *)&recv_int_data, sizeof(recv_int_data));
    printf("%s|%s|%d 读取整数 data = %d\r\n",__FILE__, __func__, __LINE__, recv_int_data);
    if(int_data == recv_int_data){
        printf("%s|%s|%d 测试成功\r\n", __FILE__, __func__, __LINE__);
        for(int i = 0; i < 3; i++){ BSP_LED_On(LED_GREEN); BSP_Delay_ms(500); BSP_LED_Off(LED_GREEN); BSP_Delay_ms(500); }
    }else{
        printf("%s|%s|%d 测试失败\r\n", __FILE__, __func__, __LINE__);
        for(int i = 0; i < 3; i++){ BSP_LED_On(LED_RED); BSP_Delay_ms(500); BSP_LED_Off(LED_RED); BSP_Delay_ms(500); }
    }
    /* ========== 测试4: 写入/读取一个小数 ========== */
    double double_data = 3.14;
    double double_recv = 0.0;
    uint8_t double_addr = 30;
    printf("%s|%s|%d EEPROM 写入一个小数并读取 ......\r\n", __FILE__, __func__, __LINE__);
    BSP_EEPROM_WriteBuffer(double_addr, (uint8_t *)&double_data, sizeof(double_data));
    printf("%s|%s|%d 写入小数 data = %f\r\n",__FILE__, __func__, __LINE__, double_data);
    BSP_EEPROM_ReadBuffer(double_addr, (uint8_t *)&double_recv, sizeof(double_recv));
    printf("%s|%s|%d 读取小数 data = %f\r\n",__FILE__, __func__, __LINE__, double_recv);
    if(fabs(double_data - double_recv) < 1e-9){
        printf("%s|%s|%d 测试成功\r\n", __FILE__, __func__, __LINE__);
        for(int i = 0; i < 3; i++){ BSP_LED_On(LED_GREEN); BSP_Delay_ms(500); BSP_LED_Off(LED_GREEN); BSP_Delay_ms(500); }
    }else{
        printf("%s|%s|%d 测试失败\r\n", __FILE__, __func__, __LINE__);
        for(int i = 0; i < 3; i++){ BSP_LED_On(LED_RED); BSP_Delay_ms(500); BSP_LED_Off(LED_RED); BSP_Delay_ms(500); }
    }
    /* ========== 测试5: 写入/读取一个字符串 ========== */
    char *write_msg = "这是一个EEPROM测试程序";
    char recv_msg[36] = {'\0'};
    uint8_t char_addr = 40;
    printf("%s|%s|%d EEPROM 写入一个字符串并读取 ......\r\n", __FILE__, __func__, __LINE__);
    BSP_EEPROM_WriteBuffer(char_addr, (uint8_t *)write_msg, strlen(write_msg));
    printf("%s|%s|%d 写入字符串 msg:[%s]\r\n",__FILE__, __func__, __LINE__, write_msg);
    BSP_EEPROM_ReadBuffer(char_addr, (uint8_t *)recv_msg, strlen(write_msg));
    printf("%s|%s|%d 读取字符串 msg:[%s]\r\n",__FILE__, __func__, __LINE__, recv_msg);
    if(!strncmp(write_msg, recv_msg, strlen(write_msg))){
        printf("%s|%s|%d 测试成功\r\n", __FILE__, __func__, __LINE__);
        for(int i = 0; i < 3; i++){ BSP_LED_On(LED_GREEN); BSP_Delay_ms(500); BSP_LED_Off(LED_GREEN); BSP_Delay_ms(500); }
    }else{
        printf("%s|%s|%d 测试失败\r\n", __FILE__, __func__, __LINE__);
        for(int i = 0; i < 3; i++){ BSP_LED_On(LED_RED); BSP_Delay_ms(500); BSP_LED_Off(LED_RED); BSP_Delay_ms(500); }
    }
    printf("%s|%s|%d EEPROM 测试完成\r\n",__FILE__, __func__, __LINE__);
}
