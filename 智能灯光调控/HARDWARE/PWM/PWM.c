#include "stm32f10x.h"                  // Device header

/*此模块代码详解请看江协科技对应P16视频*/

void PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;//将前者进行重命名
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;//和配置GPIO一样定义结构体，然后初始化参数
    TIM_OCInitTypeDef TIM_OCInitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //打开时钟


    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出模式，复用~可以将GPIO口的控制权交给片上外设也就是定时器。普通的推挽输出是由输出寄存器来控制GPIO口的
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //选择PA0
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure); //利用上述参数将GPIOA初始化


    //第一步：开启时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);/*这里选择TIM2，属于通用定时器（自由选择）。
                                                          *而TIM2是APB1总线的外设，所以这里不用APB2*/
    //第二步：选择时基单元的时钟
    TIM_InternalClockConfig(TIM2);/*此为选择内部时钟的函数，这样TIM2的时基单元就由内部函数来驱动了
    *不过因为定时器上电后默认使用内部时钟来驱动，此行代码可以省略，为了代码完整性，还是写上*/

    //第三步：配置时基单元

    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //时钟分频，这里选择1分频，也就是不分频
    //由于滤波器（消除抖动）需要一定采样频率才能工作，而这个频率可以内部时钟自己来，也可以内部时钟加时钟频率。可见关系并不大，因此这里选择不分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //这里选择向上计数

    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1; //周期，ARR自动重装器的值
    TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1; //PSC预分频器的值

    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00; //重复计数器的值，高级定时器才有，这里不需要用，就给0
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    //初始化输出比较单元

    TIM_OCStructInit(&TIM_OCInitStructure);/*自动给结构体进行一个初始化，就是说会给所有变量配置一个初始值
    *如果需要对某些单独变量赋值，像下面这样单独拿出来赋值即可。记住要放在结构体定义的下面，*/
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //采用PWM1输出比较模式
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出比较极性，选择高电平暂定因为选择了高电平驱动LED吧
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable ; //输出使能
    TIM_OCInitStructure.TIM_Pulse = 0; //即CCR的值
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    //初始化输出比较单元

    TIM_Cmd(TIM2, ENABLE); //定时器启动函数

}
void PWM1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;//将前者进行重命名
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;//和配置GPIO一样定义结构体，然后初始化参数
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //打开时钟


    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出模式，复用~可以将GPIO口的控制权交给片上外设也就是定时器。普通的推挽输出是由输出寄存器来控制GPIO口的
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //选择PA0
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure); //利用上述参数将GPIOA初始化


    //第一步：开启时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);/*这里选择TIM2，属于通用定时器（自由选择）。
                                                          *而TIM2是APB1总线的外设，所以这里不用APB2*/
    //第二步：选择时基单元的时钟
    TIM_InternalClockConfig(TIM2);/*此为选择内部时钟的函数，这样TIM2的时基单元就由内部函数来驱动了
    *不过因为定时器上电后默认使用内部时钟来驱动，此行代码可以省略，为了代码完整性，还是写上*/

    //第三步：配置时基单元

    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //时钟分频，这里选择1分频，也就是不分频
    //由于滤波器（消除抖动）需要一定采样频率才能工作，而这个频率可以内部时钟自己来，也可以内部时钟加时钟频率。可见关系并不大，因此这里选择不分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down; //这里选择向上计数

    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1; //周期，ARR自动重装器的值
    TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1; //PSC预分频器的值

    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00; //重复计数器的值，高级定时器才有，这里不需要用，就给0
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    //初始化输出比较单元

    TIM_OCStructInit(&TIM_OCInitStructure);/*自动给结构体进行一个初始化，就是说会给所有变量配置一个初始值
    *如果需要对某些单独变量赋值，像下面这样单独拿出来赋值即可。记住要放在结构体定义的下面，*/
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //采用PWM1输出比较模式
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出比较极性，选择高电平暂定因为选择了高电平驱动LED吧
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable ; //输出使能
    TIM_OCInitStructure.TIM_Pulse = 0; //即CCR的值
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    //初始化输出比较单元

    //第六步：启动定时器
    TIM_Cmd(TIM2, ENABLE); //定时器启动函数
}

void PWM_SetCompare1(uint16_t Compare)
{
    TIM_SetCompare1(TIM2, Compare);
}

void PWM_SetCompare2(uint16_t Compare)//pare1同样改成pare2
{
    TIM_SetCompare2(TIM2, Compare);
}
