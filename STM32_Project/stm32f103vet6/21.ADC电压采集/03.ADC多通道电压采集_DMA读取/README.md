# STM32 多通道 ADC + DMA 采集示例

## 1. 项目简介

本项目基于 **STM32F10x 标准外设库**，实现了一个 **多通道 ADC + DMA 循环采集** 的示例程序。

当前工程的核心特点：

- 使用 **ADC1** 进行规则组多通道采样
- 采样通道为 **PC0 ~ PC5**
- 对应 ADC 通道为 **ADC_Channel_10 ~ ADC_Channel_15**
- 使用 **DMA** 将 ADC 转换结果循环搬运到内存数组
- 主循环中直接读取 DMA 缓冲区中的最新采样值
- 通过 **USART1** 周期性打印每个通道的原始值和换算电压

该方案适合：

- 多路模拟量采集
- 传感器阵列输入
- 电压巡检
- 后续扩展为滤波、报警、阈值检测等应用

---

## 2. 工程功能概述

本工程实现了以下功能：

1. 定义 ADC 设备抽象层（ADC1 / ADC2 / ADC3）
2. 使用 `bsp_adc_input_t` 描述每个输入通道的：
   - ADC 通道号
   - GPIO 引脚
   - GPIO 时钟
3. 使用 `bsp_adc_group_config_t` 描述一组 ADC 采样参数：
   - 使用哪个 ADC 外设
   - 采样哪些通道
   - 一共采多少路
   - DMA 缓冲区地址
   - 采样时间
   - 触发源
   - 数据对齐方式
   - 是否连续采样
4. 初始化 ADC 和 DMA
5. 自动循环采样 6 路输入
6. 将采样值换算成电压并输出到串口

---

## 3. 文件说明

### `bsp_adc.h`

ADC 模块头文件，包含：

- ADC 设备编号枚举 `bsp_adc_dev_id_t`
- ADC 输入描述结构体 `bsp_adc_input_t`
- ADC 组配置结构体 `bsp_adc_group_config_t`
- ADC 初始化和读取接口声明

### `bsp_adc.c`

ADC 模块实现文件，主要实现：

- ADC 硬件资源映射表
- GPIO 模拟输入初始化
- ADC 规则组配置
- DMA 参数配置
- ADC 启动、校准和软件触发
- 从 DMA 缓冲区读取指定通道数据

### `main.c`

主程序文件，完成：

- 定义采样通道列表
- 定义 DMA 接收缓冲区
- 组织 ADC 组配置参数
- 调用初始化接口启动多通道采样
- 周期性打印每路通道的采样值和电压值

---

## 4. 当前硬件配置

本示例中使用的采样通道如下：

| 序号 | GPIO 引脚 | ADC 通道       |
| ---- | --------- | -------------- |
| 0    | PC0       | ADC_Channel_10 |
| 1    | PC1       | ADC_Channel_11 |
| 2    | PC2       | ADC_Channel_12 |
| 3    | PC3       | ADC_Channel_13 |
| 4    | PC4       | ADC_Channel_14 |
| 5    | PC5       | ADC_Channel_15 |

当前 ADC 配置为：

| 项目     | 配置值        |
| -------- | ------------- |
| ADC 外设 | ADC1          |
| ADC 模式 | 独立模式      |
| 通道数量 | 6             |
| 触发方式 | 软件触发      |
| 连续模式 | 使能          |
| 数据对齐 | 右对齐        |
| 采样时间 | 55.5 周期     |
| DMA 通道 | DMA1_Channel1 |
| DMA 模式 | 循环模式      |

---

## 5. 软件架构设计

## 5.1 ADC 设备抽象

工程使用如下枚举标识 ADC 外设：

```c
typedef enum{
    BSP_ADC_DEV1 = 0,
    BSP_ADC_DEV2,
    BSP_ADC_DEV3,
    BSP_ADC_DEV_MAX
}bsp_adc_dev_id_t;
```

这样做的好处是：

- 上层不直接依赖 `ADC1`、`ADC2`、`ADC3` 宏
- 后续更容易做统一封装
- 可以通过查表自动找到对应 ADC 的时钟、中断、DMA 信息

---

## 5.2 输入通道描述

每一路输入使用 `bsp_adc_input_t` 描述：

```c
typedef struct{
    uint8_t adc_channel;
    bsp_gpio_t adc_gpio;
}bsp_adc_input_t;
```

它描述了：

- 该输入对应哪个 ADC 通道
- 该通道对应哪个 GPIO 引脚
- 该 GPIO 端口的时钟是什么

这样可以把“通道信息”从驱动代码中独立出来，让应用层自由组合。

---

## 5.3 ADC 组配置

一组 ADC 采样参数由 `bsp_adc_group_config_t` 描述：

```c
typedef struct{
    bsp_adc_dev_id_t dev_id;
    const bsp_adc_input_t *input_list;
    uint32_t adc_mode;
    uint8_t channel_count;
    uint16_t *buffer;
    uint32_t sample_time;
    uint32_t trigger_source;
    uint32_t align;
    FunctionalState continuous_mode;
}bsp_adc_group_config_t;
```

该结构体可以描述：

- 使用哪个 ADC
- 扫描多少个通道
- 每个通道的输入定义
- 采样结果存放到哪里
- ADC 的工作模式和采样参数

这是一种典型的“配置驱动型”写法，便于复用和扩展。

---

## 6. ADC + DMA 工作流程

本工程的运行流程如下：

```text
模拟输入信号
   ↓
GPIO 模拟输入模式
   ↓
ADC1 规则组多通道扫描
   ↓
ADC1 数据寄存器 DR
   ↓
DMA1_Channel1
   ↓
adc_value[6]
   ↓
主循环读取并换算电压
   ↓
USART1 打印输出
```

---

## 7. 初始化流程说明

`BSP_ADC_InitGroup()` 是整个 ADC 组采样的核心初始化函数。

其主要流程如下：

### 7.1 参数检查

函数首先检查以下内容是否有效：

- 配置指针不为空
- ADC 设备编号合法
- 输入通道列表不为空
- 缓冲区不为空
- 通道数大于 0

若任一条件不满足，函数直接返回。

---

### 7.2 打开 ADC 时钟

根据 `cfg->dev_id` 查表得到对应硬件资源，并开启 ADC 外设时钟。

例如当前配置为 ADC1，则开启：

```c
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
```

---

### 7.3 初始化每个输入引脚

程序遍历 `input_list` 中的每个通道，分别完成：

- GPIO 端口时钟使能
- GPIO 模式配置为 `GPIO_Mode_AIN`

这样 PC0 ~ PC5 都会被配置为模拟输入。

---

### 7.4 配置 ADC 基本参数

ADC 配置包括：

- 工作模式
- 是否扫描
- 是否连续转换
- 外部触发源
- 数据对齐
- 规则组通道数

其中：

```c
ADC_InitStruct.ADC_ScanConvMode = (cfg->channel_count > 1) ? ENABLE : DISABLE;
```

表示：

- 单通道时关闭扫描
- 多通道时自动开启扫描

这也是当前多通道采样必须启用的一项配置。

---

### 7.5 配置 ADC 时钟

代码中固定设置：

```c
RCC_ADCCLKConfig(RCC_PCLK2_Div8);
```

若 APB2 = 72MHz，则 ADC 时钟为：

```text
72MHz / 8 = 9MHz
```

这满足 STM32F10x ADC 的时钟限制要求。

---

### 7.6 配置规则组顺序

程序遍历每个输入通道，并按数组顺序配置规则组采样顺序：

```c
ADC_RegularChannelConfig(hw->adc, cfg->input_list[i].adc_channel, i + 1, cfg->sample_time);
```

因此当前转换顺序是：

1. ADC_Channel_10
2. ADC_Channel_11
3. ADC_Channel_12
4. ADC_Channel_13
5. ADC_Channel_14
6. ADC_Channel_15

DMA 也会按这个顺序把数据放进缓冲区。

---

### 7.7 配置 DMA

若当前 ADC 支持 DMA 通道，则驱动会配置 DMA：

- 外设地址：ADC 数据寄存器 `DR`
- 内存地址：`cfg->buffer`
- 数据方向：外设到内存
- 缓冲区长度：`channel_count`
- 外设地址不自增
- 内存地址按是否多通道决定是否自增
- 循环模式开启

当前多通道场景下：

```c
dma_config.memory_inc = MemoryInc_Enable;
```

因此 DMA 会依次写入：

```c
adc_value[0]
adc_value[1]
adc_value[2]
adc_value[3]
adc_value[4]
adc_value[5]
```

并不断循环覆盖更新。

---

### 7.8 使能 ADC 和校准

初始化完成后，程序会：

1. 使能 ADC
2. 复位校准
3. 启动校准
4. 等待校准完成
5. 软件触发开始转换

这是 STM32F10x ADC 的标准启动流程。

---

## 8. 数据读取方式

驱动提供了读取接口：

```c
uint16_t BSP_ADC_GetValue(const bsp_adc_group_config_t *cfg, uint8_t index);
```

其本质是从 DMA 缓冲区中返回指定下标的数据：

```c
return cfg->buffer[index];
```

因此：

- `index = 0` 表示第 1 路输入
- `index = 1` 表示第 2 路输入
- 以此类推

当前 `main.c` 中直接使用了数组 `adc_value[i]`，没有调用这个接口，但两者本质一致。

---

## 9. 主程序说明

在 `main()` 中，程序首先定义了 6 路 ADC 输入：

```c
static const bsp_adc_input_t my_adc_inputs[] = {
    {ADC_Channel_10, {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC}},
    {ADC_Channel_11, {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC}},
    {ADC_Channel_12, {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC}},
    {ADC_Channel_13, {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC}},
    {ADC_Channel_14, {GPIOC, GPIO_Pin_4, RCC_APB2Periph_GPIOC}},
    {ADC_Channel_15, {GPIOC, GPIO_Pin_5, RCC_APB2Periph_GPIOC}},
};
```

然后定义 DMA 缓冲区：

```c
static uint16_t adc_value[6];
```

再定义 ADC 组配置：

```c
static const bsp_adc_group_config_t adc_cfg = {
    .dev_id         = BSP_ADC_DEV1,
    .input_list     = my_adc_inputs,
    .adc_mode       = ADC_Mode_Independent,
    .channel_count  = 6,
    .buffer         = adc_value,
    .sample_time    = ADC_SampleTime_55Cycles5,
    .trigger_source = ADC_ExternalTrigConv_None,
    .align          = ADC_DataAlign_Right,
    .continuous_mode = ENABLE,
};
```

最后调用：

```c
BSP_ADC_InitGroup(&adc_cfg);
```

完成 ADC + DMA 初始化。

---

## 10. 电压换算公式

主循环中使用如下公式换算电压：

```c
voltage[i] = adc_value[i] / 4095.0 * 3.3;
```

说明如下：

- `4095` 是 12 位 ADC 的满量程值
- `3.3` 是参考电压

举例：

| ADC 原始值 | 电压     |
| ---------- | -------- |
| 0          | 0.00V    |
| 2048       | 约 1.65V |
| 4095       | 约 3.30V |

> 注意：若实际参考电压不是 3.3V，需要将公式中的 `3.3` 改为真实参考电压。

---

## 11. 串口输出示例

程序运行后，每隔 2 秒输出一次：

```text
21.ADC采集电压
PC0: adc_value = 1234 voltage: 0.99
PC1: adc_value = 2048 voltage: 1.65
PC2: adc_value = 3000 voltage: 2.42
PC3: adc_value = 1500 voltage: 1.21
PC4: adc_value = 4095 voltage: 3.30
PC5: adc_value = 0 voltage: 0.00
```

---

## 12. 设计优点

这版代码相比“单通道硬编码”写法有明显优势：

### 12.1 通道管理更灵活

只需要修改 `my_adc_inputs[]` 和 `channel_count`，就能增减采样通道。

### 12.2 驱动复用性更高

ADC 初始化函数不再绑定某个固定通道，而是支持“按组配置”。

### 12.3 更适合多通道采集

DMA 直接搬运整组采样结果，效率高，CPU 负担小。

### 12.4 更便于扩展

后续可以继续扩展：

- 多组 ADC 配置
- 定时器触发采样
- DMA 中断处理
- 均值滤波 / 中值滤波
- 传感器封装

---

## 13. 注意事项

### 13.1 ADC2 当前未配置 DMA 通道

硬件映射表中：

- ADC1 使用 `BSP_DMA1_Channel1`
- ADC3 使用 `BSP_DMA2_Channel5`
- ADC2 设置为 `BSP_DMA_NONE`

因此当前若使用 ADC2，则不会启用 DMA 自动搬运。

---

### 13.2 多通道采样必须开启扫描模式

当前代码已经自动处理：

```c
(cfg->channel_count > 1) ? ENABLE : DISABLE
```

所以不要手动关闭多通道扫描逻辑。

---

### 13.3 缓冲区长度必须和通道数一致

`cfg->buffer` 的容量必须至少等于 `channel_count`，否则 DMA 写入可能越界。

当前示例中：

- 通道数为 6
- 缓冲区长度也为 6

二者匹配。

---

### 13.4 打印语句中的 “PC%d” 只是显示编号

当前代码：

```c
printf("PC%d: adc_value = %d voltage: %.2f\r\n", i, adc_value[i], voltage[i]);
```

这里的 `i` 是数组下标，因此会输出：

- `PC0`
- `PC1`
- `PC2`
- ...

这和真实 GPIO 恰好一致是因为本例通道刚好从 `PC0 ~ PC5` 连续排列。  
如果以后换成别的引脚组合，建议打印真实引脚描述，而不是直接打印下标。

---

### 13.5 `bsp_adc.c` 中全局变量 `volatile uint16_t adc_value = 0;` 当前没有实际使用

因为现在真正使用的是 `main.c` 中的局部静态数组：

```c
static uint16_t adc_value[6];
```

而 `bsp_adc.c` 里的这个全局单值变量与当前组采样实现不一致，建议删除，避免命名混淆。

---

## 14. 可扩展方向

本工程后续可以进一步扩展为：

- 定时器触发 ADC 周期采样
- ADC1 + ADC3 并行采集
- DMA 采样完成中断
- 多通道电压平均滤波
- NTC、光敏、电位器等传感器采集
- 数据上传到串口屏、LCD 或上位机

---

## 15. 总结

本项目实现了一个结构清晰的 **STM32F10x 多通道 ADC + DMA 采集框架**。

你当前这版代码已经具备以下特点：

- 通道配置解耦
- 驱动复用性好
- 支持多通道扫描
- 使用 DMA 自动搬运结果
- 主循环逻辑简单直观

非常适合作为以下内容的基础模板：

- 多路模拟信号采集
- 传感器输入框架
- ADC + DMA 学习示例
- 后续中大型嵌入式项目中的采样模块封装