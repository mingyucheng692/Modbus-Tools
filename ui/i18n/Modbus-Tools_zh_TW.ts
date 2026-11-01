<?xml version='1.0' encoding='utf-8'?>
<TS version="2.1" language="zh_TW">
<context>
    <name>ConnectionAlert</name>
    <message>
        <location filename="../common/ConnectionAlert.h" line="34" />
        <source>Not Connected</source>
        <translation>未連線</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="35" />
        <source>Please connect first.</source>
        <translation>請先連線設備</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="44" />
        <source>Connection Lost</source>
        <translation>連線中斷</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="45" />
        <source>Connection was closed.</source>
        <translation>連線已關閉。</translation>
    </message>
</context>
<context>
    <name>FrameParseWorker</name>
    <message>
        <location filename="../../core/modbus/parser/FrameParseWorker.cpp" line="47" />
        <source>Error: Empty input</source>
        <translation>錯誤：輸入為空</translation>
    </message>
</context>
<context>
    <name>ModbusFrameParser</name>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="32" />
        <source>Illegal Function</source>
        <translation>Illegal Function</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="34" />
        <source>Illegal Data Address</source>
        <translation>Illegal Data Address</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="36" />
        <source>Illegal Data Value</source>
        <translation>Illegal Data Value</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="38" />
        <source>Server Device Failure</source>
        <translation>Server Device Failure</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="40" />
        <source>Acknowledge</source>
        <translation>Acknowledge</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="42" />
        <source>Server Device Busy</source>
        <translation>Server Device Busy</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="44" />
        <source>Negative Acknowledge</source>
        <translation>Negative Acknowledge</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="46" />
        <source>Memory Parity Error</source>
        <translation>Memory Parity Error</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="48" />
        <source>Gateway Path Unavailable</source>
        <translation>Gateway Path Unavailable</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="50" />
        <source>Gateway Target Device Failed To Respond</source>
        <translation>Gateway Target Device Failed To Respond</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="52" />
        <source>Unknown Exception</source>
        <translation>Unknown Exception</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="132" />
        <source>Frame too short for Modbus TCP. Expected a complete MBAP + PDU, got %1 bytes</source>
        <translation>Frame too short for Modbus TCP. Expected a complete MBAP + PDU, got %1 bytes</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="141" />
        <source>Invalid TCP MBAP header or length</source>
        <translation>Invalid TCP MBAP header or length</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="147" />
        <source>TCP frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>TCP frame contains trailing bytes. Expected %1 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="155" />
        <source>Frame too short for TCP MBAP</source>
        <translation>Frame too short for TCP MBAP</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="159" />
        <source>Warning: Invalid TCP MBAP header or length (Forced)</source>
        <translation>Warning: Invalid TCP MBAP header or length (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="162" />
        <source>Warning: TCP frame contains trailing bytes (Forced)</source>
        <translation>Warning: TCP frame contains trailing bytes (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="175" />
        <source>Invalid TCP PDU length. MBAP length field is %1, so PDU length is %2</source>
        <translation>Invalid TCP PDU length. MBAP length field is %1, so PDU length is %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="202" />
        <source>Frame too short for Modbus RTU. Expected at least 5 bytes, got %1</source>
        <translation>Frame too short for Modbus RTU. Expected at least 5 bytes, got %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="218" />
        <source>CRC Mismatch. Expected %1, Got %2</source>
        <translation>CRC Mismatch. Expected %1, Got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="226" />
        <source>RTU frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>RTU frame contains trailing bytes. Expected %1 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="234" />
        <source>Frame too short for RTU</source>
        <translation>Frame too short for RTU</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="238" />
        <source>Warning: CRC Mismatch. Expected %1, Got %2 (Forced)</source>
        <translation>Warning: CRC Mismatch. Expected %1, Got %2 (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="244" />
        <source>Warning: RTU frame contains trailing bytes (Forced)</source>
        <translation>Warning: RTU frame contains trailing bytes (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="275" />
        <source>Frame incomplete for Modbus ASCII. Missing CRLF terminator or full payload</source>
        <translation>Frame incomplete for Modbus ASCII. Missing CRLF terminator or full payload</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="283" />
        <source>Invalid Modbus ASCII frame or LRC mismatch</source>
        <translation>Invalid Modbus ASCII frame or LRC mismatch</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="291" />
        <source>ASCII frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>ASCII frame contains trailing bytes. Expected %1 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="299" />
        <source>Warning: Invalid Modbus ASCII frame or LRC mismatch (Forced)</source>
        <translation>Warning: Invalid Modbus ASCII frame or LRC mismatch (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="333" />
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
        <source>Exception PDU length invalid for function 0x%1. Expected strictly 2 bytes, got %2</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="413" />
        <source>Modbus exception: %1 (code %2)</source>
        <translation>Modbus exception: %1 (code %2)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="444" />
        <source>Failed to read request fields</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="456" />
        <source>Response PDU too short for function 0x%1. Expected at least 2 bytes, got %2</source>
        <translation>Response PDU too short for function 0x%1. Expected at least 2 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="468" />
        <source>Byte count mismatch for function 0x%1. Declared %2, actual %3</source>
        <translation>Byte count mismatch for function 0x%1. Declared %2, actual %3</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="481" />
        <source>Register response byte count does not match expected quantity. Declared %1, expected %2</source>
        <translation>Register response byte count does not match expected quantity. Declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="490" />
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="663" />
        <source>Register byte count must be even, got %1</source>
        <translation>Register byte count must be even, got %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="503" />
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="675" />
        <source>Truncated register data</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="530" />
        <source>Coil response bit count does not match expected quantity. Byte count %1 cannot represent %2 bits</source>
        <translation>Coil response bit count does not match expected quantity. Byte count %1 cannot represent %2 bits</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="567" />
        <source>Write single PDU length mismatch for function 0x%1. Expected 4 bytes, got %2</source>
        <translation>Write single PDU length mismatch for function 0x%1. Expected 4 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="580" />
        <source>Failed to read write value</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="615" />
        <source>Failed to read response echo</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="626" />
        <source>Write request PDU too short</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="636" />
        <source>Failed to read write request fields</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="643" />
        <source>Write request byte count mismatch. Declared %1, actual %2</source>
        <translation>Write request byte count mismatch. Declared %1, actual %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="654" />
        <source>Register write byte count does not match quantity. Declared %1, expected %2</source>
        <translation>Register write byte count does not match quantity. Declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="696" />
        <source>Coil write byte count does not match quantity. Declared %1, expected %2</source>
        <translation>Coil write byte count does not match quantity. Declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="729" />
        <source>Unsupported function code 0x%1 for deep parsing</source>
        <translation>Unsupported function code 0x%1 for deep parsing</translation>
    </message>
</context>
<context>
    <name>ModbusPduBuilder</name>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="12" />
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="73" />
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="89" />
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="105" />
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="141" />
        <source>Invalid start address</source>
        <translation>Invalid start address</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="16" />
        <source>Invalid quantity</source>
        <translation>Invalid quantity</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="65" />
        <source>Unsupported function code</source>
        <translation>Unsupported function code</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="109" />
        <source>Invalid quantity for 0x0F</source>
        <translation>0x0F 數量無效</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="115" />
        <source>Data length mismatch for 0x0F</source>
        <translation>Data length mismatch for 0x0F</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="145" />
        <source>Invalid quantity for 0x10</source>
        <translation>0x10 數量無效</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="150" />
        <source>Data length mismatch for 0x10</source>
        <translation>Data length mismatch for 0x10</translation>
    </message>
</context>
<context>
    <name>ModbusProtocolChecks</name>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="18" />
        <source>Exception</source>
        <translation>例外</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="19" />
        <source>Bit read</source>
        <translation>Bit read</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="20" />
        <source>Register read</source>
        <translation>Register read</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="21" />
        <source>Write single</source>
        <translation>Write single</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="22" />
        <source>Write multiple</source>
        <translation>Write multiple</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="23" />
        <source>%1 response payload length mismatch: expected %2, got %3</source>
        <translation>%1 response payload length mismatch: expected %2, got %3</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="24" />
        <source>Exception function code does not match request</source>
        <translation>Exception function code does not match request</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="25" />
        <source>Response function code does not match request</source>
        <translation>Response function code does not match request</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="26" />
        <source>Request quantity missing for bit-read validation</source>
        <translation>Request quantity missing for bit-read validation</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="27" />
        <source>Bit-read response byte count does not match payload length</source>
        <translation>Bit-read response byte count does not match payload length</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="28" />
        <source>Bit-read response byte count does not match requested quantity</source>
        <translation>Bit-read response byte count does not match requested quantity</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="29" />
        <source>Request quantity missing for register-read validation</source>
        <translation>Request quantity missing for register-read validation</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="30" />
        <source>Register-read response byte count does not match payload length</source>
        <translation>Register-read response byte count does not match payload length</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="31" />
        <source>Register-read response byte count does not match requested quantity</source>
        <translation>Register-read response byte count does not match requested quantity</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="32" />
        <source>Register-read response byte count must be even</source>
        <translation>Register-read response byte count must be even</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="33" />
        <source>Write-single response echo does not match request</source>
        <translation>Write-single response echo does not match request</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="34" />
        <source>Request echo fields missing for write-multiple validation</source>
        <translation>Request echo fields missing for write-multiple validation</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="35" />
        <source>Write-multiple response echo fields are incomplete</source>
        <translation>Write-multiple response echo fields are incomplete</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="36" />
        <source>Write-multiple response echo does not match request</source>
        <translation>Write-multiple response echo does not match request</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="37" />
        <source>Unsupported function code for response validation</source>
        <translation>Unsupported function code for response validation</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="59" />
        <source>Missing or invalid expected checksum</source>
        <translation>Missing or invalid expected checksum</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="63" />
        <location filename="../../core/update/UpdateManager.cpp" line="76" />
        <source>Canceled</source>
        <translation>Canceled</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="69" />
        <source>Failed to open file for checksum calculation</source>
        <translation>Failed to open file for checksum calculation</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="80" />
        <source>Failed to read file chunk</source>
        <translation>Failed to read file chunk</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="87" />
        <source>Failed to calculate file checksum</source>
        <translation>Failed to calculate file checksum</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="92" />
        <source>Checksum mismatch. Expected: %1, Actual: %2</source>
        <translation>Checksum mismatch. Expected: %1, Actual: %2</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="480" />
        <source>Illegal Function</source>
        <translation type="unfinished">Illegal Function</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="482" />
        <source>Illegal Data Address</source>
        <translation type="unfinished">Illegal Data Address</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="484" />
        <source>Illegal Data Value</source>
        <translation type="unfinished">Illegal Data Value</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="486" />
        <source>Server Device Failure</source>
        <translation type="unfinished">Server Device Failure</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="488" />
        <source>Acknowledge</source>
        <translation type="unfinished">Acknowledge</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="490" />
        <source>Server Device Busy</source>
        <translation type="unfinished">Server Device Busy</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="492" />
        <source>Negative Acknowledge</source>
        <translation type="unfinished">Negative Acknowledge</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="494" />
        <source>Memory Parity Error</source>
        <translation type="unfinished">Memory Parity Error</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="496" />
        <source>Gateway Path Unavailable</source>
        <translation type="unfinished">Gateway Path Unavailable</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="498" />
        <source>Gateway Target Device Failed To Respond</source>
        <translation type="unfinished">Gateway Target Device Failed To Respond</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="500" />
        <source>Unknown Exception</source>
        <translation type="unfinished">Unknown Exception</translation>
    </message>
</context>
<context>
    <name>RequestSubmissionService</name>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="56" />
        <source>Invalid decimal value for 0x05</source>
        <translation>Invalid decimal value for 0x05</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="66" />
        <source>Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation>Invalid binary value for 0x05 (expected 0 or 1)</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="75" />
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="83" />
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="92" />
        <source>Invalid hex value for 0x05</source>
        <translation>Invalid hex value for 0x05</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="105" />
        <source>Empty value for 0x06</source>
        <translation>Empty value for 0x06</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="114" />
        <source>Invalid decimal value for 0x06</source>
        <translation>Invalid decimal value for 0x06</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="122" />
        <source>Binary format not supported for registers (0x06)</source>
        <translation>Binary format not supported for registers (0x06)</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="129" />
        <source>Invalid hex value for 0x06</source>
        <translation>Invalid hex value for 0x06</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="138" />
        <source>Invalid quantity for 0x0F</source>
        <translation>0x0F 數量無效</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="148" />
        <source>Binary bit count (%1) does not match Quantity (%2)</source>
        <translation>Binary bit count (%1) does not match Quantity (%2)</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="158" />
        <source>0x0F requires Hex or Binary data</source>
        <translation>0x0F requires Hex or Binary data</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="165" />
        <source>Empty value for 0x10</source>
        <translation>Empty value for 0x10</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="171" />
        <source>Invalid quantity for 0x10</source>
        <translation>0x10 數量無效</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="180" />
        <source>Invalid decimal list for 0x10</source>
        <translation>Invalid decimal list for 0x10</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="188" />
        <source>Invalid hex value for 0x10</source>
        <translation>Invalid hex value for 0x10</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="220" />
        <source>Raw data is empty</source>
        <translation>Raw data is empty</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="266" />
        <source>Slave ID (%1) exceeds maximum (%2)</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="271" />
        <source>Broadcast read is forbidden per IEC 61158</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="283" />
        <source>Unsupported function code: %1</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="289" />
        <source>Quantity must be at least 1</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="293" />
        <source>Quantity (%1) exceeds maximum allowed (%2) for function code %3</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="298" />
        <source>Single write quantity must be 1</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="304" />
        <source>Address range overflow: start=%1, quantity=%2 exceeds 65536</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="316" />
        <source>FC 05 write data payload must be exactly 2 bytes</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="322" />
        <source>FC 05 coil value must strictly be 0x0000 (OFF) or 0xFF00 (ON)</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="327" />
        <source>FC 06 write data payload must be exactly 2 bytes</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="333" />
        <source>FC 0F write coils payload size (%1) does not match expected (%2) bytes</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="340" />
        <source>FC 10 write registers payload size (%1) does not match expected (%2) bytes</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="351" />
        <source>Wire PDU length (%1 bytes) exceeds maximum 253 bytes</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="361" />
        <source>Raw data cannot be empty</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="365" />
        <source>Raw frame exceeds maximum size (%1 bytes)</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="394" />
        <source>Raw data too short</source>
        <translation type="unfinished" />
    </message>
</context>
<context>
    <name>TrafficLogController</name>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="178" />
        <source>retry</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="179" />
        <source>retries</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="193" />
        <source>Error: %1</source>
        <translation type="unfinished">錯誤：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="195" />
        <source>Error: %1 (after %2 %3)</source>
        <translation type="unfinished" />
    </message>
</context>
<context>
    <name>core::update::UpdateCommandTemplate</name>
    <message>
        <location filename="../../core/update/UpdateCommandTemplate.cpp" line="33" />
        <source># WARNING: no checksum was published for this release; verify the download source manually before continuing.</source>
        <translation># 警告:此版本未提供校驗和,請人工核對下載來源後再繼續。</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateCommandTemplate.cpp" line="40" />
        <source># 1) Download
</source>
        <translation># 1) 下載
</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateCommandTemplate.cpp" line="47" />
        <source># 2) Verify checksum (do not continue on failure)
</source>
        <translation># 2) 校驗(失敗請勿繼續)
</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateCommandTemplate.cpp" line="52" />
        <source># 3) Replace (quit Modbus-Tools first; adjust the path below if you installed elsewhere)
</source>
        <translation># 3) 取代(請先結束 Modbus-Tools;若安裝在其他目錄請先修改下方路徑)
</translation>
    </message>
</context>
<context>
    <name>core::update::UpdateManager</name>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="55" />
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="122" />
        <source>Incomplete update task parameters</source>
        <translation>Incomplete update task parameters</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="70" />
        <source>Automatic update is not supported on Windows</source>
        <translation>Automatic update is not supported on Windows</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="77" />
        <source>Updater not found</source>
        <translation>Updater not found</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="86" />
        <source>Updater integrity check failed: %1</source>
        <translation>Updater integrity check failed: %1</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="201" />
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="215" />
        <source>Automatic update is not supported on %1</source>
        <translation>Automatic update is not supported on %1</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="150" />
        <source>Failed to create update directory</source>
        <translation>Failed to create update directory</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="156" />
        <source>Invalid update URL</source>
        <translation>Invalid update URL</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="203" />
        <source>Failed to open local file for writing</source>
        <translation>Failed to open local file for writing</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="275" />
        <location filename="../../core/update/UpdateManager.cpp" line="304" />
        <source>No update install strategy available</source>
        <translation>No update install strategy available</translation>
    </message>
</context>
<context>
    <name>modbus::analyzer::value_formatter</name>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="68" />
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="89" />
        <source>Description: %1</source>
        <translation>Description: %1</translation>
    </message>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="74" />
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="93" />
        <source>Raw: %1</source>
        <translation>Raw: %1</translation>
    </message>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="75" />
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="94" />
        <source>Scale: %1</source>
        <translation>Scale: %1</translation>
    </message>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="76" />
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="96" />
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="98" />
        <source>Scaled: %1</source>
        <translation>Scaled: %1</translation>
    </message>
</context>
<context>
    <name>modbus::session::ConnectionManager</name>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="135" />
        <source>No channel attached</source>
        <translation>No channel attached</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="180" />
        <source>State machine busy</source>
        <translation>State machine busy</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="187" />
        <source>Failed to dispatch channel open</source>
        <translation>Failed to dispatch channel open</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="209" />
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="264" />
        <source>Connect timeout</source>
        <translation>Connect timeout</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="246" />
        <source>Channel entered error state</source>
        <translation>Channel entered error state</translation>
    </message>
</context>
<context>
    <name>modbus::session::RequestValidator</name>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="30" />
        <source>Invalid RTU slave id: %1</source>
        <translation>Invalid RTU slave id: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="35" />
        <source>PDU data too long: %1 bytes</source>
        <translation>PDU data too long: %1 bytes</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="50" />
        <source>Invalid read request payload length: %1</source>
        <translation>Invalid read request payload length: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="53" />
        <source>Read quantity must be at least %1</source>
        <translation>Read quantity must be at least %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="57" />
        <source>Read address range exceeds limit: start=%1 quantity=%2</source>
        <translation>Read address range exceeds limit: start=%1 quantity=%2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="64" />
        <source>Read bit quantity exceeds limit: %1</source>
        <translation>Read bit quantity exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="69" />
        <source>Read register quantity exceeds limit: %1</source>
        <translation>Read register quantity exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="77" />
        <source>Invalid write-single request payload length: %1</source>
        <translation>Invalid write-single request payload length: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="80" />
        <source>Write address out of range: %1</source>
        <translation>Write address out of range: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="84" />
        <source>Invalid single coil value: 0x%1</source>
        <translation>Invalid single coil value: 0x%1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="94" />
        <source>Invalid write-multiple request payload length: %1</source>
        <translation>Invalid write-multiple request payload length: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="98" />
        <source>Write quantity must be at least %1</source>
        <translation>Write quantity must be at least %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="102" />
        <source>Write address range exceeds limit: start=%1 quantity=%2</source>
        <translation>Write address range exceeds limit: start=%1 quantity=%2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="107" />
        <source>Write request byte count mismatch: declared %1, actual %2</source>
        <translation>Write request byte count mismatch: declared %1, actual %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="113" />
        <source>Write coil quantity exceeds limit: %1</source>
        <translation>Write coil quantity exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="117" />
        <source>Write coil byte count mismatch: declared %1, expected %2</source>
        <translation>Write coil byte count mismatch: declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="123" />
        <source>Write register quantity exceeds limit: %1</source>
        <translation>Write register quantity exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="127" />
        <source>Write register byte count mismatch: declared %1, expected %2</source>
        <translation>Write register byte count mismatch: declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="134" />
        <source>Unsupported function code: 0x%1</source>
        <translation>Unsupported function code: 0x%1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="141" />
        <source>TCP MBAP length exceeds limit: %1</source>
        <translation>TCP MBAP length exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="146" />
        <source>RTU ADU length exceeds limit: %1</source>
        <translation>RTU ADU length exceeds limit: %1</translation>
    </message>
</context>
<context>
    <name>ui::MainWindow</name>
    <message>
        <location filename="../MainWindow.cpp" line="113" />
        <location filename="../MainWindow.cpp" line="380" />
        <source>Modbus Tools</source>
        <translation>Modbus 工具</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="189" />
        <location filename="../MainWindow.cpp" line="385" />
        <source>Frame Analyzer</source>
        <translation>報文分析</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="189" />
        <location filename="../MainWindow.cpp" line="386" />
        <source>Utilities</source>
        <translation>實用工具</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="259" />
        <location filename="../MainWindow.cpp" line="391" />
        <source>Language</source>
        <translation>語言</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="244" />
        <location filename="../MainWindow.cpp" line="392" />
        <source>Settings</source>
        <translation>設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="389" />
        <source>Expand Navigation</source>
        <translation>展開導覽</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="389" />
        <source>Collapse Navigation</source>
        <translation>摺疊導覽</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="245" />
        <location filename="../MainWindow.cpp" line="394" />
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="22" />
        <source>Modbus Settings</source>
        <translation>Modbus設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="189" />
        <location filename="../MainWindow.cpp" line="383" />
        <source>Network Debugger</source>
        <translation>網路除錯器</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="189" />
        <location filename="../MainWindow.cpp" line="384" />
        <source>Serial Debugger</source>
        <translation>串口除錯助手</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="189" />
        <location filename="../MainWindow.cpp" line="382" />
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="248" />
        <location filename="../MainWindow.cpp" line="395" />
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="26" />
        <source>Update Settings</source>
        <translation>更新設定</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="252" />
        <location filename="../MainWindow.cpp" line="396" />
        <source>Open Log Folder</source>
        <translation>開啟日誌資料夾</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="273" />
        <location filename="../MainWindow.cpp" line="400" />
        <source>English (US)</source>
        <translation>英語（美國）</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="274" />
        <location filename="../MainWindow.cpp" line="401" />
        <source>简体中文</source>
        <translation>簡體中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="275" />
        <location filename="../MainWindow.cpp" line="402" />
        <source>繁體中文</source>
        <translation>繁體中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="294" />
        <location filename="../MainWindow.cpp" line="397" />
        <source>Check for Updates</source>
        <translation>檢查更新</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="32" />
        <source>Request Timeout (ms):</source>
        <translation>請求逾時(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="36" />
        <source>Enable Retry:</source>
        <translation>啟用重試：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="43" />
        <source>Retry Count:</source>
        <translation>重試次數：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="51" />
        <source>Retry Interval (ms):</source>
        <translation>重試間隔(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="61" />
        <source>Debug</source>
        <translation>Debug</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="62" />
        <source>Info</source>
        <translation>資訊</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="63" />
        <source>Warning</source>
        <translation>Warning</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="64" />
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="67" />
        <source>Log Level:</source>
        <translation>Log Level:</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="70" />
        <source>Offset (0-based)</source>
        <translation>偏移量 (從 0 開始)</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="71" />
        <source>PLC Address (1-based)</source>
        <translation>PLC 位址 (從 1 開始)</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="74" />
        <source>Address Mode:</source>
        <translation>位址模式:</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="288" />
        <location filename="../MainWindow.cpp" line="298" />
        <location filename="../MainWindow.cpp" line="393" />
        <location filename="../MainWindow.cpp" line="398" />
        <location filename="../widgets/AboutDialog.cpp" line="17" />
        <source>About</source>
        <translation>關於</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="320" />
        <source>Theme: Auto</source>
        <translation>主題：自動</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="321" />
        <source>Theme: Light</source>
        <translation>主題：淺色</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="321" />
        <source>Theme: Dark</source>
        <translation>主題：深色</translation>
    </message>
    <message>
        <location filename="../widgets/AboutDialog.cpp" line="26" />
        <source>Welcome to Modbus-Tools&lt;br&gt;Version: v%1&lt;br&gt;&lt;br&gt;An open-source Modbus communication debugging assistant.&lt;br&gt;Developer: mingyucheng692&lt;br&gt;License: MIT License&lt;br&gt;&lt;br&gt;This project is developed in spare time, completely free and open-source.&lt;br&gt;Feel free to star on GitHub or submit issues.&lt;br&gt;Your feedback keeps the project improving!&lt;br&gt;&lt;br&gt;&lt;a href="https://github.com/mingyucheng692/Modbus-Tools"&gt;🌐 Visit GitHub Repository&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href="https://github.com/mingyucheng692/Modbus-Tools/issues"&gt;🐛 Issue Tracker&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;This software is provided &amp;quot;as is&amp;quot; without warranty of any kind.</source>
        <translation>歡迎使用 Modbus-Tools&lt;br&gt;版本：v%1&lt;br&gt;&lt;br&gt;一款開源的 Modbus 通訊除錯助手。&lt;br&gt;開發者：mingyucheng692&lt;br&gt;授權條款：MIT License&lt;br&gt;&lt;br&gt;本專案由個人業餘開發，完全免費且開源。&lt;br&gt;歡迎在 GitHub 上 Star ⭐ 或提交 Issue。&lt;br&gt;您的回饋是專案持續改進的動力！&lt;br&gt;&lt;br&gt;&lt;a href="https://github.com/mingyucheng692/Modbus-Tools"&gt;🌐 造訪 GitHub 倉庫&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href="https://github.com/mingyucheng692/Modbus-Tools/issues"&gt;🐛 問題回報&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;本軟體按&amp;quot;原樣&amp;quot;提供，無任何形式的保證。</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="31" />
        <source>Every Startup</source>
        <translation>每次啟動</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="33" />
        <source>Weekly</source>
        <translation>每週</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="35" />
        <source>Monthly</source>
        <translation>每月</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="37" />
        <source>Disable Update Check</source>
        <translation>關閉更新檢查</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="39" />
        <source>Update Check Frequency:</source>
        <translation>更新檢查頻率：</translation>
    </message>
</context>
<context>
    <name>ui::UpdateCommandDialog</name>
    <message>
        <location filename="../UpdateCommandDialog.cpp" line="25" />
        <source>Update to v%1</source>
        <translation>更新到 v%1</translation>
    </message>
    <message>
        <location filename="../UpdateCommandDialog.cpp" line="29" />
        <source>Quit Modbus-Tools before running step 3. Paste each block into a terminal, in order.</source>
        <translation>執行第 3 步前請先結束 Modbus-Tools。請按順序將各段命令貼上到終端機執行。</translation>
    </message>
    <message>
        <location filename="../UpdateCommandDialog.cpp" line="38" />
        <location filename="../UpdateCommandDialog.cpp" line="62" />
        <source>Copy Commands</source>
        <translation>複製命令</translation>
    </message>
    <message>
        <location filename="../UpdateCommandDialog.cpp" line="39" />
        <source>Open Download Page</source>
        <translation>開啟下載頁面</translation>
    </message>
    <message>
        <location filename="../UpdateCommandDialog.cpp" line="40" />
        <source>Close</source>
        <translation>關閉</translation>
    </message>
    <message>
        <location filename="../UpdateCommandDialog.cpp" line="59" />
        <source>Copied!</source>
        <translation>已複製！</translation>
    </message>
</context>
<context>
    <name>ui::UpdateInteractionView</name>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="34" />
        <source>About</source>
        <translation>關於</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="37" />
        <source>Check for Updates</source>
        <translation>檢查更新</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="56" />
        <location filename="../UpdateInteractionView.cpp" line="72" />
        <source>Update Available</source>
        <translation>發現新版本</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="57" />
        <source>New version v%1 available. Open download page?</source>
        <translation>發現新版本 v%1。是否開啟下載頁面？</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="73" />
        <source>Current: v%1, Latest: v%2
Choose update method:</source>
        <translation>目前版本：v%1，最新版本：v%2
請選擇更新方式：</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="74" />
        <source>Update Main Program</source>
        <translation>更新主程式</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="75" />
        <source>Download Full Package</source>
        <translation>下載完整安裝包</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="92" />
        <source>Downloading Update...</source>
        <translation>正在下載更新...</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="92" />
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
</context>
<context>
    <name>ui::application::UpdateCoordinator</name>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="158" />
        <source>No Updates</source>
        <translation>沒有可用更新</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="159" />
        <source>You are using the latest version: v%1</source>
        <translation>您正在使用最新版本：v%1</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="169" />
        <source>Update Check Failed</source>
        <translation>更新檢查失敗</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="184" />
        <source>Update service unavailable</source>
        <translation>Update service unavailable</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="188" />
        <location filename="../application/UpdateCoordinator.cpp" line="197" />
        <source>Update Failed</source>
        <translation>更新失敗</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="232" />
        <source>Automatic Update Unsupported</source>
        <translation>Automatic Update Unsupported</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="233" />
        <source>In-app automatic update is not supported on %1. Download the latest package instead.</source>
        <translation>In-app automatic update is not supported on %1. Download the latest package instead.</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::ModbusPagePresenter</name>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="215" />
        <source>Request rejected: connection in progress, please wait.</source>
        <translation>請求已拒絕：正在建立連線，請稍候。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="217" />
        <source>Request rejected: disconnecting in progress.</source>
        <translation>請求已拒絕：正在中斷連線。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="219" />
        <source>Request rejected: device not connected.</source>
        <translation>請求已拒绝：裝置未連線。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="254" />
        <source>Device is establishing session. Read commands available; write commands locked until ready.</source>
        <translation>裝置正在建立工作階段。讀取指令可用；寫入指令將在就緒後解鎖。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="280" />
        <source>Error: Invalid raw frame format</source>
        <translation>錯誤：無效的原始幀格式</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="359" />
        <source>Broadcast write polling is strictly forbidden.</source>
        <translation>嚴禁廣播寫入輪詢。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="365" />
        <source>Write polling rejected: confirmation required.</source>
        <translation>寫入輪詢已拒絕：需要確認。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="509" />
        <source>Request timed out</source>
        <translation>請求超時</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="511" />
        <source>Not connected</source>
        <translation>未連接</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="513" />
        <source>Connection failed</source>
        <translation>連接失敗</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="515" />
        <source>Request already in progress</source>
        <translation type="unfinished">Request already in progress</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="517" />
        <source>Invalid response frame</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="519" />
        <source>Invalid PDU parameters</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="521" />
        <source>CRC checksum error</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="523" />
        <source>LRC checksum error</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="525" />
        <source>Modbus exception: %1 (0x%2)</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="529" />
        <source>Failed to send data</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="531" />
        <source>Request aborted</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="533" />
        <source>Too many invalid response bytes</source>
        <translation type="unfinished">Too many invalid response bytes</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="537" />
        <source>Unknown error</source>
        <translation type="unfinished">未知錯誤</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="249" />
        <source>Connecting — please wait before sending commands.</source>
        <translation>正在連線 — 發送指令前請稍候。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="251" />
        <source>Disconnecting...</source>
        <translation>正在中斷連線...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="257" />
        <source>Connect device to send commands.</source>
        <translation>請先連線裝置後再發送指令。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="307" />
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::ModbusSessionPresenter</name>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="59" />
        <source>Connecting to %1:%2...</source>
        <translation>正在連線 %1:%2...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="118" />
        <source>Opening %1...</source>
        <translation>正在開啟 %1...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="161" />
        <source>Disconnecting...</source>
        <translation>Disconnecting...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="183" />
        <source>Shutdown timed out; restart recommended</source>
        <translation>Shutdown timed out; restart recommended</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="189" />
        <source>Release timed out; restart recommended</source>
        <translation>Release timed out; restart recommended</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="218" />
        <source>Releasing Modbus stack...</source>
        <translation>Releasing Modbus stack...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="240" />
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="402" />
        <source>Release completed</source>
        <translation>Release completed</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="592" />
        <source>Transport connected, validating session...</source>
        <translation>Transport connected, validating session...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="621" />
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="622" />
        <source>Disconnected: %1</source>
        <translation>Disconnected: %1</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="676" />
        <source>Transport connected, waiting for device response...</source>
        <translation>Transport connected, waiting for device response...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="66" />
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="126" />
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="446" />
        <source>Failed to create Modbus stack</source>
        <translation>建立 Modbus 堆疊失敗</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="667" />
        <source>Connection failed: %1</source>
        <translation>連線失敗：%1</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::PollingController</name>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="124" />
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="165" />
        <source>Poll recovered after %1 consecutive failures</source>
        <translation>Poll recovered after %1 consecutive failures</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="219" />
        <source>Poll Error: Connection unavailable during polling (%1)</source>
        <translation>Poll Error: Connection unavailable during polling (%1)</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="222" />
        <source>Poll Error escalated after %1 consecutive failures: %2</source>
        <translation>Poll Error escalated after %1 consecutive failures: %2</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="226" />
        <source>Poll Error persists (%1 consecutive failures): %2</source>
        <translation>Poll Error persists (%1 consecutive failures): %2</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="245" />
        <source>Poll Warning: %1 consecutive failure(s): %2</source>
        <translation>Poll Warning: %1 consecutive failure(s): %2</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="293" />
        <source>Connection unavailable for %1 s; self-healing window expired</source>
        <translation>Connection unavailable for %1 s; self-healing window expired</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::TrafficLogController</name>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="74" />
        <source>Success: Response received</source>
        <translation>成功：已收到回應</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="82" />
        <source>Success: Write confirmed</source>
        <translation>成功：寫入已確認</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="91" />
        <source>Success: Broadcast write sent, no response expected</source>
        <translation>成功：廣播寫已發送，預期無回應</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="108" />
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation>發送讀取請求 FC:%1 位址:%2 數量:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="119" />
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation>發送寫入請求 FC:%1 位址:%2 資料:%3 從站:%4</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="127" />
        <source>Sending Raw Data: %1</source>
        <translation>發送原始資料：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="158" />
        <source>Poll Summary: %1 ok, %2 err, %3 retry, avg RTT %4 ms</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="165" />
        <source> (raw frames: %1 shown, %2 sampled)</source>
        <translation type="unfinished" />
    </message>
</context>
<context>
    <name>ui::common::UpdateChecker</name>
    <message>
        <location filename="../common/UpdateChecker.cpp" line="89" />
        <source>Release tag is missing</source>
        <translation>發布標籤缺失</translation>
    </message>
</context>
<context>
    <name>ui::shell::NavigationController</name>
    <message>
        <location filename="../shell/NavigationController.cpp" line="145" />
        <source>Expand Navigation</source>
        <translation>展開導覽</translation>
    </message>
    <message>
        <location filename="../shell/NavigationController.cpp" line="146" />
        <source>Collapse Navigation</source>
        <translation>摺疊導覽</translation>
    </message>
</context>
<context>
    <name>ui::views::analyzer::FrameAnalyzerView</name>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="174" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1102" />
        <source>Frame Input</source>
        <translation>報文輸入</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="180" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1103" />
        <source>Protocol:</source>
        <translation>通訊協定：</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="183" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1105" />
        <source>Auto Detect</source>
        <translation>自動偵測</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="184" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1106" />
        <source>Modbus TCP</source>
        <translation>Modbus TCP</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="185" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1107" />
        <source>Modbus RTU</source>
        <translation>Modbus RTU</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="186" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1108" />
        <source>Modbus ASCII</source>
        <translation>Modbus ASCII</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="226" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1163" />
        <source>Paste &amp; Parse</source>
        <translation>貼上並解析</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1164" />
        <source>Paste clipboard content and parse immediately (Ctrl+Shift+V)</source>
        <translation>貼上剪貼簿內容並立即解析 (Ctrl+Shift+V)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1160" />
        <source>Parse current input (Ctrl+Enter)</source>
        <translation>解析目前輸入 (Ctrl+Enter)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="528" />
        <source>Clipboard is empty</source>
        <translation>剪貼簿為空</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="219" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1159" />
        <source>Parse</source>
        <translation>解析</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="213" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1157" />
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="240" />
        <source>Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)</source>
        <translation>輸入十六進位字串（如 01 03 00 00 00 01 84 0A）</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1168" />
        <source>Enter Hex string (e.g., RTU: 01 03 00 00 00 01 84 0A, ASCII bytes: 3A 30 31 30 33 ... 0D 0A)</source>
        <translation>輸入十六進位字串（如 RTU: 01 03 00 00 00 01 84 0A，ASCII 位元組: 3A 30 31 30 33 ... 0D 0A）</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="251" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1171" />
        <source>Analysis Result</source>
        <translation>分析結果</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="267" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1140" />
        <source>Status:</source>
        <translation>狀態：</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="271" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="964" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1153" />
        <source>Ready</source>
        <translation>就緒</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="280" />
        <source>Tip: "Pause" to edit description</source>
        <translation>提示："暫停"以編輯描述</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="297" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="975" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1040" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1138" />
        <source>Pause Refresh</source>
        <translation>暫停重新整理</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1040" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1138" />
        <source>Resume Refresh</source>
        <translation>恢復重新整理</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="305" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1136" />
        <source>Stop Link</source>
        <translation>停止連動</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="639" />
        <source>Import Config</source>
        <translation>匯入設定</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="622" />
        <source>Export Config</source>
        <translation>匯出設定</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="402" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1179" />
        <source>Field</source>
        <translation>欄位</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="402" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1180" />
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="402" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1181" />
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="406" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="800" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="987" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1174" />
        <source>Structure</source>
        <translation>結構</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1112" />
        <source>Start Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>起始位址 (0-65535)。支援 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="418" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="470" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1172" />
        <source>History</source>
        <translation>歷史記錄</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="422" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1186" />
        <source>Clear History</source>
        <translation>清除歷史</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="622" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="639" />
        <source>JSON Files (*.json)</source>
        <translation>JSON 檔案 (*.json)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="633" />
        <source>Export Failed</source>
        <translation>匯出失敗</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="486" />
        <source>OK</source>
        <translation>確定</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="486" />
        <source>ERR</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="413" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1175" />
        <source>Decoded Data</source>
        <translation>解析資料</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="574" />
        <source>Invalid Address (0-65535): %1</source>
        <translation>無效位址 (0-65535): %1</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="603" />
        <source>Parse Failed</source>
        <translation>解析失敗</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="644" />
        <source>Import Failed</source>
        <translation>匯入失敗</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1014" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1147" />
        <source>LIVE: %1</source>
        <translation>連動中: %1</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1018" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1149" />
        <source>Live Data Received at %1</source>
        <translation>即時資料接收於 %1</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="566" />
        <source>Error: Empty input</source>
        <translation>錯誤：輸入為空</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="192" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1110" />
        <source>Start Addr:</source>
        <translation type="unfinished">起始位址：</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="193" />
        <source>Start Address (for Response)</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="207" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1156" />
        <source>Format</source>
        <translation>格式化</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="208" />
        <source>Format Hex String (Ctrl+Shift+F)</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="214" />
        <source>Clear Input and Results</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="220" />
        <source>Parse Input (Ctrl+Enter)</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="227" />
        <source>Paste Clipboard and Parse (Ctrl+Shift+V)</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="317" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1114" />
        <source>Decode:</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="318" />
        <source>Decode Mode</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="320" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1116" />
        <source>UInt16</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="321" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1117" />
        <source>Int16</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="322" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1118" />
        <source>Float32</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="323" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1119" />
        <source>Int32</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="324" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1120" />
        <source>UInt32</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="325" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1121" />
        <source>Float64</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="339" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1123" />
        <source>Order:</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="340" />
        <source>Byte Order</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="342" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1125" />
        <source>ABCD</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="359" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="690" />
        <source>Reset</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="371" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1130" />
        <source>Import</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="372" />
        <source>Import Register Config (JSON)</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="374" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1131" />
        <source>Export</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="375" />
        <source>Export Register Config (JSON)</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="377" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1132" />
        <source>CSV</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="378" />
        <source>Export Decoded Data to CSV</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="470" />
        <source>History ◂</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="580" />
        <source>Parsing...</source>
        <translation>解析中...</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="686" />
        <source>Reset (%1)</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="750" />
        <source>Parse Failed: %1</source>
        <translation>解析失敗：%1</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="756" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1143" />
        <source>TCP</source>
        <translation>TCP</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="757" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1144" />
        <source>RTU</source>
        <translation>RTU</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="806" />
        <source>Frame</source>
        <translation>幀</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="807" />
        <source>%1 bytes</source>
        <translation>%1 位元組</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="815" />
        <source>MBAP Header</source>
        <translation>MBAP 標頭</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="820" />
        <source>Transaction ID</source>
        <translation>交易識別碼</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="825" />
        <source>Protocol ID</source>
        <translation>協定識別碼</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="830" />
        <source>Length</source>
        <translation>長度</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="835" />
        <source>Unit ID</source>
        <translation>單元識別碼</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="841" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="857" />
        <source>Slave ID</source>
        <translation>從站識別碼</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="878" />
        <source>PDU</source>
        <translation>PDU</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="884" />
        <source>Function Code</source>
        <translation>功能碼</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="896" />
        <source>Exception Code</source>
        <translation>例外碼</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="911" />
        <source>CRC16</source>
        <translation>CRC16</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="471" />
        <source>Show History</source>
        <translation>顯示歷史</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="471" />
        <source>Hide History</source>
        <translation>隱藏歷史</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="493" />
        <source>Local time %1</source>
        <translation>Local time %1</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="362" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="691" />
        <source>Reset all custom register types to Default</source>
        <translation>重設所有自訂暫存器類型為預設</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="687" />
        <source>Reset selected %1 register type(s) to Default (Delete)</source>
        <translation>重設所選 %1 個暫存器類型為預設 (Delete)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="758" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1145" />
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="801" />
        <source>(Unavailable in Live Mode)</source>
        <translation>(Unavailable in Live Mode)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="802" />
        <source>Logical parsing is disabled for high-frequency linkage</source>
        <translation>高頻聯動下已停用邏輯解析</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="810" />
        <source>Frame Bytes</source>
        <translation>幀位元組</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="810" />
        <source>Complete raw frame</source>
        <translation>完整原始幀</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="817" />
        <source>Transaction + Protocol + Length + Unit ID</source>
        <translation>交易 + 協定 + 長度 + 單元識別碼</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="822" />
        <source>Request/response correlation ID</source>
        <translation>請求/回應關聯識別碼</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="827" />
        <source>Modbus TCP protocol identifier</source>
        <translation>Modbus TCP 協定識別碼</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="832" />
        <source>Remaining bytes after this field</source>
        <translation>此欄位之後的剩餘位元組數</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="837" />
        <source>Target slave / unit address</source>
        <translation>目標從站 / 單元位址</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="843" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="859" />
        <source>Target slave address</source>
        <translation>目標從站位址</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="846" />
        <source>ASCII Start</source>
        <translation>ASCII Start</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="848" />
        <source>Start delimiter ':'</source>
        <translation>Start delimiter ':'</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="851" />
        <source>ASCII Payload</source>
        <translation>ASCII Payload</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="853" />
        <source>ASCII hex payload before CRLF</source>
        <translation>ASCII hex payload before CRLF</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="880" />
        <source>Function code + payload</source>
        <translation>功能碼 + 負載</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="886" />
        <source>Normal response</source>
        <translation>正常回應</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="886" />
        <source>Exception response</source>
        <translation>例外回應</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="889" />
        <source>Payload</source>
        <translation>負載</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="891" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="898" />
        <source>Exception detail payload</source>
        <translation>例外詳細負載</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="891" />
        <source>Application data payload</source>
        <translation>應用資料負載</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="902" />
        <source>CRC valid</source>
        <translation>CRC 有效</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="902" />
        <source>CRC invalid</source>
        <translation>CRC 無效</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="906" />
        <source>Expected 0x%1</source>
        <translation>預期 0x%1</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="920" />
        <source>LRC</source>
        <translation>LRC</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="924" />
        <source>LRC valid</source>
        <translation>LRC valid</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="924" />
        <source>LRC invalid</source>
        <translation>LRC invalid</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="927" />
        <source>CRLF</source>
        <translation>CRLF</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="929" />
        <source>ASCII frame terminator</source>
        <translation>ASCII frame terminator</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="949" />
        <source>Success (%1)</source>
        <translation>成功（%1）</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="951" />
        <source>Forced Parsing</source>
        <translation>強制解析</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="954" />
        <source>Warnings</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="994" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1174" />
        <source>Structure (Unavailable in Live Mode)</source>
        <translation>結構（聯動模式不可用）</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="761" />
        <source>Request</source>
        <translation>請求</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="762" />
        <source>Response</source>
        <translation>回應</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="763" />
        <source>Exception</source>
        <translation>例外</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="759" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="764" />
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1146" />
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
</context>
<context>
    <name>ui::views::converter::Ieee754ConverterWidget</name>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="60" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="230" />
        <source>Precision</source>
        <translation>精度</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="66" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="231" />
        <source>Float32 (32-bit, 2 Regs)</source>
        <translation>Float32 (單精度 32位元 / 2暫存器)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="67" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="232" />
        <source>Float64 (64-bit, 4 Regs)</source>
        <translation>Float64 (雙精度 64位元 / 4暫存器)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="80" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="233" />
        <source>Quick Presets</source>
        <translation>快捷預設</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="111" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="234" />
        <source>Max Normal</source>
        <translation>最大規格化數</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="112" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="235" />
        <source>+Infinity</source>
        <translation>+無限大</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="113" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="236" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="701" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="749" />
        <source>NaN</source>
        <translation>非數 (NaN)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="122" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="237" />
        <source>Interactive Inputs (Two-Way Live Sync)</source>
        <translation>互動輸入區 (雙向即時聯動)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="128" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="238" />
        <source>Floating Point:</source>
        <translation>浮點數值:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="130" />
        <source>e.g. 123.456, -1.0, 1e-5, NaN, Inf</source>
        <translation>例如 123.456, -1.0, 1e-5, NaN, Inf</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="134" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="239" />
        <source>Hex (Big-Endian):</source>
        <translation>十六進制 (大端序):</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="136" />
        <source>e.g. 42 F6 E9 79 or 42F6E979</source>
        <translation>例如 42 F6 E9 79 或 42F6E979</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="140" />
        <source>Signed Integer:</source>
        <translation>有符號整數:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="145" />
        <source>Unsigned Integer:</source>
        <translation>無符號整數:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="152" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="242" />
        <source>Editing any field instantly derives all others in real-time.</source>
        <translation>編輯任意輸入欄位，其他欄位與位元組序矩陣均即時自動推導。</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="163" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="243" />
        <source>Four-Endian Live Matrix (Modbus Permutations)</source>
        <translation>四種 Modbus 位元組序換算矩陣</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="182" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="244" />
        <source>IEEE 754 Binary Bitfield Breakdown</source>
        <translation>IEEE 754 二進制位元欄位剖析</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="240" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="286" />
        <source>Signed Int32:</source>
        <translation>有符號 Int32:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="240" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="286" />
        <source>Signed Int64:</source>
        <translation>有符號 Int64:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="241" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="287" />
        <source>Unsigned UInt32:</source>
        <translation>無符號 UInt32:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="241" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="287" />
        <source>Unsigned UInt64:</source>
        <translation>無符號 UInt64:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="248" />
        <source>Endian Order</source>
        <translation>位元組序模式</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="249" />
        <source>Raw Hex</source>
        <translation>原始十六進制</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="250" />
        <source>Modbus Register Words</source>
        <translation>Modbus 暫存器字</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="251" />
        <source>Floating Value</source>
        <translation>浮點解析值</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="252" />
        <source>Signed Int</source>
        <translation>有符號整數</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="253" />
        <source>Unsigned Int</source>
        <translation>無符號整數</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="254" />
        <source>Actions</source>
        <translation>操作</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="554" />
        <source>Big-Endian (Standard / High-Word First)</source>
        <translation>大端序 (標準 / 高字在前)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="554" />
        <source>Big-Endian</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="555" />
        <source>Word Swap (Little-Endian Words / Middle-Endian)</source>
        <translation>字顛倒 (低字在前 / 中端序)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="555" />
        <source>Word Swap</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="556" />
        <source>Byte Swap (Word-Level Endian Inversion)</source>
        <translation>位元組顛倒 (字內位元組反轉)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="556" />
        <source>Byte Swap</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="557" />
        <source>Little-Endian (True Little-Endian / Low-Word First)</source>
        <translation>小端序 (完全小端 / 低字在前)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="557" />
        <source>Little-Endian</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="655" />
        <source>Copy Hex</source>
        <translation>複製 Hex</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="658" />
        <source>Copy Hex formatted bytes</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="662" />
        <source>Copy Words</source>
        <translation>複製暫存器字</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="665" />
        <source>Copy Modbus Register Words</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="696" />
        <source>Sign [Bit 31]: %1 (%2)</source>
        <translation>符號位 [Bit 31]: %1 (%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="696" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="745" />
        <source>Positive +</source>
        <translation>正數 (+)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="696" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="745" />
        <source>Negative -</source>
        <translation>負數 (-)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="700" />
        <source>Exp [Bits 30-23]: 0xFF (Special: %1)</source>
        <translation>階碼 [Bits 30-23]: 0xFF (特殊值: %1)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="701" />
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="749" />
        <source>Infinity</source>
        <translation>無限大</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="703" />
        <source>Exp [Bits 30-23]: 0 (Subnormal, 2^-126)</source>
        <translation>階碼 [Bits 30-23]: 0 (非規格化數, 2^-126)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="706" />
        <source>Exp [Bits 30-23]: %1 (Actual: %1 - 127 = %2, 2^%2)</source>
        <translation>階碼 [Bits 30-23]: %1 (真實指數: %1 - 127 = %2, 2^%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="718" />
        <source>Mantissa [Bits 22-0]: %1 (Significand: %2)</source>
        <translation>尾數 [Bits 22-0]: %1 (有效數: %2)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="725" />
        <source>Formula: (-1)^%1 × 2^(%2 - 127) × (1 + 0x%3/2^23) = %4</source>
        <translation>計算公式: (-1)^%1 × 2^(%2 - 127) × (1 + 0x%3/2^23) = %4</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="745" />
        <source>Sign [Bit 63]: %1 (%2)</source>
        <translation>符號位 [Bit 63]: %1 (%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="748" />
        <source>Exp [Bits 62-52]: 0x7FF (Special: %1)</source>
        <translation>階碼 [Bits 62-52]: 0x7FF (特殊值: %1)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="751" />
        <source>Exp [Bits 62-52]: 0 (Subnormal, 2^-1022)</source>
        <translation>階碼 [Bits 62-52]: 0 (非規格化數, 2^-1022)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="754" />
        <source>Exp [Bits 62-52]: %1 (Actual: %1 - 1023 = %2, 2^%2)</source>
        <translation>階碼 [Bits 62-52]: %1 (真實指數: %1 - 1023 = %2, 2^%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="765" />
        <source>Mantissa [Bits 51-0]: %1 (Significand: %2)</source>
        <translation>尾數 [Bits 51-0]: %1 (有效數: %2)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="771" />
        <source>Formula: (-1)^%1 × 2^(%2 - 1023) × (1 + Frac) = %3</source>
        <translation>計算公式: (-1)^%1 × 2^(%2 - 1023) × (1 + Frac) = %3</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="783" />
        <source>Copied Hex: %1</source>
        <translation>已複製十六進制: %1</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="792" />
        <source>Copied Register Words: %1</source>
        <translation>已複製暫存器字: %1</translation>
    </message>
</context>
<context>
    <name>ui::views::converter::ModbusFrameBuilderWidget</name>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="540" />
        <source>Invalid coil value. Use ON/OFF, 1/0, or FF00/0000 in the selected format.</source>
        <translation>線圈值無效。請輸入 ON/OFF、1/0，或所選格式下的 FF00/0000。</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="549" />
        <source>Invalid register data or count. Use Hex bytes, Decimal values (0-65535), or 16 MSB-first bits per register.</source>
        <translation>暫存器資料或數量無效。請輸入十六進位位元組、十進位數值（0-65535），或每個暫存器 16 位高位在前的二進位資料。</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="574" />
        <source>Invalid coil data or count. Use packed Hex bytes or one Decimal/Binary 0 or 1 per coil.</source>
        <translation>線圈資料或數量無效。請輸入打包的十六進位位元組，或為每個線圈輸入一個十進位/二進位 0 或 1。</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="604" />
        <source>Address range exceeds 65535.</source>
        <translation>位址範圍超出 65535。</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="83" />
        <source>Read Coils (0x01)</source>
        <translation>讀線圈 (0x01)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="85" />
        <source>Read Discrete Inputs (0x02)</source>
        <translation>讀離散輸入 (0x02)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="87" />
        <source>Read Holding Registers (0x03)</source>
        <translation>讀保持暫存器 (0x03)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="89" />
        <source>Read Input Registers (0x04)</source>
        <translation>讀輸入暫存器 (0x04)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="91" />
        <source>Write Single Coil (0x05)</source>
        <translation>寫單線圈 (0x05)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="93" />
        <source>Write Single Register (0x06)</source>
        <translation>寫單暫存器 (0x06)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="95" />
        <source>Write Multiple Coils (0x0F)</source>
        <translation>寫多線圈 (0x0F)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="97" />
        <source>Write Multiple Registers (0x10)</source>
        <translation>寫多暫存器 (0x10)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="138" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="330" />
        <source>Frame Parameters</source>
        <translation>報文參數</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="144" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="331" />
        <source>Protocol:</source>
        <translation>通訊協定:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="152" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="401" />
        <source>Slave ID:</source>
        <translation>從機位址:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="155" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="404" />
        <source>1 - 247</source>
        <translation>1 - 247</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="160" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="332" />
        <source>Function Code:</source>
        <translation>功能碼:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="181" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="334" />
        <source>Address Base:</source>
        <translation>位址基準:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="183" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="352" />
        <source>0-Based (PDU: 0-65535)</source>
        <translation>0 基準 (PDU: 0-65535)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="184" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="353" />
        <source>1-Based (PLC: 1-65536)</source>
        <translation>1 基準 (PLC: 1-65536)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="189" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="335" />
        <source>Start Address:</source>
        <translation>起始位址:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="192" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="333" />
        <source>e.g. 0, 0x0000, 40001</source>
        <translation>例如 0, 0x0000, 40001</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="196" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="336" />
        <source>Quantity:</source>
        <translation>數量:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="207" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="337" />
        <source>Write Data:</source>
        <translation>寫入資料:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="209" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="450" />
        <source>e.g. 0001 0002 or 1, 2</source>
        <translation>例如 0001 0002 或 1, 2</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="213" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="338" />
        <source>Data Format:</source>
        <translation>資料格式:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="224" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="339" />
        <source>Generated Frame</source>
        <translation>生成報文</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="231" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="340" />
        <source>Full Frame Hex:</source>
        <translation>完整報文 Hex:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="250" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="347" />
        <source>Field</source>
        <translation>欄位</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="250" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="347" />
        <source>Hex</source>
        <translation>Hex</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="250" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="347" />
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="264" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="341" />
        <source>Copy Hex (spaced)</source>
        <translation>複製 Hex (空格分隔)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="265" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="342" />
        <source>Copy Hex (compact)</source>
        <translation>複製 Hex (緊湊)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="266" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="343" />
        <source>Copy C array</source>
        <translation>複製 C 陣列</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="267" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="344" />
        <source>Inspect in Frame Analyzer</source>
        <translation>在報文分析器中驗證</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="401" />
        <source>Unit ID:</source>
        <translation>單元識別碼:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="404" />
        <source>1 - 255</source>
        <translation>1 - 255</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="444" />
        <source>e.g. ON, OFF, 1, 0, or FF00</source>
        <translation>例如 ON, OFF, 1, 0, 或 FF00</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="446" />
        <source>e.g. 1234 or 0x1234</source>
        <translation>例如 1234 或 0x1234</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="448" />
        <source>e.g. 1 0 1 1 (matching quantity)</source>
        <translation>例如 1 0 1 1 (與數量相符)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="472" />
        <source>Invalid %1 (%2). Valid range: 1 - %3.</source>
        <translation>無效的 %1 (%2)。有效範圍: 1 - %3。</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="473" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="726" />
        <source>Unit ID</source>
        <translation>單元識別碼</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="473" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="655" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="785" />
        <source>Slave ID</source>
        <translation>從機位址</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="494" />
        <source>Invalid start address: %1</source>
        <translation>無效的起始位址: %1</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="588" />
        <source>Failed to build Modbus PDU.</source>
        <translation>建構 Modbus PDU 失敗。</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="657" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="787" />
        <source>Device address %1 (0x%2)</source>
        <translation>設備位址 %1 (0x%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="660" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="731" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="789" />
        <source>Function Code</source>
        <translation>功能碼</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="669" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="693" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="740" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="764" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="797" />
        <source>Start Address</source>
        <translation>起始位址</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="671" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="678" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="686" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="695" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="742" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="749" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="757" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="766" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="799" />
        <source>Address %1 (0x%2)</source>
        <translation>位址 %1 (0x%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="672" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="696" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="743" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="767" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="800" />
        <source>Quantity</source>
        <translation>數量</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="674" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="745" />
        <source>%1 %2 (0x%3)</source>
        <translation>%1 %2 (0x%3)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="674" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="698" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="745" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="769" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="802" />
        <source>coils</source>
        <translation>線圈</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="674" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="698" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="745" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="769" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="802" />
        <source>registers</source>
        <translation>暫存器</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="676" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="747" />
        <source>Coil Address</source>
        <translation>線圈位址</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="680" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="751" />
        <source>Output Value</source>
        <translation>輸出值</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="682" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="753" />
        <source>Coil ON (0xFF00)</source>
        <translation>線圈接通 (0xFF00)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="682" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="753" />
        <source>Coil OFF (0x0000)</source>
        <translation>線圈斷開 (0x0000)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="684" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="755" />
        <source>Register Address</source>
        <translation>暫存器位址</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="688" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="759" />
        <source>Register Value</source>
        <translation>暫存器數值</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="690" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="761" />
        <source>Value: %1 (0x%2)</source>
        <translation>數值: %1 (0x%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="698" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="769" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="802" />
        <source>%1 %2</source>
        <translation>%1 %2</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="699" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="770" />
        <source>Byte Count</source>
        <translation>位元組數</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="701" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="772" />
        <source>%1 bytes</source>
        <translation>%1 位元組</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="703" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="774" />
        <source>Write Payload</source>
        <translation>寫入負載</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="705" />
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="776" />
        <source>Packed output data (%1 bytes)</source>
        <translation>封裝輸出資料 (%1 位元組)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="709" />
        <source>CRC-16</source>
        <translation>CRC-16</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="711" />
        <source>CRC-16/MODBUS (LE: Low byte first)</source>
        <translation>CRC-16/MODBUS (小端: 低位元組在前)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="716" />
        <source>Transaction ID</source>
        <translation>事務 ID</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="718" />
        <source>Transaction Identifier (0x0000)</source>
        <translation>事務識別碼 (0x0000)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="719" />
        <source>Protocol ID</source>
        <translation>通訊協定 ID</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="721" />
        <source>0x0000 = Modbus Protocol</source>
        <translation>0x0000 = Modbus 通訊協定</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="723" />
        <source>Length</source>
        <translation>長度</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="725" />
        <source>%1 bytes following</source>
        <translation>後續 %1 位元組</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="728" />
        <source>Unit identifier %1 (0x%2)</source>
        <translation>單元識別碼 %1 (0x%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="781" />
        <source>Start Delimiter</source>
        <translation>起始定界符</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="783" />
        <source>ASCII Colon delimiter</source>
        <translation>ASCII 冒號定界符</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="805" />
        <source>PDU Payload</source>
        <translation>PDU 負載</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="807" />
        <source>ASCII encoded PDU data</source>
        <translation>ASCII 編碼 PDU 資料</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="810" />
        <source>LRC Checksum</source>
        <translation>LRC 校驗和</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="812" />
        <source>Longitudinal Redundancy Check</source>
        <translation>縱向冗餘校驗 (LRC)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="814" />
        <source>End Delimiter</source>
        <translation>結束定界符</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="816" />
        <source>Carriage Return &amp; Line Feed</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="826" />
        <source>Spaced Hex copied to clipboard</source>
        <translation>已複製空格分隔 Hex 至剪貼簿</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="835" />
        <source>Compact Hex copied to clipboard</source>
        <translation>已複製緊湊 Hex 至剪貼簿</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="857" />
        <source>C array declaration copied to clipboard</source>
        <translation>已複製 C 陣列宣告至剪貼簿</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus::ModbusPage</name>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="137" />
        <location filename="../views/modbus/ModbusPage.cpp" line="320" />
        <source>0-Based</source>
        <translation>0-Based (偏移定址)</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="138" />
        <location filename="../views/modbus/ModbusPage.cpp" line="321" />
        <source>1-Based</source>
        <translation>1-Based (常規定址)</translation>
    </message>
</context>
<context>
    <name>ui::views::network::NetworkDebuggerView</name>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="298" />
        <source>Connecting to %1:%2...</source>
        <translation type="unfinished">正在連線 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="314" />
        <source>Starting TCP server on %1:%2...</source>
        <translation type="unfinished">Starting TCP server on %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="329" />
        <source>Stopping TCP server...</source>
        <translation type="unfinished">Stopping TCP server...</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="343" />
        <source>Binding UDP %1:%2 -&gt; %3:%4...</source>
        <translation type="unfinished">Binding UDP %1:%2 -&gt; %3:%4...</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="347" />
        <source>Binding UDP %1:%2...</source>
        <translation type="unfinished">Binding UDP %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="394" />
        <source>Server mode: no connected clients available</source>
        <translation type="unfinished">Server mode: no connected clients available</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="395" />
        <source>Server mode: select at least one client</source>
        <translation type="unfinished">Server mode: select at least one client</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="435" />
        <source>Closed</source>
        <translation type="unfinished">已關閉</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="436" />
        <source>Opening</source>
        <translation type="unfinished">正在開啟</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="437" />
        <source>Connected</source>
        <translation type="unfinished">已連線</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="438" />
        <source>Closing</source>
        <translation type="unfinished">正在關閉</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="439" />
        <location filename="../views/network/NetworkDebuggerView.cpp" line="550" />
        <source>Error</source>
        <translation type="unfinished">錯誤</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="440" />
        <location filename="../views/network/NetworkDebuggerView.cpp" line="551" />
        <source>Unknown</source>
        <translation type="unfinished">未知</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="478" />
        <source>State changed: %1</source>
        <translation type="unfinished">狀態已變更：%1</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="497" />
        <location filename="../views/network/NetworkDebuggerView.cpp" line="584" />
        <source>Connection failed</source>
        <translation>連接失敗</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="498" />
        <location filename="../views/network/NetworkDebuggerView.cpp" line="585" />
        <source>Connection timeout</source>
        <translation type="unfinished">連線逾時</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="499" />
        <location filename="../views/network/NetworkDebuggerView.cpp" line="586" />
        <source>Write failed</source>
        <translation type="unfinished">寫入失敗</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="500" />
        <location filename="../views/network/NetworkDebuggerView.cpp" line="587" />
        <source>Read failed</source>
        <translation type="unfinished">讀取失敗</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="501" />
        <location filename="../views/network/NetworkDebuggerView.cpp" line="588" />
        <source>Port not found</source>
        <translation type="unfinished">找不到連接埠</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="502" />
        <location filename="../views/network/NetworkDebuggerView.cpp" line="589" />
        <source>Permission denied</source>
        <translation type="unfinished">權限被拒絕</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="503" />
        <location filename="../views/network/NetworkDebuggerView.cpp" line="590" />
        <source>Connection reset by peer</source>
        <translation type="unfinished">連線被對端重設</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="504" />
        <location filename="../views/network/NetworkDebuggerView.cpp" line="591" />
        <source>Unknown error</source>
        <translation type="unfinished">未知錯誤</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="506" />
        <source>Error: %1</source>
        <translation type="unfinished">錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="523" />
        <source>Client #%1 connected: %2</source>
        <translation type="unfinished">Client #%1 connected: %2</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="530" />
        <source>Client #%1 disconnected</source>
        <translation type="unfinished">Client #%1 disconnected</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="548" />
        <source>Listening</source>
        <translation type="unfinished">Listening</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="549" />
        <source>Stopped</source>
        <translation type="unfinished">Stopped</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="576" />
        <source>Server state: %1</source>
        <translation type="unfinished">Server state: %1</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="593" />
        <source>Server Error: %1</source>
        <translation type="unfinished">Server Error: %1</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="627" />
        <source>Send Data</source>
        <translation type="unfinished">發送資料</translation>
    </message>
</context>
<context>
    <name>ui::views::network::Protocol</name>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="57" />
        <source>TCP Client</source>
        <translation type="unfinished">TCP 用戶端</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="58" />
        <source>TCP Server</source>
        <translation type="unfinished">TCP 伺服端</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="59" />
        <source>UDP</source>
        <translation type="unfinished">UDP</translation>
    </message>
</context>
<context>
    <name>ui::views::serial::SerialDebuggerView</name>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="119" />
        <source>Opening %1...</source>
        <translation>正在開啟 %1...</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="148" />
        <source>Closed</source>
        <translation>已關閉</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="149" />
        <source>Opening</source>
        <translation>正在開啟</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="150" />
        <source>Open</source>
        <translation>已開啟</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="151" />
        <source>Closing</source>
        <translation>正在關閉</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="152" />
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="153" />
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="173" />
        <source>State changed: %1</source>
        <translation>狀態已變更：%1</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="185" />
        <source>Connection failed</source>
        <translation>連接失敗</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="186" />
        <source>Connection timeout</source>
        <translation>連線逾時</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="187" />
        <source>Write failed</source>
        <translation>寫入失敗</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="188" />
        <source>Read failed</source>
        <translation>讀取失敗</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="189" />
        <source>Port not found</source>
        <translation>找不到連接埠</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="190" />
        <source>Permission denied</source>
        <translation>權限被拒絕</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="191" />
        <source>Connection reset</source>
        <translation>連線重設</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="192" />
        <source>Unknown error</source>
        <translation>未知錯誤</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="195" />
        <source>Error: %1</source>
        <translation>錯誤：%1</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="230" />
        <source>Control</source>
        <translation>控制</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="231" />
        <source>Send Data</source>
        <translation>發送資料</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="232" />
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="233" />
        <source>RTS</source>
        <translation>RTS</translation>
    </message>
</context>
<context>
    <name>ui::views::tools::UtilitiesView</name>
    <message>
        <location filename="../views/tools/UtilitiesView.cpp" line="28" />
        <location filename="../views/tools/UtilitiesView.cpp" line="48" />
        <source>IEEE 754 Converter</source>
        <translation>IEEE 754 轉換器</translation>
    </message>
    <message>
        <location filename="../views/tools/UtilitiesView.cpp" line="31" />
        <location filename="../views/tools/UtilitiesView.cpp" line="49" />
        <source>Frame Builder</source>
        <translation>報文建構器</translation>
    </message>
</context>
<context>
    <name>ui::widgets::BaseConnectionWidget</name>
    <message>
        <location filename="../widgets/BaseConnectionWidget.cpp" line="77" />
        <source>Connection Settings</source>
        <translation>連線設定</translation>
    </message>
</context>
<context>
    <name>ui::widgets::ByteMonitorWidget</name>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="163" />
        <source>Copy</source>
        <translation>複製</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="330" />
        <source>Cannot write file: %1 (%2)</source>
        <translation type="unfinished" />
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="382" />
        <source>[%1] [TX] %2</source>
        <translation>[%1] [TX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="387" />
        <source>[%1] [RX] %2</source>
        <translation>[%1] [RX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="391" />
        <source>[INFO] %1</source>
        <translation>[INFO] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="392" />
        <source>[%1] [INFO] %2</source>
        <translation>[%1] [資訊] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="401" />
        <source>[ERROR] %1</source>
        <translation>[ERROR] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="402" />
        <source>[%1] [ERROR] %2</source>
        <translation>[%1] [ERROR] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="396" />
        <source>[WARN] %1</source>
        <translation>[WARN] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="397" />
        <source>[%1] [WARN] %2</source>
        <translation>[%1] [警告] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="316" />
        <source>Text Files (*.txt);;All Files (*)</source>
        <translation>文字檔 (*.txt);;所有檔案 (*)</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="315" />
        <source>Save Log</source>
        <translation>儲存日誌</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="381" />
        <source>[TX] %1</source>
        <translation>[TX] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="386" />
        <source>[RX] %1</source>
        <translation>[RX] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="456" />
        <source>TX: %1</source>
        <translation>TX: %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="459" />
        <source>RX: %1</source>
        <translation>RX: %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="552" />
        <source>HEX</source>
        <translation>十六進制 (HEX)</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="555" />
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="561" />
        <source>Absolute</source>
        <translation>Absolute</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="562" />
        <source>Relative</source>
        <translation>Relative</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="563" />
        <source>None</source>
        <translation>無</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="566" />
        <source>TX</source>
        <translation>發送</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="567" />
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="568" />
        <source>Auto Scroll</source>
        <translation>自動捲動</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="570" />
        <source>Resume</source>
        <translation>Resume</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="570" />
        <source>Pause</source>
        <translation>暫停</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="572" />
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="573" />
        <source>Save</source>
        <translation>儲存</translation>
    </message>
</context>
<context>
    <name>ui::widgets::CollapsibleSection</name>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="94" />
        <source>Collapse</source>
        <translation>收合</translation>
    </message>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="94" />
        <source>Expand</source>
        <translation>展開</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FrameDecodedTableView</name>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="57" />
        <source>Address</source>
        <translation>位址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="58" />
        <source>Hex</source>
        <translation>16進制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="59" />
        <source>Decimal</source>
        <translation>十進制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="60" />
        <source>Binary</source>
        <translation>二進制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="61" />
        <source>Type</source>
        <translation>資料類型</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="62" />
        <source>Scale</source>
        <translation>縮放比例</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="63" />
        <source>Value</source>
        <translation>數值</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="64" />
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="182" />
        <source>[%1 Low-Word]</source>
        <translation>[%1 低字]</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="183" />
        <source>[%1 W%2]</source>
        <translation>[%1 字%2]</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="203" />
        <source>(Subordinate word of address %1)</source>
        <translation>(位址 %1 的從屬字)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="233" />
        <source>&lt;Incomplete&gt;</source>
        <translation>&lt;資料不完整&gt;</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="234" />
        <source>Incomplete register bytes for %1</source>
        <translation>暫存器位元組不足以解析 %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="251" />
        <source>Default (%1)</source>
        <translation>預設 (%1)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="461" />
        <source>Reset Selected Type to Default (%1)	Delete</source>
        <translation>重設所選類型為預設 (%1)	Delete</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="462" />
        <source>Reset Selected Type to Default	Delete</source>
        <translation>重設所選類型為預設	Delete</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="468" />
        <source>Batch Set Selected Type to...</source>
        <translation>批次設定所選類型為...</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="478" />
        <source>UInt16 (1 Reg)</source>
        <translation>UInt16 (單暫存器 16位元)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="479" />
        <source>Int16 (1 Reg)</source>
        <translation>Int16 (單暫存器 16位元)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="480" />
        <source>Float32 (Real, 2 Regs)</source>
        <translation>Float32 (實數, 2暫存器)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="481" />
        <source>Int32 (DInt, 2 Regs)</source>
        <translation>Int32 (雙整數, 2暫存器)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="482" />
        <source>UInt32 (UDInt, 2 Regs)</source>
        <translation>UInt32 (無符號雙整數, 2暫存器)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="483" />
        <source>Float64 (Double, 4 Regs)</source>
        <translation>Float64 (雙精度, 4暫存器)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="487" />
        <source>Reset All Custom Types to Default</source>
        <translation>重設所有自訂類型為預設</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="514" />
        <source>No Data</source>
        <translation>無資料</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="514" />
        <source>There is no data to export.</source>
        <translation>目前沒有可匯出的資料。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="518" />
        <source>Export CSV</source>
        <translation>匯出 CSV</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="520" />
        <source>CSV Files (*.csv)</source>
        <translation>CSV 檔案 (*.csv)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="540" />
        <source>Export Failed</source>
        <translation>匯出失敗</translation>
    </message>
</context>
<context>
    <name>ui::widgets::GenericInputWidget</name>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="339" />
        <source>Enter data to send...</source>
        <translation>輸入要發送的資料...</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="342" />
        <source>HEX</source>
        <translation>十六進制 (HEX)</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="345" />
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="348" />
        <source>Auto Send</source>
        <translation>自動發送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="351" />
        <source> ms</source>
        <translation> 毫秒</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="354" />
        <source>Send</source>
        <translation>發送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="58" />
        <location filename="../widgets/GenericInputWidget.cpp" line="360" />
        <source>No LF</source>
        <translation>無換行</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="59" />
        <location filename="../widgets/GenericInputWidget.cpp" line="361" />
        <source>CR (\r)</source>
        <translation>CR (\r)</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="60" />
        <location filename="../widgets/GenericInputWidget.cpp" line="362" />
        <source>LF (\n)</source>
        <translation>LF (\n)</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="61" />
        <location filename="../widgets/GenericInputWidget.cpp" line="363" />
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r\n)</translation>
    </message>
</context>
<context>
    <name>ui::widgets::NetworkConnectionWidget</name>
    <message>
        <location filename="../widgets/NetworkConnectionWidget.cpp" line="116" />
        <source>Port:</source>
        <translation>埠：</translation>
    </message>
</context>
<context>
    <name>ui::widgets::RegisterTypeDelegate</name>
    <message>
        <location filename="../widgets/RegisterTypeDelegate.cpp" line="25" />
        <location filename="../widgets/RegisterTypeDelegate.cpp" line="64" />
        <source>Default</source>
        <translation>預設</translation>
    </message>
</context>
<context>
    <name>ui::widgets::SerialConnectionWidget</name>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="35" />
        <source>None</source>
        <translation>無</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="36" />
        <source>Even</source>
        <translation>偶校驗</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="37" />
        <source>Odd</source>
        <translation>奇校驗</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="38" />
        <source>Space</source>
        <translation>空白校驗</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="39" />
        <source>Mark</source>
        <translation>標記校驗</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="40" />
        <source>RTS/CTS</source>
        <translation>RTS/CTS</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="41" />
        <source>XON/XOFF</source>
        <translation>XON/XOFF</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="148" />
        <location filename="../widgets/SerialConnectionWidget.cpp" line="150" />
        <location filename="../widgets/SerialConnectionWidget.cpp" line="152" />
        <source>Disconnect</source>
        <translation>中斷連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="148" />
        <source>Connecting</source>
        <translation>Connecting</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="150" />
        <source>Transport Connected</source>
        <translation>Transport Connected</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="152" />
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="146" />
        <source>Connect</source>
        <translation>連線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="146" />
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="154" />
        <source>Disconnecting</source>
        <translation>Disconnecting</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="211" />
        <location filename="../widgets/SerialConnectionWidget.cpp" line="417" />
        <source>Refresh Ports</source>
        <translation>重新整理埠</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="397" />
        <source>Port:</source>
        <translation>埠：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="400" />
        <source>Baud:</source>
        <translation>鮑特率：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="403" />
        <source>Data:</source>
        <translation>資料位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="406" />
        <source>Parity:</source>
        <translation>同位元：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="410" />
        <source>Stop:</source>
        <translation>停止位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="413" />
        <source>Flow:</source>
        <translation>Flow:</translation>
    </message>
</context>
<context>
    <name>ui::widgets::ServerClientPanel</name>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="204" />
        <source>Select one or more clients for directed send, or enable broadcast.</source>
        <translation>選擇一個或多個用戶端進行定向發送，或啟用廣播。</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="207" />
        <source>Broadcast To All Clients</source>
        <translation>向所有用戶端廣播</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="210" />
        <source>Disconnect Selected</source>
        <translation>斷開選中的用戶端</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="213" />
        <source>Disconnect All Clients</source>
        <translation>斷開所有用戶端</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="231" />
        <source>all clients</source>
        <translation>all clients</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="232" />
        <source>%1 selected</source>
        <translation>%1 selected</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="233" />
        <source>Connected Clients: %1 | Send Target: %2</source>
        <translation>已連線用戶端: %1 | 發送目標: %2</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="265" />
        <source>Client #%1 - %2</source>
        <translation>Client #%1 - %2</translation>
    </message>
</context>
<context>
    <name>ui::widgets::TcpConnectionWidget</name>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="25" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="115" />
        <source>Host:</source>
        <translation>主機：</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="26" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="58" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="116" />
        <source>Connect</source>
        <translation>連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="58" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="73" />
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="60" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="62" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="64" />
        <source>Disconnect</source>
        <translation>中斷連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="60" />
        <source>Connecting</source>
        <translation>Connecting</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="62" />
        <source>Transport Connected</source>
        <translation>Transport Connected</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="64" />
        <source>Connected</source>
        <translation>已連線</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="66" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="79" />
        <source>Disconnecting</source>
        <translation>Disconnecting</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="28" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="118" />
        <source>Listen:</source>
        <translation>Listen:</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="29" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="73" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="119" />
        <source>Start Listen</source>
        <translation>Start Listen</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="75" />
        <location filename="../widgets/TcpConnectionWidget.cpp" line="77" />
        <source>Stop</source>
        <translation>Stop</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="75" />
        <source>Starting</source>
        <translation>Starting</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="77" />
        <source>Listening</source>
        <translation>Listening</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="79" />
        <source>Stopping</source>
        <translation>Stopping</translation>
    </message>
</context>
<context>
    <name>ui::widgets::UdpConnectionWidget</name>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="71" />
        <location filename="../widgets/UdpConnectionWidget.cpp" line="125" />
        <source>Local:</source>
        <translation>Local:</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="72" />
        <location filename="../widgets/UdpConnectionWidget.cpp" line="97" />
        <location filename="../widgets/UdpConnectionWidget.cpp" line="126" />
        <source>Bind</source>
        <translation>Bind</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="73" />
        <source>Remote:</source>
        <translation>Remote:</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="74" />
        <source>Remote Port:</source>
        <translation>Remote Port:</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="97" />
        <source>Disconnected</source>
        <translation>已斷線</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="99" />
        <location filename="../widgets/UdpConnectionWidget.cpp" line="101" />
        <source>Unbind</source>
        <translation>Unbind</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="99" />
        <source>Binding</source>
        <translation>Binding</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="101" />
        <source>Bound</source>
        <translation>Bound</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="103" />
        <source>Unbinding</source>
        <translation>Unbinding</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="103" />
        <source>Disconnecting</source>
        <translation>Disconnecting</translation>
    </message>
</context>
<context>
    <name>ui::widgets::modbus::ModbusTelemetryWidget</name>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="64" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="445" />
        <source>All</source>
        <translation>全部</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="64" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="446" />
        <source>TX</source>
        <translation>發送</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="64" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="447" />
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="64" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="448" />
        <source>SYS</source>
        <translation>系統</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="64" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="449" />
        <source>Errors</source>
        <translation>僅錯誤</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="67" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="441" />
        <source>Filter:</source>
        <translation>過濾:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="71" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="453" />
        <source>Raw Frames</source>
        <translation>原始幀</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="75" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="250" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="456" />
        <source>Auto-Scroll: ON</source>
        <translation>自動滾屏: 開啟</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="81" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="459" />
        <source>Clear (Ctrl+L)</source>
        <translation>清空 (Ctrl+L)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="85" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="462" />
        <source>Export...</source>
        <translation>導出...</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="130" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="250" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="456" />
        <source>Auto-Scroll: OFF</source>
        <translation>自動滾屏: 關閉</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="144" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="262" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="282" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="465" />
        <source>Frame Details</source>
        <translation>幀詳細資訊</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="151" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="468" />
        <source>Select a frame to inspect details</source>
        <translation>選擇幀以查看詳細解析</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="168" />
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="471" />
        <source>Link to Frame Analyzer</source>
        <translation>聯動幀分析儀</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="183" />
        <source>Queue overflow: %1 dropped</source>
        <translation>佇列溢出：%1 條丟棄</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="230" />
        <source>TX: %1 | RX: %2 | Error: %3 | RTT: %4 ms</source>
        <translation>TX: %1 | RX: %2 | Error: %3 | RTT: %4 ms</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="288" />
        <source>[SYS] %1</source>
        <translation>[SYS] %1</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="294" />
        <source>[#%1] %2 | %3 | %4 bytes | %5</source>
        <translation>[#%1] %2 | %3 | %4 位元組 | %5</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="305" />
        <source>MBAP Header: TID=%1, PID=%2, Len=%3, UnitID=%4
</source>
        <translation>MBAP 頭: TID=%1, PID=%2, Len=%3, UnitID=%4
</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="311" />
        <source>Serial ADU: Slave ID=%1
</source>
        <translation>串列 ADU: 從站 ID=%1
</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="314" />
        <source>Function Code: 0x%1 (%2)
</source>
        <translation>功能碼: 0x%1 (%2)
</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="319" />
        <source>Exception Code: 0x%1 (%2)
</source>
        <translation>異常碼: 0x%1 (%2)
</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="323" />
        <source>Decoded Items (%1):
</source>
        <translation>解析資料項 (%1):
</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="352" />
        <source>Export Telemetry</source>
        <translation>導出遙測記錄</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="353" />
        <source>CSV Files (*.csv);;JSON Files (*.json)</source>
        <translation>CSV 檔案 (*.csv);;JSON 檔案 (*.json)</translation>
    </message>
</context>
<context>
    <name>ui::widgets::modbus::ModbusWorkbenchWidget</name>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="53" />
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="623" />
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="886" />
        <source>Stop Polling (Esc)</source>
        <translation>停止輪詢 (Esc)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="53" />
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="420" />
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="634" />
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="886" />
        <source>Execute (F5)</source>
        <translation>執行 (F5)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="219" />
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="884" />
        <source>Standard</source>
        <translation>標準模式</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="223" />
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="885" />
        <source>Raw Frame</source>
        <translation>原始報文</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="257" />
        <source>Addressing &amp; Parameters</source>
        <translation>定址與參數</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="276" />
        <source>Radix:</source>
        <translation>進制:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="278" />
        <source>DEC</source>
        <translation>十進制 (DEC)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="282" />
        <source>HEX</source>
        <translation>十六進制 (HEX)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="295" />
        <source>Function Code:</source>
        <translation>功能碼:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="298" />
        <source>0x01 Read Coils</source>
        <translation>0x01 讀線圈 (Read Coils)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="299" />
        <source>0x02 Read Discrete Inputs</source>
        <translation>0x02 讀離散輸入 (Read Discrete Inputs)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="300" />
        <source>0x03 Read Holding Registers</source>
        <translation>0x03 讀保持暫存器 (Read Holding Registers)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="301" />
        <source>0x04 Read Input Registers</source>
        <translation>0x04 讀輸入暫存器 (Read Input Registers)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="302" />
        <source>0x05 Write Single Coil</source>
        <translation>0x05 寫單線圈 (Write Single Coil)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="303" />
        <source>0x06 Write Single Register</source>
        <translation>0x06 寫單暫存器 (Write Single Register)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="304" />
        <source>0x0F Write Multiple Coils</source>
        <translation>0x0F 寫多線圈 (Write Multiple Coils)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="305" />
        <source>0x10 Write Multiple Registers</source>
        <translation>0x10 寫多暫存器 (Write Multiple Registers)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="312" />
        <source>Slave ID (0-247):</source>
        <translation>從站號 (0-247):</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="327" />
        <source>Start Address:</source>
        <translation>起始位址:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="336" />
        <source>Quantity:</source>
        <translation>數量:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="348" />
        <source>Write Data</source>
        <translation>寫入資料</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="364" />
        <source>ON (0xFF00)</source>
        <translation>通 ON (0xFF00)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="368" />
        <source>OFF (0x0000)</source>
        <translation>斷 OFF (0x0000)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="382" />
        <source>Register Value (0-65535):</source>
        <translation>暫存器數值 (0-65535):</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="396" />
        <source>Bit Sequence (e.g. 10110):</source>
        <translation>位元序列 (如 10110):</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="406" />
        <source>Register Values (space separated):</source>
        <translation>暫存器字列表 (空格分隔):</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="416" />
        <source>Execution &amp; Polling</source>
        <translation>執行與輪詢</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="427" />
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="888" />
        <source>Periodic Polling</source>
        <translation>週期輪詢</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="432" />
        <source>Interval:</source>
        <translation>間隔:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="444" />
        <source>[!] Write Polling Active</source>
        <translation>[!] 寫入輪詢運行中</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="459" />
        <source>Raw Frame Dispatch (HEX)</source>
        <translation>原始報文發送 (HEX)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="465" />
        <source>Enter HEX bytes (e.g. 01 03 00 00 00 0A)</source>
        <translation>輸入十六進制位元組 (如 01 03 00 00 00 0A)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="478" />
        <source>+ MBAP</source>
        <translation>+ MBAP</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="482" />
        <source>+ CRC16</source>
        <translation>+ CRC16</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="486" />
        <source>+ LRC</source>
        <translation>+ LRC</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="490" />
        <source>Format</source>
        <translation>格式化</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="500" />
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="887" />
        <source>Send Raw Frame</source>
        <translation>發送原始報文</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="536" />
        <source>Polling automatically stopped due to write operation switch</source>
        <translation>切換至寫操作，已自動停止輪詢</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="647" />
        <source>High-Risk Operation Warning</source>
        <translation>高危操作警告</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="648" />
        <source>Warning: The current function code is a write operation [FC 0x%1]. Periodic polling will continuously write physical data to the slave, which may cause frequent slave state changes or hardware damage. Are you sure you want to start write polling?</source>
        <translation>警告：當前功能碼為寫操作 [FC 0x%1]，週期輪詢將持續向從站寫入物理資料，可能導致從站狀態頻繁變動或設備損壞。確定要啟動寫入輪詢嗎？</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="708" />
        <source>Address overflow: Start (%1) + Qty (%2) &gt; 65536</source>
        <translation>位址溢出：起始 (%1) + 數量 (%2) &gt; 65536</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="722" />
        <source>Invalid register value (must be 0 ~ 65535)</source>
        <translation>無效的暫存器數值 (必須在 0 ~ 65535 範圍內)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="754" />
        <source>Broadcast read (Slave ID 0) is forbidden per IEC 61158</source>
        <translation>依據 IEC 61158 嚴禁廣播讀取 (從站號 0)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="765" />
        <source>Bit Sequence (required: %1 bits):</source>
        <translation>位元序列 (需 %1 位元):</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="768" />
        <source>Register Values (required: %1 words):</source>
        <translation>暫存器數值 (需 %1 字):</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="795" />
        <source>Broadcast (Slave ID 0) does not support periodic polling</source>
        <translation>廣播 (從站號 0) 不支援週期輪詢</translation>
    </message>
    <message>
        <source>Address Base:</source>
        <translation>定址基準:</translation>
    </message>
    <message>
        <source>0-Based</source>
        <translation>0-Based (偏移定址)</translation>
    </message>
    <message>
        <source>1-Based</source>
        <translation>1-Based (常規定址)</translation>
    </message>
</context>
<context>
    <name>ui::widgets::modbus::TelemetryTableModel</name>
    <message>
        <location filename="../widgets/modbus/TelemetryTableModel.cpp" line="32" />
        <source>Timestamp (ms)</source>
        <translation>時間戳 (ms)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/TelemetryTableModel.cpp" line="33" />
        <source>Direction</source>
        <translation>方向</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/TelemetryTableModel.cpp" line="34" />
        <source>Length</source>
        <translation>長度</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/TelemetryTableModel.cpp" line="35" />
        <source>Message HEX / Summary</source>
        <translation>報文 HEX / 摘要</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/TelemetryTableModel.cpp" line="36" />
        <source>Status</source>
        <translation>狀態</translation>
    </message>
</context>
</TS>