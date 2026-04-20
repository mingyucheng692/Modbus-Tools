<div align="center">

<img src="assets/logo.svg" alt="Modbus-Tools Logo" width="120">

# Modbus-Tools
### Professional Modbus TCP Client & Modbus RTU Master Debugging and Analysis Tool

**Visual Frame Builder · Real-time Parsing · Robust Connection Strategies**

[![GitHub Release](https://img.shields.io/github/v/release/mingyucheng692/Modbus-Tools?style=flat-square)](https://github.com/mingyucheng692/Modbus-Tools/releases) [![Release Status](https://github.com/mingyucheng692/Modbus-Tools/actions/workflows/release.yml/badge.svg?style=flat-square)](https://github.com/mingyucheng692/Modbus-Tools/actions/workflows/release.yml) [![License](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](LICENSE) [![C++20](https://img.shields.io/badge/C%2B%2B-20-orange.svg?style=flat-square)](https://isocpp.org/std/the-standard) [![Qt6](https://img.shields.io/badge/Qt-6.x-41CD52.svg?style=flat-square)](https://www.qt.io) [![CMake](https://img.shields.io/badge/CMake-3.16+-064F8C?style=flat-square&logo=cmake&logoColor=white)](https://cmake.org/) [![Google Test](https://img.shields.io/badge/Google_Test-1.12+-4285F4?style=flat-square&logo=google&logoColor=white)](https://github.com/google/googletest)

**English** | [简体中文](README_zh-CN.md) | [繁體中文](README_zh-TW.md)

</div>

---

## 🔍 About

**Modbus-Tools** is a modern, open-source utility designed for **Industrial IoT (IIoT) debugging**, **embedded system development**, and **fieldbus analysis**. Serving as a feature-rich **Modbus debugger**, it supports active communication as a **Modbus TCP Client** and **Modbus RTU Master**, providing reliable support for **register reading and writing** in industrial environments.

The core highlight of this project is its built-in **Frame Analyzer**, which can identify binary byte streams and perform structural decomposition. With the **featured Link to Analyzer (Live Linkage)** mode, developers can observe the raw data and its physical meaning during serial or network debugging. Whether validating slave devices, locating communication faults, or analyzing complex sampling data, Modbus-Tools provides robust and efficient engineering support.

---

## ✨ Features

### 🏗 Visual Frame Builder
Provides intuitive industrial protocol interaction and simplifies the debugging workflow:
- **Form-based Parameters**: Graphical input for Slave ID, Start Address, and Quantity. Slave ID and addresses (including polling) fully support **HEX (0x10, 10H)** and **DEC (16)** smart recognition and validation.
- **Function Code Coverage**: Supports common 0x01-0x06, 0x0F, 0x10 standard Modbus function codes.
- **Raw Helper Utility**: One-click "Calculate and Append CRC16 (RTU)" and "Encapsulate MBAP Header (TCP)" functions, simplifying non-standard frame testing.
- **Real-time Preview**: Synchronous display of hex raw byte streams while inputting parameters, ensuring every frame sent is as expected.

> [!TIP]
> **Configuration**: Polling `Addr` is independent, while `Slave ID` is shared with the function panel. For simultaneous monitoring of different slave IDs, opening multiple windows is recommended (TCP only).

### 🔘 Coils Binary Interaction
Interaction logic specifically designed for bit operations, enabling intuitive control of output states:
- **Binary Input Mode**: Input bit strings directly (e.g., `1 0 1 1`), which are automatically encoded and sent using 0x05 (Write Single Coil) or 0x0F (Write Multiple Coils).
- **Bit-level Awareness**: Efficiently validates remote coil and discrete input states in coordination with 0x01/0x02 read commands.

### 📊 Featured Frame Analyzer
Transforms cryptic hexadecimal messages into structured business data:
- **Protocol Decomposition**: Instantly decomposes Hex streams into key fields like Slave ID, Function Code, Address, Length, and Checksum.
- **Engineering Conversion**: Built-in **Scale Factor** to automatically convert raw register values into physical quantities (e.g., Temperature, Pressure, Frequency).
- **Multi-dimensional Endianness**: Supports **ABCD (Big Endian)**, **CDAB (Little Endian Byte Swap)**, **BADC (Big Endian Byte Swap)**, and **DCBA (Little Endian)** byte/word orders to match various PLC data alignments.
- **Semantic Annotation**: Custom descriptions for registers, reflecting real business metrics in parsing results, with support for batch history export.

### 🔗 Link to Analyzer (Live Linkage) — **Special Features**
Breaks down the barrier between traffic monitoring and the analyzer for automated decoding:
- **Instant Passthrough**: Captured **RX response packets** are automatically pushed to the analyzer, eliminating tedious manual copying of hex data.
- **Pause & Edit**: Supports "Pause Refresh" to freeze specific frames in high-speed communication for editing scale factors or register descriptions.
- **Asynchronous Parsing**: Decoding is performed asynchronously in the background, ensuring smooth UI scrolling during real-time observation.

---

## 📋 Technical Specifications

### 📡 Standard Protocol Support
| Dimension | Capability | Technical Details |
| :--- | :--- | :--- |
| **Comm Mode** | **Modbus TCP Client** & **Modbus RTU Master** | Adapts to Ethernet (RJ45) and Serial (RS232/RS485) links |
| **Function Codes** | FC01-FC04 (Read) / FC05-FC06 (Single Write) / FC15-FC16 (Multi Write) | Covers mainstream Modbus industrial scenarios |
| **Security/Check** | Auto CRC16 (RTU) / MBAP Header (TCP) | Generates and validates frame integrity and compliance |

### 🛠 General Debugging Tools
In addition to deep Modbus support, the project integrates high-performance general debugging tools:
- **General TCP Client**: Supports Text/Hex modes, suitable for non-standard network protocol verification.
- **Serial Port Monitor**: Custom baud rate, parity, and data bits, with integrated file transfer and other utility functions.

---

## 📸 Screenshots

<table>
  <tr>
    <td align="center"><b>Modbus TCP Builder</b></td>
    <td align="center"><b>Modbus TCP Write</b></td>
  </tr>
  <tr>
    <td><img src="docs/images/modbus-tcp-frame-builder.png" alt="Modbus TCP" width="400"></td>
    <td><img src="docs/images/modbus-tcp-write-decimal.png" alt="Modbus Write" width="400"></td>
  </tr>
  <tr>
    <td align="center"><b>Analyzer - Scaling & Description</b></td>
    <td align="center"><b>Analyzer - Tree Structure</b></td>
  </tr>
  <tr>
    <td><img src="docs/images/frame-analyzer-address-scale-description.png" alt="Scaling" width="400"></td>
    <td><img src="docs/images/frame-analyzer-overview.png" alt="Tree View" width="400"></td>
  </tr>
</table>

### 📺 Demo
![Linkage & Smart Input Demo](docs/images/demo.gif)

---

## 🚀 Getting Started

### Download (Windows)
1. Go to the [Releases](https://github.com/mingyucheng692/Modbus-Tools/releases) page.
2. Download the latest `Modbus-Tools-win64.zip`.
3. Unzip and run `Modbus-Tools.exe`. **Portable, no installation required**.

### Build from Source
The project is built on **Qt 6.x** and **CMake**, supporting the MSVC compiler:
```bash
# Clone the repository and its submodules
git clone --recursive https://github.com/mingyucheng692/Modbus-Tools.git  --progress
cd Modbus-Tools

# Configure using CMake
cmake -S . -B build

# Build (Release example)
cmake --build build --config Release -j
```

## 🧪 Quality & Testing

This project utilizes the Google Test (GTest) and Google Mock (GMock) frameworks for automated quality assurance. As an open-source collaborative project, the test coverage is primarily intended to optimize logical consistency and reduce defect rates. **It does not provide any form of reliability level guarantee or functional safety certification.**

### Test Coverage
- **Session Management**: Validates connect/disconnect logic, request timeout retries, and error state recovery.
- **Protocol Transport**: Covers TCP/RTU frame encapsulation, decapsulation, checksum calculation, and integrity validation.
- **Parsing Logic**: Robustness verification against various valid commands and malformed packets to prevent parsing exceptions.
- **Data Processing**: Ensures accuracy of endianness conversion, scale factors, and formatting algorithms.

### Automation & Status
- **Test Integrity**: 100% of 42 automated test cases passed across all supported tiers.
- **CI Pipeline**: GitHub Actions integration with MSVC AddressSanitizer (ASan) enabled for automated memory corruption and leak detection.
- **Reliability Verification**: Regression testing is performed on every release candidate to verify protocol consistency and session state stability.

### Running Local Tests
```powershell
cmake -B build -DMODBUS_TOOLS_BUILD_TESTS=ON
cmake --build build --target modbus_test
ctest --test-dir build -C Debug --output-on-failure
```

---

## ⚙️ Professional Engineering Capabilities
Designed for complex industrial environments, Modbus-Tools provides extensive communication control strategies:
- **Retry Mechanism**: Customizable failure retry strategies.
- **Multi-language**: Fully supports English, Simplified Chinese, and Traditional Chinese UI.
- **OTA Updates**: Integrated GitHub API for detecting and upgrading to the latest stable version.

---

## 🏗 Project Architecture

### Tech Stack

| Component | Technology | Description |
| :--- | :--- | :--- |
| **Language** | **C++20** | Modern C++: constexpr, enum class, smart pointers, std::optional |
| **GUI Framework** | **Qt 6** | Widgets, Charts, Network, SerialPort, Concurrent |
| **Logger** | **spdlog** | Asynchronous logging with file rotation (10MB × 20 files) |
| **Build System** | **CMake 3.16+** | Cross-platform build support with MSVC parallel compilation |
| **Unit Testing** | **Google Test** | High logic coverage intended to optimize logical consistency |
| **CI/CD** | **GitHub Actions** | Automated pipelines: build, test, and release package distribution |

### Layers

```text
┌─────────────────────────────────────────────────────────────┐
│                       UI Layer (ui/)                        │
│   MainWindow │ Views │ Widgets │ Theme │ i18n │ Settings    │
├─────────────────────────────────────────────────────────────┤
│                     Logic Layer (core/)                       │
│   ┌──────────┐   ┌────────────┐   ┌─────────────────────┐   │
│   │  Modbus  │   │  I/O Chan  │   │   Global Services   │   │
│   │  Stack   │   │ (Channel)  │   │  Logger/Settings etc│   │
│   └──────────┘   └────────────┘   └─────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│                    Infrastructure Layer                     │
│         Qt6 Framework │ spdlog │ C++ Standard Library       │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎯 Use Cases

| Scenario | Description |
| :--- | :--- |
| **Device Commissioning** | Quickly validate register read/write operations on Modbus slave devices. |
| **Protocol Analysis** | Capture and parse Modbus frames to locate communication faults. |
| **Batch Testing** | Continuously read registers in poll mode to monitor data changes. |
| **Raw Debugging** | Send custom Hex frames for non-standard or proprietary protocols. |
| **Educational Demo** | Intuitively demonstrate Modbus frame structure and communication processes. |

### Target Users
- 🏭 **Industrial Engineers**: PLC / DCS / SCADA / HMI / Instrumentation / BMS / PCS / EMS commissioning.
- 🔧 **Embedded Developers**: Validation for Modbus slave device development.
- 🔗 **System Integrators**: Site fieldbus communication troubleshooting.
- 📚 **Learners**: Modbus protocol study and research.

---

## 🤝 Contribution

Issues and PRs are welcome! We highly value contributions from the community (bug fixes, new features, documentation optimization, etc.).
1. **Fork** the repository.
2. **Feature Branch**: `git checkout -b feature/amazing-feature`.
3. **Commit**: `git commit -m 'Add amazing feature'`.
4. **Push**: `git push origin feature/amazing-feature`.
5. **Open a PR**.

Every ⭐ Star is the greatest motivation for our continuous iteration.

### Developer Certificate of Origin (DCO)

This project uses the **Developer Certificate of Origin (DCO)** to ensure clear intellectual property rights for contributions. When submitting a Pull Request, please ensure each commit includes a signature line like:

```text
Signed-off-by: Your Name <your.email@example.com>
```

You can use `git commit -s` to add this line automatically. By signing the DCO, you certify that:
- The contribution is your own work and/or you have the right to submit it;
- You agree to license the contribution under the project's MIT License;
- You understand that the contribution may be modified or redistributed.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).

Copyright © 2025 - present mingyucheng692

### 🔄 Secondary Distribution Requirements
- Any modification, derivative work, or secondary distribution of this project **must fully retain the original copyright notice and this license file**.
- Distribution must not delete, tamper with, or hide any copyright identification, author attribution, or license statement in the source code, documentation, or build artifacts.

### ⚖️ Third-Party License Summary

This project primarily uses the following third-party libraries. Users should comply with their original licenses during redistribution or secondary development.

| Dependency | License | Description |
| :--- | :--- | :--- |
| **Qt 6** | LGPLv3 | Used via dynamic linking, supporting user library replacement. No source modifications. |
| **spdlog** / **fmt** | MIT | Core logging and formatting support. Original copyrights retained. |
| **Google Test**| BSD-3-Clause | Used for development verification only. |


---

## ⚖️ Disclaimer

### 1. Intended Use
For development, debugging, and demonstration only. Use in critical or production environments at your own risk.

### 2. No Warranty
Provided "AS IS" without warranty of any kind. No guarantee is made that the software meets specific needs or that frame building/parsing results are absolutely accurate.

### 3. Liability Limits
In no event shall the authors or copyright holders be liable for any direct or indirect damages arising from the use of this software.

### 4. Compliance
Not certified for safety-critical systems (e.g., SIL, IEC 61508). Checksum algorithms are for debugging reference only. Ensure compliance with local laws.

### 5. Data & Privacy
We respect user privacy and follow the principle of data minimization:
- **Local Data**: Communication settings, parsing templates, and preferences are stored locally.
- **Auto Updates**: Update checks access the GitHub API via HTTPS, sending only the app version and platform identifier.
- **Network Comm**: Modbus TCP connections are actively initiated by the user to specified addresses only.
- **No Collection**: We do not collect, store, or transmit any personal or sensitive user information.

---

### 🙏 Acknowledgments

Thanks to the following open-source projects for making Modbus-Tools possible:
- [Qt](https://www.qt.io) — Cross-platform application framework
- [spdlog](https://github.com/gabime/spdlog) — Fast C++ logging library
- [fmt](https://github.com/fmtlib/fmt) — Modern C++ formatting library
