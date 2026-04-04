# STM32F10x 高级定时器 PWM 输入测量示例

## 1. 项目简介

本项目基于 **STM32F10x 标准外设库（StdPeriph Library）**，实现了一个用于**测量外部 PWM 信号频率与占空比**的示例工程。  
核心功能使用 **高级定时器 TIM1 / TIM8 的 PWM Input 模式**完成，并通过串口周期性输出测量结果。

当前示例代码中，实际演示的是：

- 使用 **TIM1**
- 选择 **TIM1_CH2** 作为 PWM 输入源
- 通过 **USART2** 输出测量结果
- 每隔 **1 秒** 打印一次最新的 PWM 频率与占空比

---

## 2. 功能特性

- 支持高级定时器：
  - `TIM1`
  - `TIM8`
- 支持测量参数：
  - PWM 周期计数值
  - PWM 高电平脉宽计数值
  - PWM 频率
  - PWM 占空比
- 支持输入配置：
  - CH1 / CH2 输入源选择
  - 输入捕获极性选择
  - 输入捕获预分频
  - 输入滤波
- 中断驱动更新测量结果
- BSP 风格封装，便于后续扩展和复用

---

## 3. 工程涉及文件

### 核心文件

- `bsp_advanced_timer.h`  
  高级定时器 PWM 输入测量模块头文件，定义了：
  - 枚举类型
  - 配置结构体
  - 结果结构体
  - 对外接口函数

- `bsp_advanced_timer.c`  
  高级定时器 PWM 输入测量模块实现文件，完成：
  - GPIO 初始化
  - 定时器基础配置
  - PWM Input 模式配置
  - NVIC 配置
  - 中断处理与测量结果计算

- `main.c`  
  示例入口文件，完成：
  - 系统初始化
  - TIM1 PWM 输入配置
  - 串口打印测量结果

- `stm32f10x_it.c`  
  中断服务函数文件，包含：
  - USART 中断入口
  - TIM1/TIM8 捕获比较中断入口
  - 各类 Cortex-M3 异常处理函数

---

## 4. 工作原理

### 4.1 PWM Input 模式原理

本项目使用 STM32 定时器的 **PWM Input 模式** 来测量输入 PWM 信号。

该模式的特点是：

- 选定一个输入通道作为 PWM 输入源
- 定时器内部自动配对两个输入捕获寄存器
- 一个寄存器记录 PWM 周期
- 另一个寄存器记录高电平持续时间
- 配合 **从模式复位（Reset Slave Mode）**，每个周期重新开始计数

这样即可在中断中获取：

- 周期计数值 `period_cnt`
- 高电平计数值 `high_cnt`

进一步计算：

- 占空比  
  `duty = high_cnt / period_cnt * 100%`

- 频率  
  `freq = timer_clk / (PSC + 1) / period_cnt`

---

### 4.2 当前示例配置的计数基准

在 `main.c` 中配置如下：

```c
.prescaler = 72 - 1
```

若 TIM1 输入时钟为 `72 MHz`，则定时器计数频率为：

```text
72 MHz / 72 = 1 MHz
```

也就是说：

- 每计数 1 次，约等于 `1 us`
- `period_cnt` 近似等于 PWM 周期的微秒数
- `high_cnt` 近似等于 PWM 高电平持续时间的微秒数

例如：

- 输入 PWM 为 `1 kHz`
- 周期约 `1000 us`
- 则 `period_cnt` 约为 `1000`

---

## 5. 当前代码中的硬件映射

### 5.1 高级定时器通道引脚

在 `bsp_advanced_timer.c` 中定义了如下映射：

#### TIM1

- `TIM1_CH1` → `PA8`
- `TIM1_CH2` → `PA9`

#### TIM8

- `TIM8_CH1` → `PC6`
- `TIM8_CH2` → `PC7`

---

### 5.2 当前示例实际使用的输入引脚

`main.c` 中当前配置为：

```c
.pwm_channel = BSP_TIM_PWM_INPUT_CHANNEL2
```

并且初始化的是：

```c
BSP_AdvancedTimer_Config(BSP_ADVANCED_TIMER1, &base_config, &pwm_config);
```

因此当前示例实际测量的是：

- **定时器：TIM1**
- **输入通道：CH2**
- **输入引脚：PA9**

也就是说，你需要把外部 PWM 信号接到：

```text
PA9 (TIM1_CH2)
```

---

## 6. 串口输出说明

在 `main.c` 中：

```c
BSP_USART_Config(BSP_USART2);
BSP_USART_Stdio(BSP_USART2);
```

说明当前打印输出走的是：

- **USART2**

程序每隔 1 秒检查一次是否有新的 PWM 测量值，如果有，则输出类似内容：

```text
freq = 1000 Hz, duty = 50.00%, period_cnt = 1000, high_cnt = 500
```

---

## 7. 使用方法

### 7.1 硬件连接

请确保：

1. MCU 正常供电
2. 串口已连接到上位机
3. 外部 PWM 信号接入 `PA9`
4. 外部 PWM 信号电平与 STM32 IO 电平兼容

---

### 7.2 下载运行

将工程编译并下载到 STM32F10x 开发板后：

1. 打开串口助手
2. 配置对应波特率（以你的 `BSP_USART_Config()` 实际配置为准）
3. 给 `PA9` 输入 PWM 信号
4. 观察串口打印结果

---

### 7.3 示例输出

```text
25.高级定时器测量PWM
freq = 1000 Hz, duty = 50.00%, period_cnt = 1000, high_cnt = 500
freq = 2000 Hz, duty = 25.00%, period_cnt = 500, high_cnt = 125
```

---

## 8. 关键接口说明

### 8.1 `BSP_AdvancedTimer_Config()`

```c
void BSP_AdvancedTimer_Config(bsp_advancedtimer_t timer_id,
                              bsp_advancedtimer_config_t *base_config,
                              bsp_tim_pwm_config_t *pwm_config);
```

用于初始化高级定时器 PWM 输入测量功能，内部完成：

- GPIO 输入初始化
- 定时器基础参数初始化
- PWM Input 模式初始化
- 从模式复位配置
- NVIC 配置
- 定时器启动

---

### 8.2 `BSP_TIM_CC_IRQHandler()`

```c
void BSP_TIM_CC_IRQHandler(bsp_advancedtimer_t timer_id);
```

这是高级定时器捕获比较中断的公共处理函数。  
在实际 IRQHandler 中调用，例如：

```c
void TIM1_CC_IRQHandler(void)
{
    BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER1);
}
```

该函数内部完成：

- 读取捕获值
- 计算周期
- 计算高电平时间
- 计算频率
- 计算占空比
- 更新结果标志

---

## 9. 数据结构说明

### 9.1 `bsp_advancedtimer_config_t`

用于配置定时器基础参数：

- `prescaler`：预分频值
- `counter_mode`：计数模式
- `period`：自动重装值 ARR
- `clock_div`：时钟分频
- `repetition_cnt`：重复计数器

---

### 9.2 `bsp_tim_pwm_config_t`

用于配置 PWM 输入测量参数：

- `pwm_channel`：输入通道选择
- `ic_polarity`：捕获极性
- `ic_prescaler_div`：输入捕获预分频
- `ic_filter`：输入滤波器

---

### 9.3 `bsp_tim_ic_result_t`

用于保存测量结果：

- `g_pwm_period`：周期计数值
- `g_pwm_pulse_width`：高电平计数值
- `g_pwm_duty`：占空比
- `g_pwm_freq`：频率
- `g_pwm_update`：更新标志

---

## 10. 中断说明

在 `stm32f10x_it.c` 中已接入如下中断：

### TIM1 捕获比较中断

```c
void TIM1_CC_IRQHandler(void)
{
  BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER1);
}
```

### TIM8 捕获比较中断

```c
void TIM8_CC_IRQHandler(void)
{
  BSP_TIM_CC_IRQHandler(BSP_ADVANCED_TIMER8);
}
```

### 串口中断

```c
void USART1_IRQHandler(void)
{
  BSP_USART_IRQHandler(BSP_USART1);
}
```

其余 USART2 / USART3 / UART4 / UART5 同理。

---

## 11. 容易误解的地方

### 11.1 `clock_div` 不是 PSC

很多人容易把：

- `TIM_ClockDivision`
- `TIM_Prescaler`

混为一谈。

实际上：

- `PSC` 决定定时器计数频率
- `clock_div` 对应 `CKD`，主要影响采样时钟/数字滤波相关行为

也就是说，真正影响频率计算的是：

```c
res->g_pwm_freq = CLK_FREQ / (hw->timer->PSC + 1) / res->g_pwm_period;
```

---

### 11.2 选择 CH1/CH2 输入，不等于只用一个通道

PWM Input 模式下，底层并不是只使用一个通道寄存器。

例如：

- 选择 `CH1` 作为输入源时，通常会同时配合 `CCR1` 和 `CCR2`
- 选择 `CH2` 作为输入源时，也会联合另一个寄存器一起工作

所以代码里会看到：

```c
if(pwm_channel == BSP_TIM_PWM_INPUT_CHANNEL1){
    res->g_pwm_period = TIM_GetCapture1(hw->timer);
    res->g_pwm_pulse_width = TIM_GetCapture2(hw->timer);
}else if(pwm_channel == BSP_TIM_PWM_INPUT_CHANNEL2){
    res->g_pwm_period = TIM_GetCapture2(hw->timer);
    res->g_pwm_pulse_width = TIM_GetCapture1(hw->timer);
}
```

这不是写反了，而是 PWM Input 模式的正常行为。

---

### 11.3 `period_cnt` 和 `high_cnt` 是“计数值”不是直接时间

串口输出中的：

- `period_cnt`
- `high_cnt`

本质都是定时器 tick 数，不是直接单位为秒或毫秒。

在当前配置下，由于计数频率是 1 MHz，所以可以近似认为：

- `1 count ≈ 1 us`

但这只是建立在当前预分频配置上的结果。

---

### 11.4 `volatile` 是必须的

测量结果结构体成员被定义为：

```c
volatile uint32_t ...
volatile float ...
```

这是因为：

- 数据在中断里更新
- 主循环里读取

如果没有 `volatile`，编译器可能会优化掉对变量的重复读取，导致主循环看不到中断更新后的值。

---

## 12. 配置修改示例

### 12.1 改为 TIM1_CH1 输入

将：

```c
.pwm_channel = BSP_TIM_PWM_INPUT_CHANNEL2
```

改为：

```c
.pwm_channel = BSP_TIM_PWM_INPUT_CHANNEL1
```

此时 TIM1 输入引脚改为：

```text
PA8 (TIM1_CH1)
```

---

### 12.2 改为 TIM8 输入

将：

```c
BSP_AdvancedTimer_Config(BSP_ADVANCED_TIMER1, &base_config, &pwm_config);
```

改为：

```c
BSP_AdvancedTimer_Config(BSP_ADVANCED_TIMER8, &base_config, &pwm_config);
```

并注意：

- 如果 CH1：输入引脚是 `PC6`
- 如果 CH2：输入引脚是 `PC7`

同时主循环读取结果也应改为：

```c
bsp_tim_ic_result_t *res = &result[BSP_ADVANCED_TIMER8];
```

---

## 13. 常见问题排查

### 13.1 串口一直没有输出测量值

检查：

- 是否已经正确初始化串口
- 是否真的有 PWM 输入信号
- 是否中断已使能
- `TIM1_CC_IRQHandler()` 是否被正确链接
- PWM 输入引脚是否接对
- PWM 幅值是否符合 STM32 输入电平要求

---

### 13.2 打印了标题但一直没有频率/占空比

如果只看到：

```text
25.高级定时器测量PWM
```

但没有后续测量数据，常见原因是：

- `g_pwm_update` 没有被置位
- 捕获比较中断没有触发
- PWM 信号没有输入到正确引脚
- NVIC 没配置成功
- 启动文件中断函数名与代码不匹配

---

### 13.3 频率不准

检查：

1. `CLK_FREQ` 是否与实际定时器输入时钟一致  
2. `prescaler` 配置是否正确  
3. APB2 时钟配置是否改变  
4. 输入信号是否稳定  
5. 是否需要设置 `ic_filter`

---

### 13.4 占空比不准

检查：

- 输入信号是否存在抖动
- 极性配置是否正确
- 输入滤波是否需要开启
- PWM 信号高低电平是否标准

---

## 14. 后续可扩展方向

本项目可以继续扩展为：

- 支持普通定时器输入捕获测量
- 支持多个通道同时测量
- 支持超时检测（无 PWM 输入时清零）
- 支持移动平均滤波
- 支持测量最小/最大频率限制
- 支持实时显示到 OLED / LCD
- 支持 FreeRTOS 任务化处理

---

## 15. 许可证与说明

本工程基于 STM32F10x 标准外设库模板进行扩展与封装。  
原始模板版权归 STMicroelectronics 所有；用户增加的 BSP 封装、示例逻辑与注释用于学习、开发与项目维护参考。

---

## 16. 建议阅读顺序

如果你是第一次接触这份工程，建议按下面顺序阅读：

1. `main.c`  
   先了解工程入口和如何调用模块

2. `bsp_advanced_timer.h`  
   了解有哪些配置项和数据结构

3. `bsp_advanced_timer.c`  
   理解 PWM Input 模式如何实现

4. `stm32f10x_it.c`  
   查看中断入口如何转调到 BSP 层

---

## 17. 当前示例一句话总结

**这个工程演示了如何使用 STM32F10x 的高级定时器 TIM1/TIM8，通过 PWM Input 模式测量外部 PWM 信号的频率和占空比，并经串口输出结果。**