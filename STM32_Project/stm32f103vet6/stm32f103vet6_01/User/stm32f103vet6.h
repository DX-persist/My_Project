#ifndef STM32F103VET6_H_
#define STM32F103VET6_H_

#define PERIPH_BASE 	((volatile unsigned int)0x40000000)			//外设起始地址

#define APB1PERIPH_BASE PERIPH_BASE
#define APB2PERIPH_BASE ((volatile unsigned int)(PERIPH_BASE + 0x10000))
#define AHBPERIPH_BASE	((volatile unsigned int)(PERIPH_BASE + 0x20000))

#define RCC_PERIPH_BASE ((volatile unsigned int)(AHBPERIPH_BASE + 0x1000))
#define GPIOB_BASE		((volatile unsigned int)(APB2PERIPH_BASE + 0x0C00))

#define RCC_APB2ENR		(*((volatile unsigned int *)(RCC_PERIPH_BASE + 0x18)))
#define GPIOB_CRL		(*((volatile unsigned int *)(GPIOB_BASE + 0x00)))
#define GPIOB_CRH		(*((volatile unsigned int *)(GPIOB_BASE + 0x04)))
#define GPIOB_IDR		(*((volatile unsigned int *)(GPIOB_BASE + 0x08)))
#define GPIOB_ODR		(*((volatile unsigned int *)(GPIOB_BASE + 0x0C)))
#define GPIOB_BSRR		(*((volatile unsigned int *)(GPIOB_BASE + 0x10)))
#define GPIOB_BRR		(*((volatile unsigned int *)(GPIOB_BASE + 0x14)))
#define GPIOB_LCKR		(*((volatile unsigned int *)(GPIOB_BASE + 0x18)))

#endif
