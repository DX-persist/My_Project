#include "bsp_delay.h"

volatile uint32_t g_ms_tick = 0;

#if 0
static __INLINE uint32_t SysTick_Config(uint32_t ticks)
{ 
  if (ticks > SysTick_LOAD_RELOAD_Msk)  return (1);            /* Reload value impossible */
                                                               
  SysTick->LOAD  = (ticks & SysTick_LOAD_RELOAD_Msk) - 1;      /* set reload register */
  NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  /* set Priority for Cortex-M0 System Interrupts */
  SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | 
                   SysTick_CTRL_TICKINT_Msk   | 
                   SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
  return (0);                                                  /* Function successful */
}
#endif

/* 所以每1ms就会进入中断一次，然后对里边的g_ms_tick++ */
void SysTick_Handler(void)
{
	g_ms_tick++;
}

/* SysTick_Config这个函数内部配置了SysTick寄存器的
 * 重装载值、中断优先级、初始值，并且配置使用AHB的时钟(即默认72MHz)
 * 并且启用了中断(当从重装载值减到0后就会触发中断)
 * 最后使能了定时器
 * 这里传入的参数就是 72000000 / 1000 = 72000, 所以这里的重装载值就是72000
 * 重装载值从72000减到0就会产生中断,所以这里产生中断的时间是72000 / 720000000 = 0.001s = 1ms 
 */
void BSP_TimeBase_Init(void)
{
	SysTick_Config(SystemCoreClock / 1000);
}

uint32_t BSP_GetTick(void)
{
	return g_ms_tick;
}

void BSP_Delay_us(uint32_t us)
{
    // 下面是简单忙等方法，仅用于短延时 (<1ms)
    // 不开中断，不破坏系统时基
    uint32_t count = us * (SystemCoreClock / 1000000); // 系统时钟周期换算
    while(count--) __NOP();
}

void BSP_Delay_ms(uint32_t ms)
{
	/* 现在时间减去开始时间小于延时时间就一直卡在这里 */
	uint32_t start = BSP_GetTick();
	while((BSP_GetTick() - start) < ms);
}

void BSP_Delay_s(uint32_t s)
{
	for(; s > 0; s--){
		BSP_Delay_ms(1000);
	}
#if 0
	/*每秒都进来调用一次1000ms的延时*/
	while(s--){
		BSP_Delay_ms(1000);
	}
#endif
}
