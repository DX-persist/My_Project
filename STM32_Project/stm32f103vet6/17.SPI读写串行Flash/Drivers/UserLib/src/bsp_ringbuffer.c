#include "bsp_ringbuffer.h"

/*
 * @brief  初始化环形缓冲区
 * @param  rb    RingBuffer 控制结构体指针
 * @param  buf   外部提供的数据缓冲区首地址
 * @param  size  缓冲区大小（至少为 2，预留 1 字节区分空/满）
 * @note
 *  - RingBuffer 仅负责索引管理，不负责内存分配
 *  - 缓冲区满的判定方式为：(head + 1) % size == tail
 *  - 该函数应在使用 Push/Pop 之前调用
 */
void RingBuffer_Init(RingBuffer_t *rb, uint8_t *buf, uint16_t size)
{
    /* 参数合法性检查 */
    if (!rb || !buf || size < 2) {
        return;
    }

    rb->buffer = buf;
    rb->size   = size;
    rb->head   = 0;
    rb->tail   = 0;
}

/*
 * @brief  判断环形缓冲区是否为空
 * @param  rb  RingBuffer 控制结构体指针
 * @retval 1   缓冲区为空
 * @retval 0   缓冲区非空
 * @note
 *  - 当 head == tail 时，缓冲区为空
 */
static uint8_t RingBuffer_IsEmpty(RingBuffer_t *rb)
{
    return (rb->head == rb->tail);
}

/*
 * @brief  判断环形缓冲区是否已满
 * @param  rb  RingBuffer 控制结构体指针
 * @retval 1   缓冲区已满
 * @retval 0   缓冲区未满
 * @note
 *  - 预留一个字节用于区分空与满
 *  - 当 (head + 1) % size == tail 时认为缓冲区已满
 */
static uint8_t RingBuffer_IsFull(RingBuffer_t *rb)
{
    return (((rb->head + 1) % rb->size) == rb->tail);
}

/*
 * @brief  向环形缓冲区写入一个字节
 * @param  rb    RingBuffer 控制结构体指针
 * @param  data  需要写入的数据
 * @retval 1     写入成功
 * @retval 0     写入失败（缓冲区已满）
 * @note
 *  - 常用于中断或 DMA 接收回调中
 *  - 若缓冲区已满，本实现选择丢弃新数据
 */
uint8_t RingBuffer_Push(RingBuffer_t *rb, uint8_t data)
{
    if (!RingBuffer_IsFull(rb)) {
        rb->buffer[rb->head] = data;
        rb->head = (rb->head + 1) % rb->size;
        return 1;
    } else {
        return 0;
    }
}

/*
 * @brief  从环形缓冲区读取一个字节
 * @param  rb    RingBuffer 控制结构体指针
 * @param  data  用于存放读取数据的指针
 * @retval 1     读取成功
 * @retval 0     读取失败（缓冲区为空）
 * @note
 *  - 常用于主循环或任务上下文中读取数据
 */
uint8_t RingBuffer_Pop(RingBuffer_t *rb, uint8_t *data)
{
    if (!RingBuffer_IsEmpty(rb)) {
        *data = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) % rb->size;
        return 1;
    } else {
        return 0;
    }
}

/*
 * @brief  获取当前缓冲区中已存放的数据字节数
 * @param  rb  RingBuffer 控制结构体指针
 * @retval     当前有效数据长度
 * @note
 *  - 适用于单生产者 / 单消费者模型
 *  - head 由生产者更新，tail 由消费者更新
 */
uint8_t RingBuffer_GetCount(RingBuffer_t *rb)
{
    return (rb->head - rb->tail + rb->size) % rb->size;
}

void RingBuffer_Clear(RingBuffer_t *rb)
{
	rb->head = rb->tail;
}
