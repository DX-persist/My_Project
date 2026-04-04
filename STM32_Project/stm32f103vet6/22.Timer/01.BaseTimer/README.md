# STM32F10x 基本定时器（TIM6 / TIM7）时基定时示例

## 📌 项目简介

本工程演示如何使用 **STM32F10x 基本定时器 TIM6 / TIM7**
实现**基础时基定时 + 更新中断**功能。

该示例是 STM32 定时器学习中最基础、最标准的一种用法：

> ⭐ 使用定时器产生固定周期中断，实现软件定时

---

## ⚙️ 功能说明

* 使用 **TIM7**
* 配置定时周期为 **1ms**
* 每 1000 次中断（约 1 秒）：

  * 翻转 LED

---

## 🧠 基本定时器特点

### ✔ 支持功能

* 时基计数（Time Base）
* 更新事件（Update Event）
* 更新中断（Update Interrupt）

### ❌ 不支持功能

* PWM 输出
* 输入捕获
* 输出比较
* 编码器模式

👉 因此 TIM6 / TIM7 非常适合做：

* 软件定时器
* 周期任务调度
* LED 闪烁
* 系统节拍源

---

## ⏱️ 定时计算原理

配置参数：

```c
.prescaler = 72 - 1
.period    = 1000 - 1
```

系统时钟：72 MHz

---

### 1️⃣ 定时器计数频率

```
72 MHz / 72 = 1 MHz
```

👉 1 个计数 = 1 微秒

---

### 2️⃣ 更新周期

```
1000 × 1us = 1 ms
```

👉 每 1ms 进入一次中断

---

### 3️⃣ 软件计数实现 1 秒

```c
if(basetimer_cnt[TIM7] == 1000)
```

👉 1000 × 1ms = 1 秒

---

## 📂 文件结构

```
├── bsp_base_timer.h     // 基本定时器接口定义
├── bsp_base_timer.c     // 定时器驱动实现
├── main.c               // 示例应用
```

---

## 🔧 核心接口说明

### 1️⃣ 初始化定时器

```c
void BSP_BaseTIM_Init(
    bsp_basetimer_t timer_id,
    bsp_basetimer_config_t *config
);
```

功能：

* 开启 TIM6 / TIM7 时钟
* 配置 PSC / ARR / CounterMode
* 清除初始化更新标志
* 使能更新中断
* 配置 NVIC
* 启动定时器

---

### 2️⃣ 中断处理函数

```c
void BSP_TIM_IRQHandler(bsp_basetimer_t timer_id);
```

作用：

* 判断更新中断
* 软件计数 +1
* 清除中断标志

---

## 🚀 使用步骤

### ① 配置定时器

```c
bsp_basetimer_config_t config = {
    .prescaler = 72 - 1,
    .counter_mode = BSP_TIM_COUNTER_MODE_UP,
    .period = 1000 - 1,
    .clock_div = BSP_TIM_CLOCK_DIV_1,
    .repetition_cnt = 0
};
```

---

### ② 初始化定时器

```c
BSP_BaseTIM_Init(BSP_BASE_TIMER7, &config);
```

---

### ③ 在中断文件中调用

```c
void TIM7_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_BASE_TIMER7);
}
```

---

### ④ 主循环逻辑

```c
if(basetimer_cnt[BSP_BASE_TIMER7] == 1000){
    basetimer_cnt[BSP_BASE_TIMER7] = 0;
    BSP_LED_Toggle(LED_RED);
}
```

---

## ⚠️ 注意事项

### ⚠️ 1. 初始化必须清除更新标志

```c
TIM_ClearFlag(hw->timer, TIM_IT_Update);
```

原因：

* PSC / ARR 预装载机制会触发一次更新事件
* 不清除会导致程序一启动就进一次中断

---

### ⚠️ 2. 关键参数只有两个

真正决定时间的是：

* `prescaler`
* `period`

❌ `clock_div` 基本不影响定时周期

---

### ⚠️ 3. 软件计数 ≠ 硬件 CNT

```c
basetimer_cnt[]
```

表示：

👉 更新中断次数

不是：

👉 TIMx->CNT 当前计数值

---

### ⚠️ 4. TIM6 / TIM7 专用定位

* 是“纯时基定时器”
* 最适合做系统节拍

👉 比 TIM2~5 更“干净”

---

## 📊 适用场景

* LED 周期闪烁
* 软件定时器
* 周期任务执行
* RTOS Tick 替代
* 基础定时实验

---

## 🔁 扩展方向

可以在此基础上扩展：

* 多定时器调度
* 软件任务调度器
* 精确延时系统
* 定时驱动通信协议

---

## 🧩 总结

本工程展示了 STM32 定时器中最基础、最重要的一种用法：

> ⭐ 基本定时器 + 更新中断 = 软件定时系统

特点：

* 结构简单
* 稳定可靠
* 易理解
* 工程通用性强

