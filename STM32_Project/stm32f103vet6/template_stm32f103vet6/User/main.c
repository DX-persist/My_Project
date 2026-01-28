#include "stm32f10x.h"                  // Device header

#define RCC_APB2ENR (*(volatile unsigned int *)0x40021018)
#define GPIOB_CRL (*(volatile unsigned int *)0x40010C00)
#define GPIOB_ODR (*(volatile unsigned int *)0x40010C0C)

void delay(void)
{
	int i;

	for(i = 0 ; i < 3000000; i++);
}

void LED_GREEN_ON(void)
{
	//配置PB0为低电平,点亮LED_GREEN
	GPIOB_ODR &= ~(1 << 0);
}

void LED_GREEN_OFF(void)
{
	//配置PB0为高电平,熄灭LED_GREEN
	GPIOB_ODR |= (1 << 0);
}	

void LED_BLUE_ON(void)
{
	//配置PB1为低电平，点亮LED_BLUE
	GPIOB_ODR &= ~(1 << 1);
}

void LED_BLUE_OFF(void)
{
	//配置PB1为高电平，熄灭LED_BLUE
	GPIOB_ODR |= (1 << 1);
}	

void LED_RED_ON(void)
{
	//配置PB5为低电平，点亮LED_RED
	GPIOB_ODR &= ~(1 << 5);
}

void LED_RED_OFF(void)
{
	//配置PB5为高电平，熄灭LED_RED
	GPIOB_ODR |= (1 << 5);
}	
int main(void)
{
	//使能GPIOB的时钟,GPIOB挂在到APB2总线上
	RCC_APB2ENR |= (1 << 3);

	//配置PB0为推挽输出，输出速度为10MHZ
	GPIOB_CRL &= ~(0xF << 0);
	GPIOB_CRL |= (0x1 << 0);

	//配置PB1为推挽输出，输出速度为10MHZ
	GPIOB_CRL &= ~(0xF << 4);
	GPIOB_CRL |= (0x1 << 4);

	//配置PB5为推挽输出，输出速度为10MHZ
	GPIOB_CRL &= ~(0xF << 20);
	GPIOB_CRL |= (0x1 << 20);

	LED_RED_OFF();
	LED_GREEN_OFF();
	LED_BLUE_OFF();

	while(1){
		LED_RED_ON();
		delay();
		LED_RED_OFF();
		delay();
		LED_GREEN_ON();
		delay();
		LED_GREEN_OFF();
		delay();
		LED_BLUE_ON();
		delay();
		LED_BLUE_OFF();
		delay();
	}

	return 0;
}
