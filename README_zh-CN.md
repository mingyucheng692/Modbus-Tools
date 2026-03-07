# Modbus-Tools

![Modbus-Tools Logo](assets/logo.svg)

语言: [English](README.md) | [简体中文](README_zh-CN.md) | [繁體中文](README_zh-TW.md)

一个业余时间开发的 Modbus/通用通讯调试助手，聚焦在“快速连接、快速收发、快速分析”。

- 默认界面语言：`en_US`
- 可在界面中快速切换：`简体中文 (zh_CN)`、`繁體中文 (zh_TW)`

## 功能概览

- Modbus TCP 调试：连接、功能码请求、报文收发监控
- Modbus RTU 调试：串口参数配置、请求发送、响应解析
- 通用 TCP Client：自定义数据收发与流量监控
- 通用串口工具：串口连接、原始数据发送接收
- 帧分析器：对 Modbus 报文做结构化解析
- 可配置 Modbus 超时、重试次数、重试间隔

## 技术栈

- C++20
- Qt6（Core / Network / SerialPort / Widgets / Charts）
- CMake (>= 3.16)
- spdlog（通过子模块引入）

## 工程结构

```text
Modbus-Tools/
├─ app/           # 程序入口（main.cpp）
├─ core/          # 通讯与 Modbus 核心逻辑
├─ ui/            # Qt 界面、视图、控件、多语言资源
├─ third_party/   # 第三方依赖（当前包含 spdlog 子模块）
└─ CMakeLists.txt # 顶层构建配置
```

## 构建说明（Windows / MSVC）

### 1) 拉取代码（包含子模块）

```bash
git clone https://github.com/mingyucheng692/Modbus-Tools.git
cd Modbus-Tools
```

如果已克隆但未拉子模块：

```bash
git submodule update --init --recursive
```

### 2) 配置工程

```bash
cmake -S . -B build
```

### 3) 编译

```bash
cmake --build build --config Release
```

### 4) 运行

```bash
.\build\app\Release\ModbusTools.exe
```

## 语言切换

程序启动后：

- 默认使用 `en_US`
- 在菜单栏点击 `Language`
- 可切换到 `简体中文 (zh_CN)` 或 `繁體中文 (zh_TW)`

## 依赖要求

- 已安装 Qt6，并能被 CMake 找到（`find_package(Qt6 ...)`）
- Visual Studio 2022（或兼容的 MSVC 工具链）
- CMake 3.16 及以上

## 当前状态

这是一个持续迭代的个人调试工具项目，欢迎使用。
