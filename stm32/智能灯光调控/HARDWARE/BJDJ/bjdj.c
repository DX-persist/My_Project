#include "bjdj.h"
#include "delay.h"

#define uint unsigned int
	
//�����������ת����1
//uint16_t phasecw[4] ={0x0200,0x0100,0x0080,0x0040};// D-C-B-A    9 8 7 6 5 4 3 2 1 0
//uint16_t phaseccw[4]={0x0040,0x0080,0x0100,0x0200};// A-B-C-D.

void Delay_xms(uint x)
{
 uint i,j;
 for(i=0;i<x;i++)
  for(j=0;j<112;j++);
}

void Moto_Init(void)
{

 GPIO_InitTypeDef GPIO_InitStructure;
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15 ;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 GPIO_Init(GPIOB,&GPIO_InitStructure);
 GPIO_ResetBits(GPIOB,GPIO_Pin_12 | GPIO_Pin_13 |GPIO_Pin_14 |GPIO_Pin_15 );

}

void Motorcw(uint speed)     //��ת
{  
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_SET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_RESET);
	delay_ms(speed);  
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_SET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_RESET);
	delay_ms(speed);  
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_SET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_RESET);
	delay_ms(speed);  
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_SET);
	delay_ms(speed);  
}



void Motorccw(int speed)     //��ת
{  
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_SET);
	delay_ms(speed);  
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_SET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_RESET);
	delay_ms(speed);  
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_SET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_RESET);
	delay_ms(speed);  
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_SET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_RESET);
	delay_ms(speed);  
}


void MotorStop(void)   //ֹͣ
{ 
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_RESET);
}



//����   *һ������*   *�����*  ת0.08789�ȣ����ʵת0.08789*64=5.625�ȣ�����������Ϊ5.625�ȡ�
//��ת��A-B-C-DΪ  *8������*  ����0.08789*8=0.70312�ȡ�����A-B-C-DΪһ�����ڣ���jΪ��Ҫ��ת��angle�Ƕ��������������
//����Ҫת��180�ȣ�j����256   256*0.70312=179.99872
void Motorcw_angle(int angle,int speed)
{
	int i,j;
	j=angle;
	for(i=0;i<j;i++)
	{
		Motorcw(5);
	}
}



void Motorccw_angle(int angle,int speed)
{
	int i,j;
	j=angle;
	for(i=0;i<j;i++)
	{
		Motorccw(5);
	}
		MotorStop(); 
}

