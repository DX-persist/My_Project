#!/bin/bash
# probe.sh - 检测 ST-Link 与 STM32 连接状态

command -v st-info >/dev/null 2>&1 || { echo "未安装 st-info，请安装 stlink-tools"; exit 1; }

echo "探测 ST-Link 设备..."
st-info --probe

if [ $? -eq 0 ]; then
    echo "ST-Link 已连接"

    echo "尝试 OpenOCD 探测 STM32..."
    openocd -f openocd/stm32.cfg -c "shutdown"
    if [ $? -eq 0 ]; then
        echo "STM32 芯片可访问"
    else
        echo "OpenOCD 未能访问 STM32"
    fi

else
    echo "未检测到 ST-Link，请检查线路和供电"
fi

