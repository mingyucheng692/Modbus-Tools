<div align="center">

<img src="assets/logo.svg" alt="Modbus-Tools Logo" width="120">

# Modbus-Tools
### 专业的 Modbus TCP Client 与 Modbus RTU Master 调试及协议分析工具

**可视化帧构建 · 实时报文解析 · 可靠的连接策略**

[![GitHub Release](https://img.shields.io/github/v/release/mingyucheng692/Modbus-Tools?style=flat-square)](https://github.com/mingyucheng692/Modbus-Tools/releases) [![Release Status](https://github.com/mingyucheng692/Modbus-Tools/actions/workflows/release.yml/badge.svg?style=flat-square)](https://github.com/mingyucheng692/Modbus-Tools/actions/workflows/release.yml) [![License](https://img.shields.io/badge/License-MIT-blue.svg?style=flat-square)](LICENSE)[![Stars](https://img.shields.io/github/stars/mingyucheng692/Modbus-Tools?style=flat-square&logo=github)](https://github.com/mingyucheng692/Modbus-Tools/stargazers) [![Forks](https://img.shields.io/github/forks/mingyucheng692/Modbus-Tools?style=flat-square&logo=github)](https://github.com/mingyucheng692/Modbus-Tools/network/members) [![C++20](https://img.shields.io/badge/C%2B%2B-20-orange.svg?style=flat-square)](https://isocpp.org/std/the-standard) [![Qt6](https://img.shields.io/badge/Qt-6.x-41CD52.svg?style=flat-square)](https://www.qt.io)[![CMake](https://img.shields.io/badge/CMake-3.16+-064F8C?style=flat-square&logo=cmake&logoColor=white)](https://cmake.org/)[![Google Test](https://img.shields.io/badge/Google_Test-1.12+-4285F4?style=flat-square&logo=google&logoColor=white)](https://github.com/google/googletest)

[English](README.md) | **简体中文** | [繁體中文](README_zh-TW.md)

</div>

---

## 🔍 项目概述 (About)

**Modbus-Tools** 是一款专为 **工业物联网 (IIoT) 调试**、**嵌入式系统开发**及**现场总线分析**设计的现代化开源辅助工具。作为功能丰富的 **Modbus 调试助手**，它支持作为 **Modbus TCP Client** 与 **Modbus RTU Master** 主动发起通信，有效支持了工业现场中常见的 **寄存器读取与写入** 验证需求。

本项目核心亮点在于其内置的 **报文解析器 (Frame Analyzer)**，能够识别二进制字节流并进行结构化拆解。配合特色的 **Link to Analyzer (实时联动分析)** 模式，开发者可以在串口通信测试或网络调试过程中，同步观察协议帧的原始数据及其物理意义。无论是验证从站设备、定位通信故障，还是分析复杂的采样数据，Modbus-Tools 都能提供稳健、可靠且高效的工程化支持。

---

## ✨ 核心特性 (Features)

### 🏗 可视化帧构建与原始发送
提供了直观的工业协议组包交互，简化了调试流程：
- **参数表单化**：支持 Slave ID、起始地址、寄存器数量等核心参数的图形化输入。Slave ID 与地址（含轮询地址）全面支持 **HEX (0x10, 10H)** 与 **DEC (16)** 智能识别与合法性校验。
- **功能码覆盖**：支持常用的 0x01-0x06, 0x0F, 0x10 等标准 Modbus 功能码。
- **Raw 辅助工具**：内置“一键计算并追加 CRC16 (RTU)”及“一键封装主站报头 MBAP (TCP)”功能，极大简化非标报文测试。
- **实时预览**：在输入参数及 Hex 数据时，同步展示对应的十六进制原始字节流，确保发出的每一帧都符合预期。
  
> [!TIP]
> **配置提示**：轮询栏的 `Addr` 支持独立跨段配置，而 `Slave ID` 与功能面板实时联动。

### 🔘 线圈 (Coils) 二进制下发交互
专为位操作设计的交互逻辑，实现对输出状态的直观控制：
- **Binary 输入模式**：支持直接输入比特串（如 `1 0 1 1`），系统自动将其编码并配合 0x05 (单写) 或 0x0F (多写) 功能码下发。
- **位级感知**：配合 0x01/0x02 读取指令，实现对远程设备线圈与离散输入状态的高效验证。

### 📊 特色帧解析器 (Frame Analyzer)
将枯燥的十六进制报文转化为结构化的业务数据，支持深度分析：
- **协议拆解**：即时将 Hex 流拆解为 SlaveID、功能码、起始地址、数据长度及校验值等关键字段。
- **工程量转换**：内置 **缩放因子 (Scale Factor)**，支持将寄存器原始值自动转换为浮点物理量（如温度、压力、频率）。
- **多维字节序分析**：支持 **ABCD (Big Endian)**、**CDAB (Little Endian Byte Swap)**、**BADC (Big Endian Byte Swap)**、**DCBA (Little Endian)** 四种字节/字序模式，适配各类 PLC 数据排列。
- **语义标注**：支持为寄存器自定义描述，解析结果反映真实的业务指标，并支持历史记录批量导出。

### 🔗 Link to Analyzer (实时联动分析) — **特色功能**
打破流量监控与解析器之间的壁垒，实现自动化的解析流：
- **实时穿透**：捕获的 **RX 响应报文** 可自动推送至解析器解码，消除频繁拷贝 Hex 数据的繁琐操作。
- **暂停编辑**：支持 **“暂停刷新”** 功能，方便在高速通信场景下驻留特定帧，进而编辑其缩放因子或寄存器描述。
- **异步解析**：解析动作在后台异步完成，不干扰前端流量列表的实时滚动观测。

---

## 📋 技术规格与支持特性

### 📡 标准协议支持
| 维度 | 支持能力 | 技术细节 |
| :--- | :--- | :--- |
| **通信模式** | **Modbus TCP Client** & **Modbus RTU Master** | 适配网口 (RJ45) 与串口 (RS232/RS485) 物理链路 |
| **功能码集** | FC01 ~ FC04 (读) / FC05 ~ FC06 (单写) / FC15 ~ FC16 (多写) | 涵盖工业现场主流的 Modbus 应用场景 |
| **校验机制** | 自动 CRC16 (RTU) / MBAP 头部封装 (TCP) | 生成并验证报文的合规性与完整性 |

### 🛠 通用调试工具
除了深度的 Modbus 协议支持，项目同时集成了高性能的通用调试能力：
- **通用 TCP 客户端**：支持文本/Hex 双模式发送，适配各类非标网络协议验证。
- **串口通信助手**：支持自定义波特率、校验位及数据位，集成文件传输等实用功能。

---

## 📸 界面预览 (Screenshots)

<table>
  <tr>
    <td align="center"><b>Modbus TCP 帧构建</b></td>
    <td align="center"><b>Modbus TCP 写入操作</b></td>
  </tr>
  <tr>
    <td><img src="docs/images/modbus-tcp-frame-builder.png" alt="Modbus TCP 模式" width="400"></td>
    <td><img src="docs/images/modbus-tcp-write-decimal.png" alt="Modbus 写入" width="400"></td>
  </tr>
  <tr>
    <td align="center"><b>报文分析器 - 地址,缩放因子与描述</b></td>
    <td align="center"><b>报文分析器 - 结构树</b></td>
  </tr>
  <tr>
    <td><img src="docs/images/frame-analyzer-address-scale-description.png" alt="地址,缩放因子,描述" width="400"></td>
    <td><img src="docs/images/frame-analyzer-overview.png" alt="结构树解析" width="400"></td>
  </tr>
</table>

### 📺 动态演示 (Demo)
![Linkage & Smart Input Demo](docs/images/demo.gif)

---


---

## 🚀 快速开始 (Getting Started)

### 下载与运行 (Windows)
1. 前往 [Releases](https://github.com/mingyucheng692/Modbus-Tools/releases) 页面。
2. 下载最新的 Modbus-Tools-win64.zip。
3. 解压并运行 Modbus-Tools.exe 即可，**无需安装，绿色运行**。

### 源码构建 (Build from Source)
项目基于 **Qt 6.x** 与 **CMake** 构建，支持 MSVC 编译器：
```bash
# 克隆项目及其所有子模块
git clone --recursive https://github.com/mingyucheng692/Modbus-Tools.git  --progress
cd Modbus-Tools

# 使用 CMake 配置并生成工程
cmake -S . -B build

# 执行编译（以 Release 为例）
cmake --build build --config Release -j
```

## 🧪 开发与测试

本项目集成 Google Test 单元测试框架，核心协议解析逻辑已实现高比例覆盖。

- **自动化测试**：
  - **覆盖范围**：涵盖会话层 (Session)、物理通道 (Transport) 及关键报文解析引擎 (Frame Analyzer) 的边界情况与核心逻辑。
  - **触发机制**：系统依托 GitHub Actions，在 **Git Tag (版本发布)** 时自动执行全量集成验证、单元测试及分发流程，确保发布版本的稳定性。

- **本地运行测试**：
  ```powershell
  cmake -B build -DMODBUS_TOOLS_BUILD_TESTS=ON
  cmake --build build --target modbus_test
  ctest --test-dir build -C Debug --output-on-failure
  ```

- **测试覆盖**：已实现对会话层 (Session)、传输层 (Transport) 及核心解析引擎 (Frame Analyzer) 的核心逻辑验证，确保复杂连接环境下的稳定性。
- **持续集成**：项目维护了 GitHub Action 流水线，针对正式发布的版本进行自动化的回归测试与二进制产物分发。

---

## ⚙️ 高级配置与工程化能力

为了适配复杂的工业现场环境，Modbus-Tools 提供了丰富的通信控制策略：
- **重试机制**：支持自定义失败后的重试策略。
- **多语言环境**：完整支持英文、简体中文及繁体中文 UI。
- **OTA 自动更新**：集成 GitHub API，支持检测并升级至最新稳定版。

---

## 🏗 项目架构

### 技术栈

| 组件 | 技术 | 说明 |
| :--- | :--- | :--- |
| 编程语言 | **C++20** | 现代 C++：constexpr, enum class, 智能指针, std::optional |
| GUI 框架 | **Qt 6** | Widgets, Charts, Network, SerialPort, Concurrent |
| 日志系统 | **spdlog** | 异步日志 + 文件轮转（10MB × 20 文件） |
| 构建系统 | **CMake 3.16+** | 跨平台构建支持，集成 MSVC 并行编译优化 |
| 单元测试 | **Google Test** | 广泛覆盖核心协议逻辑，确保产品级稳定性 |
| CI/CD | **GitHub Actions** | 自动化流水线：构建、测试及 Release 解析包分发 |

### 分层架构

```text
┌─────────────────────────────────────────────────────────────┐
│                       UI 层 (ui/)                           │
│   MainWindow │ Views │ Widgets │ Theme │ i18n │ Settings    │
├─────────────────────────────────────────────────────────────┤
│                     业务逻辑层 (core/)                        │
│   ┌──────────┐   ┌────────────┐   ┌─────────────────────┐   │
│   │  Modbus  │   │   I/O 通道  │   │      通用服务        │   │
│   │  协议栈   │   │  (Channel) │   │  Logger/Settings等  │   │
│   └──────────┘   └────────────┘   └─────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│                      基础设施层                               │
│         Qt6 Framework │ spdlog │ C++ Standard Library       │
└─────────────────────────────────────────────────────────────┘
```

---

## 🎯 适用场景

| 场景 | 描述 |
| :--- | :--- |
| **设备联调** | 快速验证 Modbus 从站的寄存器读写是否正常 |
| **协议分析** | 抓取并解析 Modbus 帧，定位通信故障 |
| **批量测试** | 轮询模式下持续读取寄存器，监测数据变化 |
| **非标协议调试** | Raw 模式发送自定义 Hex 帧 |
| **教学演示** | 直观展示 Modbus 帧结构与通信过程 |

### 目标用户

- 🏭 **工业自动化工程师**：PLC / DCS / SCADA / HMI / 仪器仪表 / BMS / PCS / EMS 系统调试
- 🔧 **嵌入式开发者**：Modbus 从站设备开发验证
- 🔗 **系统集成商**：现场总线通信问题排查
- 📚 **学习者**：Modbus 协议学习与研究

---

## 🤝 参与贡献

欢迎提交 Issue 和 Pull Request，我们非常重视来自社区的贡献（Bug 修正、新功能、文档优化等）！

1. **Fork** 本仓库
2. **创建特性分支**：`git checkout -b feature/amazing-feature`
3. **提交更改**：`git commit -m 'Add amazing feature'`
4. **推送到远程分支**：`git push origin feature/amazing-feature`
5. **提交 Pull Request** (PR)

您的每一个 ⭐ Star 都是支持我们持续迭代的最大动力。

### 贡献者许可协议 (DCO)

本项目采用 **开发者原创证书 (DCO)** 确保贡献的知识产权清晰。提交 Pull Request 时，请确保您的每个 commit 包含如下签名行：

```
Signed-off-by: 您的名字 <您的邮箱>
```

使用 `git commit -s` 可自动添加此行。通过签署 DCO，您确认：
- 该贡献是由您创作和/或您有权提交；
- 您同意该贡献按本项目的 MIT 许可证授权；
- 您理解该贡献可能会被修改或重新分发。

---

## 📄 开源许可

本项目基于 [MIT License](LICENSE) 开源。

Copyright © 2025 - present mingyucheng692

### 🔄 二次分发要求
- 任何对本项目的修改、衍生作品或二次分发行为，**必须完整保留原始版权声明及本许可文件**。
- 分发时不得删除、篡改或隐藏本项目源码、文档及构建产物中的任何版权标识、作者署名与许可声明。

### ⚖️ 第三方依赖许可证摘要

本项目主要使用以下第三方库。用户在二次开发或分发时，应遵守其原有的开源许可协议。

| 依赖项 | 许可证 | 说明 |
| :--- | :--- | :--- |
| **Qt 6** | LGPLv3 | 以动态链接方式使用，支持用户自由替换库文件，未修改源码。 |
| **spdlog** / **fmt** | MIT | 广泛应用于日志记录与格式化，需保留原始版权声明。 |
| **Google Test** | BSD-3-Clause | 仅用于开发环境测试。 |


---

## ⚖️ 免责声明

### 1. 用途限制
本项目仅适用于开发调试、设备检测及教学演示。在此类关键或生产环境中使用，需用户自行承担全部风险。

### 2. 无担保声明
本软件按 "原样" (AS IS) 提供。不保证软件满足特定需求，不对 Modbus 帧构建或解析结果的准确性提供绝对担保。

### 3. 责任限制
在任何情况下，作者或版权持有人均不承担因使用本软件导致的直接或间接损害责任。

### 4. 合规性说明
本软件未通过任何工业安全认证（如 SIL, IEC 61508），校验算法仅供调试参考。用户需确保其使用符合当地法律。

### 5. 数据与隐私

本软件尊重用户隐私，遵循最小数据原则：

- **本地数据**：通信配置、解析模板及用户偏好均存储于本地。
- **自动更新**：更新检查功能通过 HTTPS 访问 GitHub API，仅发送应用版本号及平台标识。
- **网络通信**：Modbus TCP 连接由用户主动发起，仅连接指定的地址。
- **不收集**：本软件不收集、存储或传输用户个人及敏感信息。

---

### 🙏 致谢

感谢以下开源项目使 Modbus-Tools 成为可能：
- [Qt](https://www.qt.io) — 跨平台应用框架
- [spdlog](https://github.com/gabime/spdlog) — 高性能 C++ 日志库
- [fmt](https://github.com/fmtlib/fmt) — 现代 C++ 格式化库
