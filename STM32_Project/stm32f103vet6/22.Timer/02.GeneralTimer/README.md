# STM32F10x 通用定时器（基础时基 + 更新中断）示例

## 📌 项目简介

本示例演示如何使用 **STM32F10x 通用定时器（TIM2 / TIM3 / TIM4 / TIM5）**
实现**基础时基定时功能（更新中断）**，并通过软件计数实现周期控制。

虽然通用定时器具备 PWM、输入捕获等功能，但本工程仅使用：

* 时基计数（Time Base）
* 更新事件（Update Event）
* 更新中断（Update Interrupt）

👉 本质上：
**把通用定时器当作“带中断的基础定时器”来使用**

---

## ⚙️ 功能说明

* 使用 **TIM4**
* 配置定时周期为 **1ms**
* 每 1000 次更新中断（约 1 秒）：

  * 翻转一次 LED

---

## 🧠 工作原理

### 1️⃣ 定时器频率计算

系统时钟：72 MHz

配置参数：

```c
.prescaler = 72 - 1
.period    = 1000 - 1
```

计算过程：

* 定时器计数频率：

```
72 MHz / 72 = 1 MHz
```

* 单次更新周期：

```
1000 / 1 MHz = 1 ms
```

---

### 2️⃣ 中断计数机制

每次 **更新中断（Update Event）**：

```c
generaltimer_cnt[timer_id]++;
```

主循环中：

```c
if(generaltimer_cnt[TIM4] == 1000)
```

👉 表示约 1 秒时间到达

---

## 📂 文件结构

```
├── bsp_general_timer.h     // 通用定时器接口定义
├── bsp_general_timer.c     // 定时器驱动实现
├── main.c                  // 示例应用
```

---

## 🔧 核心接口说明

### 初始化定时器

```c
void BSP_BaseTIM_Init(
    bsp_generaltimer_t timer_id,
    bsp_generaltimer_config_t *config
);
```

功能：

* 开启 TIMx 时钟
* 配置 PSC / ARR / CounterMode
* 使能更新中断
* 配置 NVIC
* 启动定时器

---

### 中断处理函数

```c
void BSP_TIM_IRQHandler(bsp_generaltimer_t timer_id);
```

作用：

* 判断更新中断
* 软件计数 +1
* 清除中断标志

---

## 🚀 使用方法

### 1️⃣ 配置定时器参数

```c
bsp_generaltimer_config_t config = {
    .prescaler = 72 - 1,
    .counter_mode = BSP_TIM_COUNTER_MODE_UP,
    .period = 1000 - 1,
    .clock_div = BSP_TIM_CLOCK_DIV_1,
    .repetition_cnt = 0
};
```

---

### 2️⃣ 初始化定时器

```c
BSP_BaseTIM_Init(BSP_GENERAL_TIMER4, &config);
```

---

### 3️⃣ 在中断文件中调用

```c
void TIM4_IRQHandler(void)
{
    BSP_TIM_IRQHandler(BSP_GENERAL_TIMER4);
}
```

---

### 4️⃣ 主循环使用

```c
if(generaltimer_cnt[BSP_GENERAL_TIMER4] == 1000){
    generaltimer_cnt[BSP_GENERAL_TIMER4] = 0;
    BSP_LED_Toggle(LED_BLUE);
}
```

---

## ⚠️ 注意事项

### ⚠️ 1. 初始化时必须清更新标志

```c
TIM_ClearFlag(hw->timer, TIM_IT_Update);
```

原因：

* PSC 和 ARR 使用预装载机制
* 初始化时会自动产生一次更新事件
* 若不清除，会导致程序刚启动就进入一次中断

---

### ⚠️ 2. `clock_div` 不是定时核心参数

真正决定时间的是：

* `prescaler`
* `period`

`clock_div` 只是影响采样/滤波，不影响主计数频率。

---

### ⚠️ 3. 软件计数 ≠ 硬件计数器

```c
generaltimer_cnt[]
```

表示：

👉 **更新中断次数**

不是：

👉 `TIMx->CNT` 当前计数值

---

### ⚠️ 4. 这不是 TIM6 / TIM7

虽然功能类似“基础定时器”，但实际使用的是：

* TIM2 / TIM3 / TIM4 / TIM5

👉 只是“功能上等价”，不是硬件类型相同

---

## 📊 适用场景

* 简单软件定时（ms / s 级）
* LED 闪烁控制
* 周期任务调度
* 定时触发逻辑
* 初学 STM32 定时器

---

## 🔁 可扩展方向

* 改为 PWM 输出
* 改为输入捕获测频/测脉宽
* 多定时器同步调度
* RTOS Tick 替代方案
* 精确时间基准（替代 SysTick）

---

## 🧩 总结

本工程展示了一个非常经典的用法：

> ⭐ 用通用定时器 + 更新中断，实现基础定时功能

特点：

* 简单稳定
* 易扩展
* 易理解
* 工程实用性强

