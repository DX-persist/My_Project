#include "stm32f103vet6.h"
#include "stm32f103vet6_gpio.h"

//启用GPIOB端口的时钟并配置PB0端口的输出模式
void SetGPIO_Port(void)
{
	//启用GPIOB端口时钟
	RCC->APB2ENR |= (1 << 3);

	//配置PB0为推挽输出，输出速度为10MHZ
	GPIOB->CRL &= ~(0xF << 0);		//清空PB0的四个位
	GPIOB->CRL |= (0x1 << 0);		

	//配置PB1为推挽输出，输出速度为10MHZ
	GPIOB->CRL &= ~(0xF << 4);		//清空PB1的四个位
	GPIOB->CRL |= (0x1 << 4);		
	
	//配置PB5为推挽输出，输出速度为10MHZ
	GPIOB->CRL &= ~(0xF << 20);		//清空PB5的四个位
	GPIOB->CRL |= (0x1 << 20);		
}

void delay(int ms)
{
	int i;
	for(i = 0; i < ms; i++);
}

//配置PB0为低电平
void SetPB0_Low(void)
{
	GPIOB->ODR &= ~(1 << 0);
}

//配置PB0为高电平
void SetPB0_High(void)
{
	GPIOB->ODR |= (1 << 0);
}

//配置PB1为低电平
void SetPB1_Low(void)
{
	GPIOB->BSRR |= (1 << 17);
}

void SetPB1_High(void)
{
	GPIOB->BSRR |= (1 << 1);
}

//配置PB5为低电平
void SetPB5_Low(void)
{
	GPIOB->BSRR |= (1 << 21);
}

//配置PB5为高电平
void SetPB5_High(void)
{
	GPIOB->BSRR |= (1 << 21);
}

int main(void)
{
	//SetGPIO_Port();
	RCC->APB2ENR |= (1 << 3);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHZ;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//GPIOB->BSRR |= (1 << 5);
	//GPIOB->BSRR |= (1 << 1);

	GPIO_SetBits(GPIOB, GPIO_Pin_1);
	GPIO_SetBits(GPIOB, GPIO_Pin_0);
	GPIO_SetBits(GPIOB, GPIO_Pin_5);

	while(1){
		GPIO_ResetBits(GPIOB, GPIO_Pin_0);
		GPIO_SetBits(GPIOB, GPIO_Pin_1);
		GPIO_SetBits(GPIOB, GPIO_Pin_5);
		delay(3000000);
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
		GPIO_SetBits(GPIOB, GPIO_Pin_0);
		GPIO_SetBits(GPIOB, GPIO_Pin_5);
		delay(3000000);
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);
		GPIO_SetBits(GPIOB, GPIO_Pin_1);
		GPIO_SetBits(GPIOB, GPIO_Pin_0);
		delay(3000000);
	}	
}
