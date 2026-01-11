#include "stm32f10x.h"                  // Device header
#include "delay.h"


int main(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能GPIOB组的时钟

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//将某组GPIO配置为推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    	//GPIO速度为50MHz

	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	while(1)
	{
		GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_RESET);
        GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET);
		delay_ms(1000);
		GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_RESET);
        GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_SET);
        delay_ms(1000);
	}
}
