# Modbus-Tools (工业级除错套件)

## 简介
Modbus-Tools 是一款基于 C++20 和 Qt 6 开发的高效能工业级 Modbus TCP/RTU 除错工具。它采用现代化的 Docking 介面设计，整合了即时波形监控、深度协定分析器及实体层模拟功能。

## 核心特性
- **多协定支援**: 完整支援 Modbus TCP、Modbus RTU 以及通用 TCP/串列埠除错模式。
- **先进架构**: 基于 Reactor 模式的 I/O 设计，采用独立工作执行绪，确保介面流畅与高并发处理能力。
- **Docking 介面**: 高度可自订的停靠视窗布局（连线、日志、流量监控、波形显示）。
- **协定分析器**: 提供十六进位流、协定树视图及详细栏位解释的深度封包分析。
- **波形监控**: 支援右键将暂存器数值新增至示波器进行即时曲线绘制。
- **实体模拟**: 支援模拟封包遗失率与网路延迟，用于测试系统的健壮性。

## 编译指南

### 环境要求
- **编译器**: MSVC 2019+ (需支援 C++20)
- **框架**: Qt 6.x (包含 Widgets, Network, SerialPort, Charts 模组)
- **建置系统**: CMake 3.16+
- **第三方库**: spdlog (已包含在工程中)

### 编译步骤
1. 在 VS Code 或终端机中开启专案目录。
2. 配置 CMake:
   ```bash
   cmake -B build -G "Visual Studio 17 2022" -A x64
   ```
3. 执行编译:
   ```bash
   cmake --build build --config Release
   ```
4. 执行程式:
   ```bash
   ./build/bin/Release/ModbusTools.exe
   ```

## 使用说明
1. **连线设定**: 在左侧 "Connection" 面板选择协定类型 (TCP/RTU)，配置相应参数并连线。
2. **指令发送**: 使用中间的控制面板发送 Modbus 读写请求。
3. **报文分析**: "Traffic Monitor" 显示原始流量，"Frame Analyzer" 提供深度解析。
4. **波形显示**: 在分析器中右键点击数值栏位，选择 "Add to Waveform" 即可查看即时趋势。

## 版权说明
仅供内部学习与使用。
