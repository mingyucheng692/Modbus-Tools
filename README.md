# Modbus-Tools
[English](README.md) | [简体中文](README_zh_CN.md) | [繁體中文](README_zh_TW.md)
[![spdlog](https://img.shields.io/badge/spdlog-logging-blue?logo=github)](https://github.com/gabime/spdlog)

Fast industrial Modbus TCP/RTU debugging suite built with C++20 and Qt 6. This project is developed in personal spare time. Special thanks to spdlog.

## Install
### Requirements
- **Compiler**: MSVC 2019+ (C++20 support required)
- **Framework**: Qt 6.x (Widgets, Network, SerialPort, Charts modules)
- **Build System**: CMake 3.16+
- **Third-party**: spdlog (included)

### Build from source
```bash
cmake -S . -B build;cmake --build build --config Release --parallel
```

### Run
```bash
./build/bin/Release/ModbusTools.exe
```

## Platforms
- Windows (MSVC 2019+, x64)

## Features
- **Multi-Protocol Support**: Modbus TCP, Modbus RTU, and Generic TCP/Serial.
- **Advanced Architecture**: Reactor pattern based I/O with separate worker threads for high performance.
- **Docking UI**: Customizable interface with movable panels (Connection, Logs, Traffic, Waveform).
- **Protocol Analyzer**: Deep packet inspection with hex stream, tree view, and detailed interpretation.
- **Waveform Monitor**: Real-time visualization of register values.
- **Physical Simulation**: Simulate packet loss and latency for robustness testing.

## Usage samples
1. **Connection**: Select "Modbus TCP" or "Modbus RTU" in the Connection Dock. Enter IP/Port or select Serial parameters.
2. **Control**: Use the Control Widget (center) to send Modbus requests (Read/Write).
3. **Analyze**: View raw traffic in "Traffic Monitor" and detailed parsing in "Frame Analyzer".
4. **Waveform**: Right-click a value in the Analyzer to "Add to Waveform" for plotting.

## License
Private / Internal Use Only.
