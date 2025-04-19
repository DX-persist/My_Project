#ifndef __IIC_H
#define __IIC_H

void IIC_START();
void IIC_STOP();
char IIC_ACK();
void IIC_SendByte(char dataSend);

#endif