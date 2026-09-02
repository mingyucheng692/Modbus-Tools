<?xml version='1.0' encoding='utf-8'?>
<TS version="2.1" language="zh_TW">
<context>
    <name>ConnectionAlert</name>
    <message>
        <location filename="../common/ConnectionAlert.h" line="41" />
        <source>Not Connected</source>
        <translation>未連線</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="42" />
        <source>Please connect first.</source>
        <translation>請先連線設備</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="51" />
        <source>Connection Lost</source>
        <translation>連線中斷</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="52" />
        <source>Connection was closed.</source>
        <translation>連線已關閉。</translation>
    </message>
</context>
<context>
    <name>FrameParseWorker</name>
    <message>
        <location filename="../../core/modbus/parser/FrameParseWorker.cpp" line="54" />
        <source>Error: Empty input</source>
        <translation>錯誤：輸入為空</translation>
    </message>
</context>
<context>
    <name>ModbusFrameParser</name>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="40" />
        <source>Illegal Function</source>
        <translation>Illegal Function</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="42" />
        <source>Illegal Data Address</source>
        <translation>Illegal Data Address</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="44" />
        <source>Illegal Data Value</source>
        <translation>Illegal Data Value</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="46" />
        <source>Server Device Failure</source>
        <translation>Server Device Failure</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="48" />
        <source>Acknowledge</source>
        <translation>Acknowledge</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="50" />
        <source>Server Device Busy</source>
        <translation>Server Device Busy</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="52" />
        <source>Negative Acknowledge</source>
        <translation>Negative Acknowledge</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="54" />
        <source>Memory Parity Error</source>
        <translation>Memory Parity Error</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="56" />
        <source>Gateway Path Unavailable</source>
        <translation>Gateway Path Unavailable</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="58" />
        <source>Gateway Target Device Failed To Respond</source>
        <translation>Gateway Target Device Failed To Respond</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="60" />
        <source>Unknown Exception</source>
        <translation>Unknown Exception</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="140" />
        <source>Frame too short for Modbus TCP. Expected a complete MBAP + PDU, got %1 bytes</source>
        <translation>Frame too short for Modbus TCP. Expected a complete MBAP + PDU, got %1 bytes</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="149" />
        <source>Invalid TCP MBAP header or length</source>
        <translation>Invalid TCP MBAP header or length</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="155" />
        <source>TCP frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>TCP frame contains trailing bytes. Expected %1 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="163" />
        <source>Frame too short for TCP MBAP</source>
        <translation>Frame too short for TCP MBAP</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="167" />
        <source>Warning: Invalid TCP MBAP header or length (Forced)</source>
        <translation>Warning: Invalid TCP MBAP header or length (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="170" />
        <source>Warning: TCP frame contains trailing bytes (Forced)</source>
        <translation>Warning: TCP frame contains trailing bytes (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="183" />
        <source>Invalid TCP PDU length. MBAP length field is %1, so PDU length is %2</source>
        <translation>Invalid TCP PDU length. MBAP length field is %1, so PDU length is %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="210" />
        <source>Frame too short for Modbus RTU. Expected at least 5 bytes, got %1</source>
        <translation>Frame too short for Modbus RTU. Expected at least 5 bytes, got %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="226" />
        <source>CRC Mismatch. Expected %1, Got %2</source>
        <translation>CRC Mismatch. Expected %1, Got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="234" />
        <source>RTU frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>RTU frame contains trailing bytes. Expected %1 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="242" />
        <source>Frame too short for RTU</source>
        <translation>Frame too short for RTU</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="246" />
        <source>Warning: CRC Mismatch. Expected %1, Got %2 (Forced)</source>
        <translation>Warning: CRC Mismatch. Expected %1, Got %2 (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="252" />
        <source>Warning: RTU frame contains trailing bytes (Forced)</source>
        <translation>Warning: RTU frame contains trailing bytes (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="283" />
        <source>Frame incomplete for Modbus ASCII. Missing CRLF terminator or full payload</source>
        <translation>Frame incomplete for Modbus ASCII. Missing CRLF terminator or full payload</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="291" />
        <source>Invalid Modbus ASCII frame or LRC mismatch</source>
        <translation>Invalid Modbus ASCII frame or LRC mismatch</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="299" />
        <source>ASCII frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>ASCII frame contains trailing bytes. Expected %1 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="307" />
        <source>Warning: Invalid Modbus ASCII frame or LRC mismatch (Forced)</source>
        <translation>Warning: Invalid Modbus ASCII frame or LRC mismatch (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="339" />
        <source>Empty frame data</source>
        <translation>Empty frame data</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="353" />
        <source>Unable to identify protocol. Frame length: %1 bytes, data: %2</source>
        <translation>Unable to identify protocol. Frame length: %1 bytes, data: %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="390" />
        <source>Empty PDU. Function code is missing from the frame</source>
        <translation>Empty PDU. Function code is missing from the frame</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="404" />
        <source>Exception PDU too short for function 0x%1. Expected 2 bytes, got %2</source>
        <translation>Exception PDU too short for function 0x%1. Expected 2 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="413" />
        <source>Modbus exception: %1 (code %2)</source>
        <translation>Modbus exception: %1 (code %2)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="451" />
        <source>Response PDU too short for function 0x%1. Expected at least 2 bytes, got %2</source>
        <translation>Response PDU too short for function 0x%1. Expected at least 2 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="463" />
        <source>Byte count mismatch for function 0x%1. Declared %2, actual %3</source>
        <translation>Byte count mismatch for function 0x%1. Declared %2, actual %3</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="477" />
        <source>Register response byte count does not match expected quantity. Declared %1, expected %2</source>
        <translation>Register response byte count does not match expected quantity. Declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="486" />
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="620" />
        <source>Register byte count must be even, got %1</source>
        <translation>Register byte count must be even, got %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="518" />
        <source>Coil response bit count does not match expected quantity. Byte count %1 cannot represent %2 bits</source>
        <translation>Coil response bit count does not match expected quantity. Byte count %1 cannot represent %2 bits</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="553" />
        <source>Write single PDU length mismatch for function 0x%1. Expected 4 bytes, got %2</source>
        <translation>Write single PDU length mismatch for function 0x%1. Expected 4 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="600" />
        <source>Write request byte count mismatch. Declared %1, actual %2</source>
        <translation>Write request byte count mismatch. Declared %1, actual %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="611" />
        <source>Register write byte count does not match quantity. Declared %1, expected %2</source>
        <translation>Register write byte count does not match quantity. Declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="645" />
        <source>Coil write byte count does not match quantity. Declared %1, expected %2</source>
        <translation>Coil write byte count does not match quantity. Declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="677" />
        <source>Unsupported function code 0x%1 for deep parsing</source>
        <translation>Unsupported function code 0x%1 for deep parsing</translation>
    </message>
</context>
<context>
    <name>ModbusPduBuilder</name>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="19" />
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="80" />
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="96" />
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="112" />
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="148" />
        <source>Invalid start address</source>
        <translation>Invalid start address</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="23" />
        <source>Invalid quantity</source>
        <translation>Invalid quantity</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="72" />
        <source>Unsupported function code</source>
        <translation>Unsupported function code</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="116" />
        <source>Invalid quantity for 0x0F</source>
        <translation>0x0F 數量無效</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="122" />
        <source>Data length mismatch for 0x0F</source>
        <translation>Data length mismatch for 0x0F</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="152" />
        <source>Invalid quantity for 0x10</source>
        <translation>0x10 數量無效</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="157" />
        <source>Data length mismatch for 0x10</source>
        <translation>Data length mismatch for 0x10</translation>
    </message>
</context>
<context>
    <name>ModbusProtocolChecks</name>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="25" />
        <source>Exception</source>
        <translation>例外</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="26" />
        <source>Bit read</source>
        <translation>Bit read</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="27" />
        <source>Register read</source>
        <translation>Register read</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="28" />
        <source>Write single</source>
        <translation>Write single</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="29" />
        <source>Write multiple</source>
        <translation>Write multiple</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="30" />
        <source>%1 response payload length mismatch: expected %2, got %3</source>
        <translation>%1 response payload length mismatch: expected %2, got %3</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="31" />
        <source>Exception function code does not match request</source>
        <translation>Exception function code does not match request</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="32" />
        <source>Response function code does not match request</source>
        <translation>Response function code does not match request</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="33" />
        <source>Request quantity missing for bit-read validation</source>
        <translation>Request quantity missing for bit-read validation</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="34" />
        <source>Bit-read response byte count does not match payload length</source>
        <translation>Bit-read response byte count does not match payload length</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="35" />
        <source>Bit-read response byte count does not match requested quantity</source>
        <translation>Bit-read response byte count does not match requested quantity</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="36" />
        <source>Request quantity missing for register-read validation</source>
        <translation>Request quantity missing for register-read validation</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="37" />
        <source>Register-read response byte count does not match payload length</source>
        <translation>Register-read response byte count does not match payload length</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="38" />
        <source>Register-read response byte count does not match requested quantity</source>
        <translation>Register-read response byte count does not match requested quantity</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="39" />
        <source>Register-read response byte count must be even</source>
        <translation>Register-read response byte count must be even</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="40" />
        <source>Write-single response echo does not match request</source>
        <translation>Write-single response echo does not match request</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="41" />
        <source>Request echo fields missing for write-multiple validation</source>
        <translation>Request echo fields missing for write-multiple validation</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="42" />
        <source>Write-multiple response echo fields are incomplete</source>
        <translation>Write-multiple response echo fields are incomplete</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="43" />
        <source>Write-multiple response echo does not match request</source>
        <translation>Write-multiple response echo does not match request</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="44" />
        <source>Unsupported function code for response validation</source>
        <translation>Unsupported function code for response validation</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="66" />
        <source>Missing or invalid expected checksum</source>
        <translation>Missing or invalid expected checksum</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="70" />
        <location filename="../../core/update/UpdateManager.cpp" line="83" />
        <source>Canceled</source>
        <translation>Canceled</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="76" />
        <source>Failed to open file for checksum calculation</source>
        <translation>Failed to open file for checksum calculation</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="87" />
        <source>Failed to read file chunk</source>
        <translation>Failed to read file chunk</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="94" />
        <source>Failed to calculate file checksum</source>
        <translation>Failed to calculate file checksum</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="99" />
        <source>Checksum mismatch. Expected: %1, Actual: %2</source>
        <translation>Checksum mismatch. Expected: %1, Actual: %2</translation>
    </message>
</context>
<context>
    <name>RequestSubmissionService</name>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="63" />
        <source>Invalid decimal value for 0x05</source>
        <translation>Invalid decimal value for 0x05</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="73" />
        <source>Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation>Invalid binary value for 0x05 (expected 0 or 1)</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="82" />
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="90" />
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="99" />
        <source>Invalid hex value for 0x05</source>
        <translation>Invalid hex value for 0x05</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="112" />
        <source>Empty value for 0x06</source>
        <translation>Empty value for 0x06</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="121" />
        <source>Invalid decimal value for 0x06</source>
        <translation>Invalid decimal value for 0x06</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="129" />
        <source>Binary format not supported for registers (0x06)</source>
        <translation>Binary format not supported for registers (0x06)</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="136" />
        <source>Invalid hex value for 0x06</source>
        <translation>Invalid hex value for 0x06</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="145" />
        <source>Invalid quantity for 0x0F</source>
        <translation>0x0F 數量無效</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="155" />
        <source>Binary bit count (%1) does not match Quantity (%2)</source>
        <translation>Binary bit count (%1) does not match Quantity (%2)</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="165" />
        <source>0x0F requires Hex or Binary data</source>
        <translation>0x0F requires Hex or Binary data</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="172" />
        <source>Empty value for 0x10</source>
        <translation>Empty value for 0x10</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="178" />
        <source>Invalid quantity for 0x10</source>
        <translation>0x10 數量無效</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="187" />
        <source>Invalid decimal list for 0x10</source>
        <translation>Invalid decimal list for 0x10</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="195" />
        <source>Invalid hex value for 0x10</source>
        <translation>Invalid hex value for 0x10</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="227" />
        <source>Raw data is empty</source>
        <translation>Raw data is empty</translation>
    </message>
</context>
<context>
    <name>core::update::UpdateManager</name>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="101" />
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="168" />
        <source>Incomplete update task parameters</source>
        <translation>Incomplete update task parameters</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="116" />
        <source>Automatic update is not supported on Windows</source>
        <translation>Automatic update is not supported on Windows</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="123" />
        <source>Updater not found</source>
        <translation>Updater not found</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="132" />
        <source>Updater integrity check failed: %1</source>
        <translation>Updater integrity check failed: %1</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="243" />
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="257" />
        <source>Automatic update is not supported on %1</source>
        <translation>Automatic update is not supported on %1</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="157" />
        <source>Failed to create update directory</source>
        <translation>Failed to create update directory</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="163" />
        <source>Invalid update URL</source>
        <translation>Invalid update URL</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="210" />
        <source>Failed to open local file for writing</source>
        <translation>Failed to open local file for writing</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="282" />
        <location filename="../../core/update/UpdateManager.cpp" line="311" />
        <source>No update install strategy available</source>
        <translation>No update install strategy available</translation>
    </message>
</context>
<context>
    <name>modbus::analyzer::value_formatter</name>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="59" />
        <source>Description: %1</source>
        <translation>Description: %1</translation>
    </message>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="65" />
        <source>Raw: %1</source>
        <translation>Raw: %1</translation>
    </message>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="66" />
        <source>Scale: %1</source>
        <translation>Scale: %1</translation>
    </message>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="67" />
        <source>Scaled: %1</source>
        <translation>Scaled: %1</translation>
    </message>
</context>
<context>
    <name>modbus::session</name>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="93" />
        <source>Illegal Function</source>
        <translation>Illegal Function</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="95" />
        <source>Illegal Data Address</source>
        <translation>Illegal Data Address</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="97" />
        <source>Illegal Data Value</source>
        <translation>Illegal Data Value</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="99" />
        <source>Server Device Failure</source>
        <translation>Server Device Failure</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="101" />
        <source>Acknowledge</source>
        <translation>Acknowledge</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="103" />
        <source>Server Device Busy</source>
        <translation>Server Device Busy</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="105" />
        <source>Negative Acknowledge</source>
        <translation>Negative Acknowledge</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="107" />
        <source>Memory Parity Error</source>
        <translation>Memory Parity Error</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="109" />
        <source>Gateway Path Unavailable</source>
        <translation>Gateway Path Unavailable</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="111" />
        <source>Gateway Target Device Failed To Respond</source>
        <translation>Gateway Target Device Failed To Respond</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="113" />
        <source>Unknown Exception</source>
        <translation>Unknown Exception</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="120" />
        <source>Modbus exception response. Slave=%1 FC=0x%2 Exception=0x%3 (%4)</source>
        <translation>Modbus exception response. Slave=%1 FC=0x%2 Exception=0x%3 (%4)</translation>
    </message>
</context>
<context>
    <name>modbus::session::ConnectionManager</name>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="142" />
        <source>No channel attached</source>
        <translation>No channel attached</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="193" />
        <source>State machine busy</source>
        <translation>State machine busy</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="199" />
        <source>Failed to dispatch channel open</source>
        <translation>Failed to dispatch channel open</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="226" />
        <source>Aborted</source>
        <translation>Aborted</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="236" />
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="281" />
        <source>Connect timeout</source>
        <translation>Connect timeout</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="263" />
        <source>Channel entered error state</source>
        <translation>Channel entered error state</translation>
    </message>
</context>
<context>
    <name>modbus::session::RequestExecutor</name>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="170" />
        <source>Request already in progress</source>
        <translation>Request already in progress</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="177" />
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="191" />
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="233" />
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="338" />
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="462" />
        <source>Aborted</source>
        <translation>Aborted</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="182" />
        <source>Unknown error</source>
        <translation>未知錯誤</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="322" />
        <source>Not connected</source>
        <translation>Not connected</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="353" />
        <source>Broadcast only supports write function codes</source>
        <translation>Broadcast only supports write function codes</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="426" />
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="454" />
        <source>Timeout</source>
        <translation>Timeout</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="480" />
        <source>Too many invalid response bytes</source>
        <translation>Too many invalid response bytes</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="515" />
        <source>Incomplete RTU frame after inter-frame silence</source>
        <translation>Incomplete RTU frame after inter-frame silence</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="539" />
        <source>Timeout while waiting for full packet</source>
        <translation>Timeout while waiting for full packet</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestExecutor.cpp" line="581" />
        <source>Response parsing failed</source>
        <translation>Response parsing failed</translation>
    </message>
</context>
<context>
    <name>modbus::session::RequestValidator</name>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="37" />
        <source>Invalid RTU slave id: %1</source>
        <translation>Invalid RTU slave id: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="42" />
        <source>PDU data too long: %1 bytes</source>
        <translation>PDU data too long: %1 bytes</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="57" />
        <source>Invalid read request payload length: %1</source>
        <translation>Invalid read request payload length: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="60" />
        <source>Read quantity must be at least %1</source>
        <translation>Read quantity must be at least %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="64" />
        <source>Read address range exceeds limit: start=%1 quantity=%2</source>
        <translation>Read address range exceeds limit: start=%1 quantity=%2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="71" />
        <source>Read bit quantity exceeds limit: %1</source>
        <translation>Read bit quantity exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="76" />
        <source>Read register quantity exceeds limit: %1</source>
        <translation>Read register quantity exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="84" />
        <source>Invalid write-single request payload length: %1</source>
        <translation>Invalid write-single request payload length: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="87" />
        <source>Write address out of range: %1</source>
        <translation>Write address out of range: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="91" />
        <source>Invalid single coil value: 0x%1</source>
        <translation>Invalid single coil value: 0x%1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="101" />
        <source>Invalid write-multiple request payload length: %1</source>
        <translation>Invalid write-multiple request payload length: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="105" />
        <source>Write quantity must be at least %1</source>
        <translation>Write quantity must be at least %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="109" />
        <source>Write address range exceeds limit: start=%1 quantity=%2</source>
        <translation>Write address range exceeds limit: start=%1 quantity=%2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="114" />
        <source>Write request byte count mismatch: declared %1, actual %2</source>
        <translation>Write request byte count mismatch: declared %1, actual %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="120" />
        <source>Write coil quantity exceeds limit: %1</source>
        <translation>Write coil quantity exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="124" />
        <source>Write coil byte count mismatch: declared %1, expected %2</source>
        <translation>Write coil byte count mismatch: declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="130" />
        <source>Write register quantity exceeds limit: %1</source>
        <translation>Write register quantity exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="134" />
        <source>Write register byte count mismatch: declared %1, expected %2</source>
        <translation>Write register byte count mismatch: declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="141" />
        <source>Unsupported function code: 0x%1</source>
        <translation>Unsupported function code: 0x%1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="148" />
        <source>TCP MBAP length exceeds limit: %1</source>
        <translation>TCP MBAP length exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="153" />
        <source>RTU ADU length exceeds limit: %1</source>
        <translation>RTU ADU length exceeds limit: %1</translation>
    </message>
</context>
<context>
    <name>ui::MainWindow</name>
    <message>
        <location filename="../MainWindow.cpp" line="119" />
        <location filename="../MainWindow.cpp" line="357" />
        <source>Modbus Tools</source>
        <translation>Modbus 工具</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="184" />
        <location filename="../MainWindow.cpp" line="362" />
        <source>Frame Analyzer</source>
        <translation>報文分析</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="247" />
        <location filename="../MainWindow.cpp" line="367" />
        <source>Language</source>
        <translation>語言</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="232" />
        <location filename="../MainWindow.cpp" line="368" />
        <source>Settings</source>
        <translation>設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="365" />
        <source>Expand Navigation</source>
        <translation>展開導覽</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="365" />
        <source>Collapse Navigation</source>
        <translation>摺疊導覽</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="233" />
        <location filename="../MainWindow.cpp" line="370" />
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="29" />
        <source>Modbus Settings</source>
        <translation>Modbus設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="184" />
        <location filename="../MainWindow.cpp" line="360" />
        <source>TCP/UDP Tool</source>
        <translation>TCP/UDP 工具</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="184" />
        <location filename="../MainWindow.cpp" line="361" />
        <source>Serial Debugger</source>
        <translation>串口除錯助手</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="184" />
        <location filename="../MainWindow.cpp" line="359" />
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="236" />
        <location filename="../MainWindow.cpp" line="371" />
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="33" />
        <source>Update Settings</source>
        <translation>更新設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="240" />
        <location filename="../MainWindow.cpp" line="372" />
        <source>Open Log Folder</source>
        <translation>開啟日誌資料夾</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="261" />
        <location filename="../MainWindow.cpp" line="376" />
        <source>English (US)</source>
        <translation>英語（美國）</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="262" />
        <location filename="../MainWindow.cpp" line="377" />
        <source>简体中文</source>
        <translation>簡體中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="263" />
        <location filename="../MainWindow.cpp" line="378" />
        <source>繁體中文</source>
        <translation>繁體中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="282" />
        <location filename="../MainWindow.cpp" line="373" />
        <source>Check for Updates</source>
        <translation>檢查更新</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="37" />
        <source>Request Timeout (ms):</source>
        <translation>請求逾時(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="41" />
        <source>Enable Retry:</source>
        <translation>啟用重試：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="46" />
        <source>Retry Count:</source>
        <translation>重試次數：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="52" />
        <source>Retry Interval (ms):</source>
        <translation>重試間隔(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="62" />
        <source>Debug</source>
        <translation>Debug</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="63" />
        <source>Info</source>
        <translation>資訊</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="64" />
        <source>Warning</source>
        <translation>Warning</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="65" />
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="68" />
        <source>Log Level:</source>
        <translation>Log Level:</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="276" />
        <location filename="../MainWindow.cpp" line="286" />
        <location filename="../MainWindow.cpp" line="369" />
        <location filename="../MainWindow.cpp" line="374" />
        <location filename="../widgets/AboutDialog.cpp" line="24" />
        <source>About</source>
        <translation>關於</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="308" />
        <source>Theme: Auto</source>
        <translation>主題：自動</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="309" />
        <source>Theme: Light</source>
        <translation>主題：淺色</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="309" />
        <source>Theme: Dark</source>
        <translation>主題：深色</translation>
    </message>
    <message>
        <location filename="../widgets/AboutDialog.cpp" line="33" />
        <source>Welcome to Modbus-Tools&lt;br&gt;Version: v%1&lt;br&gt;&lt;br&gt;An open-source Modbus communication debugging assistant.&lt;br&gt;Developer: mingyucheng692&lt;br&gt;License: MIT License&lt;br&gt;&lt;br&gt;This project is developed in spare time, completely free and open-source.&lt;br&gt;Feel free to star on GitHub or submit issues.&lt;br&gt;Your feedback keeps the project improving!&lt;br&gt;&lt;br&gt;&lt;a href="https://github.com/mingyucheng692/Modbus-Tools"&gt;🌐 Visit GitHub Repository&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href="https://github.com/mingyucheng692/Modbus-Tools/issues"&gt;🐛 Issue Tracker&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;This software is provided &amp;quot;as is&amp;quot; without warranty of any kind.</source>
        <translation>歡迎使用 Modbus-Tools&lt;br&gt;版本：v%1&lt;br&gt;&lt;br&gt;一款開源的 Modbus 通訊除錯助手。&lt;br&gt;開發者：mingyucheng692&lt;br&gt;授權條款：MIT License&lt;br&gt;&lt;br&gt;本專案由個人業餘開發，完全免費且開源。&lt;br&gt;歡迎在 GitHub 上 Star ⭐ 或提交 Issue。&lt;br&gt;您的回饋是專案持續改進的動力！&lt;br&gt;&lt;br&gt;&lt;a href="https://github.com/mingyucheng692/Modbus-Tools"&gt;🌐 造訪 GitHub 倉庫&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href="https://github.com/mingyucheng692/Modbus-Tools/issues"&gt;🐛 問題回報&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;本軟體按&amp;quot;原樣&amp;quot;提供，無任何形式的保證。</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="38" />
        <source>Every Startup</source>
        <translation>每次啟動</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="40" />
        <source>Weekly</source>
        <translation>每週</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="42" />
        <source>Monthly</source>
        <translation>每月</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="44" />
        <source>Disable Update Check</source>
        <translation>關閉更新檢查</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="46" />
        <source>Update Check Frequency:</source>
        <translation>更新檢查頻率：</translation>
    </message>
</context>
<context>
    <name>ui::UpdateInteractionView</name>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="41" />
        <source>About</source>
        <translation>關於</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="44" />
        <source>Check for Updates</source>
        <translation>檢查更新</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="63" />
        <location filename="../UpdateInteractionView.cpp" line="71" />
        <source>Update Available</source>
        <translation>發現新版本</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="64" />
        <source>New version v%1 available. Open download page?</source>
        <translation>發現新版本 v%1。是否開啟下載頁面？</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="72" />
        <source>Current: v%1, Latest: v%2
Choose update method:</source>
        <translation>目前版本：v%1，最新版本：v%2
請選擇更新方式：</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="73" />
        <source>Update Main Program</source>
        <translation>更新主程式</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="74" />
        <source>Download Full Package</source>
        <translation>下載完整安裝包</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="91" />
        <source>Downloading Update...</source>
        <translation>正在下載更新...</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="91" />
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
</context>
<context>
    <name>ui::application::UpdateCoordinator</name>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="155" />
        <source>No Updates</source>
        <translation>沒有可用更新</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="156" />
        <source>You are using the latest version: v%1</source>
        <translation>您正在使用最新版本：v%1</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="166" />
        <source>Update Check Failed</source>
        <translation>更新檢查失敗</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="181" />
        <source>Update service unavailable</source>
        <translation>Update service unavailable</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="185" />
        <location filename="../application/UpdateCoordinator.cpp" line="194" />
        <source>Update Failed</source>
        <translation>更新失敗</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="216" />
        <source>Automatic Update Unsupported</source>
        <translation>Automatic Update Unsupported</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="217" />
        <source>In-app automatic update is not supported on %1. Download the latest package instead.</source>
        <translation>In-app automatic update is not supported on %1. Download the latest package instead.</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::ModbusPagePresenter</name>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="250" />
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="284" />
        <source>Error: Request service not available</source>
        <translation>錯誤：請求服務不可用</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="264" />
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="292" />
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::ModbusSessionPresenter</name>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="119" />
        <source>Connecting to %1:%2...</source>
        <translation>正在連線 %1:%2...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="184" />
        <source>Opening %1...</source>
        <translation>正在開啟 %1...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="225" />
        <source>Disconnecting...</source>
        <translation>Disconnecting...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="248" />
        <source>Shutdown timed out; restart recommended</source>
        <translation>Shutdown timed out; restart recommended</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="254" />
        <source>Release timed out; restart recommended</source>
        <translation>Release timed out; restart recommended</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="282" />
        <source>Releasing Modbus stack...</source>
        <translation>Releasing Modbus stack...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="304" />
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="515" />
        <source>Release completed</source>
        <translation>Release completed</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="718" />
        <source>Transport connected, validating session...</source>
        <translation>Transport connected, validating session...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="747" />
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="748" />
        <source>Disconnected: %1</source>
        <translation>Disconnected: %1</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="801" />
        <source>Transport connected, waiting for device response...</source>
        <translation>Transport connected, waiting for device response...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="125" />
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="191" />
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="562" />
        <source>Failed to create Modbus stack</source>
        <translation>建立 Modbus 堆疊失敗</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="793" />
        <source>Connection failed: %1</source>
        <translation>連線失敗：%1</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::PollingController</name>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="102" />
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="141" />
        <source>Poll recovered after %1 consecutive failures</source>
        <translation>Poll recovered after %1 consecutive failures</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="195" />
        <source>Poll Error: Connection unavailable during polling (%1)</source>
        <translation>Poll Error: Connection unavailable during polling (%1)</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="198" />
        <source>Poll Error escalated after %1 consecutive failures: %2</source>
        <translation>Poll Error escalated after %1 consecutive failures: %2</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="202" />
        <source>Poll Error persists (%1 consecutive failures): %2</source>
        <translation>Poll Error persists (%1 consecutive failures): %2</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="221" />
        <source>Poll Warning: %1 consecutive failure(s): %2</source>
        <translation>Poll Warning: %1 consecutive failure(s): %2</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="245" />
        <source>Connection unavailable for %1 s; self-healing window expired</source>
        <translation>Connection unavailable for %1 s; self-healing window expired</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::TrafficLogController</name>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="79" />
        <source>Success: Response received</source>
        <translation>成功：已收到回應</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="87" />
        <source>Success: Write confirmed</source>
        <translation>成功：寫入已確認</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="96" />
        <source>Success: Broadcast write sent, no response expected</source>
        <translation>成功：廣播寫已發送，預期無回應</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="113" />
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation>發送讀取請求 FC:%1 位址:%2 數量:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="124" />
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation>發送寫入請求 FC:%1 位址:%2 資料:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="132" />
        <source>Sending Raw Data: %1</source>
        <translation>發送原始資料：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="159" />
        <source>%1 ms</source>
        <translation>%1 ms</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="160" />
        <source>--</source>
        <translation>--</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="162" />
        <source>Poll Summary FC:%1 Addr:%2 Qty:%3 Slave:%4 Success:%5 Error:%6 Retries:%7 Avg Success RTT:%8</source>
        <translation>Poll Summary FC:%1 Addr:%2 Qty:%3 Slave:%4 Success:%5 Error:%6 Retries:%7 Avg Success RTT:%8</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="175" />
        <source> | Raw frames shown %1, dropped %2</source>
        <translation> | Raw frames shown %1, dropped %2</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="201" />
        <source>%1 after %2 %3</source>
        <translation>%1 after %2 %3</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="212" />
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="217" />
        <source>Error: %1 (failed after %2 %3, see log for details)</source>
        <translation>Error: %1 (failed after %2 %3, see log for details)</translation>
    </message>
</context>
<context>
    <name>ui::common::UpdateChecker</name>
    <message>
        <location filename="../common/UpdateChecker.cpp" line="90" />
        <source>Release tag is missing</source>
        <translation>發布標籤缺失</translation>
    </message>
</context>
<context>
    <name>ui::shell::NavigationController</name>
    <message>
        <location filename="../shell/NavigationController.cpp" line="142" />
        <source>Expand Navigation</source>
        <translation>展開導覽</translation>
    </message>
    <message>
        <location filename="../shell/NavigationController.cpp" line="143" />
        <source>Collapse Navigation</source>
        <translation>摺疊導覽</translation>
    </message>
</context>
<context>
    <name>ui::views::generic_serial::GenericSerialView</name>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="137" />
        <source>Opening %1...</source>
        <translation>正在開啟 %1...</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="160" />
        <source>Closed</source>
        <translation>已關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="161" />
        <source>Opening</source>
        <translation>正在開啟</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="162" />
        <source>Open</source>
        <translation>已開啟</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="163" />
        <source>Closing</source>
        <translation>正在關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="164" />
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="165" />
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="185" />
        <source>State changed: %1</source>
        <translation>狀態已變更：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="200" />
        <source>Auto-reconnect exhausted (%1 attempts)</source>
        <translation>Auto-reconnect exhausted (%1 attempts)</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="208" />
        <source>Auto-reconnect in %1ms (%2)</source>
        <translation>Auto-reconnect in %1ms (%2)</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="220" />
        <source>Connection failed</source>
        <translation>連線失敗</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="221" />
        <source>Connection timeout</source>
        <translation>連線逾時</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="222" />
        <source>Write failed</source>
        <translation>寫入失敗</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="223" />
        <source>Read failed</source>
        <translation>讀取失敗</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="224" />
        <source>Port not found</source>
        <translation>找不到連接埠</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="225" />
        <source>Permission denied</source>
        <translation>權限被拒絕</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="226" />
        <source>Connection reset</source>
        <translation>連線重設</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="227" />
        <source>Unknown error</source>
        <translation>未知錯誤</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="229" />
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="264" />
        <source>Control</source>
        <translation>控制</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="265" />
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="266" />
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="267" />
        <source>RTS</source>
        <translation>RTS</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="292" />
        <source>Reconnecting to %1...</source>
        <translation>Reconnecting to %1...</translation>
    </message>
</context>
<context>
    <name>ui::views::generic_tcp::GenericTcpView</name>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="309" />
        <source>Connecting to %1:%2...</source>
        <translation>正在連線 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="325" />
        <source>Starting TCP server on %1:%2...</source>
        <translation>Starting TCP server on %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="340" />
        <source>Stopping TCP server...</source>
        <translation>Stopping TCP server...</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="354" />
        <source>Binding UDP %1:%2 -&gt; %3:%4...</source>
        <translation>Binding UDP %1:%2 -&gt; %3:%4...</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="358" />
        <source>Binding UDP %1:%2...</source>
        <translation>Binding UDP %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="404" />
        <source>Server mode: no connected clients available</source>
        <translation>Server mode: no connected clients available</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="405" />
        <source>Server mode: select at least one client</source>
        <translation>Server mode: select at least one client</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="439" />
        <source>Closed</source>
        <translation>已關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="440" />
        <source>Opening</source>
        <translation>正在開啟</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="441" />
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="442" />
        <source>Closing</source>
        <translation>正在關閉</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="443" />
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="584" />
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="444" />
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="585" />
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="482" />
        <source>State changed: %1</source>
        <translation>狀態已變更：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="504" />
        <source>Auto-reconnect exhausted (%1 attempts)</source>
        <translation>Auto-reconnect exhausted (%1 attempts)</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="512" />
        <source>Auto-reconnect in %1ms (%2)</source>
        <translation>Auto-reconnect in %1ms (%2)</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="531" />
        <source>Connection failed</source>
        <translation>連線失敗</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="532" />
        <source>Connection timeout</source>
        <translation>連線逾時</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="533" />
        <source>Write failed</source>
        <translation>寫入失敗</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="534" />
        <source>Read failed</source>
        <translation>讀取失敗</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="535" />
        <source>Port not found</source>
        <translation>找不到連接埠</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="536" />
        <source>Permission denied</source>
        <translation>權限被拒絕</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="537" />
        <source>Connection reset by peer</source>
        <translation>連線被對端重設</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="538" />
        <source>Unknown error</source>
        <translation>未知錯誤</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="677" />
        <source>Auto-reconnecting to %1:%2 (attempt %3)...</source>
        <translation>Auto-reconnecting to %1:%2 (attempt %3)...</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="540" />
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="557" />
        <source>Client #%1 connected: %2</source>
        <translation>Client #%1 connected: %2</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="564" />
        <source>Client #%1 disconnected</source>
        <translation>Client #%1 disconnected</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="582" />
        <source>Listening</source>
        <translation>Listening</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="583" />
        <source>Stopped</source>
        <translation>Stopped</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="610" />
        <source>Server state: %1</source>
        <translation>Server state: %1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="617" />
        <source>Server Error: %1</source>
        <translation>Server Error: %1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="650" />
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
</context>
<context>
    <name>ui::views::generic_tcp::Protocol</name>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="64" />
        <source>TCP Client</source>
        <translation>TCP 用戶端</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="65" />
        <source>TCP Server</source>
        <translation>TCP 伺服端</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="66" />
        <source>UDP</source>
        <translation>UDP</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus::ModbusPage</name>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="86" />
        <source>TCP</source>
        <translation>TCP</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="88" />
        <source>RTU</source>
        <translation>RTU</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="90" />
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="415" />
        <source>[%1] %2</source>
        <translation>[%1] %2</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="457" />
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="475" />
        <source>TX</source>
        <translation>傳送</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="484" />
        <source>Data Monitor</source>
        <translation>資料監視</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="485" />
        <source>Receive Data</source>
        <translation>接收資料</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="486" />
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="487" />
        <location filename="../views/modbus/ModbusPage.cpp" line="488" />
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="489" />
        <location filename="../views/modbus/ModbusPage.cpp" line="490" />
        <source>Clear</source>
        <translation>清除</translation>
    </message>
</context>
<context>
    <name>ui::widgets::BaseConnectionWidget</name>
    <message>
        <location filename="../widgets/BaseConnectionWidget.cpp" line="134" />
        <source>Connection Settings</source>
        <translation>連線設定</translation>
    </message>
    <message>
        <location filename="../widgets/BaseConnectionWidget.cpp" line="137" />
        <source>Auto Reconnect</source>
        <translation>Auto Reconnect</translation>
    </message>
</context>
<context>
    <name>ui::widgets::ByteMonitorWidget</name>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="169" />
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="388" />
        <source>[%1] [TX] %2</source>
        <translation>[%1] [TX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="393" />
        <source>[%1] [RX] %2</source>
        <translation>[%1] [RX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="397" />
        <source>[INFO] %1</source>
        <translation>[INFO] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="398" />
        <source>[%1] [INFO] %2</source>
        <translation>[%1] [資訊] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="407" />
        <source>[ERROR] %1</source>
        <translation>[ERROR] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="408" />
        <source>[%1] [ERROR] %2</source>
        <translation>[%1] [ERROR] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="402" />
        <source>[WARN] %1</source>
        <translation>[WARN] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="403" />
        <source>[%1] [WARN] %2</source>
        <translation>[%1] [警告] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="322" />
        <source>Text Files (*.txt);;All Files (*)</source>
        <translation>文字檔 (*.txt);;所有檔案 (*)</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="321" />
        <source>Save Log</source>
        <translation>儲存日誌</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="329" />
        <source>Cannot write file: %1</source>
        <translation>無法寫入檔案：%1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="387" />
        <source>[TX] %1</source>
        <translation>[TX] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="392" />
        <source>[RX] %1</source>
        <translation>[RX] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="462" />
        <source>TX: %1</source>
        <translation>TX: %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="465" />
        <source>RX: %1</source>
        <translation>RX: %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="558" />
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="561" />
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="567" />
        <source>Absolute</source>
        <translation>Absolute</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="568" />
        <source>Relative</source>
        <translation>Relative</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="569" />
        <source>None</source>
        <translation>無</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="572" />
        <source>TX</source>
        <translation>傳送</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="573" />
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="574" />
        <source>Auto Scroll</source>
        <translation>自動捲動</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="576" />
        <source>Resume</source>
        <translation>Resume</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="576" />
        <source>Pause</source>
        <translation>暫停</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="578" />
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="579" />
        <source>Save</source>
        <translation>儲存</translation>
    </message>
</context>
<context>
    <name>ui::widgets::CollapsibleSection</name>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="101" />
        <source>Collapse</source>
        <translation>收合</translation>
    </message>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="101" />
        <source>Expand</source>
        <translation>展開</translation>
    </message>
</context>
<context>
    <name>ui::widgets::ControlWidget</name>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="113" />
        <source>Invalid Address format or range (0-65535): %1</source>
        <translation>無效的位址格式或範圍 (0-65535): %1</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="125" />
        <source>Confirm Address</source>
        <translation>確認位址</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="126" />
        <source>The polling address is set to 0. Are you sure you want to continue?</source>
        <translation>輪詢位址設置為 0。確定要繼續嗎？</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="130" />
        <source>Do not show this again</source>
        <translation>不再顯示</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="157" />
        <source>%1 ms</source>
        <translation>%1 ms</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="158" />
        <source>--</source>
        <translation>--</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="349" />
        <source>Enable Polling</source>
        <translation>啟用輪詢</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="355" />
        <source>Interval(ms):</source>
        <translation>間隔(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="358" />
        <source>FC:</source>
        <translation>功能碼：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="361" />
        <source>01-Read Coils</source>
        <translation>01-讀取線圈</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="362" />
        <source>02-Read Discrete</source>
        <translation>02-讀取離散輸入</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="363" />
        <source>03-Read Holding</source>
        <translation>03-讀取保持暫存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="364" />
        <source>04-Read Input</source>
        <translation>04-讀取輸入暫存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="367" />
        <source>Addr:</source>
        <translation>位址：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="370" />
        <source>Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>位址 (0-65535)。支援 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="373" />
        <source>Qty:</source>
        <translation>數量：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="390" />
        <source>Last Success RTT</source>
        <translation>Last Success RTT</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="352" />
        <source>Link to Analyzer</source>
        <translation>聯動分析儀</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FrameAnalyzerWidget</name>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="176" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1070" />
        <source>Frame Input</source>
        <translation>報文輸入</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="182" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1071" />
        <source>Protocol:</source>
        <translation>通訊協定：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="185" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1073" />
        <source>Auto Detect</source>
        <translation>自動偵測</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="186" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1074" />
        <source>Modbus TCP</source>
        <translation>Modbus TCP</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="187" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1075" />
        <source>Modbus RTU</source>
        <translation>Modbus RTU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="188" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1076" />
        <source>Modbus ASCII</source>
        <translation>Modbus ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="192" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1078" />
        <source>Start Address (for Response):</source>
        <translation>起始位址（用於回應）：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="206" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1094" />
        <source>Format Hex</source>
        <translation>格式化 Hex</translation>
    </message>
    <message>
        <source>Paste &amp; Parse</source>
        <translation>貼上並解析</translation>
    </message>
    <message>
        <source>Paste clipboard content and parse immediately (Ctrl+Shift+V)</source>
        <translation>貼上剪貼簿內容並立即解析 (Ctrl+Shift+V)</translation>
    </message>
    <message>
        <source>Parse current input (Ctrl+Enter)</source>
        <translation>解析目前輸入 (Ctrl+Enter)</translation>
    </message>
    <message>
        <source>Clipboard is empty</source>
        <translation>剪貼簿為空</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="211" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1121" />
        <source>Parse</source>
        <translation>解析</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="216" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1122" />
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="226" />
        <source>Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)</source>
        <translation>輸入十六進位字串（如 01 03 00 00 00 01 84 0A）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1125" />
        <source>Enter Hex string (e.g., RTU: 01 03 00 00 00 01 84 0A, ASCII bytes: 3A 30 31 30 33 ... 0D 0A)</source>
        <translation>輸入十六進位字串（如 RTU: 01 03 00 00 00 01 84 0A，ASCII 位元組: 3A 30 31 30 33 ... 0D 0A）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="237" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1128" />
        <source>Analysis Result</source>
        <translation>分析結果</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="253" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1105" />
        <source>Status:</source>
        <translation>狀態：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="257" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="939" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1118" />
        <source>Ready</source>
        <translation>就緒</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="266" />
        <source>Tip: "Pause" to edit description</source>
        <translation>提示："暫停"以編輯描述</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="283" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="951" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1012" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1103" />
        <source>Pause Refresh</source>
        <translation>暫停重新整理</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1012" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1103" />
        <source>Resume Refresh</source>
        <translation>恢復重新整理</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="291" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1101" />
        <source>Stop Link</source>
        <translation>停止連動</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="303" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1082" />
        <source>Decode Mode:</source>
        <translation>解碼模式：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="305" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1084" />
        <source>Unsigned</source>
        <translation>無符號</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="306" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1085" />
        <source>Signed</source>
        <translation>有符號</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="340" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="627" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1095" />
        <source>Import Config</source>
        <translation>匯入設定</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="342" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="611" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1096" />
        <source>Export Config</source>
        <translation>匯出設定</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="344" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="657" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1097" />
        <source>Export CSV</source>
        <translation>匯出 CSV</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="368" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1136" />
        <source>Field</source>
        <translation>欄位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="368" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="376" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1137" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1141" />
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="368" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="376" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1138" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1141" />
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="372" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="748" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="960" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1131" />
        <source>Structure</source>
        <translation>結構</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="376" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1141" />
        <source>Address</source>
        <translation>位址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="376" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1141" />
        <source>Hex</source>
        <translation>十六進位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="376" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1141" />
        <source>Decimal</source>
        <translation>十進位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="376" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1141" />
        <source>Binary</source>
        <translation>二進位</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="376" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1141" />
        <source>Scale</source>
        <translation>倍率</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="653" />
        <source>There is no data to export.</source>
        <translation>沒有可導出的資料。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1080" />
        <source>Start Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>起始位址 (0-65535)。支援 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="407" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1129" />
        <source>History</source>
        <translation>歷史記錄</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="411" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1143" />
        <source>Clear History</source>
        <translation>清除歷史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="611" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="627" />
        <source>JSON Files (*.json)</source>
        <translation>JSON 檔案 (*.json)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="653" />
        <source>No Data</source>
        <translation>無資料</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="659" />
        <source>CSV Files (*.csv)</source>
        <translation>CSV 檔案 (*.csv)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="621" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="679" />
        <source>Export Failed</source>
        <translation>匯出失敗</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="502" />
        <source>OK</source>
        <translation>確定</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="502" />
        <source>ERR</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="402" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1132" />
        <source>Decoded Data</source>
        <translation>解析資料</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="562" />
        <source>Invalid Address (0-65535): %1</source>
        <translation>無效位址 (0-65535): %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="591" />
        <source>Parse Failed</source>
        <translation>解析失敗</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="632" />
        <source>Import Failed</source>
        <translation>匯入失敗</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="986" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1112" />
        <source>LIVE: %1</source>
        <translation>連動中: %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="990" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1114" />
        <source>Live Data Received at %1</source>
        <translation>即時資料接收於 %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="553" />
        <source>Error: Empty input</source>
        <translation>錯誤：輸入為空</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="569" />
        <source>Parsing...</source>
        <translation>解析中...</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="697" />
        <source>Parse Failed: %1</source>
        <translation>解析失敗：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="704" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1108" />
        <source>TCP</source>
        <translation>TCP</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="705" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1109" />
        <source>RTU</source>
        <translation>RTU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="754" />
        <source>Frame</source>
        <translation>幀</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="755" />
        <source>%1 bytes</source>
        <translation>%1 位元組</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="763" />
        <source>MBAP Header</source>
        <translation>MBAP 標頭</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="768" />
        <source>Transaction ID</source>
        <translation>交易識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="773" />
        <source>Protocol ID</source>
        <translation>協定識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="778" />
        <source>Length</source>
        <translation>長度</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="783" />
        <source>Unit ID</source>
        <translation>單元識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="789" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="805" />
        <source>Slave ID</source>
        <translation>從站識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="826" />
        <source>PDU</source>
        <translation>PDU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="832" />
        <source>Function Code</source>
        <translation>功能碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="844" />
        <source>Exception Code</source>
        <translation>例外碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="859" />
        <source>CRC16</source>
        <translation>CRC16</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="487" />
        <source>Show History</source>
        <translation>顯示歷史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="487" />
        <source>Hide History</source>
        <translation>隱藏歷史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="509" />
        <source>Local time %1</source>
        <translation>Local time %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="319" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1087" />
        <source>Byte Order:</source>
        <translation>位元組順序：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="321" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1089" />
        <source>ABCD(default)</source>
        <translation>ABCD(預設)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="706" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1110" />
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="749" />
        <source>(Unavailable in Live Mode)</source>
        <translation>(Unavailable in Live Mode)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="750" />
        <source>Logical parsing is disabled for high-frequency linkage</source>
        <translation>高頻聯動下已停用邏輯解析</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="758" />
        <source>Frame Bytes</source>
        <translation>幀位元組</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="758" />
        <source>Complete raw frame</source>
        <translation>完整原始幀</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="765" />
        <source>Transaction + Protocol + Length + Unit ID</source>
        <translation>交易 + 協定 + 長度 + 單元識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="770" />
        <source>Request/response correlation ID</source>
        <translation>請求/回應關聯識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="775" />
        <source>Modbus TCP protocol identifier</source>
        <translation>Modbus TCP 協定識別碼</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="780" />
        <source>Remaining bytes after this field</source>
        <translation>此欄位之後的剩餘位元組數</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="785" />
        <source>Target slave / unit address</source>
        <translation>目標從站 / 單元位址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="791" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="807" />
        <source>Target slave address</source>
        <translation>目標從站位址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="794" />
        <source>ASCII Start</source>
        <translation>ASCII Start</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="796" />
        <source>Start delimiter ':'</source>
        <translation>Start delimiter ':'</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="799" />
        <source>ASCII Payload</source>
        <translation>ASCII Payload</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="801" />
        <source>ASCII hex payload before CRLF</source>
        <translation>ASCII hex payload before CRLF</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="828" />
        <source>Function code + payload</source>
        <translation>功能碼 + 負載</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="834" />
        <source>Normal response</source>
        <translation>正常回應</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="834" />
        <source>Exception response</source>
        <translation>例外回應</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="837" />
        <source>Payload</source>
        <translation>負載</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="839" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="846" />
        <source>Exception detail payload</source>
        <translation>例外詳細負載</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="839" />
        <source>Application data payload</source>
        <translation>應用資料負載</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="850" />
        <source>CRC valid</source>
        <translation>CRC 有效</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="850" />
        <source>CRC invalid</source>
        <translation>CRC 無效</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="854" />
        <source>Expected 0x%1</source>
        <translation>預期 0x%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="868" />
        <source>LRC</source>
        <translation>LRC</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="872" />
        <source>LRC valid</source>
        <translation>LRC valid</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="872" />
        <source>LRC invalid</source>
        <translation>LRC invalid</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="875" />
        <source>CRLF</source>
        <translation>CRLF</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="877" />
        <source>ASCII frame terminator</source>
        <translation>ASCII frame terminator</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="923" />
        <source>Success (%1)</source>
        <translation>成功（%1）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="925" />
        <source>Forced Parsing</source>
        <translation>強制解析</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="928" />
        <source>Warnings</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="966" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1131" />
        <source>Structure (Unavailable in Live Mode)</source>
        <translation>結構（聯動模式不可用）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="709" />
        <source>Request</source>
        <translation>請求</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="710" />
        <source>Response</source>
        <translation>回應</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="711" />
        <source>Exception</source>
        <translation>例外</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="707" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="712" />
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1111" />
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FunctionWidget</name>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="409" />
        <source>Standard</source>
        <translation>標準</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="412" />
        <source>Raw</source>
        <translation>原始</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="415" />
        <source>Slave ID:</source>
        <translation>從站ID：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="418" />
        <source>Start Addr:</source>
        <translation>起始位址：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="429" />
        <source>Quantity:</source>
        <translation>數量：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="431" />
        <source>Write Data:</source>
        <translation>寫入資料：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="432" />
        <source>Format:</source>
        <translation>格式：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="438" />
        <source>Hex</source>
        <translation>十六進位</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="440" />
        <source>Binary</source>
        <translation>二進位</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="386" />
        <source>Space separated hex (e.g., 01 02)</source>
        <translation>空格分隔的十六進位 (如 01 02)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="336" />
        <location filename="../widgets/FunctionWidget.cpp" line="355" />
        <source>Invalid Unit ID format or range (0-255): %1</source>
        <translation>Invalid Unit ID format or range (0-255): %1</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="337" />
        <location filename="../widgets/FunctionWidget.cpp" line="356" />
        <source>Invalid Slave ID format or range (0-255): %1</source>
        <translation>無效的 Slave ID 格式或範圍 (0-255): %1</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="341" />
        <location filename="../widgets/FunctionWidget.cpp" line="360" />
        <source>Invalid Address format or range (0-65535): %1</source>
        <translation>無效的位址格式或範圍 (0-65535): %1</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="388" />
        <source>Space separated decimal (e.g., 100 200)</source>
        <translation>空格分隔的十進位 (如 100 200)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="390" />
        <source>Bit string (e.g., 1 1 0 1)</source>
        <translation>二進位位元字串 (如 1 1 0 1)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="471" />
        <source>Raw Hex Data (ASCII bytes, e.g., 3A 30 31 30 33 30 30 30 30 30 30 30 31 46 42 0D 0A):</source>
        <translation>原始十六進位資料（ASCII 位元組，例如 3A 30 31 30 33 30 30 30 30 30 30 30 31 46 42 0D 0A）：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="480" />
        <source>Add LRC + Encode ASCII</source>
        <translation>加入 LRC 並編碼為 ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="415" />
        <source>Unit ID:</source>
        <translation>Unit ID:</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="422" />
        <source>Unit ID (0-255). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>Unit ID (0-255). Supports HEX (0x10 or 10H) and DEC (16).</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="423" />
        <source>Slave ID (0-255). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>Slave ID (0-255)。支援 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="426" />
        <source>Start Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>起始位址 (0-65535)。支援 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="439" />
        <source>Decimal</source>
        <translation>十進位</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="443" />
        <source>Read Coils (0x01)</source>
        <translation>讀取線圈 (0x01)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="446" />
        <source>Read Discrete Inputs (0x02)</source>
        <translation>讀取離散輸入 (0x02)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="449" />
        <source>Read Holding Registers (0x03)</source>
        <translation>讀取保持暫存器 (0x03)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="452" />
        <source>Read Input Registers (0x04)</source>
        <translation>讀取輸入暫存器 (0x04)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="455" />
        <source>Write Single Coil (0x05)</source>
        <translation>寫入單線圈 (0x05)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="458" />
        <source>Write Single Register (0x06)</source>
        <translation>寫入單暫存器 (0x06)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="461" />
        <source>Write Multiple Coils (0x0F)</source>
        <translation>寫入多線圈 (0x0F)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="464" />
        <source>Write Multiple Registers (0x10)</source>
        <translation>寫入多暫存器 (0x10)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="469" />
        <source>Raw Hex Data (e.g., 00 00 00 00 00 06 01 03 00 00 00 01):</source>
        <translation>Raw Hex Data (e.g., 00 00 00 00 00 06 01 03 00 00 00 01):</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="472" />
        <source>Raw Hex Data (e.g., 01 03 00 00 00 01 84 0A):</source>
        <translation>原始十六進位資料(如 01 03 00 00 00 01 84 0A)：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="475" />
        <source>Send Raw</source>
        <translation>發送原始資料</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="481" />
        <source>Append CRC</source>
        <translation>計算并追加 CRC</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="484" />
        <source>Add MBAP</source>
        <translation>添加 MBAP 頭</translation>
    </message>
</context>
<context>
    <name>ui::widgets::GenericInputWidget</name>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="346" />
        <source>Enter data to send...</source>
        <translation>輸入要發送的資料...</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="349" />
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="352" />
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="355" />
        <source>Auto Send</source>
        <translation>自動發送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="358" />
        <source> ms</source>
        <translation> 毫秒</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="361" />
        <source>Send</source>
        <translation>發送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="66" />
        <location filename="../widgets/GenericInputWidget.cpp" line="367" />
        <source>No LF</source>
        <translation>無換行</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="67" />
        <location filename="../widgets/GenericInputWidget.cpp" line="368" />
        <source>CR (\r)</source>
        <translation>CR (\r)</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="68" />
        <location filename="../widgets/GenericInputWidget.cpp" line="369" />
        <source>LF (\n)</source>
        <translation>LF (\n)</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="69" />
        <location filename="../widgets/GenericInputWidget.cpp" line="370" />
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r\n)</translation>
    </message>
</context>
<context>
    <name>ui::widgets::NetworkConnectionWidget</name>
    <message>
        <location filename="../widgets/NetworkConnectionWidget.cpp" line="123" />
        <source>Port:</source>
        <translation>埠：</translation>
    </message>
</context>
<context>
    <name>ui::widgets::SerialConnectionWidget</name>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="42" />
        <source>None</source>
        <translation>無</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="43" />
        <source>Even</source>
        <translation>偶校驗</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="44" />
        <source>Odd</source>
        <translation>奇校驗</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="45" />
        <source>Space</source>
        <translation>空白校驗</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="46" />
        <source>Mark</source>
        <translation>標記校驗</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="47" />
        <source>RTS/CTS</source>
        <translation>RTS/CTS</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="48" />
        <source>XON/XOFF</source>
        <translation>XON/XOFF</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="155" />
        <location filename="../widgets/SerialConnectionWidget.cpp" line="157" />
        <location filename="../widgets/SerialConnectionWidget.cpp" line="159" />
        <source>Disconnect</source>
        <translation>中斷連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="155" />
        <source>Connecting</source>
        <translation>Connecting</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="157" />
        <source>Transport Connected</source>
        <translation>Transport Connected</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="159" />
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="153" />
        <source>Connect</source>
        <translation>連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="153" />
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="161" />
        <source>Disconnecting</source>
        <translation>Disconnecting</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="217" />
        <location filename="../widgets/SerialConnectionWidget.cpp" line="412" />
        <source>Refresh Ports</source>
        <translation>重新整理埠</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="392" />
        <source>Port:</source>
        <translation>埠：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="395" />
        <source>Baud:</source>
        <translation>鮑特率：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="398" />
        <source>Data:</source>
        <translation>資料位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="401" />
        <source>Parity:</source>
        <translation>同位元：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="405" />
        <source>Stop:</source>
        <translation>停止位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="408" />
        <source>Flow:</source>
        <translation>Flow:</translation>
    </message>
</context>
<context>
    <name>ui::widgets::ServerClientPanel</name>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="211" />
        <source>Select one or more clients for directed send, or enable broadcast.</source>
        <translation>選擇一個或多個用戶端進行定向發送，或啟用廣播。</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="214" />
        <source>Broadcast To All Clients</source>
        <translation>向所有用戶端廣播</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="217" />
        <source>Disconnect Selected</source>
        <translation>斷開選中的用戶端</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="220" />
        <source>Disconnect All Clients</source>
        <translation>斷開所有用戶端</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="238" />
        <source>all clients</source>
        <translation>all clients</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="239" />
        <source>%1 selected</source>
        <translation>%1 selected</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="240" />
        <source>Connected Clients: %1 | Send Target: %2</source>
        <translation>已連線用戶端: %1 | 發送目標: %2</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="272" />
        <source>Client #%1 - %2</source>
        <translation>Client #%1 - %2</translation>
    </message>
</context>
<context>
    <name>ui::widgets::TcpConnectionWidget</name>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="32" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="124" />
        <source>Host:</source>
        <translation>主機：</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="33" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="67" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="125" />
        <source>Connect</source>
        <translation>連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="67" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="82" />
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="69" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="71" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="73" />
        <source>Disconnect</source>
        <translation>中斷連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="69" />
        <source>Connecting</source>
        <translation>Connecting</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="71" />
        <source>Transport Connected</source>
        <translation>Transport Connected</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="73" />
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="75" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="88" />
        <source>Disconnecting</source>
        <translation>Disconnecting</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="35" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="129" />
        <source>Listen:</source>
        <translation>Listen:</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="36" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="82" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="130" />
        <source>Start Listen</source>
        <translation>Start Listen</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="84" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="86" />
        <source>Stop</source>
        <translation>Stop</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="84" />
        <source>Starting</source>
        <translation>Starting</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="86" />
        <source>Listening</source>
        <translation>Listening</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="88" />
        <source>Stopping</source>
        <translation>Stopping</translation>
    </message>
</context>
<context>
    <name>ui::widgets::TrafficMonitorWidget</name>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="185" />
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="272" />
        <source>Paused +%1</source>
        <translation>Paused +%1</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="262" />
        <source>Raw Frames: poll frames sampled 1/%1, manual frames always shown</source>
        <translation>Raw Frames: poll frames sampled 1/%1, manual frames always shown</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="325" />
        <source>[Trace:%1] </source>
        <translation>[Trace:%1] </translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="342" />
        <source>[%1] %2[TX] %3</source>
        <translation>[%1] %2[TX] %3</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="353" />
        <source>[%1] %2[RX] %3</source>
        <translation>[%1] %2[RX] %3</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="358" />
        <source>[%1] %2[WARN] %3</source>
        <translation>[%1] %2[WARN] %3</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="363" />
        <source>[%1] %2[ERROR] %3</source>
        <translation>[%1] %2[ERROR] %3</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="368" />
        <source>[%1] %2[INFO] %3</source>
        <translation>[%1] %2[INFO] %3</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="480" />
        <source> (×%1)</source>
        <translation> (×%1)</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="533" />
        <source>Save Log</source>
        <translation>儲存日誌</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="533" />
        <source>Text Files (*.txt);;All Files (*)</source>
        <translation>文字檔 (*.txt);;所有檔案 (*)</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="558" />
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="582" />
        <source>Save failed: %1</source>
        <translation>儲存失敗：%1</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="596" />
        <source>Cannot write file: %1</source>
        <translation>無法寫入檔案：%1</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="690" />
        <source>Traffic Monitor</source>
        <translation>通訊監視</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="691" />
        <source>Auto Scroll</source>
        <translation>自動捲動</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="692" />
        <source>Pause</source>
        <translation>暫停</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="693" />
        <source>Raw Frames</source>
        <translation>原始幀</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="698" />
        <source>All</source>
        <translation>全部</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="699" />
        <source>Info</source>
        <translation>資訊</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="700" />
        <source>Warn</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="701" />
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="703" />
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="704" />
        <source>Log Level Filter</source>
        <translation>日誌等級篩選</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="706" />
        <source>TX</source>
        <translation>傳送</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="707" />
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="710" />
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="711" />
        <source>Save</source>
        <translation>儲存</translation>
    </message>
</context>
<context>
    <name>ui::widgets::UdpConnectionWidget</name>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="78" />
        <location filename="../widgets/UdpConnectionWidget.cpp" line="134" />
        <source>Local:</source>
        <translation>Local:</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="79" />
        <location filename="../widgets/UdpConnectionWidget.cpp" line="106" />
        <location filename="../widgets/UdpConnectionWidget.cpp" line="135" />
        <source>Bind</source>
        <translation>Bind</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="80" />
        <source>Remote:</source>
        <translation>Remote:</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="81" />
        <source>Remote Port:</source>
        <translation>Remote Port:</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="106" />
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="108" />
        <location filename="../widgets/UdpConnectionWidget.cpp" line="110" />
        <source>Unbind</source>
        <translation>Unbind</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="108" />
        <source>Binding</source>
        <translation>Binding</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="110" />
        <source>Bound</source>
        <translation>Bound</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="112" />
        <source>Unbinding</source>
        <translation>Unbinding</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="112" />
        <source>Disconnecting</source>
        <translation>Disconnecting</translation>
    </message>
</context>
</TS>