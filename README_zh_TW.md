# Modbus-Tools
[English](README.md) | [简体中文](README_zh_CN.md) | [繁體中文](README_zh_TW.md)
[![spdlog](https://img.shields.io/badge/spdlog-logging-blue?logo=github)](https://github.com/gabime/spdlog)

以 C++20 與 Qt 6 打造的工業級 Modbus TCP/RTU 除錯套件，主打高效能與工程化除錯體驗。本專案為個人業餘時間開發，特別感謝 spdlog。

## 安裝
### 環境需求
- **編譯器**: MSVC 2019+ (需支援 C++20)
- **框架**: Qt 6.x (包含 Widgets, Network, SerialPort, Charts 模組)
- **建置系統**: CMake 3.16+
- **第三方庫**: spdlog (已包含在工程中)

### 原始碼編譯
```bash
cmake -S . -B build;cmake --build build --config Release --parallel
```

### 執行
```bash
./build/bin/Release/ModbusTools.exe
```

## 支援平台
- Windows (MSVC 2019+, x64)

## 核心特性
- **多協定支援**: 完整支援 Modbus TCP、Modbus RTU 以及通用 TCP/串列埠除錯模式。
- **先進架構**: 基於 Reactor 模式的 I/O 設計，採用獨立工作執行緒，確保介面流暢與高並發處理能力。
- **Docking 介面**: 高度可自訂的停靠視窗布局（連線、日誌、流量監控、波形顯示）。
- **協定分析器**: 提供十六進位流、協定樹視圖及詳細欄位解釋的深度封包分析。
- **波形監控**: 支援右鍵將暫存器數值新增至示波器進行即時曲線繪製。
- **實體模擬**: 支援模擬封包遺失率與網路延遲，用於測試系統的健壯性。

## 使用範例
1. **連線設定**: 在左側 "Connection" 面板選擇協定類型 (TCP/RTU)，配置相應參數並連線。
2. **指令送出**: 使用中間的控制面板發送 Modbus 讀寫請求。
3. **報文分析**: "Traffic Monitor" 顯示原始流量，"Frame Analyzer" 提供深度解析。
4. **波形顯示**: 在分析器中右鍵點擊數值欄位，選擇 "Add to Waveform" 即可查看即時趨勢。

## 版權說明
僅供內部學習與使用。
