# STM32 ADC + DMA 电压采集示例

## 项目简介

本项目基于 **STM32F10x 标准外设库**，实现了一个简单的 **ADC 结合 DMA 的电压采集示例**。  
程序通过初始化 ADC 外设、配置 GPIO 为模拟输入模式，并使用 **DMA 循环搬运** 的方式，将 ADC 转换结果自动写入内存变量 `adc_value`，主循环中再对其进行电压换算并通过串口周期性输出。

当前示例中使用的是：

- **ADC3 的通道 11**
- 对应引脚：**PC1**
- 通过 **DMA2 Channel5** 搬运 ADC3 数据
- 采样结果通过 **USART1 串口** 每 1 秒输出一次

输出内容包括：

- ADC 原始采样值
- 根据参考电压换算后的电压值

---

## 功能特点

- 支持 **ADC1 / ADC2 / ADC3** 多通道统一封装
- 通过枚举统一管理 ADC 通道资源
- 支持：
  - 单通道采样
  - 连续转换模式
  - 软件触发
  - DMA 循环搬运
- ADC 转换结果自动写入全局变量 `adc_value`
- 主循环无需等待中断标志即可直接读取最新采样值
- 可通过串口实时观察电压变化

---

## 工程结构说明

### `bsp_adc.h`

ADC 模块头文件，主要包含：

- ADC 通道枚举 `bsp_adc_channel_t`
- ADC 配置结构体 `bsp_adc_config_t`
- ADC 对外接口声明
- 全局采样变量 `adc_value`

### `bsp_adc.c`

ADC 模块实现文件，主要功能包括：

- 建立逻辑 ADC 通道到硬件资源的映射表
- GPIO 模拟输入初始化
- ADC 参数初始化
- DMA 参数配置与初始化
- ADC 校准和启动
- ADC 数据读取接口

### `main.c`

主程序文件，完成以下功能：

- 初始化 LED、时基、串口和 ADC
- 启动 ADC3 Channel11 连续采样
- 使用 DMA 自动搬运 ADC 数据到 `adc_value`
- 在主循环中完成电压换算
- 每隔 1 秒打印一次采样结果

---

## 硬件资源说明

当前示例使用的关键硬件资源如下：

| 模块  | 资源           | 说明               |
| ----- | -------------- | ------------------ |
| ADC   | ADC3 Channel11 | 模拟输入采样通道   |
| GPIO  | PC1            | ADC3_IN11 输入引脚 |
| DMA   | DMA2 Channel5  | ADC3 数据搬运通道  |
| USART | USART1         | 串口打印输出       |

---

## ADC 与 DMA 工作原理

## 1. 基本思路

本工程中 ADC 工作在 **连续转换模式**，并开启 DMA 请求。  
每当 ADC 完成一次转换后，DMA 会自动把 ADC 数据寄存器 `DR` 中的值搬运到内存变量 `adc_value` 中。

因此主循环中只需要直接读取 `adc_value`，无需手动判断转换完成标志。

---

## 2. 数据流向

数据流向如下：

```text
PC1 模拟电压
   ↓
ADC3 Channel11
   ↓
ADC3->DR
   ↓
DMA2 Channel5
   ↓
adc_value
```

---

## 3. 当前实现的特点

当前 DMA 配置为：

- 外设地址固定：ADC 数据寄存器
- 内存地址固定：`adc_value`
- 传输方向：外设到内存
- 数据宽度：16 位
- 缓冲区大小：1
- 循环模式：开启

这意味着：

- 每次 ADC 转换完成后，DMA 都会把最新结果覆盖到 `adc_value`
- `adc_value` 中始终保存“最近一次采样值”
- 非常适合单通道连续采样场景

---

## 主要代码说明

## 1. ADC 配置结构体

`main()` 中的 ADC 配置如下：

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

各字段含义如下：

| 字段              | 当前值                      | 说明                   |
| ----------------- | --------------------------- | ---------------------- |
| `adc_mode`        | `ADC_Mode_Independent`      | ADC 独立工作模式       |
| `scan_mode`       | `DISABLE`                   | 禁止扫描，仅采样单通道 |
| `continuous_mode` | `ENABLE`                    | 连续转换模式           |
| `trigger_source`  | `ADC_ExternalTrigConv_None` | 软件触发               |
| `align`           | `ADC_DataAlign_Right`       | 结果右对齐             |
| `channel_count`   | `1`                         | 规则组只有 1 个通道    |
| `sample_time`     | `ADC_SampleTime_55Cycles5`  | 采样时间 55.5 周期     |

---

## 2. DMA 配置说明

在 `BSP_ADC_Init()` 中，DMA 配置如下：

```c
dma_config.periph_addr = (uint32_t)&(hw->adc->DR);
dma_config.memory_addr = (uint32_t)&adc_value;
dma_config.dir = DIR_Periph_SRC;
dma_config.buffer_size = 1;
dma_config.periph_inc = PeripheralInc_Disable;
dma_config.memory_inc = MemoryInc_Disable;
dma_config.periph_data_size = PeripheralDataSize_HalfWord;
dma_config.memory_data_size = MemoryDataSize_HalfWord;
dma_config.mode = DMA_Mode_Cir;
dma_config.priority = DMA_Priority_M;
dma_config.m2m = DMA_M2M_DISABLE;
```

说明如下：

| 字段               | 说明                         |
| ------------------ | ---------------------------- |
| `periph_addr`      | ADC 数据寄存器地址           |
| `memory_addr`      | 内存目标地址，即 `adc_value` |
| `dir`              | 外设作为数据源               |
| `buffer_size`      | 仅搬运 1 个数据              |
| `periph_inc`       | 外设地址不自增               |
| `memory_inc`       | 内存地址不自增               |
| `periph_data_size` | 外设数据宽度为半字           |
| `memory_data_size` | 内存数据宽度为半字           |
| `mode`             | 循环模式                     |
| `priority`         | 中优先级                     |
| `m2m`              | 禁止内存到内存               |

---

## 3. ADC 初始化流程

`BSP_ADC_Init()` 完成了以下操作：

1. 参数合法性检查
2. 使能 GPIO 和 ADC 时钟
3. 将 ADC 输入引脚配置为模拟输入
4. 初始化 ADC 工作参数
5. 初始化 DMA 参数
6. 配置 ADC 时钟分频
7. 配置规则组通道顺序和采样时间
8. 使能 ADC 的 DMA 功能
9. 使能 ADC
10. 执行 ADC 校准
11. 软件触发启动 ADC 转换

---

## 4. 主循环逻辑

主循环中逻辑非常简单：

```c
while(1){
    voltage = adc_value / 4095.0 * 3.3;
    if(BSP_GetTick() - last_tick >= 1000){
        last_tick = BSP_GetTick();
        printf("adc_value = %d voltage: %.2f\r\n", adc_value, voltage);
    }
}
```

含义如下：

- `adc_value` 由 DMA 自动更新
- 主循环只负责读取当前值并进行电压换算
- 每隔 1000ms 输出一次

---

## 电压换算公式

ADC 为 12 位精度，满量程对应：

```text
4095
```

若参考电压为 3.3V，则输入电压的换算公式为：

```c
voltage = adc_value / 4095.0 * 3.3;
```

例如：

| adc_value | 电压近似值 |
| --------- | ---------- |
| 0         | 0.00V      |
| 2048      | 1.65V      |
| 4095      | 3.30V      |

> 注意：若系统实际参考电压不是 3.3V，需要将公式中的 `3.3` 替换为真实参考电压。

---

## 使用方法

## 1. 硬件连接

将待测模拟电压信号接入：

- **PC1（ADC3_IN11）**

请确保输入电压在芯片 ADC 允许范围内，通常为：

- **0V ~ 3.3V**

---

## 2. 编译下载

将工程编译并下载到 STM32F10x 开发板。

---

## 3. 打开串口工具

打开串口助手，并将参数配置为与 `USART1` 初始化一致，即可观察输出结果。

串口输出示例：

```text
21.ADC采集电压
adc_value = 1234 voltage: 0.99
adc_value = 2048 voltage: 1.65
adc_value = 3072 voltage: 2.48
```

---

## 注意事项

## 1. 当前实现是单通道 DMA 采样

当前 DMA 只把一个 ADC 转换结果搬运到单个变量 `adc_value`，适用于：

- 单通道
- 连续采样
- 实时读取当前值

如果要扩展为多通道扫描采样，通常需要：

- 开启扫描模式
- 配置多个规则组通道
- 将 DMA 目标改为数组缓冲区

---

## 2. 当前代码已不依赖 ADC 中断标志读取数据

相比之前使用 `convert_flag[]` 的方式，现在通过 DMA 自动搬运，主循环中可直接读取 `adc_value`。  
因此当前设计更偏向“ADC + DMA 自动采样”模式。

不过需要注意：

- `bsp_adc.h` 中仍保留了 `BSP_ADC_PriorityGroupConfig()` 和 `BSP_ADC_IRQHandler()` 的声明
- 但你提供的当前 `bsp_adc.c` 代码中并未实现这两个函数
- 如果工程里仍然引用这两个接口，链接时可能报错

---

## 3. ADC 时钟分频

代码中配置：

```c
RCC_ADCCLKConfig(RCC_PCLK2_Div8);
```

若 APB2 时钟为 72MHz，则 ADC 时钟为：

```text
72MHz / 8 = 9MHz
```

该值满足 STM32F10x ADC 时钟要求。

---

## 4. 代码中存在一个明显笔误

你给出的 `bsp_adc.c` 结构体定义中有一行单独的：

```c
a
```

即这里：

```c
uint8_t irq_sub_prio;
a
bsp_dma_channel_t dma_channel;
```

这会导致编译报错。应删除这一行多余字符后再编译。

---

## 5. ADC2 的映射表里未填写 DMA 通道

在当前硬件映射表中：

- ADC1 项填写了 `dma_channel = BSP_DMA1_Channel1`
- ADC3 项填写了 `dma_channel = BSP_DMA2_Channel5`
- 但 ADC2 项未见 `dma_channel` 初始化

如果后续调用 `BSP_ADC_Init()` 初始化 ADC2 通道，DMA 通道值可能不确定，需要补全映射配置。

---

## 6. `BSP_ADC_GetValue()` 与 DMA 方式并存

当前既保留了：

```c
uint16_t BSP_ADC_GetValue(bsp_adc_channel_t adc_channel_id)
```

又通过 DMA 持续更新 `adc_value`。

这两种方式都能拿到采样值，但当前主程序实际使用的是：

- **DMA 更新的 `adc_value`**

而不是主动调用 `BSP_ADC_GetValue()`。

---

## 可扩展方向

这个工程后续可以继续扩展为：

- 多通道 ADC + DMA 数组采样
- 定时器触发 ADC 采样
- ADC 平均滤波 / 滑动平均滤波
- 电池电压检测
- 传感器模拟量采集
- DMA 传输完成中断处理

---

## 总结

本工程实现了一个 **STM32F10x ADC + DMA 单通道连续采样** 示例，特点是：

- ADC 持续转换
- DMA 自动搬运
- 主循环直接读取最新值
- 串口定时输出电压结果

它很适合作为以下内容的入门示例：

- STM32 ADC 基本配置
- DMA 外设到内存搬运
- 连续采样场景设计
- 电压采样与换算处理