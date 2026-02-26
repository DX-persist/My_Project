#ifndef BSP_RINGBUFFER_H
#define BSP_RINGBUFFER_H

#include <stdint.h>

/*
 * 使用模型：
 *  - head 由 ISR / DMA 更新
 *  - tail 由主循环更新
 *  - 单生产者 / 单消费者
 */
typedef struct{
	uint8_t *buffer;
	uint16_t size;
	volatile uint16_t head;
	volatile uint16_t tail;
}RingBuffer_t;

extern void RingBuffer_Init(RingBuffer_t *rb, uint8_t *buf, uint16_t size);
extern uint8_t RingBuffer_Push(RingBuffer_t *rb, uint8_t data);
extern uint8_t RingBuffer_Pop(RingBuffer_t *rb, uint8_t *data);
extern uint8_t RingBuffer_GetCount(RingBuffer_t *rb);
extern void RingBuffer_Clear(RingBuffer_t *rb);

#endif
