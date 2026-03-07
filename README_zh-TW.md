# Modbus-Tools

語言: [English](README.md) | [簡體中文](README_zh-CN.md) | [繁體中文](README_zh-TW.md)

這是一個利用業餘時間開發的 Modbus／通用通訊除錯助手，聚焦於快速連線、快速收發與快速分析。

- 預設介面語言：`en_US`
- 可在介面中快速切換：`簡體中文 (zh_CN)`、`繁體中文 (zh_TW)`

## 功能概覽

- Modbus TCP 除錯：連線、功能碼請求、收發監控
- Modbus RTU 除錯：序列埠參數設定、請求送出、回應解析
- 通用 TCP Client：自訂資料收發與流量監控
- 通用序列埠工具：序列埠連線、原始資料收發
- 幀分析器：對 Modbus 報文做結構化解析
- 可設定 Modbus 逾時、重試次數、重試間隔

## 技術棧

- C++20
- Qt6（Core / Network / SerialPort / Widgets / Charts）
- CMake (>= 3.16)
- spdlog（以子模組方式引入）

## 工程結構

```text
Modbus-Tools/
├─ app/           # 程式入口（main.cpp）
├─ core/          # 通訊與 Modbus 核心邏輯
├─ ui/            # Qt 介面、檢視、元件、多語系資源
├─ third_party/   # 第三方相依（目前包含 spdlog 子模組）
└─ CMakeLists.txt # 頂層建置設定
```

## 建置說明（Windows / MSVC）

### 1) 取得程式碼（含子模組）

```bash
git clone https://github.com/mingyucheng692/Modbus-Tools.git
cd Modbus-Tools
```

若已克隆但未拉子模組：

```bash
git submodule update --init --recursive
```

### 2) 設定工程

```bash
cmake -S . -B build
```

### 3) 編譯

```bash
cmake --build build --config Release
```

### 4) 執行

```bash
.\build\app\Release\ModbusTools.exe
```

## 語言切換

程式啟動後：

- 預設使用 `en_US`
- 在選單列點擊 `Language`
- 可切換到 `簡體中文 (zh_CN)` 或 `繁體中文 (zh_TW)`

## 相依需求

- 已安裝 Qt6，且可被 CMake 找到（`find_package(Qt6 ...)`）
- Visual Studio 2022（或相容的 MSVC 工具鏈）
- CMake 3.16 以上

## 目前狀態

這是一個持續迭代的個人除錯工具專案，歡迎使用與擴充。
