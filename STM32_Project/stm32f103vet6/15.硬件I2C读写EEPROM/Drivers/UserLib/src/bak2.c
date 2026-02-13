#include "bsp_eeprom.h"

void BSP_EEPROM_Init(void)
{
    /* 由于板载 EEPROM 接到了I2C1，所以初始化I2C1 */
    BSP_HI2C_Init(BSP_EEPROM_I2C_ID);
}

/**
 * @brief 强制清除I2C总线BUSY状态
 * @param I2Cx: I2C外设指针
 * @retval None
 */
static void BSP_I2C_ForceClearBusy(I2C_TypeDef* I2Cx)
{
    if(!(I2Cx->SR2 & I2C_SR2_BUSY))
    {
        return; // 总线空闲，无需处理
    }
    
    printf("检测到总线忙(BUSY=1)，开始清除...\r\n");
    
    /* 方法1：发送STOP信号 */
    I2Cx->CR1 |= I2C_CR1_STOP;
    
    /* 等待BUSY清除 */
    uint32_t timeout = 100000;
    while((I2Cx->SR2 & I2C_SR2_BUSY) && (timeout--));
    
    if(I2Cx->SR2 & I2C_SR2_BUSY)
    {
        printf("STOP无效，执行软件复位...\r\n");
        
        /* 方法2：软件复位I2C外设 */
        I2Cx->CR1 |= I2C_CR1_SWRST;
        for(volatile int i = 0; i < 1000; i++);
        I2Cx->CR1 &= ~I2C_CR1_SWRST;
        
        /* 重新使能I2C */
        I2Cx->CR1 |= I2C_CR1_PE;
        
        printf("I2C已复位，SR2=0x%04X\r\n", I2Cx->SR2);
    }
    else
    {
        printf("总线已释放，SR2=0x%04X\r\n", I2Cx->SR2);
    }
}

/**
 * @brief 等待EEPROM完成内部写周期（应答轮询）
 * @retval None
 */
static void BSP_EEPROM_Polling(void)
{
    uint32_t timeout;
    uint8_t retry_count = 0;
    
    /* 先清除可能残留的BUSY状态 */
    BSP_I2C_ForceClearBusy(I2C1);
    
    while(1)
    {
        /* 1. 等待总线空闲 */
        timeout = 100000;
        while(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_BUSY) == SET)
        {
            if((timeout--) == 0)
            {
                if(++retry_count > 3)
                {
                    printf("ERROR: 总线持续忙，放弃等待\r\n");
                    return;
                }
                BSP_I2C_ForceClearBusy(I2C1);
                break;
            }
        }
        
        /* 2. 发送起始信号 */
        BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
        
        /* 3. 等待SB标志位 */
        timeout = 100000;
        while(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_SB) == RESET)
        {
            if((timeout--) == 0)
            {
                printf("ERROR: SB标志位超时\r\n");
                BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
                return;
            }
        }
        
        /* 4. 发送器件地址 + 写 */
        BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
        
        /* 5. 等待 ADDR 或 AF 标志位 */
        timeout = 100000;
        while(1)
        {
            if((timeout--) == 0)
            {
                printf("ERROR: 等待ADDR/AF超时\r\n");
                BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
                return;
            }
            
            /* ADDR=1: EEPROM已应答，写入完成 */
            if(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_ADDR) == SET)
            {
                /* 清除ADDR标志位 */
                BSP_HI2C_ClearFlag_Addr(BSP_EEPROM_I2C_ID);
                
                /* 发送停止信号 */
                BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
                
                /* 等待STOP完成 */
                timeout = 100000;
                while((I2C1->SR2 & I2C_SR2_BUSY) && (timeout--));
                
                return; // 写入完成
            }
            
            /* AF=1: EEPROM未应答，仍在写周期中 */
            if(BSP_HI2C_GetFlagStatus(BSP_EEPROM_I2C_ID, I2C_FLAG_AF) == SET)
            {
                /* 清除AF标志位（必须清除，否则无法发送STOP） */
                BSP_HI2C_ClearFlag(BSP_EEPROM_I2C_ID, I2C_FLAG_AF);
                
                /* 发送停止信号释放总线 */
                BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
                
                /* 等待STOP完成 */
                timeout = 100000;
                while((I2C1->SR2 & I2C_SR2_BUSY) && (timeout--));
                
                /* 短暂延时后重试 */
                for(volatile uint32_t i = 0; i < 5000; i++);
                
                break; // 跳出内循环，继续外循环轮询
            }
        }
    }
}

/**
 * @brief 向EEPROM写入单个字节
 * @param write_addr: 写入地址（0-255）
 * @param data: 要写入的数据
 * @param timeout_ms: 超时时间（毫秒）
 * @retval None
 */
void BSP_EEPROM_WriteByte(uint8_t write_addr, uint8_t data, uint32_t timeout_ms)
{
    /* 发送起始信号 */
    BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
    
    /* 等待事件5：主模式选择完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, timeout_ms);
    
    /* 发送从机地址 + 写方向 */
    BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
    
    /* 等待事件6：主发送模式选择完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, timeout_ms);
    
    /* 发送存储地址（字地址） */
    BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, write_addr);
    
    /* 等待事件8_2：字节发送完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, timeout_ms);
    
    /* 发送数据 */
    BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, data);
    
    /* 等待事件8_2：字节发送完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, timeout_ms);
    
    /* 发送停止信号 */
    BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
    
    /* 等待STOP完成，确保总线释放 */
    uint32_t timeout = timeout_ms * 0x1000;
    while((I2C1->SR2 & I2C_SR2_BUSY) && (timeout--));
    
    /* 等待EEPROM内部写周期完成（应答轮询） */
    BSP_EEPROM_Polling();
}

/**
 * @brief 向EEPROM页写入多个字节
 * @param write_addr: 写入起始地址
 * @param buffer: 数据缓冲区
 * @param size: 数据大小（不超过页大小）
 * @param timeout_ms: 超时时间（毫秒）
 * @retval None
 */
void BSP_EEPROM_WritePage(uint8_t write_addr, uint8_t *buffer, uint8_t size, uint32_t timeout_ms)
{
    if(size >= BSP_EEPROM_WRITE_PAGE_MAX) return;
    
    /* 发送起始信号 */
    BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
    
    /* 等待事件5：主模式选择完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, timeout_ms);
    
    /* 发送从机地址 + 写方向 */
    BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
    
    /* 等待事件6：主发送模式选择完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, timeout_ms);
    
    /* 发送字地址 */
    BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, write_addr);
    
    /* 等待事件8_2：字节发送完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, timeout_ms);
    
    /* 连续发送数据 */
    for(int i = 0; i < size; i++)
    {
        BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, buffer[i]);
        BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, timeout_ms);
    }
    
    /* 发送停止信号 */
    BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
    
    /* 等待STOP完成，确保总线释放 */
    uint32_t timeout = timeout_ms * 0x1000;
    while((I2C1->SR2 & I2C_SR2_BUSY) && (timeout--));
    
    /* 等待EEPROM内部写周期完成 */
    BSP_EEPROM_Polling();
}

/**
 * @brief 从EEPROM随机读取单个字节
 * @param read_addr: 读取地址
 * @param data: 接收数据的指针
 * @param timeout_ms: 超时时间（毫秒）
 * @retval None
 */
void BSP_EEPROM_ReadRandom(uint8_t read_addr, uint8_t *data, uint32_t timeout_ms)
{
    /* === 第一阶段：发送读地址（虚拟写） === */
    
    /* 发送起始信号 */
    BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
    
    /* 等待事件5：主模式选择完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, timeout_ms);
    
    /* 发送从机地址 + 写方向 */
    BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
    
    /* 等待事件6：主发送模式选择完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, timeout_ms);
    
    /* 发送读地址 */
    BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, read_addr);
    
    /* 等待事件8_2：字节发送完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, timeout_ms);
    
    /* === 第二阶段：读取数据 === */
    
    /* 重新发送起始信号（重复起始） */
    BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
    
    /* 等待事件5：主模式选择完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, timeout_ms);
    
    /* 发送从机地址 + 读方向 */
    I2C_Send7bitAddress(I2C1, BSP_EEPROM_READ_ADDR, I2C_Direction_Receiver);
    
    /* 等待ADDR标志位（不能用WaitEvent，因为它会自动清除ADDR） */
    uint32_t timeout = timeout_ms * 0x1000;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR) == RESET)
    {
        if((timeout--) == 0) return;
    }
    
    /* === 关键步骤：单字节读取的特殊处理 === */
    
    /* 1. 禁用ACK（告诉从机只读一个字节） */
    BSP_HI2C_AcknowledgeConfig(BSP_EEPROM_I2C_ID, DISABLE);
    
    /* 2. 清除ADDR标志位（读SR1后读SR2） */
    BSP_HI2C_ClearFlag_Addr(BSP_EEPROM_I2C_ID);
    
    /* 3. 发送STOP信号 */
    BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
    
    /* 4. 等待事件7：接收到数据（RXNE=1） */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_RECEIVED, timeout_ms);
    
    /* 5. 读取数据 */
    *data = BSP_HI2C_ReceiveData(BSP_EEPROM_I2C_ID);
    
    /* 6. 重新使能ACK（为下次通信做准备） */
    BSP_HI2C_AcknowledgeConfig(BSP_EEPROM_I2C_ID, ENABLE);
    
    /* 等待STOP完成 */
    timeout = timeout_ms * 0x1000;
    while((I2C1->SR2 & I2C_SR2_BUSY) && (timeout--));
}

/**
 * @brief 从EEPROM顺序读取多个字节
 * @param read_addr: 读取起始地址
 * @param buffer: 接收数据的缓冲区
 * @param size: 要读取的字节数
 * @param timeout_ms: 超时时间（毫秒）
 * @retval None
 */
void BSP_EEPROM_ReadSequential(uint8_t read_addr, uint8_t *buffer, uint8_t size, uint32_t timeout_ms)
{
    /* === 第一阶段：发送读地址（虚拟写） === */
    
    /* 发送起始信号 */
    BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
    
    /* 等待事件5：主模式选择完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, timeout_ms);
    
    /* 发送从机地址 + 写方向 */
    BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_WRITE_ADDR, BSP_I2C_Dir_Transmitt);
    
    /* 等待事件6：主发送模式选择完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, timeout_ms);
    
    /* 发送读地址 */
    BSP_HI2C_SendData(BSP_EEPROM_I2C_ID, read_addr);
    
    /* 等待事件8_2：字节发送完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_TRANSMITTED, timeout_ms);
    
    /* === 第二阶段：读取多个字节 === */
    
    /* 重新发送起始信号 */
    BSP_HI2C_Start(BSP_EEPROM_I2C_ID);
    
    /* 等待事件5：主模式选择完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_MODE_SELECT, timeout_ms);
    
    /* 发送从机地址 + 读方向 */
    BSP_HI2C_Send7bitAddress(BSP_EEPROM_I2C_ID, BSP_EEPROM_READ_ADDR, BSP_I2C_Dir_Receive);
    
    /* 等待事件6：主接收模式选择完成 */
    BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, timeout_ms);
    
    /* 连续接收数据 */
    for(int i = 0; i < size; i++)
    {
        /* 如果是最后一个字节 */
        if(i == size - 1)
        {
            /* 禁用ACK，告诉从机这是最后一个字节 */
            BSP_HI2C_AcknowledgeConfig(BSP_EEPROM_I2C_ID, DISABLE);
        }
        
        /* 等待事件7：接收到数据 */
        BSP_HI2C_WaitEvent(BSP_EEPROM_I2C_ID, I2C_EVENT_MASTER_BYTE_RECEIVED, timeout_ms);
        
        /* 读取数据 */
        buffer[i] = BSP_HI2C_ReceiveData(BSP_EEPROM_I2C_ID);
    }
    
    /* 发送停止信号 */
    BSP_HI2C_Stop(BSP_EEPROM_I2C_ID);
    
    /* 重新使能ACK（为下次通信做准备） */
    BSP_HI2C_AcknowledgeConfig(BSP_EEPROM_I2C_ID, ENABLE);
    
    /* 等待STOP完成 */
    uint32_t timeout = timeout_ms * 0x1000;
    while((I2C1->SR2 & I2C_SR2_BUSY) && (timeout--));
}

/**
 * @brief 跨页写入多个字节（自动处理页边界）
 * @param write_addr: 写入起始地址
 * @param buffer: 数据缓冲区
 * @param size: 数据大小
 * @param timeout_ms: 超时时间（毫秒）
 * @retval None
 */
void BSP_EEPROM_WriteBuffer(uint16_t write_addr, uint8_t *buffer, uint16_t size, uint32_t timeout_ms)
{
    uint16_t offset = 0;
    
    while(size > 0)
    {
        /* 计算当前页剩余空间 */
        uint8_t page_offset = write_addr % BSP_EEPROM_WRITE_PAGE_MAX;
        uint8_t page_remain = BSP_EEPROM_WRITE_PAGE_MAX - page_offset;
        
        /* 确定本次写入的字节数 */
        uint8_t write_size = (size <= page_remain) ? size : page_remain;
        
        /* 页写入 */
        BSP_EEPROM_WritePage(write_addr, &buffer[offset], write_size, timeout_ms);
        
        /* 更新参数 */
        write_addr += write_size;
        offset += write_size;
        size -= write_size;
    }
}

/**
 * @brief 读取多个字节（无长度限制）
 * @param read_addr: 读取起始地址
 * @param buffer: 接收数据的缓冲区
 * @param size: 要读取的字节数
 * @param timeout_ms: 超时时间（毫秒）
 * @retval None
 */
void BSP_EEPROM_ReadBuffer(uint16_t read_addr, uint8_t *buffer, uint16_t size, uint32_t timeout_ms)
{
    /* EEPROM顺序读取没有页的限制，可以一次读取任意长度 */
    BSP_EEPROM_ReadSequential(read_addr, buffer, size, timeout_ms);
}