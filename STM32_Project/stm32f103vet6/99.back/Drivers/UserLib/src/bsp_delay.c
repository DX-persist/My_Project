#include "bsp_delay.h"

void BSP_Delay_us(uint32_t us)
{
    /*
     * 配置 SysTick 使用 AHB 作为时钟源
     * 不启用中断
     * 使能 SysTick 定时器
    */
    SysTick->CTRL = SYSTICK_CLKSOURCE | SYSTICK_ENABLE;;
    /* 配置延时时间为 n 微秒 */
    SysTick->LOAD = 72 * us;
    /* 设定 SysTick 初始值为0 */
    SysTick->VAL = 0x00;

    /* 检测 CTRL 寄存器中的 COUNTFLAG 这一位是否被置为1，表示从
     * reload值减到0 
    */
    while(!(SysTick->CTRL & SYSTICK_COUNTFLAG));
    /*关闭定时器*/
    SysTick->CTRL = SYSTICK_DISABLE;
}

void BSP_Delay_ms(uint32_t ms)
{
    /* 方法1：循环调用 us 延时（推荐，避免溢出）*/
    while (ms--) {
        BSP_Delay_us(1000);  // 每次延时 1ms
    }
}

void BSP_Delay_s(uint32_t s)
{
    while (s--) {
        BSP_Delay_ms(1000);
    }
}