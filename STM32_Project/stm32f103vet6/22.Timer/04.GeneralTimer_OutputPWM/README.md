# STM32F10x 通用定时器 PWM 输出示例

## 1. 项目简介

本项目基于 **STM32F10x 标准外设库（StdPeriph Library）**，实现了一个使用 **通用定时器 TIM2 / TIM3 / TIM4 / TIM5** 进行 **PWM 输出 / 输出比较** 的示例工程。

这版代码的核心目标是：

- 使用通用定时器输出 PWM 波形
- 支持 CH1 / CH2 / CH3 / CH4 四个输出通道
- 支持多种输出比较模式
- 支持 CCR 预装载，避免占空比切换时出现毛刺
- 预留更新中断计数能力

当前示例中实际演示的是：

- 使用 **TIM3**
- 使用 **TIM3_CH1**
- 输出引脚为 **PA6**
- PWM 模式为 **PWM1**
- 串口输出使用 **USART1**

---

## 2. 功能特性

- 支持通用定时器：
  - `TIM2`
  - `TIM3`
  - `TIM4`
  - `TIM5`
- 支持输出通道：
  - `CH1`
  - `CH2`
  - `CH3`
  - `CH4`
- 支持输出比较模式：
  - `Timing`
  - `Active`
  - `Inactive`
  - `Toggle`
  - `PWM1`
  - `PWM2`
- 支持：
  - 主输出使能
  - 输出极性设置
  - CCR 预装载
  - GPIO 复用推挽输出
- 预留更新中断计数接口

---

## 3. 工程涉及文件

### 核心文件

- `bsp_general_timer.h`  
  通用定时器 PWM / 输出比较模块头文件，定义：
  - 定时器枚举
  - 输出通道枚举
  - 输出模式枚举
  - 基础配置结构体
  - OC 配置结构体
  - 对外接口函数

- `bsp_general_timer.c`  
  通用定时器 PWM / 输出比较模块实现文件，完成：
  - GPIO 初始化
  - 定时器基础参数初始化
  - OC/PWM 配置
  - CCR 预装载配置
  - 定时器启动

- `main.c`  
  示例入口文件，完成：
  - 系统初始化
  - TIM3 PWM 输出配置
  - 串口初始化

---

## 4. 工作原理概述

### 4.1 PWM 输出原理

PWM 输出本质上依赖：

- 计数器从 0 计数到 `ARR`
- 当计数值与 `CCR` 比较时，根据配置模式改变输出状态

在 PWM1 模式下，通常可以理解为：

- `CNT < CCR` 时输出有效电平
- `CNT >= CCR` 时输出无效电平

因此：

- `ARR` 决定周期
- `CCR` 决定占空比
- `PSC` 决定计数时钟频率

---

### 4.2 PWM 频率计算

设：

- 定时器输入时钟为 `TIM_CLK`
- 预分频值为 `PSC`
- 自动重装值为 `ARR`

则计数频率为：

```text
counter_clk = TIM_CLK / (PSC + 1)
```

PWM 频率为：

```text
pwm_freq = counter_clk / (ARR + 1)
```

---

### 4.3 占空比计算

若输出模式为 PWM1，则占空比近似为：

```text
duty = CCR / (ARR + 1)
```

---

## 5. 当前示例配置

在 `main.c` 中当前配置如下。

### 5.1 时基配置

```c
bsp_generaltimer_config_t base_config = {
    .prescaler = 7200 - 1,
    .counter_mode = BSP_TIM_COUNTER_MODE_UP,
    .period = 100 - 1,
    .clock_div = BSP_TIM_CLOCK_DIV_1,
    .repetition_cnt = 0
};
```

### 5.2 输出比较配置

```c
bsp_tim_oc_config_t oc_config = {
    .oc_channel = BSP_TIM_OC_CHANNEL1,
    .oc_mode = BSP_TIM_OC_MODE_PWM1,
    .oc_output_state = BSP_TIM_OUTPUT_STATE_ENABLE,
    .oc_output_nstate = BSP_TIM_OUTPUT_NSTATE_DISABLE,
    .oc_polarity = BSP_TIM_OC_POLARITY_HIGH,
    .oc_npolarity = BSP_TIM_OC_NPOLARITY_HIGH,
    .oc_idle_state = BSP_TIM_OC_IDLE_STATE_SET,
    .oc_nidle_state = BSP_TIM_OC_NIDLE_STATE_SET,
    .oc_ccr_value = 50
};
```

---

## 6. 当前 PWM 参数计算

假设定时器输入时钟为：

```text
72 MHz
```

并且：

- `PSC = 7200 - 1`
- `ARR = 100 - 1`
- `CCR = 50`

### 6.1 计数频率

```text
72 MHz / 7200 = 10 kHz
```

### 6.2 PWM 频率

```text
10 kHz / 100 = 100 Hz
```

### 6.3 占空比

```text
50 / 100 = 50%
```

因此当前示例输出为：

- **PWM 频率：100 Hz**
- **占空比：50%**

---

## 7. 当前硬件映射

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

## 8. 当前示例实际使用的输出引脚

当前初始化的是：

```c
BSP_GeneralTIM_Config(BSP_GENERAL_TIMER3, &base_config, &oc_config);
```

并且：

```c
.oc_channel = BSP_TIM_OC_CHANNEL1
```

因此当前实际使用的是：

- **通用定时器：TIM3**
- **输出通道：CH1**
- **输出引脚：PA6**

也就是说，PWM 波形会输出到：

```text
PA6 (TIM3_CH1)
```

---

## 9. 串口输出说明

当前代码使用：

```c
BSP_USART_Config(BSP_USART1);
BSP_USART_Stdio(BSP_USART1);
```

说明程序输出使用：

- **USART1**

程序启动后会输出：

```text
25.基本定时器定时
```

### 说明

虽然字符串中写的是“基本定时器定时”，但当前工程实际做的是：

- **通用定时器 TIM3**
- **PWM 输出**

这条打印字符串更像是旧示例残留文本，不代表当前工程真实功能。

---

## 10. 关键接口说明

### 10.1 `BSP_GeneralTIM_Config()`

```c
void BSP_GeneralTIM_Config(bsp_generaltimer_t timer_id,
                           bsp_generaltimer_config_t *base_config,
                           bsp_tim_oc_config_t *oc_config);
```

用于初始化通用定时器 PWM / 输出比较功能，内部完成：

- GPIO 初始化
- 定时器基础时基配置
- OC/PWM 参数配置
- CCR 预装载配置
- 启动定时器

---

### 10.2 `BSP_TIM_IRQHandler()`

```c
void BSP_TIM_IRQHandler(bsp_generaltimer_t timer_id);
```

这是通用定时器的公共中断处理函数。  
当前主要用于处理：

- **更新中断 TIM_IT_Update**

并对更新事件计数：

```c
generaltimer_cnt[timer_id]++;
```

### 注意

当前 `bsp_general_timer.c` 中默认把更新中断使能代码放在：

```c
#if 0
...
#endif
```

里，因此默认情况下这个中断处理逻辑通常不会真正触发，除非手动打开相关代码。

---

## 11. 数据结构说明

### 11.1 `bsp_generaltimer_config_t`

用于配置通用定时器时基参数：

- `prescaler`：预分频值
- `counter_mode`：计数模式
- `period`：ARR 自动重装值
- `clock_div`：CKD 时钟分频
- `repetition_cnt`：重复计数器保留字段

---

### 11.2 `bsp_tim_oc_config_t`

用于配置输出比较/PWM 参数：

- `oc_channel`：输出通道
- `oc_mode`：输出模式
- `oc_output_state`：主输出使能
- `oc_output_nstate`：互补输出使能保留字段
- `oc_ccr_value`：比较值/占空比值
- `oc_polarity`：主输出极性
- `oc_npolarity`：互补输出极性保留字段
- `oc_idle_state`：主输出空闲状态
- `oc_nidle_state`：互补输出空闲状态保留字段

---

## 12. 容易误解的地方

### 12.1 这是通用定时器 PWM 输出，不是输入捕获

这版代码的功能是：

- 输出比较
- PWM 输出
- GPIO 波形输出

不是输入捕获，也不是测脉宽/测频率代码。

---

### 12.2 普通定时器没有互补输出

接口里保留了这些字段：

- `oc_output_nstate`
- `oc_npolarity`
- `oc_nidle_state`

这样做有利于和高级定时器接口风格统一，但要明确：

- `TIM2 / TIM3 / TIM4 / TIM5` **没有互补输出引脚**
- 所以这些字段对普通定时器一般没有实际硬件意义

---

### 12.3 `clock_div` 不是 PSC

这个地方很容易搞混。

真正影响 PWM 频率的是：

- `prescaler`
- `period`

不是 `clock_div`。

`clock_div` 对应的是 CKD，主要和采样/滤波相关。

---

### 12.4 `CCR` 决定占空比，不直接决定频率

当前配置里：

- `ARR = 100`
- `CCR = 50`

所以占空比是 50%。

如果改 `CCR`，主要改的是占空比；  
如果改 `PSC` 或 `ARR`，主要改的是频率。

---

### 12.5 当前默认不会进入更新中断

虽然写了：

```c
BSP_TIM_IRQHandler(...)
```

但由于更新中断使能代码被放在：

```c
#if 0
...
#endif
```

中，所以默认一般不会进入该中断，除非手动打开那部分代码。

---

## 13. 使用方法

### 13.1 硬件连接

请确保：

1. STM32F10x 开发板正常供电
2. 串口已连接到上位机
3. 示波器或逻辑分析仪连接到 `PA6`
4. GPIO 复用功能未被其他外设冲突占用

---

### 13.2 软件运行步骤

1. 编译并下载程序
2. 打开串口助手
3. 打开示波器或逻辑分析仪
4. 观察 `PA6` 上的 PWM 波形
5. 核对频率与占空比是否符合预期

---

## 14. 配置修改示例

### 14.1 改为 TIM3_CH2 输出

将：

```c
.oc_channel = BSP_TIM_OC_CHANNEL1
```

改为：

```c
.oc_channel = BSP_TIM_OC_CHANNEL2
```

则输出引脚改为：

```text
PA7 (TIM3_CH2)
```

---

### 14.2 改占空比

将：

```c
.oc_ccr_value = 50
```

改成：

```c
.oc_ccr_value = 25
```

则占空比变为：

```text
25 / 100 = 25%
```

---

### 14.3 改 PWM 频率

例如将：

```c
.prescaler = 7200 - 1,
.period = 100 - 1
```

改为：

```c
.prescaler = 720 - 1,
.period = 100 - 1
```

则：

- 计数频率变为 `100 kHz`
- PWM 频率变为 `1 kHz`

---

## 15. 常见问题排查

### 15.1 串口有输出，但引脚没有波形

优先检查：

- 是否真的初始化了对应定时器
- GPIO 是否配置成复用推挽输出
- 示波器是否接对引脚
- 定时器时钟是否已开启

---

### 15.2 波形频率不对

检查：

1. `PSC` 是否正确  
2. `ARR` 是否正确  
3. 时钟树是否真的是 72MHz  
4. 是否把 `period` 理解错成频率了  

---

### 15.3 占空比不对

检查：

- `CCR` 是否正确
- `ARR` 是否正确
- 输出模式是否为 `PWM1` 或 `PWM2`
- 输出极性是否符合预期

---

### 15.4 切换占空比时波形有毛刺

检查是否启用了 CCR 预装载：

```c
BSP_TIM_OCxPreloadConfig(..., TIM_OCPreload_Enable, ...)
```

当前代码已经启用该功能。

---

## 16. 后续可扩展方向

本工程后续可以扩展为：

- 增加动态修改占空比接口
- 增加多通道同时 PWM 输出
- 增加中心对齐 PWM 示例
- 增加更新中断驱动的占空比渐变
- 增加 DMA 更新 CCR 的高级用法
- 增加频率和占空比运行时配置接口

---

## 17. 当前示例一句话总结

**本工程演示了如何使用 STM32F10x 的通用定时器 TIM2 / TIM3 / TIM4 / TIM5 配置 PWM / 输出比较功能，并通过 GPIO 输出普通 PWM 波形。**