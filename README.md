# Modbus-Tools

![Modbus-Tools Logo](assets/logo.svg)

Languages: [English](README.md) | [简体中文](README_zh-CN.md) | [繁體中文](README_zh-TW.md)

A personal Modbus / generic communication debugging assistant built in spare time, focused on fast connect, fast send/receive, and fast analysis.

- Default UI language: `en_US`
- Quick switch to: `zh_CN`, `zh_TW`

## Features

- Modbus TCP debugging: connect, function code requests, TX/RX monitoring
- Modbus RTU debugging: serial config, request sending, response parsing
- Generic TCP client: custom send/receive with traffic monitoring
- Generic serial tool: serial connect and raw data TX/RX
- Frame analyzer: structured Modbus frame parsing
- Configurable Modbus timeout, retry count, retry interval

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
git submodule update --init --recursive
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

## Status

This is an evolving personal debugging tool. Feel free to adapt it to your own workflow.
