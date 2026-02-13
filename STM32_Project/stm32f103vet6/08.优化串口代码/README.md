# STM32F103C8 Makefile Template

一个基于 **STM32F103C8T6（Cortex-M3）** 的 **纯 Makefile 工程模板**，  
支持 **Linux / 远程开发（OrangePi / Raspberry Pi）**，  
集成 **ST-Link + OpenOCD + VSCode Cortex-Debug 调试**。

适合长期复用、新项目直接拷贝。

---

## 📁 目录结构说明

```
.
├── build/                     # 编译输出目录（自动生成）
│   ├── stm32f103c8.elf
│   ├── stm32f103c8.bin
│   └── stm32f103c8.map
├── CMSIS/                     # CMSIS 核心与启动文件
├── STM32F10x_StdPeriph_Driver # STM32 标准外设库（SPL）
├── Drivers/UserLib/           # 用户可复用库（如 delay）
├── User/                      # 用户应用代码
│   ├── main.c
│   ├── stm32f10x_it.c/h
│   └── stm32f10x_conf.h
├── openocd/
│   └── stm32.cfg              # OpenOCD 配置
├── .vscode/                   # VSCode 调试与任务配置
│   ├── launch.json
│   └── tasks.json
├── Makefile                   # 主构建文件
├── stm32f103c8.ld             # 链接脚本
├── probe.sh                   # ST-Link / OpenOCD 探测脚本
└── README.md
```

---

## 🔧 工具链依赖

请确保系统已安装以下工具：

- GNU ARM Toolchain  
  ```
  arm-none-eabi-gcc
  arm-none-eabi-gdb
  ```
- OpenOCD
- stlink-tools（提供 `st-info`、`st-flash`）
- make
- VSCode（可选，用于调试）
- VSCode 插件：
  - **Cortex-Debug**

---

## ⚙️ 编译工程

在工程根目录执行：

```
make
```

生成文件：

- `build/stm32f103c8.elf`
- `build/stm32f103c8.bin`
- `build/stm32f103c8.map`

清理：

```
make clean
```

---

## 🔌 ST-Link & 芯片探测

用于快速检查 **ST-Link 是否连接**、**OpenOCD 是否能访问 STM32**：

```
./probe.sh
```

正常输出应包含：

- ST-Link 探测成功
- OpenOCD 可访问 STM32F1 芯片

---

## 🔥 烧写程序（命令行）

### 仅烧写（使用 st-flash）

```
make flash
```

### 编译 + 烧写一键完成

```
make flashall
```

默认烧写地址：

```
0x08000000
```

---

## 🐞 OpenOCD + GDB 手动调试（可选）

启动 OpenOCD：

```
openocd -f openocd/stm32.cfg
```

新终端启动 GDB：

```
arm-none-eabi-gdb build/stm32f103c8.elf
```

GDB 中常用命令：

```
target remote :3333
monitor reset halt
b main
c
```

---

## 🧠 VSCode 调试（推荐）

本模板已配置好 `.vscode/launch.json` 和 `tasks.json`。

### 使用方式：

1. 使用 **Remote-SSH** 打开工程目录
2. 选择调试配置：`Debug STM32F103C8`
3. 直接按 **F5**

调试流程：

- 自动 `make flashall`
- 启动 OpenOCD
- GDB 连接
- 程序自动停在 `main()`

---

## 🧱 Makefile 特性说明

- Cortex-M3 (`-mcpu=cortex-m3`)
- SPL + CMSIS
- `-ffunction-sections` + `--gc-sections`
- 自动生成 `.bin / .elf / .map`
- 启动文件使用 `startup_stm32f10x_md.S`
- 支持反汇编：

```
make disasm
```

---

## 🧩 扩展建议（可选）

后续可在 `Drivers/UserLib/` 中逐步加入：

- LED（GPIO 封装）
- KEY（按键 + 去抖）
- UART（printf 调试）
- Timer / SysTick 封装

模板结构无需改动，直接加源文件并在 Makefile 中加入即可。

---

## 📌 适用场景

- STM32 裸机开发
- 远程 Linux 主机开发
- 无 IDE / 轻量级工程
- 长期维护、批量新建工程

while(1) {
    // 步骤 1: 输出提示信息
    printf("请输入数据\r\n");
        ↓
    _write() 被调用
        ↓
    BSP_USART_SendByte() 逐字节发送
        ↓
    等待 TXE 标志 → 写入 DR 寄存器 → 等待 TC 标志
    
    // 步骤 2: 等待用户输入
    scanf("%s", data);
        ↓
    _read() 被调用
        ↓
    【阻塞等待】从环形缓冲区读取数据
        ↓
    遇到 '\r' 或 '\n' 时返回
    
    // 步骤 3: 回显用户输入
    printf("data = %s\r\n", data);
        ↓
    再次通过 _write() 发送
}
```

## 四、中断接收流程（后台自动执行）
```
PC 发送字节 'A'
    ↓
STM32 接收到数据
    ↓
触发 USART1 中断
    ↓
进入 USART1_IRQHandler()
    ↓
调用 BSP_USART_IRQHandler(BSP_USART1)
    ↓
检查 RXNE 标志位
    ↓
从 USART1->DR 读取数据 'A'
    ↓
RingBuffer_Push(&usart_rb[BSP_USART1], 'A')
    ↓
写入 buffer[head] = 'A'
head = (head + 1) % 256
    ↓
中断退出，返回主程序
```

## 五、环形缓冲区工作原理
```
初始状态:
┌─────────────────────────┐
│ [空] [空] [空] ... [空]  │
│  ↑                       │
│ head = tail = 0         │
└─────────────────────────┘

接收到 'H' 'e' 'l' 'l' 'o':
┌─────────────────────────┐
│ [H] [e] [l] [l] [o] ... │
│  ↑               ↑       │
│ tail=0          head=5  │
└─────────────────────────┘

scanf 读取 3 个字节后:
┌─────────────────────────┐
│ [H] [e] [l] [l] [o] ... │
│           ↑      ↑       │
│         tail=3  head=5  │
└─────────────────────────┘
```

## 六、完整数据流示例

假设用户在串口工具输入 "hello\r\n"：
```
时间轴执行流程：

T0: 程序启动，初始化完成
    └─ printf("请输入数据\r\n") 发送到 PC

T1: 用户输入 'h'
    └─ [中断] USART1 接收 → buffer[0]='h', head=1

T2: 用户输入 'e'
    └─ [中断] USART1 接收 → buffer[1]='e', head=2

T3: 用户输入 'l'
    └─ [中断] USART1 接收 → buffer[2]='l', head=3

T4: 用户输入 'l'
    └─ [中断] USART1 接收 → buffer[3]='l', head=4

T5: 用户输入 'o'
    └─ [中断] USART1 接收 → buffer[4]='o', head=5

T6: 用户输入 '\r'
    └─ [中断] USART1 接收 → buffer[5]='\r', head=6
    └─ [主循环] _read() 检测到 '\r'
        ├─ data[0]='h', data[1]='e', data[2]='l'
        ├─ data[3]='l', data[4]='o', data[5]='\n'
        ├─ data[6]='\0'
        └─ scanf 返回，tail=6

T7: printf("data = hello\r\n") 发送到 PC

T8: 循环，再次 printf("请输入数据\r\n")