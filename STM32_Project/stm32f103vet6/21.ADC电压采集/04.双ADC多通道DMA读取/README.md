# STM32 双 ADC 同步采集电压示例

## 1. 项目简介

本项目基于 **STM32F10x 标准外设库**，实现了一个 **双 ADC 同步规则组采样 + DMA 搬运** 的示例程序。

当前工程使用：

- **ADC1 + ADC2 同步规则通道模式**
- **ADC1** 采集 `PC0 ~ PC2`，对应 `ADC_Channel_10 ~ ADC_Channel_12`
- **ADC2** 采集 `PC3 ~ PC5`，对应 `ADC_Channel_13 ~ ADC_Channel_15`
- **DMA1_Channel1** 从 `ADC1->DR` 搬运 32 位数据到内存
- 每个 32 位数据中：
  - **低 16 位** 是 ADC1 的结果
  - **高 16 位** 是 ADC2 的结果

主循环中对 DMA 缓冲区进行拆包，再换算为电压值，并通过 **USART1** 每 2 秒打印一次结果。

---

## 2. 功能特点

本工程包含两种 ADC 采样模式封装：

### 2.1 单 ADC 分组采样

通过 `BSP_ADC_InitGroup()` 可实现：

- 单 ADC 单通道或多通道采样
- 多通道规则组扫描
- DMA 循环搬运到 `uint16_t` 缓冲区

### 2.2 双 ADC 同步采样

通过 `BSP_DualADC_InitGroup()` 可实现：

- ADC1 和 ADC2 同步采样
- 两个 ADC 分别采集一组通道
- 使用 DMA 搬运合并后的 32 位结果
- 适合更高吞吐量的多路模拟量采集场景

---

## 3. 文件说明

### `bsp_adc.h`

ADC 驱动头文件，定义了：

- ADC 设备编号枚举 `bsp_adc_dev_id_t`
- 单路输入描述结构体 `bsp_adc_input_t`
- 单 ADC 分组配置结构体 `bsp_adc_group_config_t`
- 双 ADC 同步配置结构体 `bsp_dual_adc_config_t`
- 对外接口：
  - `BSP_ADC_InitGroup()`
  - `BSP_ADC_GetValue()`
  - `BSP_DualADC_InitGroup()`

### `bsp_adc.c`

ADC 驱动实现文件，主要实现：

- ADC1 / ADC2 / ADC3 的硬件资源映射
- 单 ADC 分组初始化
- 双 ADC 同步初始化
- DMA 配置
- ADC 校准和启动
- 单 ADC 缓冲区取值接口

### `main.c`

主程序文件，完成：

- 初始化 LED、时基、串口
- 配置 ADC1/ADC2 各自采样的输入列表
- 初始化双 ADC 同步采样
- 从 DMA 结果中拆包
- 换算电压并周期性输出

---

## 4. 当前硬件配置

### ADC1 采样通道

| 序号 | GPIO 引脚 | ADC 通道       |
| ---- | --------- | -------------- |
| 0    | PC0       | ADC_Channel_10 |
| 1    | PC1       | ADC_Channel_11 |
| 2    | PC2       | ADC_Channel_12 |

### ADC2 采样通道

| 序号 | GPIO 引脚 | ADC 通道       |
| ---- | --------- | -------------- |
| 0    | PC3       | ADC_Channel_13 |
| 1    | PC4       | ADC_Channel_14 |
| 2    | PC5       | ADC_Channel_15 |

---

## 5. 双 ADC 同步采样原理

本工程使用的是：

```c
ADC_Mode_RegSimult
```

即：

- **规则通道同步模式**
- ADC1 作为主 ADC
- ADC2 作为从 ADC
- 当 ADC1 开始规则组转换时，ADC2 同步进行对应规则组转换

### 数据结果组织方式

在双 ADC 同步模式下，读取 `ADC1->DR` 时，得到的是一个 32 位数据：

- 低 16 位：ADC1 当前转换结果
- 高 16 位：ADC2 当前转换结果

因此 DMA 配置为：

- 外设地址：`ADC1->DR`
- 外设数据宽度：`Word`
- 内存数据宽度：`Word`

DMA 每搬运一个 32 位单元，就得到一对同步采样结果。

---

## 6. 数据流向

当前数据流如下：

```text
PC0~PC2  ---> ADC1 ---\
                        +--> ADC1->DR(32位合并结果) --> DMA1_Channel1 --> dual_adc_buf[]
PC3~PC5  ---> ADC2 ---/
```

其中：

- `dual_adc_buf[0]` 对应：
  - 低16位：PC0
  - 高16位：PC3
- `dual_adc_buf[1]` 对应：
  - 低16位：PC1
  - 高16位：PC4
- `dual_adc_buf[2]` 对应：
  - 低16位：PC2
  - 高16位：PC5

---

## 7. 主要结构体说明

## 7.1 `bsp_adc_input_t`

```c
typedef struct{
    uint8_t adc_channel;
    bsp_gpio_t adc_gpio;
}bsp_adc_input_t;
```

用于描述一个 ADC 输入，包括：

- ADC 通道号
- 对应 GPIO 引脚与 GPIO 时钟

---

## 7.2 `bsp_adc_group_config_t`

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

用于配置单 ADC 分组采样。

---

## 7.3 `bsp_dual_adc_config_t`

```c
typedef struct{
    const bsp_adc_input_t *adc1_inputs;
    uint8_t adc1_channel_count;
    const bsp_adc_input_t *adc2_inputs;
    uint8_t adc2_channel_count;
    uint32_t sample_time;
    uint32_t *buffer;
}bsp_dual_adc_config_t;
```

用于配置双 ADC 同步采样，描述：

- ADC1 的输入列表
- ADC2 的输入列表
- 两侧通道数
- 采样时间
- 双 ADC DMA 结果缓冲区

---

## 8. `BSP_DualADC_InitGroup()` 初始化流程

该函数是双 ADC 同步采样的核心，主要步骤如下：

### 8.1 参数检查

检查以下条件：

- `cfg` 不为空
- ADC1 输入列表不为空
- ADC2 输入列表不为空
- 缓冲区不为空
- ADC1 通道数不为 0
- ADC1 和 ADC2 通道数相同

若不满足则直接返回。

### 8.2 GPIO 初始化

分别遍历：

- `cfg->adc1_inputs`
- `cfg->adc2_inputs`

将所有输入引脚配置为：

```c
GPIO_Mode_AIN
```

### 8.3 开启 ADC1 / ADC2 时钟

```c
RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE);
```

并设置 ADC 时钟分频：

```c
RCC_ADCCLKConfig(RCC_PCLK2_Div8);
```

若 APB2 为 72MHz，则 ADC 时钟为：

```text
72MHz / 8 = 9MHz
```

### 8.4 配置 ADC1

ADC1 配置为：

- 同步规则组模式
- 扫描模式按通道数自动决定
- 连续转换模式
- 软件触发
- 右对齐
- 规则组通道数为 `ch_count`

并按顺序配置 ADC1 的规则组通道。

### 8.5 配置 ADC2

ADC2 配置基本与 ADC1 相同，但不直接接 DMA，不作为主触发源。  
它在同步模式下由 ADC1 驱动。

### 8.6 配置 DMA

DMA 只连接到 ADC1：

- 源地址：`ADC1->DR`
- 目标地址：`cfg->buffer`
- 数据宽度：32 位
- 缓冲区长度：`ch_count`
- 循环模式
- 高优先级

这样每个规则组位置都会对应一个 32 位结果。

### 8.7 校准并启动

依次完成：

- ADC1 使能与校准
- ADC2 使能与校准
- ADC1 软件触发开始采样

此后 ADC2 会自动与 ADC1 同步工作。

---

## 9. 主程序说明

在 `main()` 中，程序定义了两组输入：

### ADC1 输入列表

```c
static const bsp_adc_input_t adc1_inputs[] = {
    {ADC_Channel_10, {GPIOC, GPIO_Pin_0, RCC_APB2Periph_GPIOC}},
    {ADC_Channel_11, {GPIOC, GPIO_Pin_1, RCC_APB2Periph_GPIOC}},
    {ADC_Channel_12, {GPIOC, GPIO_Pin_2, RCC_APB2Periph_GPIOC}},
};
```

### ADC2 输入列表

```c
static const bsp_adc_input_t adc2_inputs[] = {
    {ADC_Channel_13, {GPIOC, GPIO_Pin_3, RCC_APB2Periph_GPIOC}},
    {ADC_Channel_14, {GPIOC, GPIO_Pin_4, RCC_APB2Periph_GPIOC}},
    {ADC_Channel_15, {GPIOC, GPIO_Pin_5, RCC_APB2Periph_GPIOC}},
};
```

### DMA 缓冲区

```c
static uint32_t dual_adc_buf[3];
```

注意它是 `uint32_t` 数组，因为每个元素保存一对 ADC 结果。

### 配置结构体

```c
static const bsp_dual_adc_config_t dual_cfg = {
    .adc1_inputs        = adc1_inputs,
    .adc1_channel_count = 3,
    .adc2_inputs        = adc2_inputs,
    .adc2_channel_count = 3,
    .sample_time        = ADC_SampleTime_55Cycles5,
    .buffer             = dual_adc_buf,
};
```

然后调用：

```c
BSP_DualADC_InitGroup(&dual_cfg);
```

完成双 ADC 同步采样初始化。

---

## 10. DMA 结果拆包方法

主循环中使用如下方式拆包：

```c
uint16_t adc1_val = (uint16_t)(dual_adc_buf[i] & 0xFFFF);
uint16_t adc2_val = (uint16_t)(dual_adc_buf[i] >> 16);
```

解释如下：

- `dual_adc_buf[i] & 0xFFFF`
  - 取低 16 位
  - 得到 ADC1 结果
- `dual_adc_buf[i] >> 16`
  - 右移 16 位
  - 得到 ADC2 结果

对应关系为：

| `dual_adc_buf[i]` | 低 16 位 | 高 16 位 |
| ----------------- | -------- | -------- |
| `dual_adc_buf[0]` | PC0      | PC3      |
| `dual_adc_buf[1]` | PC1      | PC4      |
| `dual_adc_buf[2]` | PC2      | PC5      |

---

## 11. 电压换算公式

电压换算公式为：

```c
voltage = adc_raw / 4095.0f * 3.3f;
```

其中：

- `4095` 是 12 位 ADC 满量程
- `3.3f` 是参考电压

举例：

| ADC 原始值 | 电压     |
| ---------- | -------- |
| 0          | 0.00V    |
| 2048       | 约 1.65V |
| 4095       | 约 3.30V |

---

## 12. 串口输出示例

程序运行后，每 2 秒输出一组结果，例如：

```text
22.双ADC同步采集电压
PC0: adc_value = 1234  voltage = 0.99V  |  PC3: adc_value = 2345  voltage = 1.89V
PC1: adc_value = 2048  voltage = 1.65V  |  PC4: adc_value = 3000  voltage = 2.42V
PC2: adc_value = 3500  voltage = 2.82V  |  PC5: adc_value = 4095  voltage = 3.30V
```

---

## 13. 关键注意事项

### 13.1 双 ADC 通道数必须一致

当前驱动要求：

```c
cfg->adc1_channel_count == cfg->adc2_channel_count
```

因为 DMA 每个 32 位单元都表示“一对同步结果”。

### 13.2 DMA 缓冲区必须是 `uint32_t`

不能用 `uint16_t`，因为每次搬运的是：

- ADC1 16 位
- ADC2 16 位
- 合成共 32 位

### 13.3 只有 ADC1 连接 DMA

这是双 ADC 同步模式的正常做法。  
ADC2 的结果通过 ADC1 的数据寄存器高 16 位一起读出。

### 13.4 当前 `ADC_CHANNEL_COUNT` 宏未在实现中使用

头文件中有：

```c
#define ADC_CHANNEL_COUNT 3
```

但当前实际初始化逻辑是使用配置结构体里的通道数，未直接使用这个宏。

### 13.5 `volatile uint16_t adc_value = 0;` 在当前实现中没有实际作用

`bsp_adc.c` 中保留了这个单值变量，但当前双 ADC 模式实际使用的是：

```c
static uint32_t dual_adc_buf[3];
```

这个全局单值变量容易引起混淆，建议后续删除。

### 13.6 `ADC_ExternalTrigConvCmd(ADC2, ENABLE)` 的说明

代码中这句用于允许 ADC2 跟随外部触发。  
在当前双 ADC 同步模式下，它的意图是让 ADC2 配合 ADC1 同步转换。

---

## 14. 代码扩展方向

当前工程可以继续扩展为：

- 双 ADC 更多通道同步采样
- ADC1 + ADC2 同步后再做平均滤波
- 定时器触发双 ADC 同步采样
- DMA 传输完成中断处理
- 采样结果上传上位机
- 多路传感器同步采集系统

---

## 15. 总结

本项目实现了一个结构清晰的 **STM32F10x 双 ADC 同步采样 + DMA 搬运框架**，具有以下特点：

- ADC1 / ADC2 同步采样
- DMA 自动搬运成对结果
- 主循环读取简单
- 可扩展性强
- 适合多路模拟信号同步采集

非常适合作为以下内容的学习和开发基础：

- 双 ADC 工作模式理解
- ADC + DMA 结合使用
- 多路模拟量同步采集
- 嵌入式采样模块封装