# STM32F10x 高级定时器输入捕获测量脉宽示例

## 1. 项目简介

本项目基于 **STM32F10x 标准外设库（StdPeriph Library）**，实现了一个使用 **高级定时器 TIM1 / TIM8** 进行**输入捕获测量脉宽**的示例工程。

和常见的 PWM Input 测频率/占空比方案不同，这份代码的目标更明确：

- 测量单个脉冲的**高电平宽度**
- 支持脉宽跨越定时器自动重装载周期
- 使用**更新中断统计溢出次数**
- 使用**捕获比较中断记录上升沿/下降沿时间**

当前示例中实际演示的是：

- 使用 **TIM8**
- 使用 **TIM8_CH4**
- 输入引脚为 **PC9**
- 串口输出使用 **USART1**
- 每隔 **1 秒** 打印一次当前测得的脉宽值

---

## 2. 功能特性

- 支持高级定时器：
  - `TIM1`
  - `TIM8`
- 支持输入捕获通道：
  - `CH1`
  - `CH2`
  - `CH3`
  - `CH4`
- 支持配置项：
  - 捕获极性
  - 捕获映射方式
  - 输入捕获预分频
  - 输入滤波
- 支持长脉宽测量：
  - 通过更新中断统计溢出次数
  - 可以处理高电平跨越多个计数周期的情况
- 使用 BSP 风格封装，便于扩展和复用

---

## 3. 工程涉及文件

### 核心文件

- `bsp_advanced_timer.h`  
  高级定时器输入捕获模块头文件，定义：
  - 定时器枚举
  - 输入捕获通道枚举
  - 输入捕获配置结构体
  - 测量结果结构体
  - 对外接口函数

- `bsp_advanced_timer.c`  
  高级定时器输入捕获模块实现文件，完成：
  - GPIO 初始化
  - 定时器基础参数初始化
  - 输入捕获初始化
  - 更新中断与捕获比较中断配置
  - 上升沿/下降沿捕获与脉宽计算

- `main.c`  
  示例入口文件，完成：
  - 系统初始化
  - TIM8 CH4 输入捕获配置
  - 串口周期打印脉宽

- `stm32f10x_it.c`  
  中断服务函数文件，负责：
  - Cortex-M3 异常处理
  - 串口中断入口
  - TIM1 / TIM8 更新中断入口
  - TIM1 / TIM8 捕获比较中断入口

- `bsp_nvic_group.h / bsp_nvic_group.c`  
  NVIC 优先级分组配置文件

---

## 4. 工作原理

## 4.1 总体思路

本工程使用的是**输入捕获法测脉宽**，不是 PWM Input 双通道测周期/占空比模式。

核心流程如下：

1. 初始化输入捕获通道为**上升沿捕获**
2. 当捕获到上升沿时：
   - 记录上升沿时间 `rising_cnt`
   - 清零溢出计数 `over_flow`
   - 将通道切换为**下降沿捕获**
3. 当捕获到下降沿时：
   - 记录下降沿时间 `falling_cnt`
   - 结合溢出次数计算高电平脉宽 `pulse_width`
   - 将通道重新切回**上升沿捕获**
4. 主循环中周期性读取结果并换算成 us 输出

---

## 4.2 为什么需要更新中断

当输入脉冲的高电平持续时间比较长时，定时器计数器 `CNT` 可能在上升沿和下降沿之间发生一次或多次溢出。

因此仅靠：

- `rising_cnt`
- `falling_cnt`

还不足以得到真实高电平宽度。

所以代码中启用了 **TIM_IT_Update** 更新中断，在高电平持续期间统计溢出次数：

```c
res->over_flow[ic_channel]++;
```

最终脉宽计算时把这些整周期补上。

---

## 4.3 脉宽计算逻辑

设：

- `ARR + 1 = period`
- `rising_cnt` 为上升沿捕获值
- `falling_cnt` 为下降沿捕获值
- `over_flow` 为中间溢出次数

则高电平脉宽分三种情况：

### 情况 1：下降沿计数值大于等于上升沿计数值

说明高电平可能没有跨越当前计数边界，或跨越了若干完整周期后又落在更大位置：

```c
pulse_width = over_flow * period + falling_cnt - rising_cnt;
```

### 情况 2：下降沿计数值小于上升沿计数值，且期间溢出次数大于 0

说明至少发生过一次完整回绕：

```c
pulse_width = (over_flow - 1) * period + period - rising_cnt + falling_cnt;
```

### 情况 3：下降沿计数值小于上升沿计数值，且溢出次数为 0

说明仅跨过当前计数边界：

```c
pulse_width = period - rising_cnt + falling_cnt;
```

---

## 5. 当前示例配置

在 `main.c` 中当前配置为：

```c
bsp_advancedtimer_config_t base_config = {
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
    .ic_prescaler_div = BSP_TIM_IC_PRESCALER_DIV1,
    .ic_selection = BSP_TIM_IC_SELECTION_DIRECTTI,
    .ic_filter = 0x00
};
```

### 当前配置含义

- 定时器：`TIM8`
- 输入通道：`CH4`
- 捕获初始极性：上升沿
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

因此程序中：

```c
uint16_t period = (uint16_t)((float)(1.0 / counter_freq) * 1000000);
```

计算结果为：

```text
period = 1
```

最终打印时：

```c
pulse_width * period
```

本质上就是把计数值换算成 us。

---

## 7. 一个容易误解的地方

这里的变量名：

```c
uint16_t period = ...
```

虽然叫 `period`，但它实际上不是：

- PWM 周期
- ARR 周期
- 脉冲周期

它实际表示的是：

- **单个计数 tick 对应多少 us**

更准确的语义其实更接近：

```c
tick_us
```

当前代码逻辑没问题，但阅读时容易误解，这里特别说明。

---

## 8. 当前硬件映射

在 `bsp_advanced_timer.c` 中定义了如下引脚映射。

### TIM1

- `TIM1_CH1` → `PA8`
- `TIM1_CH2` → `PA9`
- `TIM1_CH3` → `PA10`
- `TIM1_CH4` → `PA11`

### TIM8

- `TIM8_CH1` → `PC6`
- `TIM8_CH2` → `PC7`
- `TIM8_CH3` → `PC8`
- `TIM8_CH4` → `PC9`

---

## 9. 当前示例实际使用的输入引脚

当前初始化代码为：

```c
BSP_AdvancedTimer_Config(BSP_ADVANCED_TIMER8, &base_config, &ic_config);
```

并且配置为：

```c
.ic_channel = BSP_TIM_IC_CHANNEL4
```

因此当前实际使用的是：

- **高级定时器：TIM8**
- **输入捕获通道：CH4**
- **输入引脚：PC9**

也就是说，外部脉冲信号应该接到：

```text
PC9 (TIM8_CH4)
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

程序启动后会先输出：

```text
25.高级定时器测量脉宽
```

之后每隔 1 秒输出一次当前测得的脉宽，例如：

```text
the width of pulse is 150us
```

---

## 11. 关键接口说明

### 11.1 `BSP_AdvancedTimer_Config()`

```c
void BSP_AdvancedTimer_Config(bsp_advancedtimer_t timer_id,
                              bsp_advancedtimer_config_t *base_config,
                              bsp_tim_ic_config_t *ic_config);
```

用于初始化高级定时器输入捕获功能，内部完成：

- GPIO 输入初始化
- 高级定时器基础参数初始化
- 输入捕获参数初始化
- 更新中断与捕获比较中断配置
- 定时器启动

---

### 11.2 `BSP_TIM_UP_IRQHandler()`

```c
void BSP_TIM_UP_IRQHandler(bsp_advancedtimer_t timer_id);
```

这是高级定时器更新中断公共处理函数。  
用于在高电平脉宽测量期间统计计数器溢出次数。

对应实际 IRQHandler，例如：

```c
void TIM8_UP_IRQHandler(void)
{
    BSP_TIM_UP_IRQHandler(BSP_ADVANCED_TIMER8);
}
```

---

### 11.3 `BSP_TIM_CC_IRQHandler()`

```c
void BSP_TIM_CC_IRQHandler(bsp_advancedtimer_t timer_id);
```

这是高级定时器捕获比较中断公共处理函数。  
其核心职责是：

- 第一次捕获上升沿
- 切换为下降沿捕获
- 第二次捕获下降沿
- 结合溢出次数计算脉宽
- 恢复为上升沿捕获，等待下一次测量

---

## 12. 数据结构说明

### 12.1 `bsp_advancedtimer_config_t`

用于配置高级定时器基础参数：

- `prescaler`：预分频值
- `counter_mode`：计数模式
- `period`：ARR 自动重装值
- `clock_div`：CKD 时钟分频
- `repetition_cnt`：重复计数器

---

### 12.2 `bsp_tim_ic_config_t`

用于配置输入捕获参数：

- `ic_channel`：输入捕获通道
- `ic_polarity`：输入捕获极性
- `ic_selection`：输入映射方式
- `ic_prescaler_div`：输入捕获预分频
- `ic_filter`：输入滤波器

---

### 12.3 `bsp_tim_ic_result_t`

用于保存测量结果，按通道分别记录：

- `rising_cnt[]`：上升沿捕获值
- `falling_cnt[]`：下降沿捕获值
- `pulse_width[]`：脉宽计数值
- `capture_state[]`：当前捕获状态
- `over_flow[]`：期间溢出次数

---

## 13. 中断说明

在 `stm32f10x_it.c` 中，本工程使用了以下高级定时器中断入口。

### TIM1 更新中断

```c
void TIM1_UP_IRQHandler(void)
{
    BSP_TIM_UP_IRQHandler(BSP_ADVANCED_TIMER1);
}
```

### TIM1 捕获比较中断

```c
void TIM1_CC_IRQHandler(void)
{
    BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER1);
}
```

### TIM8 更新中断

```c
void TIM8_UP_IRQHandler(void)
{
    BSP_TIM_UP_IRQHandler(BSP_ADVANCED_TIMER8);
}
```

### TIM8 捕获比较中断

```c
void TIM8_CC_IRQHandler(void)
{
    BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER8);
}
```

这说明：

- 实际中断入口在 `stm32f10x_it.c`
- 业务逻辑统一封装在 `bsp_advanced_timer.c`

这样的结构更适合维护和扩展。

---

## 14. 容易误解的地方

### 14.1 这不是 PWM Input 模式

虽然最终也在测输入波形，但这份代码和 PWM Input 模式不同。

这里的方式是：

- 手动记录上升沿
- 手动切换极性
- 手动记录下降沿
- 计算高电平脉宽

所以它更准确地说是：

- **输入捕获测脉宽**

而不是：

- **PWM Input 测周期/占空比**

---

### 14.2 `BSP_TIM_OCxPolarityConfig()` 容易让人误会

代码里用到了：

```c
TIM_OCxPolarityConfig(...)
```

看名字会让人以为是在做“输出比较”配置。  
但这里实际上是借助标准库接口动态切换通道极性，以实现：

- 先捕获上升沿
- 再捕获下降沿

这是 STM32 标准库里常见写法，但很容易让第一次接手代码的人误解。

---

### 14.3 `pulse_width[]` 的单位不是 us

结构体中的：

```c
pulse_width[]
```

存的是**计数值**，不是时间值。

当前工程之所以打印出来能直接看作 us，是因为当前配置下：

```text
1 tick = 1 us
```

如果以后你改了预分频，那么这个换算关系也会变。

---

### 14.4 `clock_div` 不是 PSC

很多人容易混淆：

- `TIM_ClockDivision`
- `TIM_Prescaler`

它们并不是同一个东西：

- `PSC` 决定计数器时钟频率
- `clock_div` 对应 `CKD`，主要影响数字滤波采样相关时钟

所以真正影响时间换算的是：

```c
counter_freq = CLK_FREQ / (base_config.prescaler + 1);
```

---

### 14.5 `s_ic_channel` 应按定时器数量索引

在逻辑上，`s_ic_channel[]` 是根据 `timer_id` 访问的，因此它应表示：

- 每个定时器当前配置了哪个输入捕获通道

所以这个数组的语义应当是按**定时器数量**索引，而不是按**通道数量**索引。  
阅读或后续重构时要特别注意这一点。

---

## 15. 使用方法

### 15.1 硬件连接

请确保：

1. STM32F10x 开发板正常供电
2. 串口已连接到上位机
3. 外部脉冲输入信号接到 `PC9`
4. 输入信号电平兼容 STM32 IO 电平

---

### 15.2 软件运行步骤

1. 编译并下载程序
2. 打开串口助手
3. 配置 USART1 对应波特率
4. 向 `PC9` 输入脉冲信号
5. 观察串口输出的脉宽结果

---

## 16. 配置修改示例

### 16.1 改为 TIM8_CH1 输入

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
PC6 (TIM8_CH1)
```

---

### 16.2 改为 TIM1 测量

将：

```c
BSP_AdvancedTimer_Config(BSP_ADVANCED_TIMER8, &base_config, &ic_config);
```

改为：

```c
BSP_AdvancedTimer_Config(BSP_ADVANCED_TIMER1, &base_config, &ic_config);
```

若仍使用 CH4，则输入引脚变为：

```text
PA11 (TIM1_CH4)
```

同时打印结果时下标也应改为：

```c
result[BSP_ADVANCED_TIMER1]
```

---

## 17. 常见问题排查

### 17.1 串口只打印标题，不打印有效脉宽

如果只看到：

```text
25.高级定时器测量脉宽
```

后续结果一直是 0 或明显不对，优先检查：

- 输入信号是否真的接到正确引脚
- TIM8 / TIM1 中断是否已正确使能
- 启动文件中的中断函数名是否匹配
- 捕获通道是否配置正确
- 外部信号是否为有效高低电平脉冲

---

### 17.2 脉宽明显偏大或偏小

重点检查：

1. `CLK_FREQ` 是否与实际定时器输入时钟一致  
2. `prescaler` 是否正确  
3. 是否误改了 APB2 时钟配置  
4. 是否存在输入抖动  
5. 是否需要开启输入滤波  

---

### 17.3 长脉宽测量不准

检查：

- 更新中断是否进入
- `TIMx_UP_IRQHandler()` 是否正确转调
- 定时器 ARR 是否过小
- 溢出次数 `over_flow[]` 是否被正确累加

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

**本工程演示了如何使用 STM32F10x 的高级定时器 TIM1 / TIM8，通过输入捕获与更新中断配合的方式测量外部脉冲高电平宽度，并通过串口输出结果。**