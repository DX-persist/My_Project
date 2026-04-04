# STM32 ADC 电压采集示例

## 项目简介

本项目基于 **STM32F10x 标准外设库**，实现了一个简单的 **ADC 电压采集示例**。  
程序通过初始化 ADC 外设、配置对应 GPIO 为模拟输入模式，并使用 **中断 + 主循环轮询标志位** 的方式完成模拟电压采样。

当前示例中使用的是：

- **ADC3 的通道 11**
- 对应引脚：**PC1**
- 采样结果通过 **USART1 串口** 每 1 秒输出一次

输出内容包括：

- ADC 原始采样值
- 根据参考电压换算后的电压值

---

## 功能特点

- 支持 **ADC1 / ADC2 / ADC3** 多个通道的统一封装
- 使用枚举统一管理 ADC 通道资源
- 支持：
  - 单通道采样
  - 连续转换模式
  - 软件触发
  - 转换完成中断（EOC）
- 通过 `convert_flag[]` 在中断与主循环之间传递“转换完成”状态
- 可通过串口实时查看采样结果

---

## 工程文件说明

### `bsp_adc.h`

ADC 模块头文件，主要包含：

- ADC 通道枚举 `bsp_adc_channel_t`
- ADC 配置结构体 `bsp_adc_config_t`
- 对外接口声明：
  - `BSP_ADC_Init()`
  - `BSP_ADC_PriorityGroupConfig()`
  - `BSP_ADC_GetValue()`
  - `BSP_ADC_IRQHandler()`

### `bsp_adc.c`

ADC 模块实现文件，主要功能包括：

- 建立逻辑通道与硬件资源映射表
- GPIO 与 ADC 时钟初始化
- ADC 参数配置
- 中断优先级配置
- ADC 校准与启动
- ADC 转换结果读取
- ADC 转换完成中断处理

### `stm32f10x_it.c`

中断服务函数文件，当前与 ADC 相关的部分包括：

- `ADC1_2_IRQHandler()`：处理 ADC1 / ADC2 共用中断
- `ADC3_IRQHandler()`：处理 ADC3 中断

### `main.c`

主程序文件，完成以下功能：

- 初始化 LED、时基、串口、ADC
- 启动 ADC3 通道 11 进行连续采样
- 在主循环中读取采样值并换算电压
- 通过串口每秒打印一次采样结果

---

## 硬件资源说明

本示例使用如下 ADC 硬件资源：

| 外设 | 通道       | GPIO | 说明         |
| ---- | ---------- | ---- | ------------ |
| ADC3 | Channel 11 | PC1  | 模拟输入采样 |

串口输出使用：

| 外设   | 用途                  |
| ------ | --------------------- |
| USART1 | 打印 ADC 采样值与电压 |

---

## ADC 工作流程

### 1. 初始化流程

在 `main()` 中调用：

```c
BSP_ADC_PriorityGroupConfig();
BSP_ADC_Init(BSP_ADC3_Channel11, &config);
```

初始化过程包含：

1. 配置 NVIC 优先级分组
2. 使能 GPIO 和 ADC 时钟
3. 将 PC1 配置为模拟输入
4. 初始化 ADC 工作参数
5. 配置规则组通道顺序和采样时间
6. 使能 ADC 转换完成中断
7. 进行 ADC 校准
8. 软件触发启动 ADC 转换

---

### 2. 中断处理流程

ADC 完成一次转换后，会进入对应中断服务函数：

```c
void ADC3_IRQHandler(void)
{
    BSP_ADC_IRQHandler(ADC3);
}
```

在 `BSP_ADC_IRQHandler()` 中：

- 判断是否发生 EOC（转换完成）中断
- 若完成，则将 `convert_flag[2]` 置为 `1`
- 清除中断挂起标志

---

### 3. 主循环处理流程

主循环检测到转换完成标志后：

```c
adc_value = BSP_ADC_GetValue(BSP_ADC3_Channel11);
voltage = adc_value / 4095.0 * 3.3;
```

然后每隔 1 秒通过串口输出：

```c
printf("adc_value = %d voltage: %.2f\r\n", adc_value, voltage);
```

---

## ADC 配置参数说明

当前示例中 ADC 配置如下：

```c
bsp_adc_config_t config = {
    .adc_mode = ADC_Mode_Independent,
    .scan_mode = DISABLE,
    .continuous_mode = ENABLE,
    .trigger_source = ADC_ExternalTrigConv_None,
    .align = ADC_DataAlign_Right,
    .channel_count = 1,
    .sample_time = ADC_SampleTime_55Cycles5,
};
```

参数含义如下：

| 参数              | 当前值                      | 说明                         |
| ----------------- | --------------------------- | ---------------------------- |
| `adc_mode`        | `ADC_Mode_Independent`      | ADC 独立模式                 |
| `scan_mode`       | `DISABLE`                   | 禁止扫描模式，仅单通道采样   |
| `continuous_mode` | `ENABLE`                    | 开启连续转换                 |
| `trigger_source`  | `ADC_ExternalTrigConv_None` | 不使用外部触发，采用软件触发 |
| `align`           | `ADC_DataAlign_Right`       | 数据右对齐                   |
| `channel_count`   | `1`                         | 规则组仅 1 个通道            |
| `sample_time`     | `ADC_SampleTime_55Cycles5`  | 采样时间为 55.5 周期         |

---

## 电压换算公式

ADC 为 12 位，满量程数值为：

```text
4095
```

若参考电压为 3.3V，则输入电压计算公式为：

```c
voltage = adc_value / 4095.0 * 3.3;
```

例如：

- `adc_value = 0`，则电压约为 `0.00V`
- `adc_value = 2048`，则电压约为 `1.65V`
- `adc_value = 4095`，则电压约为 `3.30V`

> 注意：该换算默认 ADC 参考电压为 3.3V，若硬件参考电压不同，需要修改公式中的 `3.3`。

---

## 中断标志说明

程序中定义了：

```c
volatile uint8_t convert_flag[3] = {0};
```

含义如下：

| 标志位            | 对应 ADC |
| ----------------- | -------- |
| `convert_flag[0]` | ADC1     |
| `convert_flag[1]` | ADC2     |
| `convert_flag[2]` | ADC3     |

当前示例使用的是 **ADC3_Channel11**，因此实际有效的完成标志为：

```c
convert_flag[2]
```

虽然主循环中同时判断了三个标志位，但这并不影响当前功能逻辑。

---

## 注意事项

### 1. 当前为单通道采样

当前程序只配置了一个规则组通道，因此读取到的是 ADC 当前最近一次转换结果。  
如果以后扩展为多通道扫描模式，则需要进一步设计多通道数据缓存机制。

### 2. ADC1 和 ADC2 共用中断入口

在 STM32F10x 中：

- ADC1 和 ADC2 共用 `ADC1_2_IRQHandler`
- ADC3 使用独立的 `ADC3_IRQHandler`

因此中断文件中分别调用对应的统一处理函数。

### 3. ADC 时钟分频

代码中配置：

```c
RCC_ADCCLKConfig(RCC_PCLK2_Div8);
```

若 APB2 时钟为 72MHz，则 ADC 时钟为：

```text
72MHz / 8 = 9MHz
```

该值满足 STM32F10x ADC 的时钟要求。

### 4. 校准不可省略

ADC 在正式采样前进行了：

- 复位校准
- 启动校准

这一步有助于提高转换结果准确性。

---

## 串口输出示例

程序运行后，串口可能输出如下内容：

```text
21.ADC采集电压
adc_value = 1234 voltage: 0.99
adc_value = 2048 voltage: 1.65
adc_value = 3072 voltage: 2.48
```

---

## 使用方法

### 1. 硬件连接

将待测模拟电压信号接入：

- **PC1（ADC3_IN11）**

确保输入电压范围不超过芯片 ADC 允许范围，一般为：

- `0V ~ 3.3V`

### 2. 编译下载

将工程编译并下载到 STM32F10x 开发板。

### 3. 打开串口工具

配置串口参数与 `USART1` 初始化一致，即可观察 ADC 输出结果。

---

## 可扩展方向

本工程可以在此基础上继续扩展：

- 多通道扫描采样
- DMA 搬运 ADC 数据
- 定时器触发 ADC 采样
- ADC 平均滤波 / 中值滤波
- 电压、温度、传感器数据采集封装

---

## 总结

本项目实现了一个结构清晰的 STM32 ADC 采样示例，适合用于：

- 学习 STM32F10x ADC 外设初始化流程
- 理解 ADC 中断采样机制
- 掌握模拟量到数字量的电压换算方法
- 为后续多通道采样、DMA 采样等功能打基础