#ifndef __DHT22__H
#define __DHT22__H

#include "stm32f10x.h"
#include "delay.h"

#define DHT22_RCC_CLOCK RCC_APB2Periph_GPIOA
#define DHT22_GPIO_PORT GPIOA
#define DHT22_GPIO_PIN GPIO_Pin_0
#define DHT22_SetPin(x) GPIO_WriteBit(DHT22_GPIO_PORT, DHT22_GPIO_PIN, (BitAction)(x))
#define DHT22_ReadPin() GPIO_ReadInputDataBit(DHT22_GPIO_PORT, DHT22_GPIO_PIN)
#define DHT_DQ_OUT_L() GPIO_WriteBit(DHT22_GPIO_PORT, DHT22_GPIO_PIN, Bit_RESET)
#define DHT_DQ_OUT_H() GPIO_WriteBit(DHT22_GPIO_PORT, DHT22_GPIO_PIN, Bit_SET)
#define DHT_DQ_IN()    GPIO_ReadInputDataBit(DHT22_GPIO_PORT, DHT22_GPIO_PIN)


#define ACK_SUCCESS   0
#define ACK_OVER_TIME 1
#define ACK_ERROR     2

typedef struct
{
    uint8_t  humi_int;
    uint8_t  humi_dec;
    uint8_t  temp_int;
    uint8_t  temp_dec;
    uint8_t sum;
    uint8_t check_num;
    float humidity;
    float temperature;
}DHT22_Data_TypeDef;

extern uint8_t Read_DHT22_Data(DHT22_Data_TypeDef *Data);

#endif
