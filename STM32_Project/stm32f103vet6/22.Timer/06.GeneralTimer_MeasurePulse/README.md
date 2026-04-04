# STM32F10x 通用定时器输入捕获测量脉宽示例

## 1. 项目简介

本项目基于 **STM32F10x 标准外设库（StdPeriph Library）**，实现了一个使用 **通用定时器 TIM2 / TIM3 / TIM4 / TIM5** 进行**输入捕获测量脉宽**的示例工程。

这版代码的核心目标不是测 PWM 频率或占空比，而是更专注于：

- 测量单个脉冲的**高电平宽度**
- 支持高电平跨越定时器自动重装载周期
- 通过**更新中断**统计溢出次数
- 通过**捕获比较中断**记录上升沿和下降沿时间

当前示例中实际演示的是：

- 使用 **TIM5**
- 使用 **TIM5_CH4**
- 输入引脚为 **PA3**
- 串口输出使用 **USART1**
- 每隔 **1 秒** 打印一次当前测得的脉宽值

---

## 2. 功能特性

- 支持通用定时器：
  - `TIM2`
  - `TIM3`
  - `TIM4`
  - `TIM5`
- 支持输入捕获通道：
  - `CH1`
  - `CH2`
  - `CH3`
  - `CH4`
- 支持配置项：
  - 捕获极性
  - 输入捕获映射方式
  - 输入捕获预分频
  - 输入滤波
- 支持长脉宽测量：
  - 更新中断统计溢出次数
  - 可处理高电平跨越多个计数周期的情况
- BSP 风格封装，便于复用与扩展

---

## 3. 工程涉及文件

### 核心文件

- `bsp_general_timer.h`  
  通用定时器输入捕获模块头文件，定义：
  - 定时器枚举
  - 输入捕获通道枚举
  - 输入捕获配置结构体
  - 测量结果结构体
  - 对外接口函数

- `bsp_general_timer.c`  
  通用定时器输入捕获模块实现文件，完成：
  - GPIO 初始化
  - 定时器基础参数初始化
  - 输入捕获初始化
  - 更新中断与捕获比较中断配置
  - 上升沿/下降沿捕获与脉宽计算

- `main.c`  
  示例入口文件，完成：
  - 系统初始化
  - TIM5 CH4 输入捕获配置
  - 串口周期打印脉宽

- `stm32f10x_it.c`  
  中断服务函数文件，负责：
  - Cortex-M3 异常处理
  - USART 中断入口
  - TIM2 / TIM3 / TIM4 / TIM5 中断入口

---

## 4. 工作原理

### 4.1 总体思路

本工程使用的是**普通输入捕获法测脉宽**，不是 PWM Input 双通道测周期/占空比模式。

核心流程如下：

1. 初始化输入捕获通道为**上升沿捕获**
2. 当捕获到上升沿时：
   - 记录上升沿时间 `rising_cnt`
   - 清零本次测量的溢出次数 `over_flow`
   - 将通道切换为**下降沿捕获**
3. 当捕获到下降沿时：
   - 记录下降沿时间 `falling_cnt`
   - 结合溢出次数计算高电平脉宽 `pulse_width`
   - 将通道切回**上升沿捕获**
4. 主循环中周期性读取结果并换算成 us 输出

---

### 4.2 为什么需要更新中断

如果输入脉冲的高电平持续时间比较长，定时器计数器 `CNT` 可能在上升沿和下降沿之间发生一次或多次溢出。

因此仅依赖：

- `rising_cnt`
- `falling_cnt`

还不能得到完整高电平宽度。

所以代码同时使能了 **TIM_IT_Update** 更新中断，在高电平持续期间累计溢出次数。这样在下降沿到来时，就可以把跨越的整段计数周期补上。

---

### 4.3 脉宽计算逻辑

设：

- `ARR + 1 = period_cnt`
- `rising_cnt` 为上升沿捕获值
- `falling_cnt` 为下降沿捕获值
- `over_flow` 为测量期间发生的溢出次数

则高电平脉宽分三种情况。

#### 情况 1：下降沿计数值大于等于上升沿计数值

说明高电平没有跨过当前计数边界，或者跨过若干完整周期后又落在更大位置：

```c
pulse_width = over_flow * period_cnt + falling_cnt - rising_cnt;
```

#### 情况 2：下降沿计数值小于上升沿计数值，且期间发生过至少一次溢出

说明计数器至少完整回绕过一次：

```c
pulse_width = (over_flow - 1) * period_cnt + period_cnt - rising_cnt + falling_cnt;
```

#### 情况 3：下降沿计数值小于上升沿计数值，但溢出次数为 0

说明只跨过了当前 ARR 边界：

```c
pulse_width = period_cnt - rising_cnt + falling_cnt;
```

---

## 5. 当前示例配置

在 `main.c` 中当前配置为：

```c
bsp_generaltimer_config_t base_config = {
    .prescaler = 72 - 1,
    .counter_mode = BSP_TIM_COUNTER_MODE_UP,
    .period = 0xFFFF,
    .clock_div = BSP_TIM_CLOCK_DIV_1,
    .repetition_cnt = 0
};
```

以及：

```c
bsp_tim_ic_config_t ic_config = {
    .ic_channel = BSP_TIM_IC_CHANNEL4,
    .ic_polarity = BSP_TIM_IC_POLARITY_RISING,
    .ic_selection = BSP_TIM_IC_SELECTION_DIRECTTI,
    .ic_prescaler_div = BSP_TIM_IC_PRESCALER_DIV1,
    .ic_filter = 0
};
```

### 当前配置含义

- 定时器：`TIM5`
- 输入捕获通道：`CH4`
- 初始捕获极性：上升沿
- 输入映射方式：`DirectTI`
- 输入滤波：关闭
- 预分频：`72 - 1`

---

## 6. 当前计数频率与时间换算

代码中定义：

```c
#define CLK_FREQ 72000000U
```

并且：

```c
.prescaler = 72 - 1
```

因此当前计数频率为：

```text
72 MHz / 72 = 1 MHz
```

也就是说：

```text
1 tick = 1 us
```

程序中进一步计算：

```c
uint32_t timer_freq = CLK_FREQ / (base_config.prescaler + 1);
uint16_t timer_period = (uint16_t)((float)(1.0 / timer_freq) * 1000000U);
```

在当前配置下：

```text
timer_period = 1
```

因此打印时：

```c
result[BSP_GENERAL_TIMER5].pulse_width * timer_period
```

本质上就是把计数值换算成 us。

---

## 7. 一个容易误解的地方

这里的变量名：

```c
uint16_t timer_period
```

虽然名字里有 `period`，但它并不是：

- PWM 周期
- 定时器 ARR 周期
- 脉冲周期

它更准确地表示：

- **每个计数 tick 对应多少 us**

更准确的命名其实更接近：

```c
tick_us
```

当前代码逻辑没有问题，但阅读时容易误解。

---

## 8. 当前硬件映射

在 `bsp_general_timer.c` 中定义了如下引脚映射。

### TIM2

- `TIM2_CH1` → `PA0`
- `TIM2_CH2` → `PA1`
- `TIM2_CH3` → `PA2`
- `TIM2_CH4` → `PA3`

### TIM3

- `TIM3_CH1` → `PA6`
- `TIM3_CH2` → `PA7`
- `TIM3_CH3` → `PB0`
- `TIM3_CH4` → `PB1`

### TIM4

- `TIM4_CH1` → `PB6`
- `TIM4_CH2` → `PB7`
- `TIM4_CH3` → `PB8`
- `TIM4_CH4` → `PB9`

### TIM5

- `TIM5_CH1` → `PA0`
- `TIM5_CH2` → `PA1`
- `TIM5_CH3` → `PA2`
- `TIM5_CH4` → `PA3`

---

## 9. 当前示例实际使用的输入引脚

当前初始化代码为：

```c
BSP_GeneralTIM_Config(BSP_GENERAL_TIMER5, &base_config, &ic_config);
```

并且配置为：

```c
.ic_channel = BSP_TIM_IC_CHANNEL4
```

因此当前实际使用的是：

- **通用定时器：TIM5**
- **输入捕获通道：CH4**
- **输入引脚：PA3**

也就是说，外部脉冲信号应该接到：

```text
PA3 (TIM5_CH4)
```

---

## 10. 串口输出说明

当前代码使用：

```c
BSP_USART_Config(BSP_USART1);
BSP_USART_Stdio(BSP_USART1);
```

说明输出走的是：

- **USART1**

程序启动后先输出：

```text
25.通用定时器测量脉宽
```

之后每隔 1 秒输出一次当前测得的脉宽，例如：

```text
the width of pulse is 150us
```

---

## 11. 关键接口说明

### 11.1 `BSP_GeneralTIM_Config()`

```c
void BSP_GeneralTIM_Config(bsp_generaltimer_t timer_id,
                           bsp_generaltimer_config_t *base_config,
                           bsp_tim_ic_config_t *ic_config);
```

用于初始化通用定时器输入捕获功能，内部完成：

- GPIO 输入初始化
- 通用定时器基础参数初始化
- 输入捕获参数初始化
- 更新中断与捕获比较中断配置
- 定时器启动

---

### 11.2 `BSP_TIM_IRQHandler()`

```c
void BSP_TIM_IRQHandler(bsp_generaltimer_t timer_id);
```

这是通用定时器的公共中断处理函数。它统一处理两类中断源：

- **更新中断**：统计测量期间的溢出次数
- **捕获比较中断**：记录上升沿/下降沿并计算脉宽

在实际 IRQHandler 中调用，例如：

```c
void TIM5_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER5);
}
```

---

## 12. 数据结构说明

### 12.1 `bsp_generaltimer_config_t`

用于配置通用定时器基础参数：

- `prescaler`：预分频值
- `counter_mode`：计数模式
- `period`：ARR 自动重装值
- `clock_div`：CKD 时钟分频
- `repetition_cnt`：重复计数器保留字段

---

### 12.2 `bsp_tim_ic_config_t`

用于配置输入捕获参数：

- `ic_channel`：输入捕获通道
- `ic_polarity`：输入捕获极性
- `ic_selection`：输入捕获映射方式
- `ic_prescaler_div`：输入捕获预分频
- `ic_filter`：输入滤波器

---

### 12.3 `bsp_tim_ic_result_t`

用于保存测量结果：

- `rising_cnt`：上升沿捕获值
- `falling_cnt`：下降沿捕获值
- `pulse_width`：脉宽计数值
- `capture_state`：当前捕获状态
- `over_flow[]`：测量期间溢出次数

---

## 13. 中断说明

在 `stm32f10x_it.c` 中，本工程使用了以下定时器中断入口。

### TIM2 中断

```c
void TIM2_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER2);
}
```

### TIM3 中断

```c
void TIM3_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER3);
}
```

### TIM4 中断

```c
void TIM4_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER4);
}
```

### TIM5 中断

```c
void TIM5_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER5);
}
```

这说明：

- 实际中断入口在 `stm32f10x_it.c`
- 业务逻辑统一封装在 `bsp_general_timer.c`

这种结构更利于维护。

---

## 14. 容易误解的地方

### 14.1 这不是 PWM Input 模式

这份代码不是双通道 PWM Input 测频率/占空比的写法，而是：

- 先捕获上升沿
- 再切换为下降沿
- 计算高电平脉宽

所以它更准确地说是：

- **输入捕获测脉宽**

---

### 14.2 `TIMx_IRQHandler()` 里同时处理了更新中断和捕获中断

虽然 IRQ 入口只有一个：

```c
void TIM5_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER5);
}
```

但内部实际上同时处理了：

- `TIM_IT_Update`
- `TIM_IT_CCx`

如果不看中断状态判断，很容易误以为这里只处理了单一中断源。

---

### 14.3 `TIM_OCxPolarityConfig()` 名字容易让人误会

代码里通过：

```c
TIM_OCxPolarityConfig(...)
```

切换极性，以实现：

- 先捕获上升沿
- 再捕获下降沿

这个接口名字看上去像“输出比较极性配置”，但这里的实际用途是动态修改捕获极性。第一次读代码时很容易看懵。

---

### 14.4 `pulse_width` 的单位不是 us

结构体中的：

```c
pulse_width
```

保存的是**计数值**，不是时间值。

当前工程之所以能直接打印为 us，是因为现在配置下：

```text
1 tick = 1 us
```

如果以后改了预分频，这个换算关系也会变化。

---

### 14.5 `over_flow[]` 的设计语义有点绕

当前结构体里：

```c
volatile uint16_t over_flow[BSP_GENERAL_TIMER_MAX];
```

然后又在 `result[timer_id]` 中使用 `over_flow[timer_id]`。

这在逻辑上可以工作，但从设计语义上更自然的写法通常会是：

```c
volatile uint16_t over_flow;
```

因为每个 `result[timer_id]` 本来就已经对应一个定时器实例了。当前版本保持了现有逻辑，只是阅读时需要特别注意这一点。

---

## 15. 使用方法

### 15.1 硬件连接

请确保：

1. STM32F10x 开发板正常供电
2. 串口已连接到上位机
3. 外部脉冲输入信号接到 `PA3`
4. 输入信号电平兼容 STM32 IO

---

### 15.2 软件运行步骤

1. 编译并下载程序
2. 打开串口助手
3. 配置 USART1 对应串口参数
4. 向 `PA3` 输入脉冲信号
5. 观察串口输出的脉宽结果

---

## 16. 配置修改示例

### 16.1 改为 TIM5_CH1 输入

将：

```c
.ic_channel = BSP_TIM_IC_CHANNEL4
```

改为：

```c
.ic_channel = BSP_TIM_IC_CHANNEL1
```

此时输入引脚变为：

```text
PA0 (TIM5_CH1)
```

---

### 16.2 改为 TIM4 测量

将：

```c
BSP_GeneralTIM_Config(BSP_GENERAL_TIMER5, &base_config, &ic_config);
printf("...%d...", result[BSP_GENERAL_TIMER5].pulse_width * timer_period);
```

改为：

```c
BSP_GeneralTIM_Config(BSP_GENERAL_TIMER4, &base_config, &ic_config);
printf("...%d...", result[BSP_GENERAL_TIMER4].pulse_width * timer_period);
```

如果仍使用 CH4，则输入引脚改为：

```text
PB9 (TIM4_CH4)
```

---

## 17. 常见问题排查

### 17.1 串口只有标题，没有有效脉宽

如果只看到：

```text
25.通用定时器测量脉宽
```

但后续结果一直为 0 或明显不对，优先检查：

- 输入信号是否接到正确引脚
- TIM5 中断是否已正确使能
- 启动文件中中断函数名是否匹配
- 输入捕获通道是否配置正确
- 外部信号是否为有效脉冲

---

### 17.2 脉宽明显偏大或偏小

重点检查：

1. `CLK_FREQ` 是否与实际定时器输入时钟一致  
2. `prescaler` 是否正确  
3. APB1 时钟配置是否被修改  
4. 是否存在输入抖动  
5. 是否需要开启输入滤波  

---

### 17.3 长脉宽测量不准

检查：

- 更新中断是否真的进入
- `TIMx_IRQHandler()` 是否正确调用公共处理函数
- ARR 是否过小
- `over_flow` 是否被正确累加

---

## 18. 后续可扩展方向

本工程后续可以扩展为：

- 增加脉宽完成标志位
- 增加高电平/低电平双向测量
- 增加周期测量
- 增加最小/最大脉宽统计
- 增加超时判断
- 增加 OLED / LCD 实时显示
- 增加 FreeRTOS 任务化处理

---

## 19. 当前示例一句话总结

**本工程演示了如何使用 STM32F10x 的通用定时器 TIM2 / TIM3 / TIM4 / TIM5，通过输入捕获与更新中断配合的方式测量外部脉冲高电平宽度，并通过串口输出结果。**