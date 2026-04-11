<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_TW">
<context>
    <name>ConnectionAlert</name>
    <message>
        <location filename="../common/ConnectionAlert.h" line="14"/>
        <source>Not Connected</source>
        <translation>未連線</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="14"/>
        <source>Please connect first.</source>
        <translation>請先連線設備</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="18"/>
        <source>Connection Lost</source>
        <translation>連線中斷</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="18"/>
        <source>Connection was closed.</source>
        <translation>連線已關閉。</translation>
    </message>
</context>
<context>
    <name>FrameAnalyzerWidget</name>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="176"/>
        <source>Error: Empty input</source>
        <translation>錯誤：輸入為空</translation>
    </message>
</context>
<context>
    <name>ModbusFrameParser</name>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="30"/>
        <source>Illegal Function</source>
        <translation>非法功能碼</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="32"/>
        <source>Illegal Data Address</source>
        <translation>非法資料位址</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="34"/>
        <source>Illegal Data Value</source>
        <translation>非法資料值</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="36"/>
        <source>Server Device Failure</source>
        <translation>伺服器設備故障</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="38"/>
        <source>Acknowledge</source>
        <translation>確認</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="40"/>
        <source>Server Device Busy</source>
        <translation>伺服器設備忙碌</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="42"/>
        <source>Negative Acknowledge</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="44"/>
        <source>Memory Parity Error</source>
        <translation>記憶體同位檢查錯誤</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="46"/>
        <source>Gateway Path Unavailable</source>
        <translation>閘道路徑無法使用</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="48"/>
        <source>Gateway Target Device Failed To Respond</source>
        <translation>閘道目標設備無回應</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="50"/>
        <source>Unknown Exception</source>
        <translation>未知異常</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="68"/>
        <source>Empty frame data</source>
        <translation>幀資料為空</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="80"/>
        <source>Unable to identify protocol. Frame length: %1 bytes, data: %2</source>
        <translation>無法識別通訊協定。幀長度：%1 位元組，資料：%2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="123"/>
        <source>Frame too short for Modbus TCP. Expected a complete MBAP + PDU, got %1 bytes</source>
        <translation>Modbus TCP 幀太短。預期完整的 MBAP + PDU，實際 %1 字節</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="131"/>
        <source>Invalid TCP MBAP header or length</source>
        <translation>無効的 TCP MBAP 報頭或長度</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="136"/>
        <source>TCP frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>TCP 幀包含多餘字節。預期 %1 字節，實際 %2 字節</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="144"/>
        <source>Frame too short for TCP MBAP</source>
        <translation>幀長度對於 TCP MBAP 來說太短</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="148"/>
        <source>Warning: Invalid TCP MBAP header or length (Forced)</source>
        <translation>警告：TCP MBAP 報頭或長度無效（強制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="151"/>
        <source>Warning: TCP frame contains trailing bytes (Forced)</source>
        <translation>警告：TCP 幀包含多餘字節（強制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="191"/>
        <source>Frame too short for Modbus RTU. Expected at least 5 bytes, got %1</source>
        <translation>Modbus RTU 幀太短。預期至少 5 字節，實際 %1 字節</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="215"/>
        <source>RTU frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>RTU 幀包含多餘字節。預期 %1 字節，實際 %2 字節</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="223"/>
        <source>Frame too short for RTU</source>
        <translation>幀長度對於 RTU 來說太短</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="227"/>
        <source>Warning: CRC Mismatch. Expected %1, Got %2 (Forced)</source>
        <translation>警告：CRC 校驗不匹配。預期 %1，實際 %2（強制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="233"/>
        <source>Warning: RTU frame contains trailing bytes (Forced)</source>
        <translation>警告：RTU 幀包含多餘字節（強制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="380"/>
        <source>Coil response bit count does not match expected quantity. Byte count %1 cannot represent %2 bits</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="415"/>
        <source>Write single PDU length mismatch for function 0x%1. Expected 4 bytes, got %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="462"/>
        <source>Write request byte count mismatch. Declared %1, actual %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="473"/>
        <source>Register write byte count does not match quantity. Declared %1, expected %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="507"/>
        <source>Coil write byte count does not match quantity. Declared %1, expected %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="207"/>
        <source>CRC Mismatch. Expected %1, Got %2</source>
        <translation>CRC 檢查失敗。預期 %1，實際 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="164"/>
        <source>Invalid TCP PDU length. MBAP length field is %1, so PDU length is %2</source>
        <translation>無效的 TCP PDU 長度。MBAP 長度欄位為 %1，因此 PDU 長度為 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="252"/>
        <source>Empty PDU. Function code is missing from the frame</source>
        <translation>PDU 為空。幀中缺少功能碼</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="266"/>
        <source>Exception PDU too short for function 0x%1. Expected 2 bytes, got %2</source>
        <translation>功能碼 0x%1 的例外 PDU 太短。預期 2 位元組，實際 %2 位元組</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="275"/>
        <source>Modbus exception: %1 (code %2)</source>
        <translation>Modbus 例外：%1（代碼 %2）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="313"/>
        <source>Response PDU too short for function 0x%1. Expected at least 2 bytes, got %2</source>
        <translation>功能碼 0x%1 的回應 PDU 太短。預期至少 2 位元組，實際 %2 位元組</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="325"/>
        <source>Byte count mismatch for function 0x%1. Declared %2, actual %3</source>
        <translation>功能碼 0x%1 的位元組數不符。宣告 %2，實際 %3</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="339"/>
        <source>Register response byte count does not match expected quantity. Declared %1, expected %2</source>
        <translation>寄存器響應字節數與預期數量不符。聲明 %1，預期 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="348"/>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="482"/>
        <source>Register byte count must be even, got %1</source>
        <translation>暫存器位元組數必須為偶數，實際 %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="538"/>
        <source>Unsupported function code 0x%1 for deep parsing</source>
        <translation>不支援對功能碼 0x%1 進行深度解析</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="724"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="728"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1185"/>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="248"/>
        <source>Cannot write file: %1</source>
        <translation type="unfinished">無法寫入檔案：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="781"/>
        <source>Cannot open file: %1</source>
        <translation type="unfinished">無法開啟檔案：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="786"/>
        <source>Invalid JSON format.</source>
        <translation type="unfinished">無效的 JSON 格式。</translation>
    </message>
</context>
<context>
    <name>core::update::ChecksumWorker</name>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="56"/>
        <source>Missing or invalid expected checksum</source>
        <translation>缺少或無效的預期校驗值</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="70"/>
        <source>Failed to calculate file checksum</source>
        <translation>計算檔案校驗值失敗</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="75"/>
        <source>Checksum mismatch. Expected: %1, Actual: %2</source>
        <translation>校驗值不符。預期：%1，實際：%2</translation>
    </message>
</context>
<context>
    <name>core::update::UpdateManager</name>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="105"/>
        <source>Failed to access system temporary directory</source>
        <translation>無法存取系統暫存目錄</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="111"/>
        <source>Failed to create update directory</source>
        <translation>建立更新目錄失敗</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="117"/>
        <source>Invalid update URL</source>
        <translation>更新位址無效</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="163"/>
        <source>Failed to open local file for writing</source>
        <translation>無法開啟本地檔案進行寫入</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="240"/>
        <source>Failed to create update task file</source>
        <translation>建立更新任務檔案失敗</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="260"/>
        <source>Updater not found</source>
        <translation>找不到更新器</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="280"/>
        <source>Failed to launch updater (Access Denied or System Error)</source>
        <translation>啟動更新器失敗（拒絕存取或系統錯誤）</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="289"/>
        <source>Automatic update is only supported on Windows</source>
        <translation>自動更新僅支援 Windows 系統</translation>
    </message>
</context>
<context>
    <name>ui::MainWindow</name>
    <message>
        <location filename="../MainWindow.cpp" line="175"/>
        <location filename="../MainWindow.cpp" line="512"/>
        <source>Modbus Tools</source>
        <translation>Modbus 工具</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="275"/>
        <location filename="../MainWindow.cpp" line="513"/>
        <source>Modbus TCP</source>
        <translation>Modbus TCP</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="276"/>
        <location filename="../MainWindow.cpp" line="513"/>
        <source>Modbus RTU</source>
        <translation>Modbus RTU</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="277"/>
        <location filename="../MainWindow.cpp" line="513"/>
        <source>TCP Client</source>
        <translation>TCP 用戶端</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="278"/>
        <location filename="../MainWindow.cpp" line="513"/>
        <source>Serial Port</source>
        <translation>序列埠</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="279"/>
        <location filename="../MainWindow.cpp" line="513"/>
        <source>Frame Analyzer</source>
        <translation>報文分析</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="315"/>
        <location filename="../MainWindow.cpp" line="519"/>
        <source>Language</source>
        <translation>語言</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="309"/>
        <location filename="../MainWindow.cpp" line="520"/>
        <source>Settings</source>
        <translation>設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="213"/>
        <source>You are using the latest version: v%1</source>
        <translation>您正在使用最新版本：v%1</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="225"/>
        <source>Update Failed</source>
        <translation>更新失敗</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="305"/>
        <source>Expand Navigation</source>
        <translation>展開導覽</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="305"/>
        <source>Collapse Navigation</source>
        <translation>摺疊導覽</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="310"/>
        <location filename="../MainWindow.cpp" line="522"/>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="19"/>
        <source>Modbus Settings</source>
        <translation>Modbus設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="311"/>
        <location filename="../MainWindow.cpp" line="523"/>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="21"/>
        <source>Update Settings</source>
        <translation>更新設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="329"/>
        <location filename="../MainWindow.cpp" line="527"/>
        <source>English (US)</source>
        <translation>英語（美國）</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="330"/>
        <location filename="../MainWindow.cpp" line="528"/>
        <source>简体中文</source>
        <translation>簡體中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="331"/>
        <location filename="../MainWindow.cpp" line="529"/>
        <source>繁體中文</source>
        <translation>繁體中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="340"/>
        <location filename="../MainWindow.cpp" line="431"/>
        <location filename="../MainWindow.cpp" line="524"/>
        <source>Check for Updates</source>
        <translation>檢查更新</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="445"/>
        <source>New version v%1 available. Open download page?</source>
        <translation>發現新版本 v%1。是否開啟下載頁面？</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="453"/>
        <source>Current: v%1, Latest: v%2
Choose update method:</source>
        <translation>目前版本：v%1，最新版本：v%2
請選擇更新方式：</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="469"/>
        <source>Downloading Update...</source>
        <translation>正在下載更新...</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="27"/>
        <source>Request Timeout (ms):</source>
        <translation>請求逾時(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="31"/>
        <source>Enable Retry:</source>
        <translation>啟用重試：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="36"/>
        <source>Retry Count:</source>
        <translation>重試次數：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="42"/>
        <source>Retry Interval (ms):</source>
        <translation>重試間隔(ms)：</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="445"/>
        <location filename="../MainWindow.cpp" line="452"/>
        <source>Update Available</source>
        <translation>發現新版本</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="213"/>
        <source>No Updates</source>
        <translation>沒有可用更新</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="217"/>
        <source>Update Check Failed</source>
        <translation>更新檢查失敗</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="469"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="454"/>
        <source>Update Main Program</source>
        <translation>更新主程式</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="455"/>
        <source>Download Full Package</source>
        <translation>下載完整安裝包</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="339"/>
        <location filename="../MainWindow.cpp" line="342"/>
        <location filename="../MainWindow.cpp" line="430"/>
        <location filename="../MainWindow.cpp" line="521"/>
        <location filename="../MainWindow.cpp" line="525"/>
        <location filename="../widgets/AboutDialog.cpp" line="15"/>
        <source>About</source>
        <translation>關於</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="362"/>
        <source>Theme: Auto</source>
        <translation>主題：自動</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="363"/>
        <source>Theme: Light</source>
        <translation>主題：淺色</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="363"/>
        <source>Theme: Dark</source>
        <translation>主題：深色</translation>
    </message>
    <message>
        <location filename="../widgets/AboutDialog.cpp" line="24"/>
        <source>Welcome to Modbus-Tools&lt;br&gt;Version: v%1&lt;br&gt;&lt;br&gt;An open-source Modbus communication debugging assistant.&lt;br&gt;Developer: mingyucheng692&lt;br&gt;License: MIT License&lt;br&gt;&lt;br&gt;This project is developed in spare time, completely free and open-source.&lt;br&gt;Feel free to star on GitHub or submit issues.&lt;br&gt;Your feedback keeps the project improving!&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 Visit GitHub Repository&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 Issue Tracker&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;This software is provided &amp;quot;as is&amp;quot; without warranty of any kind.</source>
        <translation>歡迎使用 Modbus-Tools&lt;br&gt;版本：v%1&lt;br&gt;&lt;br&gt;一款開源的 Modbus 通訊除錯助手。&lt;br&gt;開發者：mingyucheng692&lt;br&gt;授權條款：MIT License&lt;br&gt;&lt;br&gt;本專案由個人業餘開發，完全免費且開源。&lt;br&gt;歡迎在 GitHub 上 Star ⭐ 或提交 Issue。&lt;br&gt;您的回饋是專案持續改進的動力！&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 造訪 GitHub 倉庫&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 問題回報&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;本軟體按&amp;quot;原樣&amp;quot;提供，無任何形式的保證。</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="26"/>
        <source>Every Startup</source>
        <translation type="unfinished">每次啟動</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="27"/>
        <source>Weekly</source>
        <translation type="unfinished">每週</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="28"/>
        <source>Monthly</source>
        <translation type="unfinished">每月</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="29"/>
        <source>Disable Update Check</source>
        <translation type="unfinished">關閉更新檢查</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="30"/>
        <source>Update Check Frequency:</source>
        <translation type="unfinished">更新檢查頻率：</translation>
    </message>
</context>
<context>
    <name>ui::common::UpdateChecker</name>
    <message>
        <location filename="../common/updatechecker.cpp" line="94"/>
        <source>Release tag is missing</source>
        <translation>發布標籤缺失</translation>
    </message>
</context>
<context>
    <name>ui::views::generic_serial::GenericSerialView</name>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="111"/>
        <source>File transfer started: %1 (%2 bytes)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="119"/>
        <source>File transfer progress: %1 %2/%3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="126"/>
        <source>File transfer finished: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="129"/>
        <source>File transfer failed: %1 (%2)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="132"/>
        <source>File transfer canceled: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="177"/>
        <source>Opening %1...</source>
        <translation>正在開啟 %1...</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="220"/>
        <source>Closed</source>
        <translation>已關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="221"/>
        <source>Opening</source>
        <translation>正在開啟</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="222"/>
        <source>Open</source>
        <translation>已開啟</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="223"/>
        <source>Closing</source>
        <translation>正在關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="224"/>
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="225"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="228"/>
        <source>State changed: %1</source>
        <translation>狀態已變更：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="233"/>
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="266"/>
        <source>Control</source>
        <translation>控制</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="267"/>
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="268"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="269"/>
        <source>RTS</source>
        <translation>RTS</translation>
    </message>
</context>
<context>
    <name>ui::views::generic_tcp::GenericTcpView</name>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="85"/>
        <source>File transfer started: %1 (%2 bytes)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="93"/>
        <source>File transfer progress: %1 %2/%3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="100"/>
        <source>File transfer finished: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="103"/>
        <source>File transfer failed: %1 (%2)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="106"/>
        <source>File transfer canceled: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="154"/>
        <source>Connecting to %1:%2...</source>
        <translation>正在連線 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="215"/>
        <source>Closed</source>
        <translation>已關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="216"/>
        <source>Opening</source>
        <translation>正在開啟</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="217"/>
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="218"/>
        <source>Closing</source>
        <translation>正在關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="219"/>
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="220"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="223"/>
        <source>State changed: %1</source>
        <translation>狀態已變更：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="232"/>
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="245"/>
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus_rtu::ModbusRtuView</name>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="122"/>
        <source>Opening %1...</source>
        <translation>正在開啟 %1...</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="263"/>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation>發送讀取請求 FC:%1 位址:%2 數量:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="221"/>
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="142"/>
        <source>Failed to create Modbus stack</source>
        <translation>建立 Modbus 堆疊失敗</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="185"/>
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="188"/>
        <source>Connection failed: %1</source>
        <translation>連線失敗：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="204"/>
        <source>Success: Broadcast write sent, no response expected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="210"/>
        <source>Success: Response received</source>
        <translation>成功：已收到回應</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="236"/>
        <source>Disconnected</source>
        <translation type="unfinished">已斷線</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="319"/>
        <source>Error: Invalid decimal value for 0x05</source>
        <translation>錯誤：0x05 十進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="326"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="332"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="339"/>
        <source>Error: Invalid hex value for 0x05</source>
        <translation>錯誤：0x05 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="349"/>
        <source>Error: Empty value for 0x06</source>
        <translation>錯誤：0x06 值為空</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="356"/>
        <source>Error: Invalid decimal value for 0x06</source>
        <translation>錯誤：0x06 十進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="364"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="374"/>
        <source>Error: Invalid hex value for 0x06</source>
        <translation>錯誤：0x06 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="380"/>
        <source>Error: 0x0F requires Hex data</source>
        <translation>錯誤：0x0F 需要十六進位資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="385"/>
        <source>Error: Invalid hex value for 0x0F</source>
        <translation>錯誤：0x0F 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="385"/>
        <source>Error: Binary bit count (%1) does not match Quantity (%2)</source>
        <translation>錯誤：二進位位元數 (%1) 與數量 (%2) 不相符</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="371"/>
        <source>Error: Binary format not supported for registers (0x06)</source>
        <translation>錯誤：暫存器操作 (0x06) 不支援二進位格式</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="338"/>
        <source>Error: Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation>錯誤：0x05 二進位值無效（預期為 0 或 1）</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="390"/>
        <source>Error: Invalid quantity for 0x0F</source>
        <translation>錯誤：0x0F 數量無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="395"/>
        <source>Error: Coil data length mismatch for 0x0F</source>
        <translation>錯誤：0x0F 線圈資料長度不相符</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="404"/>
        <source>Error: Empty value for 0x10</source>
        <translation>錯誤：0x10 值為空</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="409"/>
        <source>Error: Invalid quantity for 0x10</source>
        <translation>錯誤：0x10 數量無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="418"/>
        <source>Error: Invalid decimal list for 0x10</source>
        <translation>錯誤：0x10 十進位清單無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="429"/>
        <source>Error: Invalid hex value for 0x10</source>
        <translation>錯誤：0x10 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="435"/>
        <source>Error: Quantity does not match data length for 0x10</source>
        <translation>錯誤：0x10 數量與資料長度不相符</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="443"/>
        <source>Error: Unsupported write function code</source>
        <translation>錯誤：不支援的寫入功能碼</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="449"/>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation>發送寫入請求 FC:%1 位址:%2 資料:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="212"/>
        <source>Success: Write confirmed</source>
        <translation>成功：寫入已確認</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="465"/>
        <source>Sending Raw Data: %1</source>
        <translation>發送原始資料：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="219"/>
        <source>Poll Error: %1</source>
        <translation>輪詢錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="606"/>
        <source>Data Monitor</source>
        <translation>資料監視</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="607"/>
        <source>Receive Data</source>
        <translation>接收資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="608"/>
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="609"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="610"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="611"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="612"/>
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="613"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="614"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus_tcp::ModbusTcpView</name>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="137"/>
        <source>Connecting to %1:%2...</source>
        <translation>正在連線 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="33"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="221"/>
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="37"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="274"/>
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="300"/>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation>發送讀取請求 FC:%1 位址:%2 數量:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="256"/>
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="151"/>
        <source>Failed to create Modbus stack</source>
        <translation>建立 Modbus 堆疊失敗</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="225"/>
        <source>Connection failed: %1</source>
        <translation>連線失敗：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="245"/>
        <source>Success: Response received</source>
        <translation>成功：已收到回應</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="357"/>
        <source>Error: Invalid decimal value for 0x05</source>
        <translation>錯誤：0x05 十進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="364"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="370"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="377"/>
        <source>Error: Invalid hex value for 0x05</source>
        <translation>錯誤：0x05 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="387"/>
        <source>Error: Empty value for 0x06</source>
        <translation>錯誤：0x06 值為空</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="394"/>
        <source>Error: Invalid decimal value for 0x06</source>
        <translation>錯誤：0x06 十進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="402"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="412"/>
        <source>Error: Invalid hex value for 0x06</source>
        <translation>錯誤：0x06 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="418"/>
        <source>Error: 0x0F requires Hex data</source>
        <translation>錯誤：0x0F 需要十六進位資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="423"/>
        <source>Error: Invalid hex value for 0x0F</source>
        <translation>錯誤：0x0F 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="414"/>
        <source>Error: Binary format not supported for registers (0x06)</source>
        <translation>錯誤：暫存器操作 (0x06) 不支援二進位格式</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="428"/>
        <source>Error: Binary bit count (%1) does not match Quantity (%2)</source>
        <translation>錯誤：二進位位元數 (%1) 與數量 (%2) 不相符</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="381"/>
        <source>Error: Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation>錯誤：0x05 二進位值無效（預期為 0 或 1）</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="423"/>
        <source>Error: Invalid quantity for 0x0F</source>
        <translation>錯誤：0x0F 數量無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="433"/>
        <source>Error: Coil data length mismatch for 0x0F</source>
        <translation>錯誤：0x0F 線圈資料長度不相符</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="442"/>
        <source>Error: Empty value for 0x10</source>
        <translation>錯誤：0x10 值為空</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="447"/>
        <source>Error: Invalid quantity for 0x10</source>
        <translation>錯誤：0x10 數量無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="456"/>
        <source>Error: Invalid decimal list for 0x10</source>
        <translation>錯誤：0x10 十進位清單無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="467"/>
        <source>Error: Invalid hex value for 0x10</source>
        <translation>錯誤：0x10 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="473"/>
        <source>Error: Quantity does not match data length for 0x10</source>
        <translation>錯誤：0x10 數量與資料長度不相符</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="481"/>
        <source>Error: Unsupported write function code</source>
        <translation>錯誤：不支援的寫入功能碼</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="487"/>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation>發送寫入請求 FC:%1 位址:%2 資料:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="247"/>
        <source>Success: Write confirmed</source>
        <translation>成功：寫入已確認</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="504"/>
        <source>Sending Raw Data: %1</source>
        <translation>發送原始資料：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="254"/>
        <source>Poll Error: %1</source>
        <translation>輪詢錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="647"/>
        <source>Data Monitor</source>
        <translation>資料監視</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="648"/>
        <source>Receive Data</source>
        <translation>接收資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="649"/>
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="650"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="651"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="652"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="653"/>
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="654"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="655"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
</context>
<context>
    <name>ui::widgets::CollapsibleSection</name>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="92"/>
        <source>Collapse</source>
        <translation>收合</translation>
    </message>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="92"/>
        <source>Expand</source>
        <translation>展開</translation>
    </message>
</context>
<context>
    <name>ui::widgets::ControlWidget</name>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="208"/>
        <source>Enable Polling</source>
        <translation>啟用輪詢</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="211"/>
        <source>Interval(ms):</source>
        <translation>間隔(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="214"/>
        <source>FC:</source>
        <translation>功能碼：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="217"/>
        <source>01-Read Coils</source>
        <translation>01-讀取線圈</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="218"/>
        <source>02-Read Discrete</source>
        <translation>02-讀取離散輸入</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="219"/>
        <source>03-Read Holding</source>
        <translation>03-讀取保持暫存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="220"/>
        <source>04-Read Input</source>
        <translation>04-讀取輸入暫存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="223"/>
        <source>Addr:</source>
        <translation>位址：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="224"/>
        <source>Qty:</source>
        <translation>數量：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="224"/>
        <source>Link to Analyzer</source>
        <translation>聯動分析儀</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FrameAnalyzerWidget</name>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="291"/>
        <source>Description: %1</source>
        <translation>描述：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="297"/>
        <source>Raw: %1</source>
        <translation>原始值：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="298"/>
        <source>Scale: %1</source>
        <translation>倍率：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="299"/>
        <source>Scaled: %1</source>
        <translation>換算值：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="420"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1325"/>
        <source>Frame Input</source>
        <translation>報文輸入</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="426"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1328"/>
        <source>Protocol:</source>
        <translation>通訊協定：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="429"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1331"/>
        <source>Auto Detect</source>
        <translation>自動偵測</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="430"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1332"/>
        <source>Modbus TCP</source>
        <translation>Modbus TCP</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="431"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1333"/>
        <source>Modbus RTU</source>
        <translation>Modbus RTU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="435"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1336"/>
        <source>Start Address (for Response):</source>
        <translation>起始位址（用於回應）：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="449"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1347"/>
        <source>Format Hex</source>
        <translation>格式化 Hex</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="454"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1366"/>
        <source>Parse</source>
        <translation>解析</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="459"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1370"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="470"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1373"/>
        <source>Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)</source>
        <translation>輸入十六進位字串（如 01 03 00 00 00 01 84 0A）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="482"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1377"/>
        <source>Analysis Result</source>
        <translation>分析結果</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="488"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="795"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1403"/>
        <source>Ready</source>
        <translation>就緒</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="492"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1339"/>
        <source>Decode Mode:</source>
        <translation>解碼模式：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="495"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1342"/>
        <source>Unsigned</source>
        <translation>無符號</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="496"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1343"/>
        <source>Signed</source>
        <translation>有符號</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="502"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1350"/>
        <source>Import Config</source>
        <translation>匯入設定</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="503"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1354"/>
        <source>Export Config</source>
        <translation>匯出設定</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="504"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="687"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1358"/>
        <source>Export CSV</source>
        <translation>匯出 CSV</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="530"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1388"/>
        <source>Field</source>
        <translation>欄位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="530"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="541"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1389"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1393"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="530"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="541"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1390"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1393"/>
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="536"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1383"/>
        <source>Structure</source>
        <translation>結構</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="541"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1393"/>
        <source>Address</source>
        <translation>位址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="541"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1393"/>
        <source>Hex</source>
        <translation>十六進位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="541"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1393"/>
        <source>Decimal</source>
        <translation>十進位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="541"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1393"/>
        <source>Binary</source>
        <translation>二進位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="541"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1393"/>
        <source>Scale</source>
        <translation>倍率</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="554"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1384"/>
        <source>Data Details</source>
        <translation>資料詳情</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="573"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1380"/>
        <source>History</source>
        <translation>歷史記錄</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="583"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1396"/>
        <source>Clear History</source>
        <translation>清除歷史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="662"/>
        <source>Export Frame Metadata</source>
        <translation>匯出幀中繼資料</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="662"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="671"/>
        <source>JSON Files (*.json)</source>
        <translation>JSON 檔案 (*.json)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="671"/>
        <source>Import Frame Metadata</source>
        <translation>匯入幀中繼資料</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="681"/>
        <source>No Data</source>
        <translation>無資料</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="681"/>
        <source>There is no parsed data to export.</source>
        <translation>沒有可匯出的解析資料。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="689"/>
        <source>CSV Files (*.csv)</source>
        <translation>CSV 檔案 (*.csv)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="717"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1137"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1173"/>
        <source>Export Failed</source>
        <translation>匯出失敗</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="740"/>
        <source>Import Failed</source>
        <translation>匯入失敗</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="852"/>
        <source>Error: Empty input</source>
        <translation>錯誤：輸入為空</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="861"/>
        <source>Parsing...</source>
        <translation>解析中...</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="924"/>
        <source>Parse Failed: %1</source>
        <translation>解析失敗：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="934"/>
        <source>Parse error</source>
        <translation>解析錯誤</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="951"/>
        <source>Success (%1)</source>
        <translation>解析成功 (%1)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="916"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="951"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1233"/>
        <source>TCP</source>
        <translation>TCP</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="916"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="951"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1233"/>
        <source>RTU</source>
        <translation>RTU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="918"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1238"/>
        <source>Request</source>
        <translation>請求</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="919"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1239"/>
        <source>Response</source>
        <translation>回應</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="921"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="933"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="939"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1241"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="926"/>
        <source>Tip: Try selecting RTU or TCP for forced parsing.</source>
        <translation>提示：若自動識別失敗，可手動選擇 RTU 或 TCP 以嘗試強制解析。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="932"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="966"/>
        <source>Frame</source>
        <translation>幀</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="933"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="967"/>
        <source>%1 bytes</source>
        <translation>%1 位元組</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="937"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="983"/>
        <source>Frame Bytes</source>
        <translation>幀位元組</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="937"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="983"/>
        <source>Complete raw frame</source>
        <translation>完整原始幀</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="939"/>
        <source>Protocol detected before parse failed</source>
        <translation>解析失敗前偵測到的通訊協定</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="939"/>
        <source>Protocol</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="940"/>
        <source>Detailed parse failure reason</source>
        <translation>詳細解析失敗原因</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="940"/>
        <source>Error</source>
        <translation type="unfinished">錯誤</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="953"/>
        <source>Forced Parsing</source>
        <translation>強制解析</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="956"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="972"/>
        <source>Warnings</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="973"/>
        <source>Issues ignored during forced parsing</source>
        <translation>強制解析過程中忽略的問題</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="977"/>
        <source>Issue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="986"/>
        <source>MBAP Header</source>
        <translation>MBAP 標頭</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="986"/>
        <source>Transaction + Protocol + Length + Unit ID</source>
        <translation>交易識別碼 + 協定識別碼 + 長度 + 單元識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="988"/>
        <source>Transaction ID</source>
        <translation>交易識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="990"/>
        <source>Request/response correlation ID</source>
        <translation>請求/回應關聯識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="992"/>
        <source>Protocol ID</source>
        <translation>協定識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="994"/>
        <source>Modbus TCP protocol identifier</source>
        <translation>Modbus TCP 協定識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="996"/>
        <source>Length</source>
        <translation>長度</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="998"/>
        <source>Remaining bytes after this field</source>
        <translation>此欄位之後的剩餘位元組數</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1000"/>
        <source>Unit ID</source>
        <translation>單元識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1002"/>
        <source>Target slave / unit address</source>
        <translation>目標從站 / 單元位址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1005"/>
        <source>Slave ID</source>
        <translation>從站識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1007"/>
        <source>Target slave address</source>
        <translation>目標從站位址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1013"/>
        <source>PDU</source>
        <translation>PDU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1013"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1022"/>
        <source>(empty)</source>
        <translation>（空）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1013"/>
        <source>Function code + payload</source>
        <translation>功能碼 + 載荷</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1015"/>
        <source>Function Code</source>
        <translation>功能碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1017"/>
        <source>Exception response</source>
        <translation>例外回應</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1017"/>
        <source>Normal response</source>
        <translation>正常回應</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1021"/>
        <source>Payload</source>
        <translation>載荷</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1023"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1029"/>
        <source>Exception detail payload</source>
        <translation>例外詳細載荷</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1023"/>
        <source>Application data payload</source>
        <translation>應用資料載荷</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1027"/>
        <source>Exception Code</source>
        <translation>例外碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="945"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1034"/>
        <source>CRC valid</source>
        <translation>CRC 有效</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="945"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1034"/>
        <source>CRC invalid</source>
        <translation>CRC 無效</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1038"/>
        <source>Calculated: 0x%1</source>
        <translation>計算值：0x%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="943"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1042"/>
        <source>CRC16</source>
        <translation>CRC16</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1305"/>
        <source>Show History</source>
        <translation>顯示歷史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1305"/>
        <source>Hide History</source>
        <translation>隱藏歷史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1344"/>
        <source>Choose how parsed numeric values are displayed.</source>
        <translation>選擇解析後數值的顯示方式。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1351"/>
        <source>Import saved field scale and description settings from a JSON file.</source>
        <translation>從 JSON 檔匯入已儲存的欄位縮放與描述設定。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1355"/>
        <source>Export current field scale and description settings to a JSON file.</source>
        <translation>將目前欄位縮放與描述設定匯出到 JSON 檔。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1359"/>
        <source>Export the parsed register data in the current table as a CSV file.</source>
        <translation>將目前表格中的解析寄存器資料匯出為 CSV 檔。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1362"/>
        <source>Show or hide the parse history panel.</source>
        <translation>顯示或隱藏解析歷史面板。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="920"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1240"/>
        <source>Exception</source>
        <translation>異常</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FunctionWidget</name>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="278"/>
        <source>Standard</source>
        <translation>標準</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="281"/>
        <source>Raw</source>
        <translation>原始</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="284"/>
        <source>Slave ID:</source>
        <translation>從站ID：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="287"/>
        <source>Start Addr:</source>
        <translation>起始位址：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="290"/>
        <source>Quantity:</source>
        <translation>數量：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="293"/>
        <source>Write Data:</source>
        <translation>寫入資料：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="296"/>
        <source>Space separated hex (e.g., 01 02) or values</source>
        <translation>以空格分隔的十六進位(如 01 02)或數值</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="299"/>
        <source>Format:</source>
        <translation>格式：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="302"/>
        <source>Hex</source>
        <translation>十六進位</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="334"/>
        <source>Binary</source>
        <translation>二進位</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="281"/>
        <source>Space separated hex (e.g., 01 02)</source>
        <translation>空格分隔的十六進位 (如 01 02)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="283"/>
        <source>Space separated decimal (e.g., 100 200)</source>
        <translation>空格分隔的十進位 (如 100 200)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="285"/>
        <source>Bit string (e.g., 1 1 0 1)</source>
        <translation>二進位位元字串 (如 1 1 0 1)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="303"/>
        <source>Decimal</source>
        <translation>十進位</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="306"/>
        <source>Read Coils (0x01)</source>
        <translation>讀取線圈 (0x01)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="309"/>
        <source>Read Discrete Inputs (0x02)</source>
        <translation>讀取離散輸入 (0x02)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="312"/>
        <source>Read Holding Registers (0x03)</source>
        <translation>讀取保持暫存器 (0x03)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="315"/>
        <source>Read Input Registers (0x04)</source>
        <translation>讀取輸入暫存器 (0x04)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="318"/>
        <source>Write Single Coil (0x05)</source>
        <translation>寫入單線圈 (0x05)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="321"/>
        <source>Write Single Register (0x06)</source>
        <translation>寫入單暫存器 (0x06)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="324"/>
        <source>Write Multiple Coils (0x0F)</source>
        <translation>寫入多線圈 (0x0F)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="327"/>
        <source>Write Multiple Registers (0x10)</source>
        <translation>寫入多暫存器 (0x10)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="330"/>
        <source>Raw Hex Data (e.g., 01 03 00 00 00 01 84 0A):</source>
        <translation>原始十六進位資料(如 01 03 00 00 00 01 84 0A)：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="384"/>
        <source>Send Raw</source>
        <translation>發送原始資料</translation>
    </message>
    <message>
        <source>Append CRC</source>
        <translation>計算并追加 CRC</translation>
    </message>
    <message>
        <source>Add MBAP</source>
        <translation>添加 MBAP 頭</translation>
    </message>
</context>
<context>
    <name>ui::widgets::GenericInputWidget</name>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="208"/>
        <source>Select File to Send</source>
        <translation>選擇要發送的檔案</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="219"/>
        <source>Enter data to send...</source>
        <translation>輸入要發送的資料...</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="222"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="225"/>
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="228"/>
        <source>Auto Send</source>
        <translation>自動發送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="231"/>
        <source> ms</source>
        <translation> 毫秒</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="234"/>
        <source>Send File</source>
        <translation>發送檔案</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="237"/>
        <source>Send</source>
        <translation>發送</translation>
    </message>
</context>
<context>
    <name>ui::widgets::SerialConnectionWidget</name>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="57"/>
        <source>Disconnect</source>
        <translation>中斷連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="58"/>
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="61"/>
        <source>Connect</source>
        <translation>連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="62"/>
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="118"/>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="282"/>
        <source>Refresh Ports</source>
        <translation>重新整理埠</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="264"/>
        <source>Connection Settings</source>
        <translation>連線設定</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="267"/>
        <source>Port:</source>
        <translation>埠：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="270"/>
        <source>Baud:</source>
        <translation>鮑特率：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="273"/>
        <source>Data:</source>
        <translation>資料位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="276"/>
        <source>Parity:</source>
        <translation>同位元：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="279"/>
        <source>Stop:</source>
        <translation>停止位：</translation>
    </message>
</context>
<context>
    <name>ui::widgets::TcpConnectionWidget</name>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="142"/>
        <source>Disconnect</source>
        <translation>中斷連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="143"/>
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="148"/>
        <source>Connect</source>
        <translation>連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="149"/>
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="158"/>
        <source>Connection Settings</source>
        <translation>連線設定</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="161"/>
        <source>Host:</source>
        <translation>主機：</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="164"/>
        <source>Port:</source>
        <translation>埠：</translation>
    </message>
</context>
<context>
    <name>ui::widgets::TrafficMonitorWidget</name>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="94"/>
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="106"/>
        <source>[%1] [TX] %2</source>
        <translation>[%1] [TX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="122"/>
        <source>[%1] [RX] %2</source>
        <translation>[%1] [RX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="135"/>
        <source>[%1] [INFO] %2</source>
        <translation>[%1] [資訊] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="198"/>
        <source>Save Log</source>
        <translation>儲存日誌</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="198"/>
        <source>Text Files (*.txt);;All Files (*)</source>
        <translation>文字檔 (*.txt);;所有檔案 (*)</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="213"/>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="235"/>
        <source>Save failed: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="271"/>
        <source>Traffic Monitor</source>
        <translation>通訊監視</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="272"/>
        <source>Auto Scroll</source>
        <translation>自動捲動</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="275"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="276"/>
        <source>Save</source>
        <translation>儲存</translation>
    </message>
</context>
<context>
    <name>ui::widgets::UpdateSettingsDialog</name>
</context>
<context>
    <name>ModbusPduBuilder</name>
    <message>
        <source>Invalid start address</source>
        <translation>無效的起始地址</translation>
    </message>
    <message>
        <source>Invalid quantity</source>
        <translation>無效的數量</translation>
    </message>
    <message>
        <source>Invalid quantity for 0x0F</source>
        <translation>0x0F 數量無效</translation>
    </message>
    <message>
        <source>Data length mismatch for 0x0F</source>
        <translation>0x0F 資料長度不匹配</translation>
    </message>
    <message>
        <source>Invalid quantity for 0x10</source>
        <translation>0x10 數量無效</translation>
    </message>
    <message>
        <source>Data length mismatch for 0x10</source>
        <translation>0x10 資料長度不匹配</translation>
    </message>
</context>
<context>
    <name>ModbusClient</name>
    <message>
        <source>Modbus exception response. Slave=%1 FC=0x%2 Exception=0x%3 (%4)</source>
        <translation>Modbus 異常響應。站號=%1 功能碼=0x%2 異常碼=0x%3 (%4)</translation>
    </message>
    <message>
        <source>Illegal Function</source>
        <translation>非法功能碼</translation>
    </message>
    <message>
        <source>Illegal Data Address</source>
        <translation>非法數據地址</translation>
    </message>
    <message>
        <source>Illegal Data Value</source>
        <translation>非法數據值</translation>
    </message>
    <message>
        <source>Server Device Failure</source>
        <translation>從站設備故障</translation>
    </message>
    <message>
        <source>Acknowledge</source>
        <translation>確認</translation>
    </message>
    <message>
        <source>Server Device Busy</source>
        <translation>從站設備忙</translation>
    </message>
    <message>
        <source>Negative Acknowledge</source>
        <translation>否定確認</translation>
    </message>
    <message>
        <source>Memory Parity Error</source>
        <translation>寄存器校驗錯誤</translation>
    </message>
    <message>
        <source>Gateway Path Unavailable</source>
        <translation>網關路徑不可用</translation>
    </message>
    <message>
        <source>Gateway Target Device Failed To Respond</source>
        <translation>網關目標設備響應失敗</translation>
    </message>
    <message>
        <source>Unknown Exception</source>
        <translation>未知異常</translation>
    </message>
</context>
</TS>
