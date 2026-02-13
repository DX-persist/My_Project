#include "bsp_rccclk.h"

void HSE_SetSysClock(uint32_t RCC_PLLMul)
{
	/* 由于上电后已经默认配置系统时钟为72MHz,所以将RCC时钟的配置恢复成默认状态 */
	RCC_DeInit();

	/* 使能外部高速时钟 */
	RCC_HSEConfig(RCC_HSE_ON);
	/* 判断外部高速时钟是否已经就绪 */
	ErrorStatus HSEStatus = 0;
	HSEStatus = RCC_WaitForHSEStartUp();
	/* 外部高速时钟就绪 */
	if(HSEStatus == SUCCESS){
		/* 启用预取缓冲区 */
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		/* 设置flash等待状态为2个周期 */
		FLASH_SetLatency(FLASH_Latency_2);

		/* 配置AHB总线的分频系数 */
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		/* 配置APB1总线的分频系数 最大36MHz */
		RCC_PCLK1Config(RCC_HCLK_Div2);
		/* 配置APB2总线的分频系数 */
		RCC_PCLK2Config(RCC_HCLK_Div1);

		/* 配置锁相环：锁相环时钟 = HSE * 9 = 72MHz */
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul);
		/* 启用锁相环 */
		RCC_PLLCmd(ENABLE);
		/* 判断PLL锁相环时钟是否就绪 */
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

		/* 配置系统时钟源：选择锁相环时钟源为系统时钟源 */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		/* 返回被作为系统的时钟源 */
		while(RCC_GetSYSCLKSource() != 0x08);
	}else{
		/* 外部高速时钟未就绪 */

	}
}

void HSI_SetSysClock(uint32_t RCC_PLLMul)
{
//	__IO uint32_t HSIStatus = 0, StartUpCounter = 0;
	/* 由于上电后已经默认配置系统时钟为72MHz,所以将RCC时钟的配置恢复成默认状态 */
	RCC_DeInit();

	/* 使能内部高速时钟 */
	RCC_HSICmd(ENABLE);
	/* 判断内部高速时钟是否已经就绪 */
#if 0
	do{
		HSIStatus = RCC_CR & RCC_CR_HSIRDY;
		StartUpCounter++;
	}while((HSIStatus == 0) && (StartUpConuter != 0x0500));
	
	if(HSIStatus != RESET)
		HSIStatus = (uint32_t)0x01;
	else
		HSIStatus = (uint32_t)0x00;
#endif
	/* 内部高速时钟就绪 */
	if(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == SET){
		/* 启用预取缓冲区 */
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		/* 设置flash等待状态为2个周期 */
		FLASH_SetLatency(FLASH_Latency_2);

		/* 配置AHB总线的分频系数 */
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		/* 配置APB1总线的分频系数 最大36MHz */
		RCC_PCLK1Config(RCC_HCLK_Div2);
		/* 配置APB2总线的分频系数 */
		RCC_PCLK2Config(RCC_HCLK_Div1);

		/* 配置锁相环时钟源：锁相环时钟 = HSI / 2 * 倍频因子 */
		RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul);
		/* 启用锁相环 */
		RCC_PLLCmd(ENABLE);
		/* 判断PLL锁相环时钟是否就绪 */
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

		/* 配置系统时钟源：选择锁相环时钟源为系统时钟源 */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		/* 返回被作为系统的时钟源 */
		while(RCC_GetSYSCLKSource() != 0x08);
	}else{
		/* 内部高速时钟未就绪 */

	}
}
void MCO_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &GPIO_InitStruct);

	RCC_MCOConfig(RCC_MCO_SYSCLK);
}
