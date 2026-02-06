# Modbus-Tools
[English](README.md) | [简体中文](README_zh_CN.md) | [繁體中文](README_zh_TW.md)
[![spdlog](https://img.shields.io/badge/spdlog-logging-blue?logo=github)](https://github.com/gabime/spdlog)

基于 C++20 与 Qt 6 的工业级 Modbus TCP/RTU 调试套件，面向高性能与工程化调试场景。本项目为个人业余时间开发，特别感谢 spdlog。

## 安装
### 环境要求
- **编译器**: MSVC 2019+ (需支持 C++20)
- **框架**: Qt 6.x (包含 Widgets, Network, SerialPort, Charts 模块)
- **构建系统**: CMake 3.16+
- **第三方库**: spdlog (已包含在工程中)

### 源码编译
```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

### 运行
```bash
./build/bin/Release/ModbusTools.exe
```

## 支持平台
- Windows (MSVC 2019+, x64)

## 核心特性
- **多协议支持**: 完整支持 Modbus TCP、Modbus RTU 以及通用 TCP/串口调试模式。
- **先进架构**: 基于 Reactor 模式的 I/O 设计，采用独立工作线程，确保界面流畅与高并发处理能力。
- **Docking 界面**: 高度可定制的停靠窗口布局（连接、日志、流量监控、波形显示）。
- **协议分析器**: 提供十六进制流、协议树视图及详细字段解释的深度包分析。
- **波形监控**: 支持右键将寄存器数值添加到示波器进行实时曲线绘制。
- **物理模拟**: 支持模拟丢包率与网络延迟，用于测试系统的健壮性。

## 使用示例
1. **连接设置**: 在左侧 "Connection" 面板选择协议类型 (TCP/RTU)，配置相应参数并连接。
2. **指令发送**: 使用中间的控制面板发送 Modbus 读写请求。
3. **报文分析**: "Traffic Monitor" 显示原始流量，"Frame Analyzer" 提供深度解析。
4. **波形显示**: 在分析器中右键点击数值字段，选择 "Add to Waveform" 即可查看实时趋势。

## 版权说明
仅供内部学习与使用。
