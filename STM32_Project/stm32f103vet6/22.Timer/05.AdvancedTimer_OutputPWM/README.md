# STM32F10x 高级定时器 PWM / 互补输出 / BDTR 配置示例

## 1. 项目简介

本项目基于 **STM32F10x 标准外设库（StdPeriph Library）**，实现了一个使用 **高级定时器 TIM1 / TIM8** 输出 PWM 波形，并支持：

- 主输出
- 互补输出
- BDTR（Break and Dead-Time）配置
- 刹车输入 BKIN
- 死区时间插入
- 自动输出控制

这类配置通常用于：

- 电机控制
- 半桥 / 全桥驱动
- 功率开关控制
- 需要刹车保护的 PWM 场景

当前示例中实际演示的是：

- 使用 **TIM1**
- 使用 **CH2 主输出**
- 使用 **CH2N 互补输出**
- 配置 **PWM1 模式**
- 使能 **Break / Dead-Time**
- 串口输出使用 **USART1**

---

## 2. 功能特性

- 支持高级定时器：
  - `TIM1`
  - `TIM8`
- 支持输出通道：
  - `CH1`
  - `CH2`
  - `CH3`
  - `CH4`
- 支持互补输出：
  - `CH1N`
  - `CH2N`
  - `CH3N`
- 支持输出比较模式：
  - Timing
  - Active
  - Inactive
  - Toggle
  - PWM1
  - PWM2
- 支持 BDTR 相关功能：
  - 死区时间
  - 刹车输入
  - 刹车极性
  - 自动输出
  - 锁定级别
  - OSSI / OSSR 状态控制

---

## 3. 工程涉及文件

### 核心文件

- `bsp_advanced_timer.h`  
  高级定时器 BSP 头文件，定义：
  - 定时器枚举
  - 输出比较通道枚举
  - 输出模式枚举
  - BDTR 相关枚举
  - 基础配置结构体
  - 输出比较配置结构体
  - BDTR 配置结构体

- `bsp_advanced_timer.c`  
  高级定时器 BSP 实现文件，完成：
  - GPIO 初始化
  - 定时器基础参数初始化
  - PWM / 输出比较初始化
  - BDTR 配置
  - 主输出使能
  - 定时器启动

- `stm32f10x_it.c`  
  中断服务函数文件，包含：
  - Cortex-M3 异常处理
  - USART 中断入口
  - TIM1 / TIM8 更新中断入口

- `main.c`  
  示例入口文件，完成：
  - 系统初始化
  - TIM1 PWM / 互补输出配置
  - 串口初始化

---

## 4. 工作原理概述

### 4.1 高级定时器 PWM 输出

高级定时器与普通定时器相比，除了能输出普通 PWM 外，还支持：

- 互补输出
- 死区时间插入
- Break 刹车保护
- 主输出使能控制

这使得 TIM1 / TIM8 很适合驱动：

- MOSFET 半桥
- MOSFET 全桥
- 三相逆变
- BLDC / PMSM 电机驱动

---

### 4.2 主输出与互补输出

以 CH2 为例：

- `CH2`：主输出
- `CH2N`：互补输出

在功率驱动中，这两个信号常分别驱动上桥臂和下桥臂。  
为了避免上下桥臂同时导通造成直通，需要在两者切换时插入**死区时间**。

---

### 4.3 BDTR 的作用

BDTR 是 **Break and Dead-Time Register**，主要用于高级定时器输出保护与驱动控制。

本项目中用到的 BDTR 相关功能包括：

- **Dead-Time**：插入死区时间
- **Break**：刹车输入保护
- **Break Polarity**：设置刹车触发电平
- **Automatic Output**：故障解除后是否自动恢复输出
- **OSSI / OSSR**：失效输出状态管理
- **Lock Level**：锁定关键配置，防止运行中被误改

---

## 5. 当前示例配置

在 `main.c` 中，当前配置如下。

### 5.1 时基配置

```c
bsp_advancedtimer_config_t base_config = {
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
    .oc_channel = BSP_TIM_OC_CHANNEL2,
    .oc_mode = BSP_TIM_OC_MODE_PWM1,
    .oc_output_state = BSP_TIM_OUTPUT_STATE_ENABLE,
    .oc_output_nstate = BSP_TIM_OUTPUT_NSTATE_ENABLE,
    .oc_polarity = BSP_TIM_OC_POLARITY_HIGH,
    .oc_npolarity = BSP_TIM_OC_NPOLARITY_LOW,
    .oc_idle_state = BSP_TIM_OC_IDLE_STATE_RESET,
    .oc_nidle_state = BSP_TIM_OC_NIDLE_STATE_SET,
    .oc_ccr_value = 50
};
```

### 5.3 BDTR 配置

```c
bsp_tim_bdtr_config_t bdtr_config = {
    .bdtr_ossi_state = BSP_TIM_BDTR_OSSI_STATE_ENABLE,
    .bdtr_ossr_state = BSP_TIM_BDTR_OSSR_STATE_ENABLE,
    .bdtr_lock_level = BSP_TIM_BDTR_LOCK_LEVEL_OFF,
    .bdtr_break = BSP_TIM_BDTR_BREAK_ENABLE,
    .bdtr_break_polarity = BSP_TIM_BDTR_BREAK_POLARITY_LOW,
    .bdtr_auto_output = BSP_TIM_BDTR_AUTO_OUTPUT_ENABLE,
    .bdtr_dead_time = 11
};
```

---

## 6. 当前 PWM 参数计算

假设 TIM1 输入时钟为：

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

### 6.2 PWM 周期频率

```text
10 kHz / 100 = 100 Hz
```

### 6.3 占空比

```text
CCR / ARR计数周期 = 50 / 100 = 50%
```

所以当前示例的 PWM 输出参数为：

- **PWM 频率：100 Hz**
- **占空比：50%**

---

## 7. 当前硬件映射

在 `bsp_advanced_timer.c` 中定义了如下高级定时器引脚映射。

### TIM1

- `TIM1_CH1`  → `PA8`
- `TIM1_CH1N` → `PB13`
- `TIM1_CH2`  → `PA9`
- `TIM1_CH2N` → `PB14`
- `TIM1_CH3`  → `PA10`
- `TIM1_CH3N` → `PB15`
- `TIM1_CH4`  → `PA11`
- `TIM1_BKIN` → `PB12`

### TIM8

- `TIM8_CH1`  → `PC6`
- `TIM8_CH1N` → `PA7`
- `TIM8_CH2`  → `PC7`
- `TIM8_CH2N` → `PB0`
- `TIM8_CH3`  → `PC8`
- `TIM8_CH3N` → `PB1`
- `TIM8_CH4`  → `PC9`
- `TIM8_BKIN` → `PA6`

---

## 8. 当前示例实际使用的输出引脚

当前初始化的是：

```c
BSP_AdvancedTimer_Config(BSP_ADVANCED_TIMER1, &base_config, &oc_config, &bdtr_config);
```

并且：

```c
.oc_channel = BSP_TIM_OC_CHANNEL2
```

因此当前实际使用的是：

- **高级定时器：TIM1**
- **主输出通道：CH2**
- **互补输出通道：CH2N**
- **刹车输入：BKIN**

即：

- 主输出引脚：`PA9`
- 互补输出引脚：`PB14`
- 刹车输入引脚：`PB12`

---

## 9. 串口输出说明

当前代码中：

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

虽然字符串里写的是“基本定时器定时”，但当前工程实际上配置的是：

- **高级定时器 TIM1**
- **PWM / 互补输出**
- **BDTR 功能**

这条打印字符串更像是旧示例残留文本，不代表当前工程真实功能。

---

## 10. 关键接口说明

### 10.1 `BSP_AdvancedTimer_Config()`

```c
void BSP_AdvancedTimer_Config(bsp_advancedtimer_t timer_id,
                              bsp_advancedtimer_config_t *base_config,
                              bsp_tim_oc_config_t *oc_config,
                              bsp_tim_bdtr_config_t *bdtr_config);
```

用于初始化高级定时器 PWM/互补输出功能，内部完成：

- GPIO 初始化
- 定时器基础参数配置
- OC/PWM 配置
- CCR 预装载配置
- BDTR 配置
- 主输出使能
- 定时器启动

---

### 10.2 `BSP_TIM_IRQHandler()`

```c
void BSP_TIM_IRQHandler(bsp_advancedtimer_t timer_id);
```

这是高级定时器的公共中断处理函数。  
当前主要用于处理：

- **更新中断 TIM_IT_Update**

并对更新事件计数：

```c
advancedtimer_cnt[timer_id]++;
```

### 注意

当前 `bsp_advanced_timer.c` 里默认把更新中断使能代码放在：

```c
#if 0
...
#endif
```

中，因此默认情况下这个中断处理逻辑通常不会真正触发，除非手动打开相关代码。

---

## 11. 数据结构说明

### 11.1 `bsp_advancedtimer_config_t`

用于配置高级定时器时基参数：

- `prescaler`：预分频值
- `counter_mode`：计数模式
- `period`：ARR 自动重装值
- `clock_div`：CKD 时钟分频
- `repetition_cnt`：重复计数器

---

### 11.2 `bsp_tim_oc_config_t`

用于配置输出比较/PWM 参数：

- `oc_channel`：输出通道
- `oc_mode`：输出模式
- `oc_output_state`：主输出使能
- `oc_output_nstate`：互补输出使能
- `oc_ccr_value`：比较值/占空比值
- `oc_polarity`：主输出极性
- `oc_npolarity`：互补输出极性
- `oc_idle_state`：主输出空闲状态
- `oc_nidle_state`：互补输出空闲状态

---

### 11.3 `bsp_tim_bdtr_config_t`

用于配置 BDTR 参数：

- `bdtr_ossr_state`：运行模式失效输出状态
- `bdtr_ossi_state`：空闲模式失效输出状态
- `bdtr_lock_level`：锁定级别
- `bdtr_dead_time`：死区时间编码值
- `bdtr_break`：刹车使能
- `bdtr_break_polarity`：刹车极性
- `bdtr_auto_output`：自动输出使能

---

## 12. 中断说明

在 `stm32f10x_it.c` 中，当前工程使用了以下高级定时器中断入口。

### TIM1 更新中断

```c
void TIM1_UP_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_ADVANCED_TIMER1);
}
```

### TIM8 更新中断

```c
void TIM8_UP_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_ADVANCED_TIMER8);
}
```

### 需要注意

虽然中断入口已经写好，但 `bsp_advanced_timer.c` 中默认并没有真正打开更新中断配置代码，所以当前这两个 IRQ 通常不会触发。

---

## 13. 容易误解的地方

### 13.1 这不是输入捕获工程

这版代码和前面“测脉宽”“测频率/占空比”的几版不同。  
当前工程是：

- **PWM 输出**
- **互补输出**
- **BDTR 配置**
- **刹车保护**

不是输入捕获，也不是 PWM Input 模式。

---

### 13.2 CH4 没有互补输出

虽然枚举里有：

```c
BSP_TIM_OC_CHANNEL4
```

但高级定时器中：

- `CH1`、`CH2`、`CH3` 有互补输出 `CHxN`
- `CH4` 没有 `CH4N`

所以如果使用 CH4，就不能指望有互补输出引脚。

---

### 13.3 `BSP_TIM_GPIO_Init()` 中 CH4 有空指针风险

当前代码中：

```c
const bsp_gpio_t *chn_gpio = BSP_TIM_GetCHNGPIO(hw, oc_channel);
uint32_t enable_clk = ch_gpio->rcc_clk | chn_gpio->rcc_clk | hw->bkin_gpio.rcc_clk;
```

如果 `oc_channel = BSP_TIM_OC_CHANNEL4`，那么 `chn_gpio` 会是 `NULL`，这里就会有空指针访问风险。

所以目前代码**并不适合直接无修改地切到 CH4**。

---

### 13.4 `BSP_TIM_OCxInit()` 和 `BSP_TIM_OCxPreloadConfig()` 目前只支持 CH1~CH3

当前包装函数里没有补上：

- `TIM_OC4Init()`
- `TIM_OC4PreloadConfig()`

所以如果以后要完整支持 CH4 PWM 输出，建议把这两个分支加上。

---

### 13.5 `bdtr_dead_time = 11` 不是直接 11us

这里的：

```c
.bdtr_dead_time = 11
```

是 **DTG 编码值**，不是直接的时间单位。  
真实死区时间需要结合：

- 定时器时钟
- STM32 参考手册里的 DTG 编码规则

来换算。

---

### 13.6 当前默认不会进入更新中断

虽然写了：

```c
void TIM1_UP_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_ADVANCED_TIMER1);
}
```

但因为初始化中断代码被 `#if 0` 屏蔽了，所以默认不会进这个中断。  
如果需要中断统计更新事件，需要手动打开那部分代码。

---

## 14. 使用方法

### 14.1 硬件连接

请确保：

1. STM32F10x 开发板正常供电
2. 串口已连接到上位机
3. PWM 输出引脚已正确连接到示波器或负载
4. 如需验证互补输出，请同时观察主输出与互补输出
5. 如需验证 BKIN，请给 BKIN 输入正确的触发电平

---

### 14.2 当前示例引脚

当前示例建议观察以下引脚：

- `PA9`：TIM1_CH2 主输出
- `PB14`：TIM1_CH2N 互补输出
- `PB12`：TIM1_BKIN 刹车输入

---

### 14.3 软件运行步骤

1. 编译并下载程序
2. 打开串口助手
3. 打开示波器
4. 观察 `PA9` 和 `PB14` 波形
5. 如有条件，可测试 BKIN 输入对输出的影响

---

## 15. 配置修改示例

### 15.1 改为 TIM8 输出

将：

```c
BSP_AdvancedTimer_Config(BSP_ADVANCED_TIMER1, &base_config, &oc_config, &bdtr_config);
```

改为：

```c
BSP_AdvancedTimer_Config(BSP_ADVANCED_TIMER8, &base_config, &oc_config, &bdtr_config);
```

若仍使用 CH2，则输出引脚变为：

- `PC7`：TIM8_CH2
- `PB0`：TIM8_CH2N

---

### 15.2 改占空比

将：

```c
.oc_ccr_value = 50
```

改小或改大即可，例如：

```c
.oc_ccr_value = 25
```

表示占空比约为：

```text
25 / 100 = 25%
```

---

### 15.3 改 PWM 频率

当前频率由：

- `PSC`
- `ARR`

共同决定。

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

则计数频率变为 100kHz，PWM 频率变为 1kHz。

---

## 16. 常见问题排查

### 16.1 串口有输出，但引脚没有波形

优先检查：

- 是否真的使用了高级定时器 TIM1 / TIM8
- `TIM_CtrlPWMOutputs()` 是否被调用
- GPIO 是否配置成复用推挽输出
- 示波器是否接对引脚

---

### 16.2 主输出有波形，但互补输出没有

检查：

- 当前是否选的是 CH1~CH3
- `oc_output_nstate` 是否使能
- 互补引脚是否正确
- 是否受 BKIN 刹车影响

---

### 16.3 BKIN 一直把输出关掉

检查：

- BKIN 引脚电平是否符合配置的刹车极性
- 当前配置里是：
  - `bdtr_break = ENABLE`
  - `bdtr_break_polarity = LOW`
- 这意味着 **低电平会触发刹车**

如果 BKIN 被拉低，输出可能被关断。

---

### 16.4 波形频率或占空比不对

检查：

1. `PSC` 是否正确  
2. `ARR` 是否正确  
3. `CCR` 是否正确  
4. 时钟树是否真的是 72MHz  
5. 是否误以为 `period` 是频率本身

---

### 16.5 死区时间和预期不一致

检查：

- `bdtr_dead_time` 只是编码值
- 真实死区时间要查参考手册对应换算表
- 不要直接把数值当成 us

---

## 17. 后续可扩展方向

本工程后续可以扩展为：

- 完整支持 CH4 输出
- 增加中断方式动态修改占空比
- 增加刹车状态检测与串口输出
- 增加多通道同步 PWM 输出
- 增加中心对齐 PWM 模式示例
- 增加三相互补 PWM 输出模板
- 增加电机驱动应用示例

---

## 18. 当前示例一句话总结

**本工程演示了如何使用 STM32F10x 的高级定时器 TIM1 / TIM8，配置 PWM 主输出、互补输出以及 BDTR（死区、刹车、自动输出）功能，适合作为高级 PWM 驱动模板。**