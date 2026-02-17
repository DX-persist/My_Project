/**
 * @file    bsp_eeprom.c
 * @brief   EEPROM(AT24C02)驱动实现文件
 * @note    基于软件模拟I2C接口实现EEPROM的字节写入/读取、页写入、顺序读取等功能
 *          支持写周期轮询检测,确保数据可靠写入
 */
#include "bsp_eeprom.h"

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
 * @brief  向EEPROM写入一页数据 (支持跨页自动分块)
 * @param  reg_addr: EEPROM内部起始地址
 * @param  data:     要写入的数据缓冲区指针
 * @param  size:     要写入的数据大小
 * 
 * @note   核心逻辑：
 *         I2C EEPROM有页大小限制(AT24C02为8字节)，不能一次性跨页写入，
 *         否则地址会回卷覆盖页首数据。
 *         本函数自动计算当前页剩余空间，将大数据块拆分成多次写入。
 */
uint8_t BSP_EEPROM_WritePage(uint8_t reg_addr, uint8_t *data, uint16_t size)
{
    uint8_t retval = 0; // 默认为成功

    while(size > 0){
        /* 先求出当前页还可以写入多少字节 */
        /* 例如: PageSize=8, Addr=2 -> Remain=6 */
        uint8_t remain = BSP_EEPROM_WRITE_PAGE_MAX - (reg_addr % BSP_EEPROM_WRITE_PAGE_MAX);

        /* 判断要写入字节数是否小于当前页剩余空间:
         * 若小于, 直接写入size长度
         * 若大于, 先把当前页剩下的字节(remain)写完
         */
        uint8_t len = (size < remain) ? (uint8_t)size : remain;

        /* 调用底层I2C Buffer写入 */
        retval = BSP_SI2C_WriteBuffer(BSP_EEPROM_I2C_ID, EEPROM_DEV_ADDR, reg_addr, data, len);
        if(retval != 0) return retval; // I2C错误

        /* 等待内部写入完成 */
        if(BSP_EEPROM_Polling() == 1){
            printf("%s|%s|%d 写入超时\r\n",__FILE__, __func__,__LINE__);
            return 1;
        }

        /* 更新地址及指针，准备写下一块 */
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
 * @brief EEPROM功能测试函数
 * @note 测试EEPROM的读写功能是否正常
 */
void BSP_EEPROM_Test(void)
{
    uint8_t write_data = 15;                        // 单字节测试数据
    uint8_t read_data = 0;                          // 单字节读取缓冲
    uint8_t handler_addr = 0;                       // 页写入起始地址
    uint8_t write_buffer[8] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80}; // 页写入测试数据
    uint8_t recv_buf[8] = {0};                      // 页读取缓冲

    printf("============EEPROM 测试开始=========\r\n");
    BSP_EEPROM_Init();

    /* ========== 测试1: EEPROM 写入一个字节 + 读取一个字节 ========== */
    printf("%s|%s|%d EEPROM 写入一个字节 + 读取一个字节测试......\r\n", __FILE__, __func__, __LINE__);
    BSP_EEPROM_WriteByte(15, write_data);           // 写入数据15到地址15
    printf("%s|%s|%d 写入数据：%d\r\n", __FILE__, __func__, __LINE__, write_data);
    BSP_EEPROM_ReadRandom(15, &read_data);          // 从地址15读取数据
    printf("%s|%s|%d 读取到的数据：%d\r\n", __FILE__, __func__, __LINE__, read_data);
    
    /* 比较写入和读取的数据 */
    if(write_data == read_data){
        printf("%s|%s|%d 写入一个字节 + 读取一个字节测试成功\r\n", __FILE__, __func__, __LINE__);
        // 成功: 绿灯闪烁3次
        for(int i = 0; i < 3; i++){
            BSP_LED_On(LED_GREEN);
            BSP_Delay_ms(500);
            BSP_LED_Off(LED_GREEN);
            BSP_Delay_ms(500);
        }
    }else{
        printf("%s|%s|%d 写入一个字节 + 读取一个字节测试失败\r\n", __FILE__, __func__, __LINE__);
        // 失败: 红灯闪烁3次
        for(int i = 0; i < 3; i++){
            BSP_LED_On(LED_RED);
            BSP_Delay_ms(500);
            BSP_LED_Off(LED_RED);
            BSP_Delay_ms(500);
        }
    }
    
    /* ========== 测试2: EEPROM 页写入 + 顺序读取 ========== */
    printf("%s|%s|%d EEPROM 页写入 + 顺序读取测试 ......\r\n", __FILE__, __func__, __LINE__);
    BSP_EEPROM_WritePage(handler_addr, write_buffer, 8);    // 页写入8字节到地址0
    printf("%s|%s|%d 写入数据: \r\n", __FILE__, __func__, __LINE__);
    for(int i = 0; i < 8; i++){
        printf("%s|%s|%d buffer[%d] = %d\r\n",__FILE__, __func__, __LINE__,i, write_buffer[i]);
    }

    BSP_EEPROM_ReadSequential(handler_addr, recv_buf, 8);   // 顺序读取8字节从地址0
    printf("%s|%s|%d 读取数据: \r\n", __FILE__, __func__, __LINE__);
    for(int i = 0; i < 8; i++){
        printf("%s|%s|%d buffer[%d] = %d\r\n",__FILE__, __func__, __LINE__,i, recv_buf[i]);
    }

    /* 比较写入和读取的数据 */
    uint8_t flag = 1; // 默认为成功
    for(int i = 0; i < 8; i++){
        if(write_buffer[i] != recv_buf[i]){
            flag = 0;       // 数据不匹配
            break;
        }
    }

    /* 根据比较结果显示LED */
    if(flag == 0){
        printf("%s|%s|%d EEPROM 页写入 + 顺序读取测试失败\r\n", __FILE__, __func__, __LINE__);
        // 失败: 红灯闪烁3次
        for(int i = 0; i < 3; i++){
            BSP_LED_On(LED_RED);
            BSP_Delay_ms(500);
            BSP_LED_Off(LED_RED);
            BSP_Delay_ms(500);
            return;
        }
    }else{
        printf("%s|%s|%d EEPROM 页写入 + 顺序读取测试成功\r\n", __FILE__, __func__, __LINE__);
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

