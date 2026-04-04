# STM32 定时器实验：PWM 输出与 PWM 测量

## 📌 项目简介

本项目基于 STM32F10x 标准外设库，实现了一个典型定时器应用：

- 使用 **通用定时器 TIM3** 产生多路 PWM 输出
- 使用 **高级定时器 TIM1** 测量 PWM 输入信号
- 使用 **USART1** 打印测量结果

---

## 🧩 功能概述

| 模块    | 功能                      |
| ------- | ------------------------- |
| TIM3    | 输出 4 路 PWM             |
| TIM1    | 测量 PWM（频率 + 占空比） |
| USART1  | 打印结果                  |
| SysTick | 提供时间基准              |

---

## 🔌 硬件连接说明

⚠️ 必须连接，否则无法测量！

TIM3_CH1  →  TIM1_CH1

---

## 📍 推荐引脚

| 功能      | 引脚 |
| --------- | ---- |
| TIM3_CH1  | PA6  |
| TIM3_CH2  | PA7  |
| TIM3_CH3  | PB0  |
| TIM3_CH4  | PB1  |
| TIM1_CH1  | PA8  |
| USART1_TX | PA9  |
| USART1_RX | PA10 |

---

## ⚙️ 定时器参数

### TIM3（PWM输出）

- PSC = 7200 - 1
- ARR = 100 - 1

👉 PWM频率 ≈ 100 Hz

---

### 占空比

- CH1 = 30%
- CH2 = 40%
- CH3 = 60%
- CH4 = 70%

---

### TIM1（PWM输入测量）

- PSC = 72 - 1
- 分辨率 ≈ 1μs

---

## 📐 公式

PWM频率：
f = f_tim / ((PSC+1)*(ARR+1))

占空比：
Duty = CCR/(ARR+1)*100%

输入测量：
f = f_tim / ((PSC+1)*period_cnt)
Duty = high_cnt/period_cnt*100%

---

## 📊 实验现象

串口输出示例：

freq = 100 Hz, duty = 30.00%, period_cnt = 10000, high_cnt = 3000

---

## 🔁 程序流程

初始化 → PWM输出 → PWM测量 → 串口打印

---

## ⚠️ 注意事项

1. 必须接线
2. printf需支持浮点
3. 注意中断同步
4. 默认72MHz时钟

---

## 🧠 总结

TIM3 产生PWM，TIM1测量PWM，USART打印结果