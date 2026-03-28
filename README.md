# Modbus-Tools

![Modbus-Tools Logo](assets/logo.svg)

Languages: [English](README.md) | [简体中文](README_zh-CN.md) | [繁體中文](README_zh-TW.md)

A personal Modbus / generic communication debugging assistant built in spare time, focused on fast connect, fast send/receive, and fast analysis.

- Default UI language: `en_US`
- Quick switch to: `zh_CN`, `zh_TW`

## 💡 Design Philosophy & Core Features

The goal of this tool is to simplify the basic workflow of **"sending frames, viewing logs, and analyzing frames"** during the testing phase.
It does not pursue a comprehensive feature set, but focuses on improving usability and efficiency when debugging basic communication protocols.

> **⚠️ Note: This tool is designed ONLY for development, testing, and learning purposes. It provides NO guarantees regarding communication reliability or stability. Do NOT use it in any production environment or mission-critical system. See the Disclaimer below.**

### 🔌 1. Fast Connection & Frame Building
- **Intuitive Configuration**: Modbus TCP / RTU connection parameters are located on the left panel for quick setup.
- **Quick Command Sending**: Fill in `Slave ID`, `Start Address`, and `Data`, then click the corresponding function code button to send. The program automatically builds the frame and calculates the checksum (CRC/LRC).
- **Flexible Data Formats**: Supports switching between `Hex` and `Decimal` formats to accommodate different data input requirements.
- **Raw Mode**: Provides a Raw mode for sending custom Hex frames, suitable for verifying non-standard protocols or abnormal workflows.

![Modbus-TCP Fast Frame Builder](docs/images/modbus-tcp-frame-builder.png)

### 🕵️‍♂️ 2. Traffic Monitor & Logging
- **Separated TX/RX Display**: Transmitted (TX) and received (RX) data are identified by different colors, accompanied by millisecond-level timestamps.
- **Data Stream Filtering**: Supports filtering to view only TX or RX data via checkboxes, facilitating observation of specific directional frames during continuous polling.
- **Copy & Record**: Supports quick copying of individual frame contents, and the current complete communication log can be exported and saved to assist in test recording.

### 🧩 3. Frame Analyzer
This module is used to assist in analyzing Hex frames. After parsing, it displays the frame structure and data in a tabular format.
- **Decode Mode Switch**: Supports switching between `Unsigned` and `Signed` modes, synchronously updating the corresponding decimal, Hex, binary, and numerical values.
- **Multiplier Scaling (Scale)**: Supports setting conversion multipliers (e.g., `0.1`) for specific registers to visually display the converted engineering values in the table.
- **Register Description**: Supports adding custom descriptions (e.g., "Voltage", "Speed") to register addresses for easy cross-referencing.
- **Configuration Persistence & JSON Templates**: Configured multipliers and descriptions are automatically saved. Configurations can be exported as JSON templates for reuse when testing similar devices.
- **Utility Features**: Provides Hex format cleaning, response frame start address configuration, and basic protocol auto-detection (TCP/RTU).

![Frame Analyzer Overview](docs/images/frame-analyzer-overview.png)
![Frame Analyzer Address Scale Description](docs/images/frame-analyzer-address-scale-description.png)

### 🛠️ 4. Auxiliary Tools & Configuration
In addition to the core Modbus functionality, some basic generic testing tools are provided:
- **Generic TCP Client**: For basic custom network frame transmission and reception testing.
- **Generic Serial Tool**: For basic serial port transmission and reception testing (supports ASCII/Hex).
- **Request Statistics Panel**: Provides basic TX / RX / FAIL / RTT statistical data.
- **Retry Strategy Configuration**: Allows custom configuration of Modbus request timeouts, retry counts, and retry intervals.

## Tech Stack

- C++20
- Qt6 (Core / Network / SerialPort / Widgets / Charts)
- CMake (>= 3.16)
- spdlog (submodule)

## Project Structure

```text
Modbus-Tools/
├─ app/           # App entry (main.cpp)
├─ core/          # Communication + Modbus core logic
├─ ui/            # Qt UI, views, widgets, i18n resources
├─ third_party/   # Third-party deps (spdlog submodule)
└─ CMakeLists.txt # Top-level build config
```

## Build (Windows / MSVC)

### 1) Clone (with submodules)

```bash
git clone https://github.com/mingyucheng692/Modbus-Tools.git
cd Modbus-Tools
```

If already cloned without submodules:

```bash
git submodule update --init --recursive --progress
```

### 2) Configure

```bash
cmake -S . -B build
```

### 3) Build

```bash
cmake --build build --config Release
```

### 4) Run

```bash
.\build\app\Release\Modbus-Tools.exe
```

## Language

After launch:

- Default: `en_US`
- Menu bar `Language` → `简体中文 (zh_CN)` / `繁體中文 (zh_TW)`

## Requirements

- Qt6 installed and discoverable by CMake (`find_package(Qt6 ...)`)
- Visual Studio 2022 (or compatible MSVC toolchain)
- CMake 3.16+

## ⚠️ Disclaimer

### 1. Nature of the Software
This software is provided **"AS IS"** without any express or implied warranties, including but not limited to merchantability, fitness for a particular purpose, and non-infringement. This software is a **free open-source tool developed in spare time**.

### 2. Intended Use
This tool is designed only for **testing, debugging, and learning purposes**. It supports multiple communication protocols including Modbus, TCP, and serial. It is not suitable for mission-critical systems, production environments, or any scenario where failures could cause major losses.

### 3. Risk Warning
Improper use of this software may cause:
- **Equipment failure or damage** – sending incorrect commands or data may alter parameters or cause hardware faults
- **Data loss or corruption** – device configuration or stored data may be overwritten or lost
- **Production interruption** – incorrect operations may stop industrial processes or network services
- **Network security risks** – improper TCP/serial operations may expose vulnerabilities or interrupt communication
- **Property loss or personal injury** – in hazardous environments, incorrect operations may cause safety incidents

### 4. Limitation of Liability
The developer (mingyucheng692) **assumes no responsibility** for:
- any direct, indirect, incidental, special, or consequential damages
- loss of profits, data, or business opportunities
- equipment repair or replacement costs
- production downtime and related losses
- network interruption or communication failures
- legal disputes arising from use of this software

### 5. User Responsibilities
By using this software, you agree to:
- ✅ Fully understand the communication protocols (Modbus, TCP, serial) and your device specifications
- ✅ Perform sufficient testing in a **safe, isolated environment** before industrial deployment
- ✅ Confirm target device address, port, parameters, and data before any operation
- ✅ Back up original data before modifying device configuration
- ✅ Ensure operations comply with network and industrial safety policies
- ✅ Assume all risks of using this software
- ✅ Comply with all applicable laws and regulations

### 6. No Professional Advice
This software does not constitute professional engineering advice. For industrial deployment, consult qualified engineers and follow your organization’s safety procedures.

### 7. Agreement
**Downloading, installing, or using this software indicates that you have read, understood, and agreed to this disclaimer. If you do not agree, do not use this software.**

## 🙏 Acknowledgements

This project uses the following open-source library. Thanks to the author for the outstanding contribution:

- **[spdlog](https://github.com/gabime/spdlog)** (MIT License): A high-performance C++ logging library that provides reliable logging support for this project.

## Status

This is an evolving personal debugging tool. Feel free to adapt it to your own workflow.
