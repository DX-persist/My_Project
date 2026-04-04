# STM32F10x 通用定时器 PWM 输入测量示例

## 1. 项目简介

本项目基于 **STM32F10x 标准外设库（StdPeriph Library）**，实现了一个使用**通用定时器 TIM2 / TIM3 / TIM4 / TIM5**测量外部 PWM 信号频率与占空比的示例工程。

当前示例代码中，实际演示的是：

- 使用 **TIM4**
- 选择 **TIM4_CH2** 作为 PWM 输入源
- 外部 PWM 输入引脚为 **PB7**
- 通过 **USART1** 输出测量结果
- 每隔 **1 秒** 打印一次最新的 PWM 频率与占空比

---

## 2. 功能特性

- 支持通用定时器：
  - `TIM2`
  - `TIM3`
  - `TIM4`
  - `TIM5`
- 支持测量参数：
  - PWM 周期计数值
  - PWM 高电平脉宽计数值
  - PWM 频率
  - PWM 占空比
- 支持配置项：
  - CH1 / CH2 输入源选择
  - 输入捕获极性
  - 输入捕获预分频
  - 输入滤波
- 使用中断方式更新测量结果
- 采用 BSP 封装，便于复用和移植

---

## 3. 工程涉及文件

### 核心文件

- `bsp_general_timer.h`  
  通用定时器 PWM 输入测量模块头文件，定义：
  - 枚举类型
  - 配置结构体
  - 测量结果结构体
  - 对外接口函数

- `bsp_general_timer.c`  
  通用定时器 PWM 输入测量模块实现文件，负责：
  - GPIO 初始化
  - 定时器基础配置
  - PWM Input 模式配置
  - 中断配置
  - 捕获结果计算

- `stm32f10x_it.c`  
  中断服务函数文件，负责：
  - Cortex-M3 异常处理
  - USART 中断入口
  - TIM2/TIM3/TIM4/TIM5 中断入口

- `main.c`  
  示例入口程序，负责：
  - 系统初始化
  - TIM4 PWM 输入配置
  - 串口输出测量结果

---

## 4. 工作原理

### 4.1 PWM Input 模式原理

本项目使用 STM32 定时器的 **PWM Input 模式** 测量外部 PWM 信号。

工作方式如下：

- 选择 CH1 或 CH2 作为 PWM 输入源
- 定时器内部自动使用两个捕获寄存器协同工作
- 一个寄存器保存周期计数值
- 另一个寄存器保存高电平计数值
- 配合 **从模式复位（Reset Slave Mode）**，在每个有效边沿到来时让 CNT 清零

这样可以在中断中得到：

- `period_cnt`：一个 PWM 周期对应的计数值
- `high_cnt`：PWM 高电平宽度对应的计数值

再进一步计算：

- 占空比  
  `duty = high_cnt / period_cnt * 100%`

- 频率  
  `freq = timer_clk / period_cnt`

其中：

```text
timer_clk = CLK_FREQ / (PSC + 1)
```

---

## 5. 当前示例的参数配置

在 `main.c` 中，当前基础配置如下：

```c
.prescaler = 36 - 1
.period = 0xFFFF
.counter_mode = BSP_TIM_COUNTER_MODE_UP
.clock_div = BSP_TIM_CLOCK_DIV_1
```

### 5.1 当前计数频率

代码中定义：

```c
#define CLK_FREQ 72000000U
```

因此当前计数频率为：

```text
72 MHz / 36 = 2 MHz
```

也就是说：

```text
1 tick = 0.5 us
```

### 5.2 一个容易误解的地方

原代码旁边有一条旧注释：

```c
/**< 72MHz / 72 = 1MHz => 1 tick = 1us */
```

但这和当前配置 **不一致**。  
因为现在的 `prescaler = 36 - 1`，所以：

- 不是 `1 MHz`
- 而是 `2 MHz`
- 即 `1 tick = 0.5 us`

这个地方特别容易导致后续换算错误，建议以当前实际代码为准。

---

## 6. 当前硬件映射

在 `bsp_general_timer.c` 中定义了如下通道映射：

### TIM2

- `TIM2_CH1` → `PA0`
- `TIM2_CH2` → `PA1`

### TIM3

- `TIM3_CH1` → `PA6`
- `TIM3_CH2` → `PA7`

### TIM4

- `TIM4_CH1` → `PB6`
- `TIM4_CH2` → `PB7`

### TIM5

- `TIM5_CH1` → `PA0`
- `TIM5_CH2` → `PA1`

---

## 7. 当前示例实际使用的输入引脚

`main.c` 中当前配置为：

```c
.input_channel = BSP_TIM_PWM_INPUT_CHANNEL2
```

并且初始化的是：

```c
BSP_GeneralTIM_Config(BSP_GENERAL_TIMER4, &base_config, &pwm_config);
```

因此当前实际使用的是：

- **定时器：TIM4**
- **输入通道：CH2**
- **输入引脚：PB7**

也就是说，外部 PWM 信号应接到：

```text
PB7 (TIM4_CH2)
```

---

## 8. 串口输出说明

当前示例使用：

```c
BSP_USART_Config(BSP_USART1);
BSP_USART_Stdio(BSP_USART1);
```

说明串口打印走的是：

- **USART1**

程序每隔 1 秒检查一次是否有新的测量值，如果有，则输出类似：

```text
25.通用定时器测量PWM
freq = 1000 Hz, duty = 50.00%, period_cnt = 2000, high_cnt = 1000
```

### 说明

因为当前计数频率是 **2 MHz**，所以：

- 若输入 PWM 为 `1 kHz`
- 周期为 `1000 us`
- 则周期计数值为：

```text
1000 us / 0.5 us = 2000
```

高电平若为 50%，则：

```text
high_cnt = 1000
```

---

## 9. 关键接口说明

### 9.1 `BSP_GeneralTIM_Config()`

```c
void BSP_GeneralTIM_Config(bsp_generaltimer_t timer_id,
                           bsp_generaltimer_config_t *base_config,
                           bsp_tim_pwm_input_config_t *pwm_config);
```

用于初始化通用定时器 PWM 输入测量功能，内部完成：

- GPIO 输入初始化
- 通用定时器基础配置
- PWM Input 模式配置
- 触发源选择
- 从模式复位配置
- 中断配置
- 定时器启动

---

### 9.2 `BSP_TIM_IRQHandler()`

```c
void BSP_TIM_IRQHandler(bsp_generaltimer_t timer_id);
```

这是通用定时器的公共中断处理函数。  
在实际 IRQHandler 中调用，例如：

```c
void TIM4_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER4);
}
```

该函数内部负责：

- 读取 CCR1 / CCR2
- 判断周期值与高电平值对应关系
- 计算频率
- 计算占空比
- 更新 `g_pwm_update` 标志

---

## 10. 数据结构说明

### 10.1 `bsp_generaltimer_config_t`

用于配置定时器基础参数：

- `prescaler`：预分频值
- `counter_mode`：计数模式
- `period`：ARR 自动重装值
- `clock_div`：CKD 时钟分频
- `repetition_cnt`：重复计数器（接口兼容保留）

---

### 10.2 `bsp_tim_pwm_input_config_t`

用于配置 PWM 输入参数：

- `input_channel`：输入源通道
- `ic_polarity`：捕获极性
- `ic_prescaler`：捕获预分频
- `ic_filter`：输入滤波器

---

### 10.3 `bsp_tim_ic_result_t`

用于保存测量结果：

- `g_pwm_period`：周期计数值
- `g_pwm_pulse_width`：高电平脉宽计数值
- `g_pwm_duty`：占空比
- `g_pwm_freq`：频率
- `g_pwm_update`：更新标志

---

## 11. 中断说明

在 `stm32f10x_it.c` 中，当前已接入以下定时器中断：

```c
void TIM2_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER2);
}

void TIM3_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER3);
}

void TIM4_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER4);
}

void TIM5_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER5);
}
```

这意味着：

- 真实中断入口在 `stm32f10x_it.c`
- 实际业务处理逻辑统一放在 `bsp_general_timer.c`

这种写法的优点是：

- 中断入口清晰
- 功能逻辑集中
- 易于扩展和维护

---

## 12. 容易误解的地方

### 12.1 `TIMx_IRQHandler` 不等于“更新中断专用”

虽然函数名是：

```c
TIM4_IRQHandler()
```

但当前实际处理中断来源是：

```c
TIM_IT_CC1 / TIM_IT_CC2
```

也就是 **捕获比较中断**，用于 PWM 输入测量。

这里容易让人误以为是“普通更新中断”，其实不是。

---

### 12.2 选择 CH1/CH2 输入时，CCR1/CCR2 的含义会变化

代码中有如下逻辑：

```c
if(pwm_channel == BSP_TIM_PWM_INPUT_CHANNEL1){
    res->g_pwm_period      = TIM_GetCapture1(hw->timer);
    res->g_pwm_pulse_width = TIM_GetCapture2(hw->timer);
}else{
    res->g_pwm_period      = TIM_GetCapture2(hw->timer);
    res->g_pwm_pulse_width = TIM_GetCapture1(hw->timer);
}
```

这不是写反了，而是 PWM Input 模式的正常行为。  
因为底层会自动配对两个输入捕获寄存器协同测量。

---

### 12.3 `clock_div` 不是 PSC

很多人容易混淆：

- `TIM_ClockDivision`
- `TIM_Prescaler`

实际上：

- `PSC` 决定计数器时钟频率
- `clock_div` 对应 `CKD`，主要影响数字滤波相关采样时钟

所以频率计算时真正关键的是：

```c
uint32_t timer_freq = CLK_FREQ / (hw->timer->PSC + 1);
```

---

### 12.4 `period_cnt` 和 `high_cnt` 是“计数值”而不是直接时间

输出中的：

- `period_cnt`
- `high_cnt`

本质上都是定时器 tick 数，不是直接的 us / ms / s。

是否能直接换算为时间，取决于当前计数频率。

在当前配置下：

```text
1 count = 0.5 us
```

---

### 12.5 `volatile` 必不可少

结果结构体成员使用了 `volatile`：

```c
volatile uint32_t ...
volatile float ...
```

原因是：

- 数据在中断里更新
- 主循环里读取

如果没有 `volatile`，主循环可能读取不到中断更新后的新值。

---

## 13. 使用方法

### 13.1 硬件连接

请确保：

1. STM32F10x 开发板正常供电
2. 串口已连接到上位机
3. 外部 PWM 信号接到 `PB7`
4. PWM 电平兼容 STM32 IO

---

### 13.2 软件运行步骤

1. 编译并下载程序
2. 打开串口助手
3. 配置 USART1 对应的串口参数
4. 向 PB7 输入 PWM 信号
5. 观察串口输出结果

---

## 14. 配置修改示例

### 14.1 改为 TIM4_CH1 输入

将：

```c
.input_channel = BSP_TIM_PWM_INPUT_CHANNEL2
```

改为：

```c
.input_channel = BSP_TIM_PWM_INPUT_CHANNEL1
```

此时输入引脚变为：

```text
PB6 (TIM4_CH1)
```

---

### 14.2 改为 TIM3 测量

将：

```c
BSP_GeneralTIM_Config(BSP_GENERAL_TIMER4, &base_config, &pwm_config);
bsp_tim_ic_result_t *res = &result[BSP_GENERAL_TIMER4];
```

改为：

```c
BSP_GeneralTIM_Config(BSP_GENERAL_TIMER3, &base_config, &pwm_config);
bsp_tim_ic_result_t *res = &result[BSP_GENERAL_TIMER3];
```

如果仍使用 CH2，则输入引脚改为：

```text
PA7 (TIM3_CH2)
```

---

## 15. 常见问题排查

### 15.1 串口只有标题没有测量结果

如果只看到：

```text
25.通用定时器测量PWM
```

但没有后续数据，通常说明：

- PWM 信号未输入到正确引脚
- 定时器中断未触发
- NVIC 配置不正确
- 启动文件中断名称不匹配
- `g_pwm_update` 没有被置位

---

### 15.2 频率明显不对

重点检查：

1. `CLK_FREQ` 是否正确  
2. `prescaler` 是否和实际预期一致  
3. APB1 时钟配置是否变化  
4. PWM 输入源是否稳定  
5. 是否需要开启输入滤波

---

### 15.3 占空比不准

检查：

- 极性配置是否正确
- 输入信号是否有抖动
- 线路是否干扰较大
- 是否需要加滤波

---

## 16. 后续可扩展方向

本工程后续可以扩展为：

- 支持无输入超时清零
- 支持多路 PWM 同时测量
- 支持平均值滤波
- 支持 OLED / LCD 实时显示
- 支持 FreeRTOS 任务化读取
- 支持更完整的异常与错误码处理

---

## 17. 当前示例一句话总结

**本工程演示了如何使用 STM32F10x 的通用定时器 TIM2/TIM3/TIM4/TIM5，通过 PWM Input 模式测量外部 PWM 信号的频率和占空比，并通过串口输出结果。**