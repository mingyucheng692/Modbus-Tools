# Modbus-Tools (工业级调试套件)

## 简介
Modbus-Tools 是一款基于 C++20 和 Qt 6 开发的高性能工业级 Modbus TCP/RTU 调试工具。它采用现代化的 Docking 界面设计，集成了实时波形监控、深度协议分析器及物理层模拟功能。

## 核心特性
- **多协议支持**: 完整支持 Modbus TCP、Modbus RTU 以及通用 TCP/串口调试模式。
- **先进架构**: 基于 Reactor 模式的 I/O 设计，采用独立工作线程，确保界面流畅与高并发处理能力。
- **Docking 界面**: 高度可定制的停靠窗口布局（连接、日志、流量监控、波形显示）。
- **协议分析器**: 提供十六进制流、协议树视图及详细字段解释的深度包分析。
- **波形监控**: 支持右键将寄存器数值添加到示波器进行实时曲线绘制。
- **物理模拟**: 支持模拟丢包率与网络延迟，用于测试系统的健壮性。

## 编译指南

### 环境要求
- **编译器**: MSVC 2019+ (需支持 C++20)
- **框架**: Qt 6.x (包含 Widgets, Network, SerialPort, Charts 模块)
- **构建系统**: CMake 3.16+
- **第三方库**: spdlog (已包含在工程中)

### 编译步骤
1. 在 VS Code 或终端中打开项目目录。
2. 配置 CMake:
   ```bash
   cmake -B build -G "Visual Studio 17 2022" -A x64
   ```
3. 执行编译:
   ```bash
   cmake --build build --config Release
   ```
4. 运行程序:
   ```bash
   ./build/bin/Release/ModbusTools.exe
   ```

## 使用说明
1. **连接设置**: 在左侧 "Connection" 面板选择协议类型 (TCP/RTU)，配置相应参数并连接。
2. **指令发送**: 使用中间的控制面板发送 Modbus 读写请求。
3. **报文分析**: "Traffic Monitor" 显示原始流量，"Frame Analyzer" 提供深度解析。
4. **波形显示**: 在分析器中右键点击数值字段，选择 "Add to Waveform" 即可查看实时趋势。

## 版权说明
仅供内部学习与使用。
