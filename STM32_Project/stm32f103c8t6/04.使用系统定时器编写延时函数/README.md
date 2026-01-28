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
