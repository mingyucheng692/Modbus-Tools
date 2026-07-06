<div align="center">

<img src="assets/logo.svg" alt="Modbus-Tools Logo" width="120">

# Modbus-Tools
基於 Qt 與 C++ 的桌面端 Modbus 調試與分析工具。

[English](README.md) | [简体中文](README_zh-CN.md) | **繁體中文**

</div>

## 專案定位

`Modbus-Tools` 面向日常調試、聯調與問題定位，不做過度包裝。
目前工程實際提供的能力包括：

- `Modbus TCP Client`
- `Modbus RTU Master`
- `Frame Analyzer`，用於解析 Modbus TCP / RTU 原始報文
- `Link to Analyzer`，用於把即時 Modbus 回應聯動到解析器
- 通用 `TCP Client / TCP Server / UDP` 調試頁面
- 獨立的 `Serial Debugger` 頁面

它適合開發、驗證、實驗室與現場排障，不是安全認證產品。

## 核心能力

- 常見 Modbus 讀寫流程的可視化請求構建
- RTU `CRC16` 追加與 TCP `MBAP` 封裝等原始報文輔助能力
- 輪詢請求與回應/錯誤追蹤
- 報文解析、位元組序處理和縮放因子換算
- 英文、簡體中文、繁體中文三語介面

## 平台範圍

- 原始碼構建支援 `Windows`、`Linux`、`macOS`
- 發布打包透過 CMake 目標 `release_bundle` 完成
- 內建更新器 / 自動更新目前僅支援 `Windows`
- 在非 Windows 平台上，主程式仍可構建，但 updater 目標會按設計停用

## 構建

### 下載執行

- Windows 使用者可直接從 [Releases](https://github.com/mingyucheng692/Modbus-Tools/releases) 下載預編譯套件

### 從原始碼構建

單配置產生器，例如 `Ninja`：

```bash
git clone --recursive https://github.com/mingyucheng692/Modbus-Tools.git
cd Modbus-Tools
cmake -S . -B build -DMODBUS_TOOLS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --target Modbus-Tools --parallel
```

多配置產生器，例如 `Visual Studio 17 2022`：

```bash
git clone --recursive https://github.com/mingyucheng692/Modbus-Tools.git
cd Modbus-Tools
cmake -S . -B build -G "Visual Studio 17 2022" -DMODBUS_TOOLS_BUILD_TESTS=ON
cmake --build build --target Modbus-Tools --config Release --parallel
```

### 生成發布包

單配置產生器，例如 `Ninja`：

```bash
cmake -S . -B build_release -DMODBUS_TOOLS_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build_release --target release_bundle --parallel
```

多配置產生器，例如 `Visual Studio 17 2022`：

```bash
cmake -S . -B build_release -G "Visual Studio 17 2022" -DMODBUS_TOOLS_BUILD_TESTS=OFF
cmake --build build_release --target release_bundle --config Release --parallel
```

### 說明

- 工具鏈基線：`C++20`、`Qt 6`、`CMake 3.16+`
- 可選開關包括 `MODBUS_TOOLS_ENABLE_ASAN`、`MODBUS_TOOLS_ENABLE_UBSAN`、`MODBUS_TOOLS_ENABLE_CLANG_TIDY`、`MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS`
- Linux CI 會先安裝額外的 Qt/XCB 執行與構建依賴

## 品質與測試

自動化檢查是工程護欄，不是認證結論。

- 測試按目標拆分為 `test_core_logic`、`test_qt_core`、`test_ui_widgets`、`test_integration`、`test_modbus_stability`、`test_modbus_concurrency`、`test_modbus_fuzz`
- CI 在 `Windows`、`Linux`、`macOS` 上執行構建矩陣
- Linux CI 包含非阻斷 `clang-tidy`
- 開啟 ASan 測試時，Linux 會構建並執行確定性 fuzz 目標

### 在地執行測試

```bash
cmake -S . -B build -DMODBUS_TOOLS_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

如果使用 Visual Studio 這類多組態生成器，請在構建命令中增加 `--config Debug` 或 `--config Release`，並在 `ctest` 中增加 `-C Debug` 或 `-C Release`。

## 倉庫結構

```text
core/   協定、工作階段、傳輸、解析、更新邏輯
infra/  IO、日誌、平台輔助層
ui/     頁面、元件、展示邏輯、多語言
tests/  單元測試、整合測試、定向回歸目標
app/    桌面程式入口
```

## 邊界說明

- 目前 Modbus 協定能力僅涵蓋 `master` 端：`Modbus TCP master（client）` 與 `Modbus RTU master`
- 通用網路調試能力與 Modbus 角色是兩套功能：另有獨立的 `TCP client`、`TCP server`、`UDP` 助手頁面
- 自動更新目前僅支援 `Windows`
- 同時調試多條鏈路時，啟動多個應用實例 —— 一實例一工作階段
- 倉庫優先關注執行期正確性、穩定性與可調試性，而不是功能堆疊
- 軟體按 `MIT` 許可證提供，定位仍然是開發與排障工具

## 參與貢獻

歡迎提交 Issue 與 Pull Request。

如果提交程式碼，請使用 `git commit -s`，讓每個提交包含 DCO 簽名：

```text
Signed-off-by: 您的名字 <your.email@example.com>
```

## 說明

- 使用邊界、無擔保與責任限制見 [DISCLAIMER.md](DISCLAIMER.md)
- 致謝：感謝 `Qt`、`spdlog`、`fmt` 與 `GoogleTest`

## 授權

本專案基於 [MIT License](LICENSE) 開源。
