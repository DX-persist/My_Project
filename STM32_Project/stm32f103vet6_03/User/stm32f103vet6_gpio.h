#ifndef STM32F103VET6_GPIO_H_
#define STM32F103VET6_GPIO_H_

#include "stm32f103vet6.h"

#define GPIO_Pin_0          ((uint16_t)0x0001)          /*0000 0000 0000 0001*/
#define GPIO_Pin_1          ((uint16_t)0x0002)          /*0000 0000 0000 0010*/
#define GPIO_Pin_2          ((uint16_t)0x0004)          /*0000 0000 0000 0100*/
#define GPIO_Pin_3          ((uint16_t)0x0008)          /*0000 0000 0000 1000*/
#define GPIO_Pin_4          ((uint16_t)0x0010)          /*0000 0000 0001 0000*/
#define GPIO_Pin_5          ((uint16_t)0x0020)          /*0000 0000 0010 0000*/
#define GPIO_Pin_6          ((uint16_t)0x0040)          /*0000 0000 0100 0000*/
#define GPIO_Pin_7          ((uint16_t)0x0080)          /*0000 0000 1000 0000*/
#define GPIO_Pin_8          ((uint16_t)0x0100)          /*0000 0001 0000 0000*/
#define GPIO_Pin_9          ((uint16_t)0x0200)          /*0000 0010 0000 0000*/
#define GPIO_Pin_10         ((uint16_t)0x0400)          /*0000 0100 0000 0000*/
#define GPIO_Pin_11         ((uint16_t)0x0800)          /*0000 1000 0000 0000*/
#define GPIO_Pin_12         ((uint16_t)0x1000)          /*0001 0000 0000 0000*/
#define GPIO_Pin_13         ((uint16_t)0x2000)          /*0010 0000 0000 0000*/
#define GPIO_Pin_14         ((uint16_t)0x4000)          /*0100 0000 0000 0000*/
#define GPIO_Pin_15         ((uint16_t)0x8000)          /*1000 0000 0000 0000*/
#define GPIO_Pin_All        ((uint16_t)0xFFFF)          /*1111 1111 1111 1111*/

typedef enum{
    GPIO_Speed_10MHZ = 1,
    GPIO_Speed_2MHZ,
    GPIO_Speed_50MHZ
}GPIOSpeed_TypeDef;

typedef enum
{ GPIO_Mode_AIN = 0x0,
  GPIO_Mode_IN_FLOATING = 0x04,
  GPIO_Mode_IPD = 0x28,
  GPIO_Mode_IPU = 0x48,
  GPIO_Mode_Out_OD = 0x14,
  GPIO_Mode_Out_PP = 0x10,
  GPIO_Mode_AF_OD = 0x1C,
  GPIO_Mode_AF_PP = 0x18
}GPIOMode_TypeDef;

typedef struct{
    uint16_t GPIO_Pin;
    GPIOSpeed_TypeDef GPIO_Speed;
    GPIOMode_TypeDef GPIO_Mode;
}GPIO_InitTypeDef;

extern void GPIO_SetBits(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x);
extern void GPIO_ResetBits(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x);
extern void GPIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* GPIO_InitStruct);

#endif