<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_TW">
<context>
    <name>ConnectionAlert</name>
    <message>
        <location filename="../common/ConnectionAlert.h" line="23"/>
        <source>Not Connected</source>
        <translation>未連線</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="23"/>
        <source>Please connect first.</source>
        <translation>請先連線設備</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="27"/>
        <source>Connection Lost</source>
        <translation>連線中斷</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="27"/>
        <source>Connection was closed.</source>
        <translation>連線已關閉。</translation>
    </message>
</context>
<context>
    <name>FrameParseWorker</name>
    <message>
        <location filename="../../core/modbus/parser/FrameParseWorker.cpp" line="105"/>
        <source>Error: Empty input</source>
        <translation>錯誤：輸入為空</translation>
    </message>
</context>
<context>
    <name>ModbusFrameParser</name>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="39"/>
        <source>Illegal Function</source>
        <translation>非法功能碼</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="41"/>
        <source>Illegal Data Address</source>
        <translation>非法資料位址</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="43"/>
        <source>Illegal Data Value</source>
        <translation>非法資料值</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="45"/>
        <source>Server Device Failure</source>
        <translation>伺服器設備故障</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="47"/>
        <source>Acknowledge</source>
        <translation>確認</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="49"/>
        <source>Server Device Busy</source>
        <translation>伺服器設備忙碌</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="51"/>
        <source>Negative Acknowledge</source>
        <translation>否定確認</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="53"/>
        <source>Memory Parity Error</source>
        <translation>記憶體同位檢查錯誤</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="55"/>
        <source>Gateway Path Unavailable</source>
        <translation>閘道路徑無法使用</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="57"/>
        <source>Gateway Target Device Failed To Respond</source>
        <translation>閘道目標設備無回應</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="59"/>
        <source>Unknown Exception</source>
        <translation>未知異常</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="78"/>
        <source>Empty frame data</source>
        <translation>幀資料為空</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="90"/>
        <source>Unable to identify protocol. Frame length: %1 bytes, data: %2</source>
        <translation>無法識別通訊協定。幀長度：%1 位元組，資料：%2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="140"/>
        <source>Frame too short for Modbus TCP. Expected a complete MBAP + PDU, got %1 bytes</source>
        <translation>Modbus TCP 幀太短。預期完整的 MBAP + PDU，實際 %1 字節</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="148"/>
        <source>Invalid TCP MBAP header or length</source>
        <translation>無効的 TCP MBAP 報頭或長度</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="153"/>
        <source>TCP frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>TCP 幀包含多餘字節。預期 %1 字節，實際 %2 字節</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="161"/>
        <source>Frame too short for TCP MBAP</source>
        <translation>幀長度對於 TCP MBAP 來說太短</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="165"/>
        <source>Warning: Invalid TCP MBAP header or length (Forced)</source>
        <translation>警告：TCP MBAP 報頭或長度無效（強制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="168"/>
        <source>Warning: TCP frame contains trailing bytes (Forced)</source>
        <translation>警告：TCP 幀包含多餘字節（強制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="208"/>
        <source>Frame too short for Modbus RTU. Expected at least 5 bytes, got %1</source>
        <translation>Modbus RTU 幀太短。預期至少 5 字節，實際 %1 字節</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="232"/>
        <source>RTU frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>RTU 幀包含多餘字節。預期 %1 字節，實際 %2 字節</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="240"/>
        <source>Frame too short for RTU</source>
        <translation>幀長度對於 RTU 來說太短</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="244"/>
        <source>Warning: CRC Mismatch. Expected %1, Got %2 (Forced)</source>
        <translation>警告：CRC 校驗不匹配。預期 %1，實際 %2（強制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="250"/>
        <source>Warning: RTU frame contains trailing bytes (Forced)</source>
        <translation>警告：RTU 幀包含多餘字節（強制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="224"/>
        <source>CRC Mismatch. Expected %1, Got %2</source>
        <translation>CRC 檢查失敗。預期 %1，實際 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="181"/>
        <source>Invalid TCP PDU length. MBAP length field is %1, so PDU length is %2</source>
        <translation>無效的 TCP PDU 長度。MBAP 長度欄位為 %1，因此 PDU 長度為 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="271"/>
        <source>Empty PDU. Function code is missing from the frame</source>
        <translation>PDU 為空。幀中缺少功能碼</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="285"/>
        <source>Exception PDU too short for function 0x%1. Expected 2 bytes, got %2</source>
        <translation>功能碼 0x%1 的例外 PDU 太短。預期 2 位元組，實際 %2 位元組</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="294"/>
        <source>Modbus exception: %1 (code %2)</source>
        <translation>Modbus 例外：%1（代碼 %2）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="332"/>
        <source>Response PDU too short for function 0x%1. Expected at least 2 bytes, got %2</source>
        <translation>功能碼 0x%1 的回應 PDU 太短。預期至少 2 位元組，實際 %2 位元組</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="344"/>
        <source>Byte count mismatch for function 0x%1. Declared %2, actual %3</source>
        <translation>功能碼 0x%1 的位元組數不符。宣告 %2，實際 %3</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="358"/>
        <source>Register response byte count does not match expected quantity. Declared %1, expected %2</source>
        <translation>寄存器響應字節數與預期數量不符。聲明 %1，預期 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="367"/>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="501"/>
        <source>Register byte count must be even, got %1</source>
        <translation>暫存器位元組數必須為偶數，實際 %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="399"/>
        <source>Coil response bit count does not match expected quantity. Byte count %1 cannot represent %2 bits</source>
        <translation>線圈響應位計數與預期數量不符。位元組數 %1 無法表示 %2 位</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="434"/>
        <source>Write single PDU length mismatch for function 0x%1. Expected 4 bytes, got %2</source>
        <translation>功能碼 0x%1 的單次寫入 PDU 長度不匹配。預期 4 位元組，實際 %2 位元組</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="481"/>
        <source>Write request byte count mismatch. Declared %1, actual %2</source>
        <translation>寫入請求位元組數不匹配。宣告 %1，實際 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="492"/>
        <source>Register write byte count does not match quantity. Declared %1, expected %2</source>
        <translation>暫存器寫入位元組數與數量不符。宣告 %1，預期 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="526"/>
        <source>Coil write byte count does not match quantity. Declared %1, expected %2</source>
        <translation>線圈寫入位元組數與數量不符。宣告 %1，預期 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="557"/>
        <source>Unsupported function code 0x%1 for deep parsing</source>
        <translation>不支援對功能碼 0x%1 進行深度解析</translation>
    </message>
</context>
<context>
    <name>ModbusPduBuilder</name>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="18"/>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="34"/>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="48"/>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="61"/>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="94"/>
        <source>Invalid start address</source>
        <translation>無效的起始地址</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="21"/>
        <source>Invalid quantity</source>
        <translation>無效的數量</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="64"/>
        <source>Invalid quantity for 0x0F</source>
        <translation>0x0F 數量無效</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="69"/>
        <source>Data length mismatch for 0x0F</source>
        <translation>0x0F 資料長度不匹配</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="97"/>
        <source>Invalid quantity for 0x10</source>
        <translation>0x10 數量無效</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="101"/>
        <source>Data length mismatch for 0x10</source>
        <translation>0x10 資料長度不匹配</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="270"/>
        <source>Cannot write file: %1</source>
        <translation>無法寫入檔案：%1</translation>
    </message>
</context>
<context>
    <name>ValueFormatter</name>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="59"/>
        <source>Description: %1</source>
        <translation>描述：%1</translation>
    </message>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="65"/>
        <source>Raw: %1</source>
        <translation>原始值：%1</translation>
    </message>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="66"/>
        <source>Scale: %1</source>
        <translation>倍率：%1</translation>
    </message>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="67"/>
        <source>Scaled: %1</source>
        <translation>換算值：%1</translation>
    </message>
</context>
<context>
    <name>core::update::ChecksumWorker</name>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="65"/>
        <source>Missing or invalid expected checksum</source>
        <translation>缺少或無效的預期校驗值</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="79"/>
        <source>Failed to calculate file checksum</source>
        <translation>計算檔案校驗值失敗</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="84"/>
        <source>Checksum mismatch. Expected: %1, Actual: %2</source>
        <translation>校驗值不符。預期：%1，實際：%2</translation>
    </message>
</context>
<context>
    <name>core::update::UpdateManager</name>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="114"/>
        <source>Failed to access system temporary directory</source>
        <translation>無法存取系統暫存目錄</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="120"/>
        <source>Failed to create update directory</source>
        <translation>建立更新目錄失敗</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="126"/>
        <source>Invalid update URL</source>
        <translation>更新位址無效</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="173"/>
        <source>Failed to open local file for writing</source>
        <translation>無法開啟本地檔案進行寫入</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="253"/>
        <source>Failed to create update task file</source>
        <translation>建立更新任務檔案失敗</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="274"/>
        <source>Updater not found</source>
        <translation>找不到更新器</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="296"/>
        <source>Failed to launch updater (Access Denied or System Error)</source>
        <translation>啟動更新器失敗（拒絕存取或系統錯誤）</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="307"/>
        <source>Automatic update is only supported on Windows</source>
        <translation>自動更新僅支援 Windows 系統</translation>
    </message>
</context>
<context>
    <name>ui::MainWindow</name>
    <message>
        <location filename="../MainWindow.cpp" line="184"/>
        <location filename="../MainWindow.cpp" line="541"/>
        <source>Modbus Tools</source>
        <translation>Modbus 工具</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="304"/>
        <location filename="../MainWindow.cpp" line="542"/>
        <source>Modbus TCP</source>
        <translation>Modbus TCP</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="305"/>
        <location filename="../MainWindow.cpp" line="542"/>
        <source>Modbus RTU</source>
        <translation>Modbus RTU</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="306"/>
        <location filename="../MainWindow.cpp" line="542"/>
        <source>TCP Client</source>
        <translation>TCP 用戶端</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="307"/>
        <location filename="../MainWindow.cpp" line="542"/>
        <source>Serial Port</source>
        <translation>序列埠</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="308"/>
        <location filename="../MainWindow.cpp" line="542"/>
        <source>Frame Analyzer</source>
        <translation>報文分析</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="344"/>
        <location filename="../MainWindow.cpp" line="548"/>
        <source>Language</source>
        <translation>語言</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="338"/>
        <location filename="../MainWindow.cpp" line="549"/>
        <source>Settings</source>
        <translation>設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="235"/>
        <source>You are using the latest version: v%1</source>
        <translation>您正在使用最新版本：v%1</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="250"/>
        <location filename="../MainWindow.cpp" line="254"/>
        <source>Update Failed</source>
        <translation>更新失敗</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="334"/>
        <source>Expand Navigation</source>
        <translation>展開導覽</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="334"/>
        <source>Collapse Navigation</source>
        <translation>摺疊導覽</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="339"/>
        <location filename="../MainWindow.cpp" line="551"/>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="28"/>
        <source>Modbus Settings</source>
        <translation>Modbus設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="340"/>
        <location filename="../MainWindow.cpp" line="552"/>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="30"/>
        <source>Update Settings</source>
        <translation>更新設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="358"/>
        <location filename="../MainWindow.cpp" line="556"/>
        <source>English (US)</source>
        <translation>英語（美國）</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="359"/>
        <location filename="../MainWindow.cpp" line="557"/>
        <source>简体中文</source>
        <translation>簡體中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="360"/>
        <location filename="../MainWindow.cpp" line="558"/>
        <source>繁體中文</source>
        <translation>繁體中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="369"/>
        <location filename="../MainWindow.cpp" line="460"/>
        <location filename="../MainWindow.cpp" line="553"/>
        <source>Check for Updates</source>
        <translation>檢查更新</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="474"/>
        <source>New version v%1 available. Open download page?</source>
        <translation>發現新版本 v%1。是否開啟下載頁面？</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="482"/>
        <source>Current: v%1, Latest: v%2
Choose update method:</source>
        <translation>目前版本：v%1，最新版本：v%2
請選擇更新方式：</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="498"/>
        <source>Downloading Update...</source>
        <translation>正在下載更新...</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="36"/>
        <source>Request Timeout (ms):</source>
        <translation>請求逾時(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="40"/>
        <source>Enable Retry:</source>
        <translation>啟用重試：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="45"/>
        <source>Retry Count:</source>
        <translation>重試次數：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="51"/>
        <source>Retry Interval (ms):</source>
        <translation>重試間隔(ms)：</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="474"/>
        <location filename="../MainWindow.cpp" line="481"/>
        <source>Update Available</source>
        <translation>發現新版本</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="235"/>
        <source>No Updates</source>
        <translation>沒有可用更新</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="239"/>
        <source>Update Check Failed</source>
        <translation>更新檢查失敗</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="498"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="483"/>
        <source>Update Main Program</source>
        <translation>更新主程式</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="484"/>
        <source>Download Full Package</source>
        <translation>下載完整安裝包</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="368"/>
        <location filename="../MainWindow.cpp" line="371"/>
        <location filename="../MainWindow.cpp" line="459"/>
        <location filename="../MainWindow.cpp" line="550"/>
        <location filename="../MainWindow.cpp" line="554"/>
        <location filename="../widgets/AboutDialog.cpp" line="24"/>
        <source>About</source>
        <translation>關於</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="391"/>
        <source>Theme: Auto</source>
        <translation>主題：自動</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="392"/>
        <source>Theme: Light</source>
        <translation>主題：淺色</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="392"/>
        <source>Theme: Dark</source>
        <translation>主題：深色</translation>
    </message>
    <message>
        <location filename="../widgets/AboutDialog.cpp" line="33"/>
        <source>Welcome to Modbus-Tools&lt;br&gt;Version: v%1&lt;br&gt;&lt;br&gt;An open-source Modbus communication debugging assistant.&lt;br&gt;Developer: mingyucheng692&lt;br&gt;License: MIT License&lt;br&gt;&lt;br&gt;This project is developed in spare time, completely free and open-source.&lt;br&gt;Feel free to star on GitHub or submit issues.&lt;br&gt;Your feedback keeps the project improving!&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 Visit GitHub Repository&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 Issue Tracker&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;This software is provided &amp;quot;as is&amp;quot; without warranty of any kind.</source>
        <translation>歡迎使用 Modbus-Tools&lt;br&gt;版本：v%1&lt;br&gt;&lt;br&gt;一款開源的 Modbus 通訊除錯助手。&lt;br&gt;開發者：mingyucheng692&lt;br&gt;授權條款：MIT License&lt;br&gt;&lt;br&gt;本專案由個人業餘開發，完全免費且開源。&lt;br&gt;歡迎在 GitHub 上 Star ⭐ 或提交 Issue。&lt;br&gt;您的回饋是專案持續改進的動力！&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 造訪 GitHub 倉庫&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 問題回報&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;本軟體按&amp;quot;原樣&amp;quot;提供，無任何形式的保證。</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="35"/>
        <source>Every Startup</source>
        <translation>每次啟動</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="36"/>
        <source>Weekly</source>
        <translation>每週</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="37"/>
        <source>Monthly</source>
        <translation>每月</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="38"/>
        <source>Disable Update Check</source>
        <translation>關閉更新檢查</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="39"/>
        <source>Update Check Frequency:</source>
        <translation>更新檢查頻率：</translation>
    </message>
</context>
<context>
    <name>ui::common::UpdateChecker</name>
    <message>
        <location filename="../common/updatechecker.cpp" line="107"/>
        <source>Release tag is missing</source>
        <translation>發布標籤缺失</translation>
    </message>
</context>
<context>
    <name>ui::views::generic_serial::GenericSerialView</name>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="120"/>
        <source>File transfer started: %1 (%2 bytes)</source>
        <translation>檔案傳輸已開始：%1 (%2 位元組)</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="128"/>
        <source>File transfer progress: %1 %2/%3</source>
        <translation>檔案傳輸進度：%1 %2/%3</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="135"/>
        <source>File transfer finished: %1</source>
        <translation>檔案傳輸完成：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="138"/>
        <source>File transfer failed: %1 (%2)</source>
        <translation>檔案傳輸失敗：%1 (%2)</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="141"/>
        <source>File transfer canceled: %1</source>
        <translation>檔案傳輸已取消：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="186"/>
        <source>Opening %1...</source>
        <translation>正在開啟 %1...</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="229"/>
        <source>Closed</source>
        <translation>已關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="230"/>
        <source>Opening</source>
        <translation>正在開啟</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="231"/>
        <source>Open</source>
        <translation>已開啟</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="232"/>
        <source>Closing</source>
        <translation>正在關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="233"/>
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="234"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="237"/>
        <source>State changed: %1</source>
        <translation>狀態已變更：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="242"/>
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="275"/>
        <source>Control</source>
        <translation>控制</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="276"/>
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="277"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="278"/>
        <source>RTS</source>
        <translation>RTS</translation>
    </message>
</context>
<context>
    <name>ui::views::generic_tcp::GenericTcpView</name>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="94"/>
        <source>File transfer started: %1 (%2 bytes)</source>
        <translation>檔案傳輸已開始：%1 (%2 位元組)</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="102"/>
        <source>File transfer progress: %1 %2/%3</source>
        <translation>檔案傳輸進度：%1 %2/%3</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="109"/>
        <source>File transfer finished: %1</source>
        <translation>檔案傳輸完成：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="112"/>
        <source>File transfer failed: %1 (%2)</source>
        <translation>檔案傳輸失敗：%1 (%2)</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="115"/>
        <source>File transfer canceled: %1</source>
        <translation>檔案傳輸已取消：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="163"/>
        <source>Connecting to %1:%2...</source>
        <translation>正在連線 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="224"/>
        <source>Closed</source>
        <translation>已關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="225"/>
        <source>Opening</source>
        <translation>正在開啟</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="226"/>
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="227"/>
        <source>Closing</source>
        <translation>正在關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="228"/>
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="229"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="232"/>
        <source>State changed: %1</source>
        <translation>狀態已變更：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="241"/>
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="254"/>
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus_rtu::ModbusRtuView</name>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="173"/>
        <source>Opening %1...</source>
        <translation>正在開啟 %1...</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="324"/>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation>發送讀取請求 FC:%1 位址:%2 數量:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="281"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="319"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="467"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="506"/>
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="193"/>
        <source>Failed to create Modbus stack</source>
        <translation>建立 Modbus 堆疊失敗</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="239"/>
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="242"/>
        <source>Connection failed: %1</source>
        <translation>連線失敗：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="259"/>
        <source>Success: Broadcast write sent, no response expected</source>
        <translation>成功：廣播寫已發送，預期無回應</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="265"/>
        <source>Success: Response received</source>
        <translation>成功：已收到回應</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="297"/>
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="342"/>
        <source>Unsupported function code</source>
        <translation>不支援的功能碼</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="352"/>
        <source>Error: Invalid decimal value for 0x05</source>
        <translation>錯誤：0x05 十進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="367"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="373"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="380"/>
        <source>Error: Invalid hex value for 0x05</source>
        <translation>錯誤：0x05 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="389"/>
        <source>Error: Empty value for 0x06</source>
        <translation>錯誤：0x06 值為空</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="396"/>
        <source>Error: Invalid decimal value for 0x06</source>
        <translation>錯誤：0x06 十進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="406"/>
        <source>Error: Invalid hex value for 0x06</source>
        <translation>錯誤：0x06 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="426"/>
        <source>Error: Binary bit count (%1) does not match Quantity (%2)</source>
        <translation>錯誤：二進位位元數 (%1) 與數量 (%2) 不相符</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="401"/>
        <source>Error: Binary format not supported for registers (0x06)</source>
        <translation>錯誤：暫存器操作 (0x06) 不支援二進位格式</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="360"/>
        <source>Error: Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation>錯誤：0x05 二進位值無效（預期為 0 或 1）</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="418"/>
        <source>Error: Invalid quantity for 0x0F</source>
        <translation>錯誤：0x0F 數量無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="440"/>
        <source>Error: Empty value for 0x10</source>
        <translation>錯誤：0x10 值為空</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="445"/>
        <source>Error: Invalid quantity for 0x10</source>
        <translation>錯誤：0x10 數量無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="453"/>
        <source>Error: Invalid decimal list for 0x10</source>
        <translation>錯誤：0x10 十進位清單無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="459"/>
        <source>Error: Invalid hex value for 0x10</source>
        <translation>錯誤：0x10 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="473"/>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation>發送寫入請求 FC:%1 位址:%2 資料:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="267"/>
        <source>Success: Write confirmed</source>
        <translation>成功：寫入已確認</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="490"/>
        <source>Sending Raw Data: %1</source>
        <translation>發送原始資料：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="279"/>
        <source>Poll Error: %1</source>
        <translation>輪詢錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="434"/>
        <source>Error: 0x0F requires Hex or Binary data</source>
        <translation>錯誤：0x0F 需要十六進位或二進位資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="635"/>
        <source>Data Monitor</source>
        <translation>資料監視</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="636"/>
        <source>Receive Data</source>
        <translation>接收資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="637"/>
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="638"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="639"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="640"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="641"/>
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="642"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="643"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus_tcp::ModbusTcpView</name>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="188"/>
        <source>Connecting to %1:%2...</source>
        <translation>正在連線 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="44"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="273"/>
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="48"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="333"/>
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="359"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="548"/>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation>發送讀取請求 FC:%1 位址:%2 數量:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="314"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="354"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="503"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="543"/>
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="202"/>
        <source>Failed to create Modbus stack</source>
        <translation>建立 Modbus 堆疊失敗</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="277"/>
        <source>Connection failed: %1</source>
        <translation>連線失敗：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="298"/>
        <source>Success: Response received</source>
        <translation>成功：已收到回應</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="378"/>
        <source>Unsupported function code</source>
        <translation>不支援的功能碼</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="388"/>
        <source>Error: Invalid decimal value for 0x05</source>
        <translation>錯誤：0x05 十進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="403"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="409"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="416"/>
        <source>Error: Invalid hex value for 0x05</source>
        <translation>錯誤：0x05 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="425"/>
        <source>Error: Empty value for 0x06</source>
        <translation>錯誤：0x06 值為空</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="432"/>
        <source>Error: Invalid decimal value for 0x06</source>
        <translation>錯誤：0x06 十進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="442"/>
        <source>Error: Invalid hex value for 0x06</source>
        <translation>錯誤：0x06 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="437"/>
        <source>Error: Binary format not supported for registers (0x06)</source>
        <translation>錯誤：暫存器操作 (0x06) 不支援二進位格式</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="462"/>
        <source>Error: Binary bit count (%1) does not match Quantity (%2)</source>
        <translation>錯誤：二進位位元數 (%1) 與數量 (%2) 不相符</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="396"/>
        <source>Error: Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation>錯誤：0x05 二進位值無效（預期為 0 或 1）</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="454"/>
        <source>Error: Invalid quantity for 0x0F</source>
        <translation>錯誤：0x0F 數量無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="476"/>
        <source>Error: Empty value for 0x10</source>
        <translation>錯誤：0x10 值為空</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="481"/>
        <source>Error: Invalid quantity for 0x10</source>
        <translation>錯誤：0x10 數量無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="489"/>
        <source>Error: Invalid decimal list for 0x10</source>
        <translation>錯誤：0x10 十進位清單無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="495"/>
        <source>Error: Invalid hex value for 0x10</source>
        <translation>錯誤：0x10 十六進位值無效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="509"/>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation>發送寫入請求 FC:%1 位址:%2 資料:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="300"/>
        <source>Success: Write confirmed</source>
        <translation>成功：寫入已確認</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="527"/>
        <source>Sending Raw Data: %1</source>
        <translation>發送原始資料：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="312"/>
        <source>Poll Error: %1</source>
        <translation>輪詢錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="470"/>
        <source>Error: 0x0F requires Hex or Binary data</source>
        <translation>錯誤：0x0F 需要十六進位或二進位資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="677"/>
        <source>Data Monitor</source>
        <translation>資料監視</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="678"/>
        <source>Receive Data</source>
        <translation>接收資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="679"/>
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="680"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="681"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="682"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="683"/>
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="684"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="685"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
</context>
<context>
    <name>ui::widgets::CollapsibleSection</name>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="101"/>
        <source>Collapse</source>
        <translation>收合</translation>
    </message>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="101"/>
        <source>Expand</source>
        <translation>展開</translation>
    </message>
</context>
<context>
    <name>ui::widgets::ControlWidget</name>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="116"/>
        <source>Invalid Address format or range (0-65535): %1</source>
        <translation>無效的位址格式或範圍 (0-65535): %1</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="128"/>
        <source>Confirm Address</source>
        <translation>確認位址</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="129"/>
        <source>The polling address is set to 0. Are you sure you want to continue?</source>
        <translation>輪詢位址設置為 0。確定要繼續嗎？</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="133"/>
        <source>Do not show this again</source>
        <translation>不再顯示</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="325"/>
        <source>Enable Polling</source>
        <translation>啟用輪詢</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="331"/>
        <source>Interval(ms):</source>
        <translation>間隔(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="334"/>
        <source>FC:</source>
        <translation>功能碼：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="337"/>
        <source>01-Read Coils</source>
        <translation>01-讀取線圈</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="338"/>
        <source>02-Read Discrete</source>
        <translation>02-讀取離散輸入</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="339"/>
        <source>03-Read Holding</source>
        <translation>03-讀取保持暫存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="340"/>
        <source>04-Read Input</source>
        <translation>04-讀取輸入暫存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="343"/>
        <source>Addr:</source>
        <translation>位址：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="346"/>
        <source>Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>位址 (0-65535)。支援 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="349"/>
        <source>Qty:</source>
        <translation>數量：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="328"/>
        <source>Link to Analyzer</source>
        <translation>聯動分析儀</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FrameAnalyzerWidget</name>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="302"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="952"/>
        <source>Frame Input</source>
        <translation>報文輸入</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="308"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="953"/>
        <source>Protocol:</source>
        <translation>通訊協定：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="311"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="955"/>
        <source>Auto Detect</source>
        <translation>自動偵測</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="312"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="760"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="956"/>
        <source>Modbus TCP</source>
        <translation>Modbus TCP</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="313"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="760"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="957"/>
        <source>Modbus RTU</source>
        <translation>Modbus RTU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="317"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="959"/>
        <source>Start Address (for Response):</source>
        <translation>起始位址（用於回應）：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="331"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="975"/>
        <source>Format Hex</source>
        <translation>格式化 Hex</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="336"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="998"/>
        <source>Parse</source>
        <translation>解析</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="341"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="999"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="351"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1001"/>
        <source>Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)</source>
        <translation>輸入十六進位字串（如 01 03 00 00 00 01 84 0A）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="362"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1004"/>
        <source>Analysis Result</source>
        <translation>分析結果</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="378"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="986"/>
        <source>Status:</source>
        <translation>狀態：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="382"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="840"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="995"/>
        <source>Ready</source>
        <translation>就緒</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="391"/>
        <source>Tip: &quot;Pause&quot; to edit description</source>
        <translation>提示：&quot;暫停&quot;以編輯描述</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="408"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="413"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="852"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="984"/>
        <source>Pause Refresh</source>
        <translation>暫停重新整理</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="413"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="984"/>
        <source>Resume Refresh</source>
        <translation>恢復重新整理</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="422"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="982"/>
        <source>Stop Link</source>
        <translation>停止連動</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="435"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="963"/>
        <source>Decode Mode:</source>
        <translation>解碼模式：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="437"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="965"/>
        <source>Unsigned</source>
        <translation>無符號</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="438"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="966"/>
        <source>Signed</source>
        <translation>有符號</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="472"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="667"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="976"/>
        <source>Import Config</source>
        <translation>匯入設定</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="474"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="650"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="977"/>
        <source>Export Config</source>
        <translation>匯出設定</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="476"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="698"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="978"/>
        <source>Export CSV</source>
        <translation>匯出 CSV</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="500"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1012"/>
        <source>Field</source>
        <translation>欄位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="500"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="508"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1013"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1017"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="500"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="508"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1014"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1017"/>
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="504"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="861"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1007"/>
        <source>Structure</source>
        <translation>結構</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="508"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1017"/>
        <source>Address</source>
        <translation>位址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="508"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1017"/>
        <source>Hex</source>
        <translation>十六進位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="508"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1017"/>
        <source>Decimal</source>
        <translation>十進位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="508"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1017"/>
        <source>Binary</source>
        <translation>二進位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="508"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1017"/>
        <source>Scale</source>
        <translation>倍率</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="694"/>
        <source>There is no data to export.</source>
        <translation>沒有可導出的資料。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="762"/>
        <source>Raw Hex</source>
        <translation>原始十六進位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="783"/>
        <source>Valid</source>
        <translation>有效</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="783"/>
        <source>Invalid (Expected 0x%1)</source>
        <translation>無效 (預期 0x%1)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="830"/>
        <source>Success</source>
        <translation>成功</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="830"/>
        <source>Success with warnings</source>
        <translation>成功（帶警告）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="868"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1007"/>
        <source>Structure (Live Mode)</source>
        <translation>結構（連動模式）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="961"/>
        <source>Start Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>起始位址 (0-65535)。支援 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="539"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1005"/>
        <source>History</source>
        <translation>歷史記錄</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="543"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1019"/>
        <source>Clear History</source>
        <translation>清除歷史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="650"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="667"/>
        <source>JSON Files (*.json)</source>
        <translation>JSON 檔案 (*.json)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="694"/>
        <source>No Data</source>
        <translation>無資料</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="700"/>
        <source>CSV Files (*.csv)</source>
        <translation>CSV 檔案 (*.csv)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="660"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="720"/>
        <source>Export Failed</source>
        <translation>匯出失敗</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="222"/>
        <source>OK</source>
        <translation>確定</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="222"/>
        <source>ERR</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="534"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1008"/>
        <source>Decoded Data</source>
        <translation>解析資料</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="598"/>
        <source>Invalid Address (0-65535): %1</source>
        <translation>無效位址 (0-65535): %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="627"/>
        <source>Parse Failed</source>
        <translation>解析失敗</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="672"/>
        <source>Import Failed</source>
        <translation>匯入失敗</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="884"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="989"/>
        <source>LIVE: %1</source>
        <translation>連動中: %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="888"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="991"/>
        <source>Live Data Received at %1</source>
        <translation>即時資料接收於 %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="589"/>
        <source>Error: Empty input</source>
        <translation>錯誤：輸入為空</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="605"/>
        <source>Parsing...</source>
        <translation>解析中...</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="739"/>
        <source>Parse Failed: %1</source>
        <translation>解析失敗：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="988"/>
        <source>TCP</source>
        <translation>TCP</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="988"/>
        <source>RTU</source>
        <translation>RTU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="758"/>
        <source>Frame</source>
        <translation>幀</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="759"/>
        <source>%1 bytes</source>
        <translation>%1 位元組</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="765"/>
        <source>MBAP Header</source>
        <translation>MBAP 標頭</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="766"/>
        <source>Transaction ID</source>
        <translation>交易識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="767"/>
        <source>Protocol ID</source>
        <translation>協定識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="768"/>
        <source>Length</source>
        <translation>長度</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="769"/>
        <source>Unit ID</source>
        <translation>單元識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="771"/>
        <source>Slave ID</source>
        <translation>從站識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="774"/>
        <source>PDU</source>
        <translation>PDU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="776"/>
        <source>Function Code</source>
        <translation>功能碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="779"/>
        <source>Exception Code</source>
        <translation>例外碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="784"/>
        <source>CRC16</source>
        <translation>CRC16</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="207"/>
        <source>Show History</source>
        <translation>顯示歷史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="207"/>
        <source>Hide History</source>
        <translation>隱藏歷史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="451"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="968"/>
        <source>Byte Order:</source>
        <translation>位元組順序：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="453"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="970"/>
        <source>ABCD(default)</source>
        <translation>ABCD(預設)</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FunctionWidget</name>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="386"/>
        <source>Standard</source>
        <translation>標準</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="389"/>
        <source>Raw</source>
        <translation>原始</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="392"/>
        <source>Slave ID:</source>
        <translation>從站ID：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="395"/>
        <source>Start Addr:</source>
        <translation>起始位址：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="404"/>
        <source>Quantity:</source>
        <translation>數量：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="406"/>
        <source>Write Data:</source>
        <translation>寫入資料：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="407"/>
        <source>Format:</source>
        <translation>格式：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="413"/>
        <source>Hex</source>
        <translation>十六進位</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="415"/>
        <source>Binary</source>
        <translation>二進位</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="363"/>
        <source>Space separated hex (e.g., 01 02)</source>
        <translation>空格分隔的十六進位 (如 01 02)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="316"/>
        <location filename="../widgets/FunctionWidget.cpp" line="333"/>
        <source>Invalid Slave ID format or range (0-255): %1</source>
        <translation>無效的 Slave ID 格式或範圍 (0-255): %1</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="320"/>
        <location filename="../widgets/FunctionWidget.cpp" line="337"/>
        <source>Invalid Address format or range (0-65535): %1</source>
        <translation>無效的位址格式或範圍 (0-65535): %1</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="365"/>
        <source>Space separated decimal (e.g., 100 200)</source>
        <translation>空格分隔的十進位 (如 100 200)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="367"/>
        <source>Bit string (e.g., 1 1 0 1)</source>
        <translation>二進位位元字串 (如 1 1 0 1)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="398"/>
        <source>Slave ID (0-255). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>Slave ID (0-255)。支援 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="401"/>
        <source>Start Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>起始位址 (0-65535)。支援 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="414"/>
        <source>Decimal</source>
        <translation>十進位</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="418"/>
        <source>Read Coils (0x01)</source>
        <translation>讀取線圈 (0x01)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="421"/>
        <source>Read Discrete Inputs (0x02)</source>
        <translation>讀取離散輸入 (0x02)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="424"/>
        <source>Read Holding Registers (0x03)</source>
        <translation>讀取保持暫存器 (0x03)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="427"/>
        <source>Read Input Registers (0x04)</source>
        <translation>讀取輸入暫存器 (0x04)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="430"/>
        <source>Write Single Coil (0x05)</source>
        <translation>寫入單線圈 (0x05)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="433"/>
        <source>Write Single Register (0x06)</source>
        <translation>寫入單暫存器 (0x06)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="436"/>
        <source>Write Multiple Coils (0x0F)</source>
        <translation>寫入多線圈 (0x0F)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="439"/>
        <source>Write Multiple Registers (0x10)</source>
        <translation>寫入多暫存器 (0x10)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="442"/>
        <source>Raw Hex Data (e.g., 01 03 00 00 00 01 84 0A):</source>
        <translation>原始十六進位資料(如 01 03 00 00 00 01 84 0A)：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="445"/>
        <source>Send Raw</source>
        <translation>發送原始資料</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="448"/>
        <source>Append CRC</source>
        <translation>計算并追加 CRC</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="451"/>
        <source>Add MBAP</source>
        <translation>添加 MBAP 頭</translation>
    </message>
</context>
<context>
    <name>ui::widgets::GenericInputWidget</name>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="217"/>
        <source>Select File to Send</source>
        <translation>選擇要發送的檔案</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="228"/>
        <source>Enter data to send...</source>
        <translation>輸入要發送的資料...</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="231"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="234"/>
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="237"/>
        <source>Auto Send</source>
        <translation>自動發送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="240"/>
        <source> ms</source>
        <translation> 毫秒</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="243"/>
        <source>Send File</source>
        <translation>發送檔案</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="246"/>
        <source>Send</source>
        <translation>發送</translation>
    </message>
</context>
<context>
    <name>ui::widgets::SerialConnectionWidget</name>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="66"/>
        <source>Disconnect</source>
        <translation>中斷連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="67"/>
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="70"/>
        <source>Connect</source>
        <translation>連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="71"/>
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="127"/>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="291"/>
        <source>Refresh Ports</source>
        <translation>重新整理埠</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="273"/>
        <source>Connection Settings</source>
        <translation>連線設定</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="276"/>
        <source>Port:</source>
        <translation>埠：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="279"/>
        <source>Baud:</source>
        <translation>鮑特率：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="282"/>
        <source>Data:</source>
        <translation>資料位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="285"/>
        <source>Parity:</source>
        <translation>同位元：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="288"/>
        <source>Stop:</source>
        <translation>停止位：</translation>
    </message>
</context>
<context>
    <name>ui::widgets::TcpConnectionWidget</name>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="151"/>
        <source>Disconnect</source>
        <translation>中斷連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="152"/>
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="157"/>
        <source>Connect</source>
        <translation>連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="158"/>
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="167"/>
        <source>Connection Settings</source>
        <translation>連線設定</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="170"/>
        <source>Host:</source>
        <translation>主機：</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="173"/>
        <source>Port:</source>
        <translation>埠：</translation>
    </message>
</context>
<context>
    <name>ui::widgets::TrafficMonitorWidget</name>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="103"/>
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="115"/>
        <source>[%1] [TX] %2</source>
        <translation>[%1] [TX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="131"/>
        <source>[%1] [RX] %2</source>
        <translation>[%1] [RX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="144"/>
        <source>[%1] [INFO] %2</source>
        <translation>[%1] [資訊] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="157"/>
        <source>[%1] [WARN] %2</source>
        <translation>[%1] [警告] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="220"/>
        <source>Save Log</source>
        <translation>儲存日誌</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="220"/>
        <source>Text Files (*.txt);;All Files (*)</source>
        <translation>文字檔 (*.txt);;所有檔案 (*)</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="235"/>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="257"/>
        <source>Save failed: %1</source>
        <translation>儲存失敗：%1</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="293"/>
        <source>Traffic Monitor</source>
        <translation>通訊監視</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="294"/>
        <source>Auto Scroll</source>
        <translation>自動捲動</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="297"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="298"/>
        <source>Save</source>
        <translation>儲存</translation>
    </message>
</context>
</TS>
