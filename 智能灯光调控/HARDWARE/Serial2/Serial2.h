#ifndef __Serial2_H
#define __Serial2_H
#include "stdio.h"


extern uint8_t Serial2_RxFlag;
extern char  Serial2_RxPacket[];
void Serial2_Init(void);
void Serial2_SendArray(uint8_t *Array,uint16_t Length);
void Serial2_SendByte(uint8_t Byte);
void Serial2_SendString(char *String);
uint32_t Serial2_Pow(uint32_t x,uint32_t y);
int fputc2(int ch,FILE *f);
void Serial2_SendNumber(uint32_t Number,uint8_t Length);
void Serial2_Printf(char *format,...);


#endif

