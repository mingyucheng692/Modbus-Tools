# Modbus-Tools (Industrial Debugging Suite)

## Introduction
Modbus-Tools is a high-performance, industrial-grade debugging tool designed for Modbus TCP/RTU protocols. Developed with C++20 and Qt 6, it features a modern docking interface, real-time waveform monitoring, and a powerful protocol analyzer.

## Key Features
- **Multi-Protocol Support**: Modbus TCP, Modbus RTU, and Generic TCP/Serial.
- **Advanced Architecture**: Reactor pattern based I/O with separate worker threads for high performance.
- **Docking UI**: Customizable interface with movable panels (Connection, Logs, Traffic, Waveform).
- **Protocol Analyzer**: Deep packet inspection with hex stream, tree view, and detailed interpretation.
- **Waveform Monitor**: Real-time visualization of register values.
- **Physical Simulation**: Simulate packet loss and latency for robustness testing.

## Build Instructions

### Prerequisites
- **Compiler**: MSVC 2019+ (C++20 support required)
- **Framework**: Qt 6.x (Widgets, Network, SerialPort, Charts modules)
- **Build System**: CMake 3.16+
- **Third-party**: spdlog (included)

### Steps
1. Open the project in VS Code or Terminal.
2. Configure with CMake:
   ```bash
   cmake -B build -G "Visual Studio 17 2022" -A x64
   ```
3. Build the project:
   ```bash
   cmake --build build --config Release
   ```
4. Run the executable:
   ```bash
   ./build/bin/Release/ModbusTools.exe
   ```

## Usage
1. **Connection**: Select "Modbus TCP" or "Modbus RTU" in the Connection Dock. Enter IP/Port or select Serial parameters.
2. **Control**: Use the Control Widget (center) to send Modbus requests (Read/Write).
3. **Analyze**: View raw traffic in "Traffic Monitor" and detailed parsing in "Frame Analyzer".
4. **Waveform**: Right-click a value in the Analyzer to "Add to Waveform" for plotting.

## License
Private / Internal Use Only.
