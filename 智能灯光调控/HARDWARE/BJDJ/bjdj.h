#ifndef __bjdj_H
#define __bjdj_H	 
#include "sys.h"

//ULN2003����

#define uint unsigned int

void Delay_xms(uint x);//��ʱ����
void Moto_Init(void);  //���������ʼ��
void Motorcw(uint speed);    //���������ת����
void Motorccw(int speed);   //���������ת����
void Motorcw_angle(int angle,int speed);  //���������ת�ǶȺ���
void Motorccw_angle(int angle,int speed); //���������ת�ǶȺ���
void MotorStop(void);  //�������ֹͣ����

#endif
