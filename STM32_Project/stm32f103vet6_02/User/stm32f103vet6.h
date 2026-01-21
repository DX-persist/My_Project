#ifndef STM32F103VET6_H_
#define STM32F103VET6_H_

#define PERIPH_BASE 		((volatile unsigned int)0x40000000)						//定义外设的起始地址

#define APB1PERIPH_BASE		PERIPH_BASE												//定义APB1系统总线起始地址
#define APB2PERIPH_BASE		((volatile unsigned int)PERIPH_BASE + 0x10000)			//定义APB2系统总线起始地址
#define AHBPERIPH_BASE		((volatile unsigned int)PERIPH_BASE + 0x20000)			//定义AHB系统总线起始地址

#define RCCPERIPH_BASE		((volatile unsigned int)AHBPERIPH_BASE + 0x1000)		//定义外设RCC时钟的起始地址
#define GPIOB_BASE			((volatile unsigned int)APB2PERIPH_BASE + 0x0C00)		//定义外设GPIOB的起始地址
																					
#define RCC_APB2ENR			(*(volatile unsigned int *)(RCCPERIPH_BASE + 0x18))		//定义APB2外设时钟使能寄存器地址

#define GPIOB_CRL			(*(volatile unsigned int *)(GPIOB_BASE + 0x00))			//定义GPIOB端口配置低寄存器(配置工作模式和输出速度)	
#define GPIOB_CRH			(*(volatile unsigned int *)(GPIOB_BASE + 0x04))			//定义GPIOB端口配置高寄存器
#define GPIOB_IDR			(*(volatile unsigned int *)(GPIOB_BASE + 0x08))			//定义GPIOB端口输入数据寄存器
#define GPIOB_ODR			(*(volatile unsigned int *)(GPIOB_BASE + 0x0C))			//定义GPIOB端口输出数据寄存器
#define GPIOB_BSRR			(*(volatile unsigned int *)(GPIOB_BASE + 0x10))			//定义GPIOB端口位设置/清除寄存器
#define GPIOB_BRR			(*(volatile unsigned int *)(GPIOB_BASE + 0x14))			//定义GPIOB端口位清除寄存器
#define GPIOB_LCKR			(*(volatile unsigned int *)(GPIOB_BASE + 0x18))			//定义GPIOB端口配置锁定寄存器											

#endif
