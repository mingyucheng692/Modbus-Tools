# Modbus-Tools

![Modbus-Tools Logo](assets/logo.svg)

語言: [English](README.md) | [簡體中文](README_zh-CN.md) | [繁體中文](README_zh-TW.md)

這是一個利用業餘時間開發的 Modbus／通用通訊除錯助手，聚焦於快速連線、快速收發與快速分析。

- 預設介面語言：`en_US`
- 可在介面中快速切換：`簡體中文 (zh_CN)`、`繁體中文 (zh_TW)`

## 功能概覽

- Modbus TCP 除錯：連線、功能碼請求、收發監控
- Modbus RTU 除錯：序列埠參數設定、請求送出、回應解析
- Modbus TCP/RTU 統一請求統計：TX / RX / FAIL / RTT
- 通用 TCP Client：自訂資料收發與流量監控
- 通用序列埠工具：序列埠連線、原始資料收發
- 幀分析器：對 Modbus 報文做結構化解析，支援中繼資料編輯（Scale、Description）
- 幀分析器資料表支持換算後 Value 欄（`raw * scale`），並支援欄寬拖曳調整
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
git submodule update --init --recursive --progress
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
.\build\app\Release\Modbus-Tools.exe
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

## ⚠️ 免責聲明

### 1. 軟體性質
本軟體按 **「原樣」（AS IS）** 提供，不提供任何形式的明示或暗示保證，包括但不限於適銷性、特定用途適用性及非侵權保證。本軟體為 **個人業餘時間開發的免費開源工具**。

### 2. 用途說明
本工具僅設計用於 **測試、除錯與學習目的**。支援多種通訊協定，包括 Modbus、TCP 與序列埠。不適用於關鍵任務系統、生產環境或任何故障可能導致重大損失的場景。

### 3. 風險警示
不當使用本軟體可能導致：
- **設備故障或損壞** – 對設備送出錯誤指令或資料可能變更參數或造成硬體故障
- **資料遺失或損毀** – 設備設定或儲存資料可能被覆寫或遺失
- **生產中斷** – 錯誤操作可能導致工業流程或網路服務停機
- **網路安全風險** – 不當的 TCP/序列埠操作可能暴露漏洞或中斷通訊
- **財產損失或人身傷害** – 在危險環境中，錯誤操作可能引發安全事故

### 4. 責任限制
開發者（mingyucheng692）**不承擔任何**以下責任：
- 任何直接、間接、偶然、特殊或後果性損害
- 利潤、資料或商業機會損失
- 設備維修或更換費用
- 生產停機及相關損失
- 網路中斷或通訊故障
- 因使用本軟體引發的法律糾紛

### 5. 使用者責任
使用本軟體即表示您同意：
- ✅ 充分理解通訊協定（Modbus、TCP、序列埠等）及您的設備規格
- ✅ 在工業部署前於 **安全、隔離的環境** 中充分測試
- ✅ 任何操作前確認目標設備位址、連接埠、參數與資料
- ✅ 修改設備設定前備份原有資料
- ✅ 確保操作符合網路與工業安全政策
- ✅ 自行承擔使用本軟體的所有風險
- ✅ 遵守所有適用的法律法規

### 6. 非專業建議
本軟體不構成專業工程建議。工業部署請諮詢合格工程師並遵循您組織的安全規程。

### 7. 協議確認
**下載、安裝或使用本軟體，即表示您已閱讀、理解並同意受本免責聲明約束。如不同意，請勿使用本軟體。**

## 🙏 致謝

本專案使用了以下開源函式庫，感謝作者的傑出貢獻：

- **[spdlog](https://github.com/gabime/spdlog)** (MIT License): 高效能 C++ 日誌函式庫，為本專案提供可靠的日誌記錄支援。

## 目前狀態

這是一個持續迭代的個人除錯工具專案，歡迎使用與擴充。
