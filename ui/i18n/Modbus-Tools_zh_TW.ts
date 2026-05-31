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
    <name>ModbusProtocolChecks</name>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="22"/>
        <source>Exception</source>
        <translation>例外</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="23"/>
        <source>Bit read</source>
        <translation>位元讀取</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="24"/>
        <source>Register read</source>
        <translation>暫存器讀取</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="25"/>
        <source>Write single</source>
        <translation>單筆寫入</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="26"/>
        <source>Write multiple</source>
        <translation>多筆寫入</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="27"/>
        <source>%1 response payload length mismatch: expected %2, got %3</source>
        <translation>%1 回應負載長度不符。預期 %2，實際 %3</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="28"/>
        <source>Exception function code does not match request</source>
        <translation>例外功能碼與請求不符</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="29"/>
        <source>Response function code does not match request</source>
        <translation>回應功能碼與請求不符</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="30"/>
        <source>Request quantity missing for bit-read validation</source>
        <translation>位元讀取校驗缺少請求數量</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="31"/>
        <source>Bit-read response byte count does not match payload length</source>
        <translation>位元讀取回應位元組數與負載長度不符</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="32"/>
        <source>Bit-read response byte count does not match requested quantity</source>
        <translation>位元讀取回應位元組數與請求數量不符</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="33"/>
        <source>Request quantity missing for register-read validation</source>
        <translation>暫存器讀取校驗缺少請求數量</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="34"/>
        <source>Register-read response byte count does not match payload length</source>
        <translation>暫存器讀取回應位元組數與負載長度不符</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="35"/>
        <source>Register-read response byte count does not match requested quantity</source>
        <translation>暫存器讀取回應位元組數與請求數量不符</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="36"/>
        <source>Register-read response byte count must be even</source>
        <translation>暫存器讀取回應位元組數必須為偶數</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="37"/>
        <source>Write-single response echo does not match request</source>
        <translation>單筆寫入回應回顯與請求不符</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="38"/>
        <source>Request echo fields missing for write-multiple validation</source>
        <translation>多筆寫入校驗缺少請求回顯欄位</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="39"/>
        <source>Write-multiple response echo fields are incomplete</source>
        <translation>多筆寫入回應回顯欄位不完整</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="40"/>
        <source>Write-multiple response echo does not match request</source>
        <translation>多筆寫入回應回顯與請求不符</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="41"/>
        <source>Unsupported function code for response validation</source>
        <translation>回應校驗不支援該功能碼</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="540"/>
        <source>Cannot write file: %1</source>
        <translation>無法寫入檔案：%1</translation>
    </message>
    <message>
        <location filename="../../app/main.cpp" line="43"/>
        <location filename="../../app/main.cpp" line="49"/>
        <source>Startup Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../app/main.cpp" line="44"/>
        <source>Failed to initialize application logging.
%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../app/main.cpp" line="50"/>
        <source>Failed to initialize application logging.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../shell/NavigationController.cpp" line="142"/>
        <source>Expand Navigation</source>
        <translation type="unfinished">展開導覽</translation>
    </message>
    <message>
        <location filename="../shell/NavigationController.cpp" line="142"/>
        <source>Collapse Navigation</source>
        <translation type="unfinished">摺疊導覽</translation>
    </message>
</context>
<context>
    <name>ReconnectPolicy</name>
    <message>
        <location filename="../../core/ReconnectPolicy.cpp" line="84"/>
        <source>Attempt %1/%2</source>
        <translation>重連嘗試 %1/%2</translation>
    </message>
</context>
<context>
    <name>RequestSubmissionService</name>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="50"/>
        <source>Unsupported function code</source>
        <translation type="unfinished">不支援的功能碼</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="60"/>
        <source>Invalid decimal value for 0x05</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="70"/>
        <source>Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="79"/>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="87"/>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="96"/>
        <source>Invalid hex value for 0x05</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="107"/>
        <source>Empty value for 0x06</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="116"/>
        <source>Invalid decimal value for 0x06</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="123"/>
        <source>Binary format not supported for registers (0x06)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="130"/>
        <source>Invalid hex value for 0x06</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="143"/>
        <source>Invalid quantity for 0x0F</source>
        <translation type="unfinished">0x0F 數量無效</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="153"/>
        <source>Binary bit count (%1) does not match Quantity (%2)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="163"/>
        <source>0x0F requires Hex or Binary data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="171"/>
        <source>Empty value for 0x10</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="177"/>
        <source>Invalid quantity for 0x10</source>
        <translation type="unfinished">0x10 數量無效</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="187"/>
        <source>Invalid decimal list for 0x10</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="195"/>
        <source>Invalid hex value for 0x10</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="219"/>
        <source>Raw data is empty</source>
        <translation type="unfinished"></translation>
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
        <location filename="../../core/update/UpdateManager.cpp" line="73"/>
        <source>Missing or invalid expected checksum</source>
        <translation>缺少或無效的預期校驗值</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="87"/>
        <source>Failed to calculate file checksum</source>
        <translation>計算檔案校驗值失敗</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="92"/>
        <source>Checksum mismatch. Expected: %1, Actual: %2</source>
        <translation>校驗值不符。預期：%1，實際：%2</translation>
    </message>
</context>
<context>
    <name>core::update::UpdateManager</name>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="134"/>
        <source>Failed to access system temporary directory</source>
        <translation>無法存取系統暫存目錄</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="140"/>
        <source>Failed to create update directory</source>
        <translation>建立更新目錄失敗</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="146"/>
        <source>Invalid update URL</source>
        <translation>更新位址無效</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="193"/>
        <source>Failed to open local file for writing</source>
        <translation>無法開啟本地檔案進行寫入</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="273"/>
        <source>Failed to create update task file</source>
        <translation>建立更新任務檔案失敗</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="300"/>
        <source>Updater not found</source>
        <translation>找不到更新器</translation>
    </message>
    <message>
        <source>Failed to launch updater (Access Denied or System Error)</source>
        <translation type="vanished">啟動更新器失敗（拒絕存取或系統錯誤）</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="294"/>
        <source>Automatic update is only supported on Windows</source>
        <translation>自動更新僅支援 Windows 系統</translation>
    </message>
</context>
<context>
    <name>ui::MainWindow</name>
    <message>
        <location filename="../MainWindow.cpp" line="125"/>
        <location filename="../MainWindow.cpp" line="411"/>
        <source>Modbus Tools</source>
        <translation>Modbus 工具</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="187"/>
        <location filename="../MainWindow.cpp" line="412"/>
        <source>Modbus TCP</source>
        <translation>Modbus TCP</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="187"/>
        <location filename="../MainWindow.cpp" line="412"/>
        <source>Modbus RTU</source>
        <translation>Modbus RTU</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="187"/>
        <location filename="../MainWindow.cpp" line="412"/>
        <source>TCP Client</source>
        <translation>TCP 用戶端</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="187"/>
        <location filename="../MainWindow.cpp" line="412"/>
        <source>Serial Port</source>
        <translation>序列埠</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="187"/>
        <location filename="../MainWindow.cpp" line="412"/>
        <source>Frame Analyzer</source>
        <translation>報文分析</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="246"/>
        <location filename="../MainWindow.cpp" line="416"/>
        <source>Language</source>
        <translation>語言</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="232"/>
        <location filename="../MainWindow.cpp" line="417"/>
        <source>Settings</source>
        <translation>設定</translation>
    </message>
    <message>
        <source>You are using the latest version: v%1</source>
        <translation type="vanished">您正在使用最新版本：v%1</translation>
    </message>
    <message>
        <source>Update Failed</source>
        <translation type="vanished">更新失敗</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="414"/>
        <source>Expand Navigation</source>
        <translation>展開導覽</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="414"/>
        <source>Collapse Navigation</source>
        <translation>摺疊導覽</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="233"/>
        <location filename="../MainWindow.cpp" line="419"/>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="28"/>
        <source>Modbus Settings</source>
        <translation>Modbus設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="238"/>
        <location filename="../MainWindow.cpp" line="420"/>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="32"/>
        <source>Update Settings</source>
        <translation>更新設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="260"/>
        <location filename="../MainWindow.cpp" line="424"/>
        <source>English (US)</source>
        <translation>英語（美國）</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="261"/>
        <location filename="../MainWindow.cpp" line="425"/>
        <source>简体中文</source>
        <translation>簡體中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="262"/>
        <location filename="../MainWindow.cpp" line="426"/>
        <source>繁體中文</source>
        <translation>繁體中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="273"/>
        <location filename="../MainWindow.cpp" line="354"/>
        <location filename="../MainWindow.cpp" line="421"/>
        <source>Check for Updates</source>
        <translation>檢查更新</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="374"/>
        <source>New version v%1 available. Open download page?</source>
        <translation>發現新版本 v%1。是否開啟下載頁面？</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="381"/>
        <source>Current: v%1, Latest: v%2
Choose update method:</source>
        <translation>目前版本：v%1，最新版本：v%2
請選擇更新方式：</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="398"/>
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
        <location filename="../MainWindow.cpp" line="373"/>
        <location filename="../MainWindow.cpp" line="380"/>
        <source>Update Available</source>
        <translation>發現新版本</translation>
    </message>
    <message>
        <source>No Updates</source>
        <translation type="vanished">沒有可用更新</translation>
    </message>
    <message>
        <source>Update Check Failed</source>
        <translation type="vanished">更新檢查失敗</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="398"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="382"/>
        <source>Update Main Program</source>
        <translation>更新主程式</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="383"/>
        <source>Download Full Package</source>
        <translation>下載完整安裝包</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="272"/>
        <location filename="../MainWindow.cpp" line="279"/>
        <location filename="../MainWindow.cpp" line="351"/>
        <location filename="../MainWindow.cpp" line="418"/>
        <location filename="../MainWindow.cpp" line="422"/>
        <location filename="../widgets/AboutDialog.cpp" line="24"/>
        <source>About</source>
        <translation>關於</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="303"/>
        <source>Theme: Auto</source>
        <translation>主題：自動</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="304"/>
        <source>Theme: Light</source>
        <translation>主題：淺色</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="304"/>
        <source>Theme: Dark</source>
        <translation>主題：深色</translation>
    </message>
    <message>
        <location filename="../widgets/AboutDialog.cpp" line="33"/>
        <source>Welcome to Modbus-Tools&lt;br&gt;Version: v%1&lt;br&gt;&lt;br&gt;An open-source Modbus communication debugging assistant.&lt;br&gt;Developer: mingyucheng692&lt;br&gt;License: MIT License&lt;br&gt;&lt;br&gt;This project is developed in spare time, completely free and open-source.&lt;br&gt;Feel free to star on GitHub or submit issues.&lt;br&gt;Your feedback keeps the project improving!&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 Visit GitHub Repository&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 Issue Tracker&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;This software is provided &amp;quot;as is&amp;quot; without warranty of any kind.</source>
        <translation>歡迎使用 Modbus-Tools&lt;br&gt;版本：v%1&lt;br&gt;&lt;br&gt;一款開源的 Modbus 通訊除錯助手。&lt;br&gt;開發者：mingyucheng692&lt;br&gt;授權條款：MIT License&lt;br&gt;&lt;br&gt;本專案由個人業餘開發，完全免費且開源。&lt;br&gt;歡迎在 GitHub 上 Star ⭐ 或提交 Issue。&lt;br&gt;您的回饋是專案持續改進的動力！&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 造訪 GitHub 倉庫&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 問題回報&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;本軟體按&amp;quot;原樣&amp;quot;提供，無任何形式的保證。</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="37"/>
        <source>Every Startup</source>
        <translation>每次啟動</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="39"/>
        <source>Weekly</source>
        <translation>每週</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="41"/>
        <source>Monthly</source>
        <translation>每月</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="43"/>
        <source>Disable Update Check</source>
        <translation>關閉更新檢查</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="45"/>
        <source>Update Check Frequency:</source>
        <translation>更新檢查頻率：</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::ModbusSessionPresenter</name>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="38"/>
        <source>Connecting to %1:%2...</source>
        <translation type="unfinished">正在連線 %1:%2...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="64"/>
        <source>Opening %1...</source>
        <translation type="unfinished">正在開啟 %1...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="99"/>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="328"/>
        <source>Disconnected</source>
        <translation type="unfinished">已斷線</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="228"/>
        <source>Failed to create Modbus stack</source>
        <translation type="unfinished">建立 Modbus 堆疊失敗</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="302"/>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="372"/>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="394"/>
        <source>Connected</source>
        <translation type="unfinished">已連線</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="380"/>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="402"/>
        <source>Connection failed: %1</source>
        <translation type="unfinished">連線失敗：%1</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::PollingController</name>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="101"/>
        <source>Error: %1</source>
        <translation type="unfinished">錯誤：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="138"/>
        <source>Poll recovered after %1 consecutive failures</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="183"/>
        <source>Poll Error: Connection unavailable during polling (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="186"/>
        <source>Poll Error escalated after %1 consecutive failures: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="190"/>
        <source>Poll Error persists (%1 consecutive failures): %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="206"/>
        <source>Poll escalated after %1 consecutive failures</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="209"/>
        <source>Poll Warning: %1 consecutive failure(s): %2</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::TrafficLogController</name>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="58"/>
        <source>Success: Response received</source>
        <translation type="unfinished">成功：已收到回應</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="64"/>
        <source>Success: Write confirmed</source>
        <translation type="unfinished">成功：寫入已確認</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="70"/>
        <source>Success: Broadcast write sent, no response expected</source>
        <translation type="unfinished">成功：廣播寫已發送，預期無回應</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="80"/>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation type="unfinished">發送讀取請求 FC:%1 位址:%2 數量:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="87"/>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation type="unfinished">發送寫入請求 FC:%1 位址:%2 資料:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="93"/>
        <source>Sending Raw Data: %1</source>
        <translation type="unfinished">發送原始資料：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="115"/>
        <source>%1 ms</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="116"/>
        <source>--</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="122"/>
        <source>Poll Summary FC:%1 Addr:%2 Qty:%3 Slave:%4 Success:%5 Error:%6 Retries:%7 Avg Success RTT:%8</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="145"/>
        <source>%1 after %2 %3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="156"/>
        <source>Error: %1</source>
        <translation type="unfinished">錯誤：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="161"/>
        <source>Error: %1 (failed after %2 %3, see log for details)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::common::UpdateChecker</name>
    <message>
        <location filename="../common/UpdateChecker.cpp" line="107"/>
        <source>Release tag is missing</source>
        <translation>發布標籤缺失</translation>
    </message>
</context>
<context>
    <name>ui::views::generic_serial::GenericSerialView</name>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="126"/>
        <source>File transfer started: %1 (%2 bytes)</source>
        <translation>檔案傳輸已開始：%1 (%2 位元組)</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="137"/>
        <source>File transfer progress: %1 %2/%3</source>
        <translation>檔案傳輸進度：%1 %2/%3</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="143"/>
        <source>File transfer finished: %1</source>
        <translation>檔案傳輸完成：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="146"/>
        <source>File transfer failed: %1 (%2)</source>
        <translation>檔案傳輸失敗：%1 (%2)</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="149"/>
        <source>File transfer canceled: %1</source>
        <translation>檔案傳輸已取消：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="200"/>
        <source>Opening %1...</source>
        <translation>正在開啟 %1...</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="248"/>
        <source>Closed</source>
        <translation>已關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="249"/>
        <source>Opening</source>
        <translation>正在開啟</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="250"/>
        <source>Open</source>
        <translation>已開啟</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="251"/>
        <source>Closing</source>
        <translation>正在關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="252"/>
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="253"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="256"/>
        <source>State changed: %1</source>
        <translation>狀態已變更：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="269"/>
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="302"/>
        <source>Control</source>
        <translation>控制</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="303"/>
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="304"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="305"/>
        <source>RTS</source>
        <translation>RTS</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="314"/>
        <source>Auto-reconnect exhausted (%1 attempts)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="324"/>
        <source>Auto-reconnect in %1ms (%2)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="351"/>
        <source>Reconnecting to %1...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::views::generic_tcp::GenericTcpView</name>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="115"/>
        <source>File transfer started: %1 (%2 bytes)</source>
        <translation>檔案傳輸已開始：%1 (%2 位元組)</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="126"/>
        <source>File transfer progress: %1 %2/%3</source>
        <translation>檔案傳輸進度：%1 %2/%3</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="132"/>
        <source>File transfer finished: %1</source>
        <translation>檔案傳輸完成：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="135"/>
        <source>File transfer failed: %1 (%2)</source>
        <translation>檔案傳輸失敗：%1 (%2)</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="138"/>
        <source>File transfer canceled: %1</source>
        <translation>檔案傳輸已取消：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="272"/>
        <source>Connecting to %1:%2...</source>
        <translation>正在連線 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="294"/>
        <source>Starting TCP server on %1:%2...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="306"/>
        <source>Stopping TCP server...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="316"/>
        <source>Binding UDP %1:%2 -&gt; %3:%4...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="320"/>
        <source>Binding UDP %1:%2...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="372"/>
        <source>Server mode: use client list to send data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="393"/>
        <source>Server mode: file transfer not supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="417"/>
        <source>Closed</source>
        <translation>已關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="418"/>
        <source>Opening</source>
        <translation>正在開啟</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="419"/>
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="420"/>
        <source>Closing</source>
        <translation>正在關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="421"/>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="473"/>
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="422"/>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="474"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="425"/>
        <source>State changed: %1</source>
        <translation>狀態已變更：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="445"/>
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="454"/>
        <source>Client #%1 connected: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="458"/>
        <source>Client #%1 disconnected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="471"/>
        <source>Listening</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="472"/>
        <source>Stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="477"/>
        <source>Server state: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="482"/>
        <source>Server Error: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="487"/>
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="496"/>
        <source>Auto-reconnect exhausted (%1 attempts)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="506"/>
        <source>Auto-reconnect in %1ms (%2)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="533"/>
        <source>Reconnecting to %1:%2...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::views::modbus_rtu::ModbusRtuPage</name>
    <message>
        <source>Opening %1...</source>
        <translation type="vanished">正在開啟 %1...</translation>
    </message>
    <message>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation type="vanished">發送讀取請求 FC:%1 位址:%2 數量:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="274"/>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="296"/>
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <source>Failed to create Modbus stack</source>
        <translation type="vanished">建立 Modbus 堆疊失敗</translation>
    </message>
    <message>
        <source>Connected</source>
        <translation type="vanished">已連線</translation>
    </message>
    <message>
        <source>Connection failed: %1</source>
        <translation type="vanished">連線失敗：%1</translation>
    </message>
    <message>
        <source>Success: Broadcast write sent, no response expected</source>
        <translation type="vanished">成功：廣播寫已發送，預期無回應</translation>
    </message>
    <message>
        <source>Success: Response received</source>
        <translation type="vanished">成功：已收到回應</translation>
    </message>
    <message>
        <source>Disconnected</source>
        <translation type="vanished">已斷線</translation>
    </message>
    <message>
        <source>Unsupported function code</source>
        <translation type="vanished">不支援的功能碼</translation>
    </message>
    <message>
        <source>Error: Invalid decimal value for 0x05</source>
        <translation type="vanished">錯誤：0x05 十進位值無效</translation>
    </message>
    <message>
        <source>Error: Invalid hex value for 0x05</source>
        <translation type="vanished">錯誤：0x05 十六進位值無效</translation>
    </message>
    <message>
        <source>Error: Empty value for 0x06</source>
        <translation type="vanished">錯誤：0x06 值為空</translation>
    </message>
    <message>
        <source>Error: Invalid decimal value for 0x06</source>
        <translation type="vanished">錯誤：0x06 十進位值無效</translation>
    </message>
    <message>
        <source>Error: Invalid hex value for 0x06</source>
        <translation type="vanished">錯誤：0x06 十六進位值無效</translation>
    </message>
    <message>
        <source>Error: Binary bit count (%1) does not match Quantity (%2)</source>
        <translation type="vanished">錯誤：二進位位元數 (%1) 與數量 (%2) 不相符</translation>
    </message>
    <message>
        <source>Error: Binary format not supported for registers (0x06)</source>
        <translation type="vanished">錯誤：暫存器操作 (0x06) 不支援二進位格式</translation>
    </message>
    <message>
        <source>Error: Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation type="vanished">錯誤：0x05 二進位值無效（預期為 0 或 1）</translation>
    </message>
    <message>
        <source>Error: Invalid quantity for 0x0F</source>
        <translation type="vanished">錯誤：0x0F 數量無效</translation>
    </message>
    <message>
        <source>Error: Empty value for 0x10</source>
        <translation type="vanished">錯誤：0x10 值為空</translation>
    </message>
    <message>
        <source>Error: Invalid quantity for 0x10</source>
        <translation type="vanished">錯誤：0x10 數量無效</translation>
    </message>
    <message>
        <source>Error: Invalid decimal list for 0x10</source>
        <translation type="vanished">錯誤：0x10 十進位清單無效</translation>
    </message>
    <message>
        <source>Error: Invalid hex value for 0x10</source>
        <translation type="vanished">錯誤：0x10 十六進位值無效</translation>
    </message>
    <message>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation type="vanished">發送寫入請求 FC:%1 位址:%2 資料:%3 從站:%4</translation>
    </message>
    <message>
        <source>Success: Write confirmed</source>
        <translation type="vanished">成功：寫入已確認</translation>
    </message>
    <message>
        <source>Sending Raw Data: %1</source>
        <translation type="vanished">發送原始資料：%1</translation>
    </message>
    <message>
        <source>Poll Error: %1</source>
        <translation type="vanished">輪詢錯誤：%1</translation>
    </message>
    <message>
        <source>Error: 0x0F requires Hex or Binary data</source>
        <translation type="vanished">錯誤：0x0F 需要十六進位或二進位資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="262"/>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="288"/>
        <source>Error: Request service not available</source>
        <translation>錯誤：請求服務不可用</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="379"/>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="391"/>
        <source>[%1] %2</source>
        <translation>[%1] %2</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="380"/>
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="392"/>
        <source>TX</source>
        <translation>傳送</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="397"/>
        <source>Data Monitor</source>
        <translation>資料監視</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="398"/>
        <source>Receive Data</source>
        <translation>接收資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="399"/>
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="400"/>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="401"/>
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="402"/>
        <location filename="../views/modbus_rtu/ModbusRtuPage.cpp" line="403"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus_tcp::ModbusTcpPage</name>
    <message>
        <source>Connecting to %1:%2...</source>
        <translation type="vanished">正在連線 %1:%2...</translation>
    </message>
    <message>
        <source>Connected</source>
        <translation type="vanished">已連線</translation>
    </message>
    <message>
        <source>Disconnected</source>
        <translation type="vanished">已斷線</translation>
    </message>
    <message>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation type="vanished">發送讀取請求 FC:%1 位址:%2 數量:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="270"/>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="292"/>
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <source>Failed to create Modbus stack</source>
        <translation type="vanished">建立 Modbus 堆疊失敗</translation>
    </message>
    <message>
        <source>Connection failed: %1</source>
        <translation type="vanished">連線失敗：%1</translation>
    </message>
    <message>
        <source>Success: Response received</source>
        <translation type="vanished">成功：已收到回應</translation>
    </message>
    <message>
        <source>Unsupported function code</source>
        <translation type="vanished">不支援的功能碼</translation>
    </message>
    <message>
        <source>Error: Invalid decimal value for 0x05</source>
        <translation type="vanished">錯誤：0x05 十進位值無效</translation>
    </message>
    <message>
        <source>Error: Invalid hex value for 0x05</source>
        <translation type="vanished">錯誤：0x05 十六進位值無效</translation>
    </message>
    <message>
        <source>Error: Empty value for 0x06</source>
        <translation type="vanished">錯誤：0x06 值為空</translation>
    </message>
    <message>
        <source>Error: Invalid decimal value for 0x06</source>
        <translation type="vanished">錯誤：0x06 十進位值無效</translation>
    </message>
    <message>
        <source>Error: Invalid hex value for 0x06</source>
        <translation type="vanished">錯誤：0x06 十六進位值無效</translation>
    </message>
    <message>
        <source>Error: Binary format not supported for registers (0x06)</source>
        <translation type="vanished">錯誤：暫存器操作 (0x06) 不支援二進位格式</translation>
    </message>
    <message>
        <source>Error: Binary bit count (%1) does not match Quantity (%2)</source>
        <translation type="vanished">錯誤：二進位位元數 (%1) 與數量 (%2) 不相符</translation>
    </message>
    <message>
        <source>Error: Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation type="vanished">錯誤：0x05 二進位值無效（預期為 0 或 1）</translation>
    </message>
    <message>
        <source>Error: Invalid quantity for 0x0F</source>
        <translation type="vanished">錯誤：0x0F 數量無效</translation>
    </message>
    <message>
        <source>Error: Empty value for 0x10</source>
        <translation type="vanished">錯誤：0x10 值為空</translation>
    </message>
    <message>
        <source>Error: Invalid quantity for 0x10</source>
        <translation type="vanished">錯誤：0x10 數量無效</translation>
    </message>
    <message>
        <source>Error: Invalid decimal list for 0x10</source>
        <translation type="vanished">錯誤：0x10 十進位清單無效</translation>
    </message>
    <message>
        <source>Error: Invalid hex value for 0x10</source>
        <translation type="vanished">錯誤：0x10 十六進位值無效</translation>
    </message>
    <message>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation type="vanished">發送寫入請求 FC:%1 位址:%2 資料:%3 從站:%4</translation>
    </message>
    <message>
        <source>Success: Write confirmed</source>
        <translation type="vanished">成功：寫入已確認</translation>
    </message>
    <message>
        <source>Sending Raw Data: %1</source>
        <translation type="vanished">發送原始資料：%1</translation>
    </message>
    <message>
        <source>Poll Error: %1</source>
        <translation type="vanished">輪詢錯誤：%1</translation>
    </message>
    <message>
        <source>Error: 0x0F requires Hex or Binary data</source>
        <translation type="vanished">錯誤：0x0F 需要十六進位或二進位資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="258"/>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="284"/>
        <source>Error: Request service not available</source>
        <translation>錯誤：請求服務不可用</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="375"/>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="387"/>
        <source>[%1] %2</source>
        <translation>[%1] %2</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="376"/>
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="388"/>
        <source>TX</source>
        <translation>傳送</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="393"/>
        <source>Data Monitor</source>
        <translation>資料監視</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="394"/>
        <source>Receive Data</source>
        <translation>接收資料</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="395"/>
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="396"/>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="397"/>
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="398"/>
        <location filename="../views/modbus_tcp/ModbusTcpPage.cpp" line="399"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
</context>
<context>
    <name>ui::widgets::ByteMonitorWidget</name>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="185"/>
        <source>Copy</source>
        <translation type="unfinished">複製</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="248"/>
        <source>[%1] [TX] %2</source>
        <translation type="unfinished">[%1] [TX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="257"/>
        <source>[%1] [RX] %2</source>
        <translation type="unfinished">[%1] [RX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="263"/>
        <source>[INFO] %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="264"/>
        <source>[%1] [INFO] %2</source>
        <translation type="unfinished">[%1] [資訊] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="270"/>
        <source>[ERROR] %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="271"/>
        <source>[%1] [ERROR] %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="277"/>
        <source>[WARN] %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="278"/>
        <source>[%1] [WARN] %2</source>
        <translation type="unfinished">[%1] [警告] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="305"/>
        <source>Record Log</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="306"/>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="348"/>
        <source>Text Files (*.txt);;All Files (*)</source>
        <translation type="unfinished">文字檔 (*.txt);;所有檔案 (*)</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="313"/>
        <source>Failed to start recording: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="319"/>
        <source>Stop</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="347"/>
        <source>Save Log</source>
        <translation type="unfinished">儲存日誌</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="355"/>
        <source>Cannot write file: %1</source>
        <translation type="unfinished">無法寫入檔案：%1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="454"/>
        <source>TX: %1 (%2 pkts)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="459"/>
        <source>RX: %1 (%2 pkts)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="464"/>
        <source>Rate: %1 / %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="563"/>
        <source>HEX</source>
        <translation type="unfinished">HEX</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="566"/>
        <source>ASCII</source>
        <translation type="unfinished">ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="572"/>
        <source>Absolute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="573"/>
        <source>Relative</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="574"/>
        <source>None</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="577"/>
        <source>TX</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="578"/>
        <source>RX</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="579"/>
        <source>Auto Scroll</source>
        <translation type="unfinished">自動捲動</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="581"/>
        <source>Resume</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="581"/>
        <source>Pause</source>
        <translation type="unfinished">暫停</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="583"/>
        <source>Clear</source>
        <translation type="unfinished">清除</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="584"/>
        <source>Save</source>
        <translation type="unfinished">儲存</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="586"/>
        <source>Record</source>
        <translation type="unfinished"></translation>
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
        <location filename="../widgets/ControlWidget.cpp" line="112"/>
        <source>Invalid Address format or range (0-65535): %1</source>
        <translation>無效的位址格式或範圍 (0-65535): %1</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="124"/>
        <source>Confirm Address</source>
        <translation>確認位址</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="125"/>
        <source>The polling address is set to 0. Are you sure you want to continue?</source>
        <translation>輪詢位址設置為 0。確定要繼續嗎？</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="129"/>
        <source>Do not show this again</source>
        <translation>不再顯示</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="156"/>
        <source>%1 ms</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="157"/>
        <source>--</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="348"/>
        <source>Enable Polling</source>
        <translation>啟用輪詢</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="354"/>
        <source>Interval(ms):</source>
        <translation>間隔(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="357"/>
        <source>FC:</source>
        <translation>功能碼：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="360"/>
        <source>01-Read Coils</source>
        <translation>01-讀取線圈</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="361"/>
        <source>02-Read Discrete</source>
        <translation>02-讀取離散輸入</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="362"/>
        <source>03-Read Holding</source>
        <translation>03-讀取保持暫存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="363"/>
        <source>04-Read Input</source>
        <translation>04-讀取輸入暫存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="366"/>
        <source>Addr:</source>
        <translation>位址：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="369"/>
        <source>Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>位址 (0-65535)。支援 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="372"/>
        <source>Qty:</source>
        <translation>數量：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="389"/>
        <source>Last Success RTT</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="351"/>
        <source>Link to Analyzer</source>
        <translation>聯動分析儀</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FrameAnalyzerWidget</name>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="302"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1063"/>
        <source>Frame Input</source>
        <translation>報文輸入</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="308"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1064"/>
        <source>Protocol:</source>
        <translation>通訊協定：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="311"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1066"/>
        <source>Auto Detect</source>
        <translation>自動偵測</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="312"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1067"/>
        <source>Modbus TCP</source>
        <translation>Modbus TCP</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="313"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1068"/>
        <source>Modbus RTU</source>
        <translation>Modbus RTU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="317"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1070"/>
        <source>Start Address (for Response):</source>
        <translation>起始位址（用於回應）：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="331"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1086"/>
        <source>Format Hex</source>
        <translation>格式化 Hex</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="336"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1109"/>
        <source>Parse</source>
        <translation>解析</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="341"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1110"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="351"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1112"/>
        <source>Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)</source>
        <translation>輸入十六進位字串（如 01 03 00 00 00 01 84 0A）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="362"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1115"/>
        <source>Analysis Result</source>
        <translation>分析結果</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="378"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1097"/>
        <source>Status:</source>
        <translation>狀態：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="382"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="932"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1106"/>
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
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="944"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1003"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1095"/>
        <source>Pause Refresh</source>
        <translation>暫停重新整理</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1003"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1095"/>
        <source>Resume Refresh</source>
        <translation>恢復重新整理</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="416"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1093"/>
        <source>Stop Link</source>
        <translation>停止連動</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="428"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1074"/>
        <source>Decode Mode:</source>
        <translation>解碼模式：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="430"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1076"/>
        <source>Unsigned</source>
        <translation>無符號</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="431"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1077"/>
        <source>Signed</source>
        <translation>有符號</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="465"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="660"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1087"/>
        <source>Import Config</source>
        <translation>匯入設定</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="467"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="643"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1088"/>
        <source>Export Config</source>
        <translation>匯出設定</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="469"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="691"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1089"/>
        <source>Export CSV</source>
        <translation>匯出 CSV</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="493"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1123"/>
        <source>Field</source>
        <translation>欄位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="493"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="501"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1124"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1128"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="493"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="501"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1125"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1128"/>
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="497"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="777"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="953"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1118"/>
        <source>Structure</source>
        <translation>結構</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="501"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1128"/>
        <source>Address</source>
        <translation>位址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="501"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1128"/>
        <source>Hex</source>
        <translation>十六進位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="501"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1128"/>
        <source>Decimal</source>
        <translation>十進位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="501"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1128"/>
        <source>Binary</source>
        <translation>二進位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="501"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1128"/>
        <source>Scale</source>
        <translation>倍率</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="687"/>
        <source>There is no data to export.</source>
        <translation>沒有可導出的資料。</translation>
    </message>
    <message>
        <source>Raw Hex</source>
        <translation type="vanished">原始十六進位</translation>
    </message>
    <message>
        <source>Valid</source>
        <translation type="vanished">有效</translation>
    </message>
    <message>
        <source>Invalid (Expected 0x%1)</source>
        <translation type="vanished">無效 (預期 0x%1)</translation>
    </message>
    <message>
        <source>Success</source>
        <translation type="vanished">成功</translation>
    </message>
    <message>
        <source>Success with warnings</source>
        <translation type="vanished">成功（帶警告）</translation>
    </message>
    <message>
        <source>Structure (Live Mode)</source>
        <translation type="vanished">結構（連動模式）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1072"/>
        <source>Start Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>起始位址 (0-65535)。支援 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="532"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1116"/>
        <source>History</source>
        <translation>歷史記錄</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="536"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1130"/>
        <source>Clear History</source>
        <translation>清除歷史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="643"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="660"/>
        <source>JSON Files (*.json)</source>
        <translation>JSON 檔案 (*.json)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="687"/>
        <source>No Data</source>
        <translation>無資料</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="693"/>
        <source>CSV Files (*.csv)</source>
        <translation>CSV 檔案 (*.csv)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="653"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="713"/>
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
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="527"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1119"/>
        <source>Decoded Data</source>
        <translation>解析資料</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="591"/>
        <source>Invalid Address (0-65535): %1</source>
        <translation>無效位址 (0-65535): %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="620"/>
        <source>Parse Failed</source>
        <translation>解析失敗</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="665"/>
        <source>Import Failed</source>
        <translation>匯入失敗</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="976"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1100"/>
        <source>LIVE: %1</source>
        <translation>連動中: %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="980"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1102"/>
        <source>Live Data Received at %1</source>
        <translation>即時資料接收於 %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="582"/>
        <source>Error: Empty input</source>
        <translation>錯誤：輸入為空</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="598"/>
        <source>Parsing...</source>
        <translation>解析中...</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="732"/>
        <source>Parse Failed: %1</source>
        <translation>解析失敗：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="739"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1099"/>
        <source>TCP</source>
        <translation>TCP</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="740"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1099"/>
        <source>RTU</source>
        <translation>RTU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="783"/>
        <source>Frame</source>
        <translation>幀</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="784"/>
        <source>%1 bytes</source>
        <translation>%1 位元組</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="792"/>
        <source>MBAP Header</source>
        <translation>MBAP 標頭</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="797"/>
        <source>Transaction ID</source>
        <translation>交易識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="802"/>
        <source>Protocol ID</source>
        <translation>協定識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="807"/>
        <source>Length</source>
        <translation>長度</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="812"/>
        <source>Unit ID</source>
        <translation>單元識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="818"/>
        <source>Slave ID</source>
        <translation>從站識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="832"/>
        <source>PDU</source>
        <translation>PDU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="838"/>
        <source>Function Code</source>
        <translation>功能碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="850"/>
        <source>Exception Code</source>
        <translation>例外碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="865"/>
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
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="444"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1079"/>
        <source>Byte Order:</source>
        <translation>位元組順序：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="446"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1081"/>
        <source>ABCD(default)</source>
        <translation>ABCD(預設)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="778"/>
        <source>(Unavailable in Live Mode)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="779"/>
        <source>Logical parsing is disabled for high-frequency linkage</source>
        <translation>高頻聯動下已停用邏輯解析</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="787"/>
        <source>Frame Bytes</source>
        <translation>幀位元組</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="787"/>
        <source>Complete raw frame</source>
        <translation>完整原始幀</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="794"/>
        <source>Transaction + Protocol + Length + Unit ID</source>
        <translation>交易 + 協定 + 長度 + 單元識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="799"/>
        <source>Request/response correlation ID</source>
        <translation>請求/回應關聯識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="804"/>
        <source>Modbus TCP protocol identifier</source>
        <translation>Modbus TCP 協定識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="809"/>
        <source>Remaining bytes after this field</source>
        <translation>此欄位之後的剩餘位元組數</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="814"/>
        <source>Target slave / unit address</source>
        <translation>目標從站 / 單元位址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="820"/>
        <source>Target slave address</source>
        <translation>目標從站位址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="834"/>
        <source>Function code + payload</source>
        <translation>功能碼 + 負載</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="840"/>
        <source>Normal response</source>
        <translation>正常回應</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="840"/>
        <source>Exception response</source>
        <translation>例外回應</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="843"/>
        <source>Payload</source>
        <translation>負載</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="845"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="852"/>
        <source>Exception detail payload</source>
        <translation>例外詳細負載</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="845"/>
        <source>Application data payload</source>
        <translation>應用資料負載</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="856"/>
        <source>CRC valid</source>
        <translation>CRC 有效</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="856"/>
        <source>CRC invalid</source>
        <translation>CRC 無效</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="860"/>
        <source>Expected 0x%1</source>
        <translation>預期 0x%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="915"/>
        <source>Success (%1)</source>
        <translation>成功（%1）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="917"/>
        <source>Forced Parsing</source>
        <translation>強制解析</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="920"/>
        <source>Warnings</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="960"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1118"/>
        <source>Structure (Unavailable in Live Mode)</source>
        <translation>結構（聯動模式不可用）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="743"/>
        <source>Request</source>
        <translation>請求</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="744"/>
        <source>Response</source>
        <translation>回應</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="745"/>
        <source>Exception</source>
        <translation>例外</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="741"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="746"/>
        <source>Unknown</source>
        <translation>未知</translation>
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
        <location filename="../widgets/GenericInputWidget.cpp" line="301"/>
        <source>Select File to Send</source>
        <translation>選擇要發送的檔案</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="361"/>
        <source>Enter data to send...</source>
        <translation>輸入要發送的資料...</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="364"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="367"/>
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="370"/>
        <source>Auto Send</source>
        <translation>自動發送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="373"/>
        <source> ms</source>
        <translation> 毫秒</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="376"/>
        <source>Send File</source>
        <translation>發送檔案</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="379"/>
        <source>Send</source>
        <translation>發送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="67"/>
        <location filename="../widgets/GenericInputWidget.cpp" line="385"/>
        <source>No LF</source>
        <translation>無換行</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="68"/>
        <location filename="../widgets/GenericInputWidget.cpp" line="386"/>
        <source>CR (\r)</source>
        <translation>CR (\r)</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="69"/>
        <location filename="../widgets/GenericInputWidget.cpp" line="387"/>
        <source>LF (\n)</source>
        <translation>LF (\n)</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="70"/>
        <location filename="../widgets/GenericInputWidget.cpp" line="388"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r\n)</translation>
    </message>
</context>
<context>
    <name>ui::widgets::SerialConnectionWidget</name>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="39"/>
        <source>None</source>
        <translation>無</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="40"/>
        <source>Even</source>
        <translation>偶校驗</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="41"/>
        <source>Odd</source>
        <translation>奇校驗</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="42"/>
        <source>Space</source>
        <translation>空白校驗</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="43"/>
        <source>Mark</source>
        <translation>標記校驗</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="44"/>
        <source>RTS/CTS</source>
        <translation>RTS/CTS</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="45"/>
        <source>XON/XOFF</source>
        <translation>XON/XOFF</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="137"/>
        <source>Disconnect</source>
        <translation>中斷連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="138"/>
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="141"/>
        <source>Connect</source>
        <translation>連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="142"/>
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="201"/>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="428"/>
        <source>Refresh Ports</source>
        <translation>重新整理埠</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="405"/>
        <source>Connection Settings</source>
        <translation>連線設定</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="408"/>
        <source>Port:</source>
        <translation>埠：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="411"/>
        <source>Baud:</source>
        <translation>鮑特率：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="414"/>
        <source>Data:</source>
        <translation>資料位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="417"/>
        <source>Parity:</source>
        <translation>同位元：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="421"/>
        <source>Stop:</source>
        <translation>停止位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="424"/>
        <source>Flow:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="431"/>
        <source>Auto Reconnect</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::widgets::TcpConnectionWidget</name>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="317"/>
        <source>Disconnect</source>
        <translation>中斷連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="318"/>
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="338"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="361"/>
        <source>Connect</source>
        <translation>連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="341"/>
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="370"/>
        <source>Listen:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="381"/>
        <source>Local:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="396"/>
        <source>Connection Settings</source>
        <translation>連線設定</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="359"/>
        <source>Host:</source>
        <translation>主機：</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="31"/>
        <source>TCP Client</source>
        <translation>TCP 用戶端</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="32"/>
        <source>TCP Server</source>
        <translation>TCP 伺服端</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="33"/>
        <source>UDP</source>
        <translation>UDP</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="309"/>
        <source>Stop</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="310"/>
        <source>Listening</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="313"/>
        <source>Unbind</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="314"/>
        <source>Bound</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="332"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="372"/>
        <source>Start Listen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="335"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="383"/>
        <source>Bind</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="360"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="371"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="382"/>
        <source>Port:</source>
        <translation>埠：</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="400"/>
        <source>Remote:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="403"/>
        <source>Remote Port:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="406"/>
        <source>Auto Reconnect</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::widgets::TrafficMonitorWidget</name>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="185"/>
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="332"/>
        <source>[%1] [TX] %2</source>
        <translation>[%1] [TX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="343"/>
        <source>[%1] [RX] %2</source>
        <translation>[%1] [RX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="358"/>
        <source>[%1] [INFO] %2</source>
        <translation>[%1] [資訊] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="348"/>
        <source>[%1] [WARN] %2</source>
        <translation>[%1] [警告] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="260"/>
        <source>Raw Frames may affect UI fluency</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="269"/>
        <source>Paused +%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="353"/>
        <source>[%1] [ERROR] %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="480"/>
        <source>Save Log</source>
        <translation>儲存日誌</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="480"/>
        <source>Text Files (*.txt);;All Files (*)</source>
        <translation>文字檔 (*.txt);;所有檔案 (*)</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="505"/>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="527"/>
        <source>Save failed: %1</source>
        <translation>儲存失敗：%1</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="634"/>
        <source>Traffic Monitor</source>
        <translation>通訊監視</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="635"/>
        <source>Auto Scroll</source>
        <translation>自動捲動</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="636"/>
        <source>Pause</source>
        <translation>暫停</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="637"/>
        <source>Raw Frames</source>
        <translation>原始幀</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="642"/>
        <source>All</source>
        <translation>全部</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="643"/>
        <source>Info</source>
        <translation>資訊</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="644"/>
        <source>Warn</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="645"/>
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="647"/>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="648"/>
        <source>Log Level Filter</source>
        <translation>日誌等級篩選</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="650"/>
        <source>TX</source>
        <translation>傳送</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="651"/>
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <source>Raw Frames mode may reduce UI smoothness under high-frequency polling.</source>
        <translation type="vanished">高頻輪詢下，原始幀模式可能降低介面流暢度。</translation>
    </message>
    <message>
        <source>Paused (%1 new)</source>
        <translation type="vanished">已暫停（新增 %1）</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="654"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="655"/>
        <source>Save</source>
        <translation>儲存</translation>
    </message>
</context>
</TS>
