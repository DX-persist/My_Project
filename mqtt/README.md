
# 🔗 OneNET MQTT C Client

本项目是一个基于 C 语言编写的 MQTT 客户端示例，适用于连接中国移动 OneNET 平台的直连设备。项目使用 [Mosquitto](https://mosquitto.org/) MQTT 库实现 MQTT 协议通信，使用 [cJSON](https://github.com/DaveGamble/cJSON) 实现 JSON 解析。

## 📦 项目结构

```
.
├── bin/                    # 存放编译后的可执行文件
├── include/               # 头文件目录（onenet_config.h、onenet_mqtt.h 等）
├── obj/                   # 编译中间产物 .o 文件
├── src/                   # 源码文件夹
│   ├── main.c             # 示例主程序
│   ├── onenet_mqtt.c      # MQTT 初始化、连接、发布订阅函数实现
│   └── cJSON_Parse.c      # 控制指令解析模块
├── build.sh               # 自动编译脚本
└── README.md              # 本文件
```

---

## 🚀 功能说明

本客户端实现了以下功能：

- ✅ MQTT 连接 OneNET 平台（用户名为 Product ID，密码为 Token）
- ✅ 属性上报 (`thing/property/post`)
- ✅ 属性上报确认接收 (`thing/property/post/reply`)
- ✅ 控制指令接收 (`thing/property/set`)
- ✅ 控制指令解析（如控制 LED 开关）
- ✅ 设置应答回复 (`thing/property/set_reply`)
- ✅ 支持多线程非阻塞网络处理（基于 `mosquitto_loop_start()`）

---

## 🛠️ 编译说明

项目已附带编译脚本 `build.sh`，自动完成编译与链接。

### 🔧 编译依赖

- `gcc` 编译器
- `libmosquitto` MQTT 库
- `libcjson` JSON 解析库（默认系统支持）

可使用如下命令安装依赖：

```bash
sudo apt update
sudo apt install -y libmosquitto-dev libcjson-dev
```

---

### ▶️ 编译方式

在项目根目录下运行：

```bash
chmod +x build.sh
./build.sh
```

编译完成后，将在 `bin/` 目录下生成可执行文件：

```bash
./bin/onenet_mqtt
```

---

## 📡 使用说明

1. 修改 `include/onenet_config.h`，配置你的 OneNET 设备信息：

```c
#define ONENET_PRODUCT_ID     "你的ProductID"
#define ONENET_DEVICE_ID      "你的DeviceID"
#define ONENET_DEVICE_TOKEN   "你的Token"
#define ONENET_HOST           "mqtts.heclouds.com"
#define ONENET_PORT           1883
```

2. 运行程序：

```bash
./bin/onenet_mqtt
```

3. 终端输出将显示 MQTT 连接、属性上报、控制指令接收、LED 控制等状态信息。

---

## 📬 MQTT 交互示例

平台向设备下发指令：

```json
{
  "id": "12345678",
  "params": {
    "led": true
  }
}
```

设备控制后，将回复：

```json
{
  "id": "12345678",
  "code": 200,
  "msg": "success"
}
```

---

## 📎 TODO

- [ ] 增加日志系统
- [ ] 支持 TLS/SSL 安全连接
- [ ] 支持其他设备功能扩展（如温湿度上传等）
- [ ] 封装为通用可移植 MQTT 客户端库

---

## 🙋 作者

作者：DX

联系方式：483603153@qq.com（替换为你自己的）

---

## 📄 License

本项目基于 MIT 许可协议开源，欢迎学习与扩展！
