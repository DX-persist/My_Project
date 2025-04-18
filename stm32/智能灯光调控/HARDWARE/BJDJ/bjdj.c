#include "bjdj.h"
#include "delay.h"

#define uint unsigned int
	
//步进电机正反转数组1
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

void Motorcw(uint speed)     //正转
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



void Motorccw(int speed)     //反转
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


void MotorStop(void)   //停止
{ 
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_RESET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_RESET);
}



//由于   *一个脉冲*   *输出轴*  转0.08789度（电机实转0.08789*64=5.625度），即步进角为5.625度。
//则转完A-B-C-D为  *8个脉冲*  ，即0.08789*8=0.70312度。若称A-B-C-D为一个周期，则j为需要的转完angle角度所需的周期数。
//所以要转过180度，j等于256   256*0.70312=179.99872
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

