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
.\build\app\Release\ModbusTools.exe
```

## Language

After launch:

- Default: `en_US`
- Menu bar `Language` → `简体中文 (zh_CN)` / `繁體中文 (zh_TW)`

## Requirements

- Qt6 installed and discoverable by CMake (`find_package(Qt6 ...)`)
- Visual Studio 2022 (or compatible MSVC toolchain)
- CMake 3.16+

## Status

This is an evolving personal debugging tool. Feel free to adapt it to your own workflow.
