#include "stm32f103vet6.h"

//启用GPIOB端口的时钟并配置PB0端口的输出模式
void SetGPIO_Port(void)
{
	//启用GPIOB端口时钟
	RCC_APB2ENR &= ~(1 << 3);
	RCC_APB2ENR |= (1 << 3);

	//配置PB0为推挽输出，输出速度为10MHZ
	GPIOB_CRL &= ~(0xF << 0);		//清空PB0的四个位
	GPIOB_CRL |= (0x1 << 0);		

	//配置PB1为推挽输出，输出速度为10MHZ
	GPIOB_CRL &= ~(0xF << 4);		//清空PB1的四个位
	GPIOB_CRL |= (0x1 << 4);		
	
	//配置PB5为推挽输出，输出速度为10MHZ
	GPIOB_CRL &= ~(0xF << 20);		//清空PB5的四个位
	GPIOB_CRL |= (0x1 << 20);		
}

void delay(int ms)
{
	int i;
	for(i = 0; i < ms; i++);
}

//配置PB0为低电平
void SetPB0_Low(void)
{
	GPIOB_ODR &= ~(1 << 0);
}

//配置PB0为高电平
void SetPB0_High(void)
{
	GPIOB_ODR |= (1 << 0);
}

//配置PB1为低电平
void SetPB1_Low(void)
{
	GPIOB_BSRR |= (1 << 17);
}

void SetPB1_High(void)
{
	GPIOB_BSRR |= (1 << 1);
}

//配置PB5为低电平
void SetPB5_Low(void)
{
	GPIOB_BSRR |= (1 << 21);
}

//配置PB5为高电平
void SetPB5_High(void)
{
	GPIOB_BSRR |= (1 << 21);
}

int main(void)
{
	SetGPIO_Port();
	
	GPIOB_BSRR |= (1 << 1);
	GPIOB_BSRR |= (1 << 5);

	while(1){
		GPIOB_BRR |= (1 << 0);
		delay(3000000);
		GPIOB_BSRR |= (1 << 0);
		delay(3000000);
	}	
}
