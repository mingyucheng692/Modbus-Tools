<div align="center">

<img src="assets/logo.svg" alt="Modbus-Tools Logo" width="120">

# Modbus-Tools
基于 Qt 与 C++ 的桌面端 Modbus 调试与分析工具。

[English](README.md) | **简体中文** | [繁體中文](README_zh-TW.md)

</div>

## 项目定位

`Modbus-Tools` 面向日常调试、联调和问题定位，不做过度包装。
当前工程实际提供的能力包括：

- `Modbus TCP Client`
- `Modbus RTU Master`
- `Frame Analyzer`，用于解析 Modbus TCP / RTU 原始报文
- `Link to Analyzer`，用于把实时 Modbus 响应联动到解析器
- 通用 `TCP Client / TCP Server / UDP` 调试页面
- 独立的 `Serial Debugger` 页面

它适合开发、验证、实验室和现场排障，不是安全认证产品。

## 核心能力

- 常见 Modbus 读写流程的可视化请求构建
- RTU `CRC16` 追加与 TCP `MBAP` 封装等原始报文辅助能力
- 轮询请求与响应/错误跟踪
- 报文解析、字节序处理和缩放因子换算
- 英文、简体中文、繁体中文三语界面

## 平台范围

- 源码构建支持 `Windows`、`Linux`、`macOS`
- 发布打包通过 CMake 目标 `release_bundle` 完成
- 内置更新器 / 自动更新目前仅支持 `Windows`
- 在非 Windows 平台上，主程序仍可构建，但 updater 目标会按设计禁用

## 构建

### 下载安装

- Windows 用户可直接从 [Releases](https://github.com/mingyucheng692/Modbus-Tools/releases) 下载预编译包

### 从源码构建

单配置生成器，例如 `Ninja`：

```bash
git clone --recursive https://github.com/mingyucheng692/Modbus-Tools.git
cd Modbus-Tools
cmake -S . -B build -DMODBUS_TOOLS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --target Modbus-Tools --parallel
```

多配置生成器，例如 `Visual Studio 17 2022`：

```bash
git clone --recursive https://github.com/mingyucheng692/Modbus-Tools.git
cd Modbus-Tools
cmake -S . -B build -G "Visual Studio 17 2022" -DMODBUS_TOOLS_BUILD_TESTS=ON
cmake --build build --target Modbus-Tools --config Release --parallel
```

### 生成发布包

单配置生成器，例如 `Ninja`：

```bash
cmake -S . -B build_release -DMODBUS_TOOLS_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build_release --target release_bundle --parallel
```

多配置生成器，例如 `Visual Studio 17 2022`：

```bash
cmake -S . -B build_release -G "Visual Studio 17 2022" -DMODBUS_TOOLS_BUILD_TESTS=OFF
cmake --build build_release --target release_bundle --config Release --parallel
```

### 说明

- 工具链基线：`C++20`、`Qt 6`、`CMake 3.16+`
- 可选开关包括 `MODBUS_TOOLS_ENABLE_ASAN`、`MODBUS_TOOLS_ENABLE_UBSAN`、`MODBUS_TOOLS_ENABLE_CLANG_TIDY`、`MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS`
- Linux CI 会先安装额外的 Qt/XCB 运行和构建依赖

## 质量与测试

自动化检查是工程护栏，不是认证结论。

- 测试按目标拆分为 `test_core_logic`、`test_qt_core`、`test_ui_widgets`、`test_integration`、`test_modbus_fuzz`
- CI 在 `Windows`、`Linux`、`macOS` 上执行构建矩阵
- Linux CI 包含非阻断 `clang-tidy`
- 开启 ASan 测试时，Linux 会构建并运行确定性 fuzz 目标

### 本地运行测试

```bash
cmake -S . -B build -DMODBUS_TOOLS_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

如果使用 Visual Studio 这类多配置生成器，请在构建命令中增加 `--config Debug` 或 `--config Release`，并在 `ctest` 中增加 `-C Debug` 或 `-C Release`。

## 仓库结构

```text
core/   协议、会话、传输、解析、更新逻辑
infra/  IO、日志、平台辅助层
ui/     页面、控件、展示逻辑、多语言
tests/  单元测试、集成测试、定向回归目标
app/    桌面程序入口
```

## 边界说明

- 当前 Modbus 协议能力仅覆盖 `master` 侧：`Modbus TCP master（client）` 与 `Modbus RTU master`
- 通用网络调试能力与 Modbus 角色是两套功能：另有独立的 `TCP client`、`TCP server`、`UDP` 助手页面
- 自动更新当前仅支持 `Windows`
- 同时调试多条链路时，启动多个应用实例 —— 一实例一会话
- 仓库优先关注运行时正确性、稳定性和可调试性，而不是功能堆叠
- 软件按 `MIT` 许可证提供，定位仍然是开发与排障工具

## 参与贡献

欢迎提交 Issue 和 Pull Request。

如果提交代码，请使用 `git commit -s`，让每个提交包含 DCO 签名：

```text
Signed-off-by: 您的名字 <your.email@example.com>
```

## 说明

- 使用边界、无担保与责任限制见 [DISCLAIMER.md](DISCLAIMER.md)
- 致谢：感谢 `Qt`、`spdlog`、`fmt` 与 `GoogleTest`

## 许可证

本项目基于 [MIT License](LICENSE) 开源。
