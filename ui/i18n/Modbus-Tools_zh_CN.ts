<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>FrameParseWorker</name>
    <message>
        <location filename="../../core/modbus/parser/FrameParseWorker.cpp" line="47"/>
        <source>Error: Empty input</source>
        <translation>错误：输入为空</translation>
    </message>
</context>
<context>
    <name>ModbusFrameParser</name>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="32"/>
        <source>Illegal Function</source>
        <translation>Illegal Function</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="34"/>
        <source>Illegal Data Address</source>
        <translation>Illegal Data Address</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="36"/>
        <source>Illegal Data Value</source>
        <translation>Illegal Data Value</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="38"/>
        <source>Server Device Failure</source>
        <translation>Server Device Failure</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="40"/>
        <source>Acknowledge</source>
        <translation>Acknowledge</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="42"/>
        <source>Server Device Busy</source>
        <translation>Server Device Busy</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="44"/>
        <source>Negative Acknowledge</source>
        <translation>Negative Acknowledge</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="46"/>
        <source>Memory Parity Error</source>
        <translation>Memory Parity Error</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="48"/>
        <source>Gateway Path Unavailable</source>
        <translation>Gateway Path Unavailable</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="50"/>
        <source>Gateway Target Device Failed To Respond</source>
        <translation>Gateway Target Device Failed To Respond</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="52"/>
        <source>Unknown Exception</source>
        <translation>Unknown Exception</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="132"/>
        <source>Frame too short for Modbus TCP. Expected a complete MBAP + PDU, got %1 bytes</source>
        <translation>Frame too short for Modbus TCP. Expected a complete MBAP + PDU, got %1 bytes</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="141"/>
        <source>Invalid TCP MBAP header or length</source>
        <translation>Invalid TCP MBAP header or length</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="147"/>
        <source>TCP frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>TCP frame contains trailing bytes. Expected %1 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="155"/>
        <source>Frame too short for TCP MBAP</source>
        <translation>Frame too short for TCP MBAP</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="159"/>
        <source>Warning: Invalid TCP MBAP header or length (Forced)</source>
        <translation>Warning: Invalid TCP MBAP header or length (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="162"/>
        <source>Warning: TCP frame contains trailing bytes (Forced)</source>
        <translation>Warning: TCP frame contains trailing bytes (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="175"/>
        <source>Invalid TCP PDU length. MBAP length field is %1, so PDU length is %2</source>
        <translation>Invalid TCP PDU length. MBAP length field is %1, so PDU length is %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="202"/>
        <source>Frame too short for Modbus RTU. Expected at least 5 bytes, got %1</source>
        <translation>Frame too short for Modbus RTU. Expected at least 5 bytes, got %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="218"/>
        <source>CRC Mismatch. Expected %1, Got %2</source>
        <translation>CRC Mismatch. Expected %1, Got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="226"/>
        <source>RTU frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>RTU frame contains trailing bytes. Expected %1 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="234"/>
        <source>Frame too short for RTU</source>
        <translation>Frame too short for RTU</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="238"/>
        <source>Warning: CRC Mismatch. Expected %1, Got %2 (Forced)</source>
        <translation>Warning: CRC Mismatch. Expected %1, Got %2 (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="244"/>
        <source>Warning: RTU frame contains trailing bytes (Forced)</source>
        <translation>Warning: RTU frame contains trailing bytes (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="275"/>
        <source>Frame incomplete for Modbus ASCII. Missing CRLF terminator or full payload</source>
        <translation>Frame incomplete for Modbus ASCII. Missing CRLF terminator or full payload</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="283"/>
        <source>Invalid Modbus ASCII frame or LRC mismatch</source>
        <translation>Invalid Modbus ASCII frame or LRC mismatch</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="291"/>
        <source>ASCII frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>ASCII frame contains trailing bytes. Expected %1 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="299"/>
        <source>Warning: Invalid Modbus ASCII frame or LRC mismatch (Forced)</source>
        <translation>Warning: Invalid Modbus ASCII frame or LRC mismatch (Forced)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="333"/>
        <source>Empty frame data</source>
        <translation>Empty frame data</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="353"/>
        <source>Unable to identify protocol. Frame length: %1 bytes, data: %2</source>
        <translation>Unable to identify protocol. Frame length: %1 bytes, data: %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="390"/>
        <source>Empty PDU. Function code is missing from the frame</source>
        <translation>Empty PDU. Function code is missing from the frame</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="404"/>
        <source>Exception PDU length invalid for function 0x%1. Expected strictly 2 bytes, got %2</source>
        <translation>功能码 0x%1 的异常 PDU 长度无效：严格应为 2 字节，实际为 %2 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="413"/>
        <source>Modbus exception: %1 (code %2)</source>
        <translation>Modbus exception: %1 (code %2)</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="444"/>
        <source>Failed to read request fields</source>
        <translation>读取请求字段失败</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="456"/>
        <source>Response PDU too short for function 0x%1. Expected at least 2 bytes, got %2</source>
        <translation>Response PDU too short for function 0x%1. Expected at least 2 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="468"/>
        <source>Byte count mismatch for function 0x%1. Declared %2, actual %3</source>
        <translation>Byte count mismatch for function 0x%1. Declared %2, actual %3</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="481"/>
        <source>Register response byte count does not match expected quantity. Declared %1, expected %2</source>
        <translation>Register response byte count does not match expected quantity. Declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="490"/>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="663"/>
        <source>Register byte count must be even, got %1</source>
        <translation>Register byte count must be even, got %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="503"/>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="675"/>
        <source>Truncated register data</source>
        <translation>寄存器数据被截断</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="530"/>
        <source>Coil response bit count does not match expected quantity. Byte count %1 cannot represent %2 bits</source>
        <translation>Coil response bit count does not match expected quantity. Byte count %1 cannot represent %2 bits</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="567"/>
        <source>Write single PDU length mismatch for function 0x%1. Expected 4 bytes, got %2</source>
        <translation>Write single PDU length mismatch for function 0x%1. Expected 4 bytes, got %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="580"/>
        <source>Failed to read write value</source>
        <translation>读取写入值失败</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="615"/>
        <source>Failed to read response echo</source>
        <translation>读取响应回显失败</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="626"/>
        <source>Write request PDU too short</source>
        <translation>写请求 PDU 过短</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="636"/>
        <source>Failed to read write request fields</source>
        <translation>读取写请求字段失败</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="643"/>
        <source>Write request byte count mismatch. Declared %1, actual %2</source>
        <translation>Write request byte count mismatch. Declared %1, actual %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="654"/>
        <source>Register write byte count does not match quantity. Declared %1, expected %2</source>
        <translation>Register write byte count does not match quantity. Declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="696"/>
        <source>Coil write byte count does not match quantity. Declared %1, expected %2</source>
        <translation>Coil write byte count does not match quantity. Declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="729"/>
        <source>Unsupported function code 0x%1 for deep parsing</source>
        <translation>Unsupported function code 0x%1 for deep parsing</translation>
    </message>
</context>
<context>
    <name>ModbusPduBuilder</name>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="31"/>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="109"/>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="125"/>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="141"/>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="180"/>
        <source>Invalid start address</source>
        <translation>Invalid start address</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="18"/>
        <source>Address range overflow: start %1 + quantity %2 exceeds 65536</source>
        <translation>地址范围溢出：起始地址 %1 + 数量 %2 超过 65536</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="37"/>
        <source>Invalid quantity for function code 0x%1 (allowed 1-%2)</source>
        <translation>功能码 0x%1 的数量无效（允许范围 1-%2）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="65"/>
        <source>Missing coil value for 0x05</source>
        <translation>缺少 0x05 线圈写入值</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="101"/>
        <source>Unsupported function code</source>
        <translation>Unsupported function code</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="145"/>
        <source>Invalid quantity for 0x0F</source>
        <translation>0x0F 数量无效</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="154"/>
        <source>Data length mismatch for 0x0F</source>
        <translation>Data length mismatch for 0x0F</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="184"/>
        <source>Invalid quantity for 0x10</source>
        <translation>0x10 数量无效</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="192"/>
        <source>Data length mismatch for 0x10</source>
        <translation>Data length mismatch for 0x10</translation>
    </message>
</context>
<context>
    <name>ModbusProtocolChecks</name>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="18"/>
        <source>Exception</source>
        <translation>异常</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="19"/>
        <source>Bit read</source>
        <translation>Bit read</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="20"/>
        <source>Register read</source>
        <translation>Register read</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="21"/>
        <source>Write single</source>
        <translation>Write single</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="22"/>
        <source>Write multiple</source>
        <translation>Write multiple</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="23"/>
        <source>%1 response payload length mismatch: expected %2, got %3</source>
        <translation>%1 response payload length mismatch: expected %2, got %3</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="24"/>
        <source>Exception function code does not match request</source>
        <translation>Exception function code does not match request</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="25"/>
        <source>Exception code is outside the valid range 0x01..0x0A</source>
        <translation>异常码超出有效范围 0x01..0x0A</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="26"/>
        <source>Response function code does not match request</source>
        <translation>Response function code does not match request</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="27"/>
        <source>Request quantity missing for bit-read validation</source>
        <translation>Request quantity missing for bit-read validation</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="28"/>
        <source>Bit-read response byte count does not match payload length</source>
        <translation>Bit-read response byte count does not match payload length</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="29"/>
        <source>Bit-read response byte count does not match requested quantity</source>
        <translation>Bit-read response byte count does not match requested quantity</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="30"/>
        <source>Request quantity missing for register-read validation</source>
        <translation>Request quantity missing for register-read validation</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="31"/>
        <source>Register-read response byte count does not match payload length</source>
        <translation>Register-read response byte count does not match payload length</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="32"/>
        <source>Register-read response byte count does not match requested quantity</source>
        <translation>Register-read response byte count does not match requested quantity</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="33"/>
        <source>Write-single response echo does not match request</source>
        <translation>Write-single response echo does not match request</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="34"/>
        <source>Request echo fields missing for write-multiple validation</source>
        <translation>Request echo fields missing for write-multiple validation</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="35"/>
        <source>Write-multiple response echo fields are incomplete</source>
        <translation>Write-multiple response echo fields are incomplete</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="36"/>
        <source>Write-multiple response echo does not match request</source>
        <translation>Write-multiple response echo does not match request</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="37"/>
        <source>Unsupported function code for response validation</source>
        <translation>Unsupported function code for response validation</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../infra/io/SerialChannel.cpp" line="98"/>
        <location filename="../../infra/io/SerialChannel.cpp" line="215"/>
        <source>%1 is busy or in use, unable to open</source>
        <translation>%1 正忙或被占用，无法打开</translation>
    </message>
</context>
<context>
    <name>RequestSubmissionService</name>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="56"/>
        <source>Invalid decimal value for 0x05</source>
        <translation>Invalid decimal value for 0x05</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="66"/>
        <source>Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation>Invalid binary value for 0x05 (expected 0 or 1)</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="75"/>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="83"/>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="92"/>
        <source>Invalid hex value for 0x05</source>
        <translation>Invalid hex value for 0x05</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="105"/>
        <source>Empty value for 0x06</source>
        <translation>Empty value for 0x06</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="114"/>
        <source>Invalid decimal value for 0x06</source>
        <translation>Invalid decimal value for 0x06</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="122"/>
        <source>Binary format not supported for registers (0x06)</source>
        <translation>Binary format not supported for registers (0x06)</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="129"/>
        <source>Invalid hex value for 0x06</source>
        <translation>Invalid hex value for 0x06</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="138"/>
        <source>Invalid quantity for 0x0F</source>
        <translation>0x0F 数量无效</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="148"/>
        <source>Binary bit count (%1) does not match Quantity (%2)</source>
        <translation>Binary bit count (%1) does not match Quantity (%2)</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="158"/>
        <source>0x0F requires Hex or Binary data</source>
        <translation>0x0F requires Hex or Binary data</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="165"/>
        <source>Empty value for 0x10</source>
        <translation>Empty value for 0x10</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="171"/>
        <source>Invalid quantity for 0x10</source>
        <translation>0x10 数量无效</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="180"/>
        <source>Invalid decimal list for 0x10</source>
        <translation>Invalid decimal list for 0x10</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="188"/>
        <source>Invalid hex value for 0x10</source>
        <translation>Invalid hex value for 0x10</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="220"/>
        <source>Raw data is empty</source>
        <translation>Raw data is empty</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="266"/>
        <source>Slave ID (%1) exceeds maximum (%2)</source>
        <translation>从站地址 (%1) 超过最大值 (%2)</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="271"/>
        <source>Broadcast read is forbidden per IEC 61158</source>
        <translation>根据 IEC 61158 规范禁止广播读取</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="283"/>
        <source>Unsupported function code: %1</source>
        <translation>不支持的功能码：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="289"/>
        <source>Quantity must be at least 1</source>
        <translation>数量必须至少为 1</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="293"/>
        <source>Quantity (%1) exceeds maximum allowed (%2) for function code %3</source>
        <translation>数量 (%1) 超过功能码 %3 允许的最大值 (%2)</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="298"/>
        <source>Single write quantity must be 1</source>
        <translation>单点写入数量必须为 1</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="304"/>
        <source>Address range overflow: start=%1, quantity=%2 exceeds 65536</source>
        <translation>地址范围溢出：起始地址=%1，数量=%2 超过 65536</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="316"/>
        <source>FC 05 write data payload must be exactly 2 bytes</source>
        <translation>FC 05 写入负载必须严格为 2 字节</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="322"/>
        <source>FC 05 coil value must strictly be 0x0000 (OFF) or 0xFF00 (ON)</source>
        <translation>FC 05 线圈值必须严格为 0x0000 (OFF) 或 0xFF00 (ON)</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="327"/>
        <source>FC 06 write data payload must be exactly 2 bytes</source>
        <translation>FC 06 写入负载必须严格为 2 字节</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="333"/>
        <source>FC 0F write coils payload size (%1) does not match expected (%2) bytes</source>
        <translation>FC 0F 写多线圈负载长度 (%1) 与预期 (%2) 字节不符</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="340"/>
        <source>FC 10 write registers payload size (%1) does not match expected (%2) bytes</source>
        <translation>FC 10 写多寄存器负载长度 (%1) 与预期 (%2) 字节不符</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="351"/>
        <source>Wire PDU length (%1 bytes) exceeds maximum 253 bytes</source>
        <translation>报文 PDU 长度（%1 字节）超过最大限制 253 字节</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="361"/>
        <source>Raw data cannot be empty</source>
        <translation>原始报文不能为空</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="365"/>
        <source>Raw frame exceeds maximum size (%1 bytes)</source>
        <translation>原始帧超过最大长度限制（%1 字节）</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="394"/>
        <source>Raw data too short</source>
        <translation>原始报文过短</translation>
    </message>
</context>
<context>
    <name>SerialConfig</name>
    <message>
        <location filename="../../infra/io/SerialConfig.cpp" line="56"/>
        <source>Port name is empty</source>
        <translation>串口名称为空</translation>
    </message>
    <message>
        <location filename="../../infra/io/SerialConfig.cpp" line="60"/>
        <source>Unsupported baud rate: %1</source>
        <translation>不支持的波特率：%1</translation>
    </message>
    <message>
        <location filename="../../infra/io/SerialConfig.cpp" line="64"/>
        <source>Invalid data bits: %1 (expected 5, 6, 7, or 8)</source>
        <translation>无效的数据位：%1（应为 5、6、7 或 8）</translation>
    </message>
    <message>
        <location filename="../../infra/io/SerialConfig.cpp" line="70"/>
        <source>Invalid stop bits: %1 (expected 1, 1.5, or 2)</source>
        <translation>无效的停止位：%1（应为 1、1.5 或 2）</translation>
    </message>
    <message>
        <location filename="../../infra/io/SerialConfig.cpp" line="75"/>
        <source>Unsupported parity setting: %1</source>
        <translation>不支持的校验位设置：%1</translation>
    </message>
    <message>
        <location filename="../../infra/io/SerialConfig.cpp" line="80"/>
        <source>Unsupported flow control setting: %1</source>
        <translation>不支持的流控设置：%1</translation>
    </message>
</context>
<context>
    <name>TcpChannel</name>
    <message>
        <location filename="../../infra/io/TcpChannel.cpp" line="134"/>
        <source>Invalid IP address: %1</source>
        <translation>无效的 IP 地址：%1</translation>
    </message>
    <message>
        <location filename="../../infra/io/TcpChannel.cpp" line="142"/>
        <source>Invalid port: %1 (expected 1-65535)</source>
        <translation>无效的端口：%1（应为 1-65535）</translation>
    </message>
</context>
<context>
    <name>core::update::UpdateCommandTemplate</name>
    <message>
        <location filename="../../core/update/UpdateCommandTemplate.cpp" line="33"/>
        <source># WARNING: no checksum was published for this release; verify the download source manually before continuing.</source>
        <translation># 警告:此版本未提供校验和,请人工核对下载来源后再继续。</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateCommandTemplate.cpp" line="40"/>
        <source># 1) Download
</source>
        <translation># 1) 下载
</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateCommandTemplate.cpp" line="47"/>
        <source># 2) Verify checksum (do not continue on failure)
</source>
        <translation># 2) 校验(失败请勿继续)
</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateCommandTemplate.cpp" line="52"/>
        <source># 3) Replace (quit Modbus-Tools first; adjust the path below if you installed elsewhere)
</source>
        <translation># 3) 替换(请先退出 Modbus-Tools;若安装在其他目录请先修改下方路径)
</translation>
    </message>
</context>
<context>
    <name>core::update::UpdateManager</name>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="55"/>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="122"/>
        <source>Incomplete update task parameters</source>
        <translation>Incomplete update task parameters</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="70"/>
        <source>Automatic update is not supported on Windows</source>
        <translation>Automatic update is not supported on Windows</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="77"/>
        <source>Updater not found</source>
        <translation>Updater not found</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="86"/>
        <source>Updater integrity check failed: %1</source>
        <translation>Updater integrity check failed: %1</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="201"/>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="215"/>
        <source>Automatic update is not supported on %1</source>
        <translation>Automatic update is not supported on %1</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="59"/>
        <source>Missing or invalid expected checksum</source>
        <translation>缺少预期的校验和或格式无效</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="63"/>
        <location filename="../../core/update/UpdateManager.cpp" line="76"/>
        <source>Canceled</source>
        <translation>已取消</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="69"/>
        <source>Failed to open file for checksum calculation</source>
        <translation>无法打开文件以计算校验和</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="80"/>
        <source>Failed to read file chunk</source>
        <translation>读取文件数据块失败</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="87"/>
        <source>Failed to calculate file checksum</source>
        <translation>计算文件校验和失败</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="92"/>
        <source>Checksum mismatch. Expected: %1, Actual: %2</source>
        <translation>校验和不匹配。预期：%1，实际：%2</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="150"/>
        <source>Failed to create update directory</source>
        <translation>Failed to create update directory</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="156"/>
        <source>Invalid update URL</source>
        <translation>Invalid update URL</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="203"/>
        <source>Failed to open local file for writing</source>
        <translation>Failed to open local file for writing</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="275"/>
        <location filename="../../core/update/UpdateManager.cpp" line="304"/>
        <source>No update install strategy available</source>
        <translation>No update install strategy available</translation>
    </message>
</context>
<context>
    <name>logging</name>
    <message>
        <location filename="../../infra/logging/Logger.cpp" line="86"/>
        <source>Log directory path is empty.</source>
        <translation>日志目录路径为空。</translation>
    </message>
    <message>
        <location filename="../../infra/logging/Logger.cpp" line="94"/>
        <source>Failed to create log directory: %1</source>
        <translation>创建日志目录失败：%1</translation>
    </message>
    <message>
        <location filename="../../infra/logging/Logger.cpp" line="118"/>
        <source>Log file is not writable: %1</source>
        <translation>日志文件不可写：%1</translation>
    </message>
</context>
<context>
    <name>modbus::analyzer::value_formatter</name>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="73"/>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="98"/>
        <source>Description: %1</source>
        <translation>Description: %1</translation>
    </message>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="80"/>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="103"/>
        <source>Raw: %1</source>
        <translation>Raw: %1</translation>
    </message>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="82"/>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="105"/>
        <source>Scale: %1</source>
        <translation>Scale: %1</translation>
    </message>
    <message>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="84"/>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="108"/>
        <location filename="../../core/analyzer/ValueFormatter.cpp" line="111"/>
        <source>Scaled: %1</source>
        <translation>Scaled: %1</translation>
    </message>
</context>
<context>
    <name>modbus::session::ConnectionManager</name>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="132"/>
        <source>No channel attached</source>
        <translation>No channel attached</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="147"/>
        <source>Connect attempt in progress</source>
        <translation>连接请求正在进行中</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="187"/>
        <source>State machine busy</source>
        <translation>State machine busy</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="200"/>
        <source>Failed to dispatch channel open</source>
        <translation>Failed to dispatch channel open</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="223"/>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="316"/>
        <source>Connect timeout</source>
        <translation>Connect timeout</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="305"/>
        <source>Channel entered error state</source>
        <translation>Channel entered error state</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/ConnectionManager.cpp" line="311"/>
        <source>Aborted</source>
        <translation>已中止</translation>
    </message>
</context>
<context>
    <name>modbus::session::RequestValidator</name>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="30"/>
        <source>Invalid RTU slave id: %1</source>
        <translation>Invalid RTU slave id: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="35"/>
        <source>PDU data too long: %1 bytes</source>
        <translation>PDU data too long: %1 bytes</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="50"/>
        <source>Invalid read request payload length: %1</source>
        <translation>Invalid read request payload length: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="53"/>
        <source>Read quantity must be at least %1</source>
        <translation>Read quantity must be at least %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="57"/>
        <source>Read address range exceeds limit: start=%1 quantity=%2</source>
        <translation>Read address range exceeds limit: start=%1 quantity=%2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="64"/>
        <source>Read bit quantity exceeds limit: %1</source>
        <translation>Read bit quantity exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="69"/>
        <source>Read register quantity exceeds limit: %1</source>
        <translation>Read register quantity exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="77"/>
        <source>Invalid write-single request payload length: %1</source>
        <translation>Invalid write-single request payload length: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="80"/>
        <source>Write address out of range: %1</source>
        <translation>Write address out of range: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="84"/>
        <source>Invalid single coil value: 0x%1</source>
        <translation>Invalid single coil value: 0x%1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="94"/>
        <source>Invalid write-multiple request payload length: %1</source>
        <translation>Invalid write-multiple request payload length: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="98"/>
        <source>Write quantity must be at least %1</source>
        <translation>Write quantity must be at least %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="102"/>
        <source>Write address range exceeds limit: start=%1 quantity=%2</source>
        <translation>Write address range exceeds limit: start=%1 quantity=%2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="107"/>
        <source>Write request byte count mismatch: declared %1, actual %2</source>
        <translation>Write request byte count mismatch: declared %1, actual %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="113"/>
        <source>Write coil quantity exceeds limit: %1</source>
        <translation>Write coil quantity exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="117"/>
        <source>Write coil byte count mismatch: declared %1, expected %2</source>
        <translation>Write coil byte count mismatch: declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="123"/>
        <source>Write register quantity exceeds limit: %1</source>
        <translation>Write register quantity exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="127"/>
        <source>Write register byte count mismatch: declared %1, expected %2</source>
        <translation>Write register byte count mismatch: declared %1, expected %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="134"/>
        <source>Unsupported function code: 0x%1</source>
        <translation>Unsupported function code: 0x%1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="141"/>
        <source>TCP MBAP length exceeds limit: %1</source>
        <translation>TCP MBAP length exceeds limit: %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="146"/>
        <source>RTU ADU length exceeds limit: %1</source>
        <translation>RTU ADU length exceeds limit: %1</translation>
    </message>
</context>
<context>
    <name>ui::MainWindow</name>
    <message>
        <location filename="../MainWindow.cpp" line="113"/>
        <location filename="../MainWindow.cpp" line="380"/>
        <source>Modbus Tools</source>
        <translation>Modbus 工具</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="189"/>
        <location filename="../MainWindow.cpp" line="385"/>
        <source>Frame Analyzer</source>
        <translation>报文分析</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="189"/>
        <location filename="../MainWindow.cpp" line="386"/>
        <source>Utilities</source>
        <translation>实用工具</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="259"/>
        <location filename="../MainWindow.cpp" line="391"/>
        <source>Language</source>
        <translation>语言</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="244"/>
        <location filename="../MainWindow.cpp" line="392"/>
        <source>Settings</source>
        <translation>设置</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="389"/>
        <source>Expand Navigation</source>
        <translation>展开导航</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="389"/>
        <source>Collapse Navigation</source>
        <translation>折叠导航</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="245"/>
        <location filename="../MainWindow.cpp" line="394"/>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="22"/>
        <source>Modbus Settings</source>
        <translation>Modbus设置</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="189"/>
        <location filename="../MainWindow.cpp" line="383"/>
        <source>Network Debugger</source>
        <translation>网络调试器</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="189"/>
        <location filename="../MainWindow.cpp" line="384"/>
        <source>Serial Debugger</source>
        <translation>串口调试助手</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="189"/>
        <location filename="../MainWindow.cpp" line="382"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="248"/>
        <location filename="../MainWindow.cpp" line="395"/>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="26"/>
        <source>Update Settings</source>
        <translation>更新设置</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="252"/>
        <location filename="../MainWindow.cpp" line="396"/>
        <source>Open Log Folder</source>
        <translation>打开日志文件夹</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="273"/>
        <location filename="../MainWindow.cpp" line="400"/>
        <source>English (US)</source>
        <translation>英语（美国）</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="274"/>
        <location filename="../MainWindow.cpp" line="401"/>
        <source>简体中文</source>
        <translation>简体中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="275"/>
        <location filename="../MainWindow.cpp" line="402"/>
        <source>繁體中文</source>
        <translation>繁體中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="294"/>
        <location filename="../MainWindow.cpp" line="397"/>
        <source>Check for Updates</source>
        <translation>检查更新</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="32"/>
        <source>Request Timeout (ms):</source>
        <translation>请求超时(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="36"/>
        <source>Enable Retry:</source>
        <translation>启用重试：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="43"/>
        <source>Retry Count:</source>
        <translation>重试次数：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="51"/>
        <source>Retry Interval (ms):</source>
        <translation>重试间隔(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="61"/>
        <source>Debug</source>
        <translation>Debug</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="62"/>
        <source>Info</source>
        <translation>信息</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="63"/>
        <source>Warning</source>
        <translation>Warning</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="64"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="68"/>
        <source>Log Level:</source>
        <translation>Log Level:</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="71"/>
        <source>Offset (0-based)</source>
        <translation>偏移量 (从 0 开始)</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="72"/>
        <source>PLC Address (1-based)</source>
        <translation>PLC 地址 (从 1 开始)</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="75"/>
        <source>Address Mode:</source>
        <translation>地址模式:</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="288"/>
        <location filename="../MainWindow.cpp" line="298"/>
        <location filename="../MainWindow.cpp" line="393"/>
        <location filename="../MainWindow.cpp" line="398"/>
        <location filename="../widgets/AboutDialog.cpp" line="17"/>
        <source>About</source>
        <translation>关于</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="320"/>
        <source>Theme: Auto</source>
        <translation>主题：自动</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="321"/>
        <source>Theme: Light</source>
        <translation>主题：浅色</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="321"/>
        <source>Theme: Dark</source>
        <translation>主题：深色</translation>
    </message>
    <message>
        <location filename="../widgets/AboutDialog.cpp" line="26"/>
        <source>Welcome to Modbus-Tools&lt;br&gt;Version: v%1&lt;br&gt;&lt;br&gt;An open-source Modbus communication debugging assistant.&lt;br&gt;Developer: mingyucheng692&lt;br&gt;License: MIT License&lt;br&gt;&lt;br&gt;This project is developed in spare time, completely free and open-source.&lt;br&gt;Feel free to star on GitHub or submit issues.&lt;br&gt;Your feedback keeps the project improving!&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 Visit GitHub Repository&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 Issue Tracker&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;This software is provided &amp;quot;as is&amp;quot; without warranty of any kind.</source>
        <translation>欢迎使用 Modbus-Tools&lt;br&gt;版本：v%1&lt;br&gt;&lt;br&gt;一款开源的 Modbus 通讯调试助手。&lt;br&gt;开发者：mingyucheng692&lt;br&gt;许可协议：MIT License&lt;br&gt;&lt;br&gt;本项目由个人业余开发，完全免费且开源。&lt;br&gt;欢迎在 GitHub 上 Star ⭐ 或提交 Issue。&lt;br&gt;您的反馈是项目持续改进的动力！&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 访问 GitHub 仓库&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 问题反馈&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;本软件按&amp;quot;原样&amp;quot;提供，无任何形式的保证。</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="31"/>
        <source>Every Startup</source>
        <translation>每次启动</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="33"/>
        <source>Weekly</source>
        <translation>每周</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="35"/>
        <source>Monthly</source>
        <translation>每月</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="37"/>
        <source>Disable Update Check</source>
        <translation>关闭更新检测</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="39"/>
        <source>Update Check Frequency:</source>
        <translation>更新检测频率：</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="152"/>
        <source>No Updates</source>
        <translation>无可用更新</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="153"/>
        <source>You are using the latest version: v%1</source>
        <translation>您正在使用最新版本：v%1</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="163"/>
        <source>Update Check Failed</source>
        <translation>更新检查失败</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="178"/>
        <source>Update service unavailable</source>
        <translation>更新服务不可用</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="182"/>
        <location filename="../application/UpdateCoordinator.cpp" line="191"/>
        <source>Update Failed</source>
        <translation>更新失败</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="226"/>
        <source>Automatic Update Unsupported</source>
        <translation>不支持自动更新</translation>
    </message>
    <message>
        <location filename="../application/UpdateCoordinator.cpp" line="227"/>
        <source>In-app automatic update is not supported on %1. Download the latest package instead.</source>
        <translation>%1 不支持应用内自动更新，请改为下载最新安装包。</translation>
    </message>
</context>
<context>
    <name>ui::UpdateCommandDialog</name>
    <message>
        <location filename="../UpdateCommandDialog.cpp" line="25"/>
        <source>Update to v%1</source>
        <translation>更新到 v%1</translation>
    </message>
    <message>
        <location filename="../UpdateCommandDialog.cpp" line="29"/>
        <source>Quit Modbus-Tools before running step 3. Paste each block into a terminal, in order.</source>
        <translation>执行第 3 步前请先退出 Modbus-Tools。请按顺序将各段命令粘贴到终端执行。</translation>
    </message>
    <message>
        <location filename="../UpdateCommandDialog.cpp" line="38"/>
        <location filename="../UpdateCommandDialog.cpp" line="62"/>
        <source>Copy Commands</source>
        <translation>复制命令</translation>
    </message>
    <message>
        <location filename="../UpdateCommandDialog.cpp" line="39"/>
        <source>Open Download Page</source>
        <translation>打开下载页</translation>
    </message>
    <message>
        <location filename="../UpdateCommandDialog.cpp" line="40"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../UpdateCommandDialog.cpp" line="59"/>
        <source>Copied!</source>
        <translation>已复制!</translation>
    </message>
</context>
<context>
    <name>ui::UpdateInteractionView</name>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="34"/>
        <source>About</source>
        <translation>关于</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="37"/>
        <source>Check for Updates</source>
        <translation>检查更新</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="56"/>
        <location filename="../UpdateInteractionView.cpp" line="72"/>
        <source>Update Available</source>
        <translation>发现新版本</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="57"/>
        <source>New version v%1 available. Open download page?</source>
        <translation>发现新版本 v%1。是否打开下载页面？</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="73"/>
        <source>Current: v%1, Latest: v%2
Choose update method:</source>
        <translation>当前版本：v%1，最新版本：v%2
请选择更新方式：</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="74"/>
        <source>Update Main Program</source>
        <translation>更新主程序</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="75"/>
        <source>Download Full Package</source>
        <translation>下载完整安装包</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="92"/>
        <source>Downloading Update...</source>
        <translation>正在下载更新...</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="92"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::ModbusPagePresenter</name>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="259"/>
        <source>Request rejected: connection in progress, please wait.</source>
        <translation>请求已拒绝：正在建立连接，请稍候。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="261"/>
        <source>Request rejected: disconnecting in progress.</source>
        <translation>请求已拒绝：正在断开连接。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="263"/>
        <source>Request rejected: device not connected.</source>
        <translation>请求已拒绝：设备未连接。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="304"/>
        <source>Device is establishing session. Read commands available; write commands locked until ready.</source>
        <translation>设备正在建立会话。读取指令可用；写入指令将在就绪后解锁。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="330"/>
        <source>Error: Invalid raw frame format</source>
        <translation>错误：无效的原始帧格式</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="409"/>
        <source>Broadcast write polling is strictly forbidden.</source>
        <translation>严禁广播写入轮询。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="415"/>
        <source>Write polling rejected: confirmation required.</source>
        <translation>写入轮询已拒绝：需要确认。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="530"/>
        <source>Illegal Function</source>
        <translation>非法功能码</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="532"/>
        <source>Illegal Data Address</source>
        <translation>非法数据地址</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="534"/>
        <source>Illegal Data Value</source>
        <translation>非法数据值</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="536"/>
        <source>Server Device Failure</source>
        <translation>从站设备故障</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="538"/>
        <source>Acknowledge</source>
        <translation>确认应答</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="540"/>
        <source>Server Device Busy</source>
        <translation>从站设备忙</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="542"/>
        <source>Negative Acknowledge</source>
        <translation>否认应答</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="544"/>
        <source>Memory Parity Error</source>
        <translation>存储奇偶校验错误</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="546"/>
        <source>Gateway Path Unavailable</source>
        <translation>网关路径不可用</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="548"/>
        <source>Gateway Target Device Failed To Respond</source>
        <translation>网关目标设备未响应</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="550"/>
        <source>Unknown Exception</source>
        <translation>未知异常</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="559"/>
        <source>Request timed out</source>
        <translation>请求超时</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="561"/>
        <source>Not connected</source>
        <translation>未连接</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="563"/>
        <source>Connection failed</source>
        <translation>连接失败</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="565"/>
        <source>Request already in progress</source>
        <translation>请求正在进行中</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="567"/>
        <source>Invalid response frame</source>
        <translation>无效的响应帧</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="569"/>
        <source>Invalid PDU parameters</source>
        <translation>无效的 PDU 参数</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="571"/>
        <source>CRC checksum error</source>
        <translation>CRC 校验错误</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="573"/>
        <source>LRC checksum error</source>
        <translation>LRC 校验错误</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="575"/>
        <source>Modbus exception: %1 (0x%2)</source>
        <translation>Modbus 异常：%1 (0x%2)</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="579"/>
        <source>Failed to send data</source>
        <translation>发送数据失败</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="581"/>
        <source>Request aborted</source>
        <translation>请求已中止</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="583"/>
        <source>Too many invalid response bytes</source>
        <translation>无效响应字节过多</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="587"/>
        <source>Unknown error</source>
        <translation>未知错误</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="299"/>
        <source>Connecting — please wait before sending commands.</source>
        <translation>正在连接 — 发送指令前请稍候。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="301"/>
        <source>Disconnecting...</source>
        <translation>正在断开连接...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="307"/>
        <source>Connect device to send commands.</source>
        <translation>请先连接设备后再发送指令。</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusPagePresenter.cpp" line="357"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::ModbusSessionPresenter</name>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="56"/>
        <source>Connecting to %1:%2...</source>
        <translation>正在连接 %1:%2...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="115"/>
        <source>Opening %1...</source>
        <translation>正在打开 %1...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="158"/>
        <source>Disconnecting...</source>
        <translation>Disconnecting...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="180"/>
        <source>Shutdown timed out; restart recommended</source>
        <translation>Shutdown timed out; restart recommended</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="186"/>
        <source>Release timed out; restart recommended</source>
        <translation>Release timed out; restart recommended</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="211"/>
        <source>Releasing Modbus stack...</source>
        <translation>Releasing Modbus stack...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="233"/>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="360"/>
        <source>Release completed</source>
        <translation>Release completed</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="547"/>
        <source>Transport connected, validating session...</source>
        <translation>Transport connected, validating session...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="575"/>
        <source>Disconnected</source>
        <translation>已断开</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="576"/>
        <source>Disconnected: %1</source>
        <translation>Disconnected: %1</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="624"/>
        <source>Transport connected, waiting for device response...</source>
        <translation>Transport connected, waiting for device response...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="63"/>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="123"/>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="404"/>
        <source>Failed to create Modbus stack</source>
        <translation>创建 Modbus 栈失败</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="615"/>
        <source>Connection failed: %1</source>
        <translation>连接失败：%1</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::PollingController</name>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="125"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="165"/>
        <source>Poll recovered after %1 consecutive failures</source>
        <translation>Poll recovered after %1 consecutive failures</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="219"/>
        <source>Poll Error: Connection unavailable during polling (%1)</source>
        <translation>Poll Error: Connection unavailable during polling (%1)</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="222"/>
        <source>Poll Error escalated after %1 consecutive failures: %2</source>
        <translation>Poll Error escalated after %1 consecutive failures: %2</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="226"/>
        <source>Poll Error persists (%1 consecutive failures): %2</source>
        <translation>Poll Error persists (%1 consecutive failures): %2</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="245"/>
        <source>Poll Warning: %1 consecutive failure(s): %2</source>
        <translation>Poll Warning: %1 consecutive failure(s): %2</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="293"/>
        <source>Connection unavailable for %1 s; self-healing window expired</source>
        <translation>Connection unavailable for %1 s; self-healing window expired</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::TrafficLogController</name>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="71"/>
        <source>Success: Response received</source>
        <translation>成功：收到响应</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="79"/>
        <source>Success: Write confirmed</source>
        <translation>成功：写入确认</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="88"/>
        <source>Success: Broadcast write sent, no response expected</source>
        <translation>成功：广播写已发送，预期无响应</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="105"/>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation>发送读请求 FC:%1 地址:%2 数量:%3 从站:%4</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="116"/>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation>发送写请求 FC:%1 地址:%2 数据:%3 从站:%4</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="124"/>
        <source>Sending Raw Data: %1</source>
        <translation>发送原始数据：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="155"/>
        <source>Poll Summary: %1 ok, %2 err, %3 retry, avg RTT %4 ms</source>
        <translation>轮询汇总：成功 %1，错误 %2，重试 %3，平均耗时 %4 ms</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="162"/>
        <source> (raw frames: %1 shown, %2 sampled)</source>
        <translation>（原始帧：已显示 %1，已抽样省略 %2）</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="175"/>
        <source>retry</source>
        <translation>次重试</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="176"/>
        <source>retries</source>
        <translation>次重试</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="190"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="192"/>
        <source>Error: %1 (after %2 %3)</source>
        <translation>错误：%1（经 %2 %3）</translation>
    </message>
</context>
<context>
    <name>ui::common::FileExporter</name>
    <message>
        <location filename="../common/FileExporter.cpp" line="23"/>
        <source>Failed while writing text stream</source>
        <translation>写入文本流时失败</translation>
    </message>
</context>
<context>
    <name>ui::common::ModbusDataHelper</name>
    <message>
        <location filename="../common/ModbusDataHelper.cpp" line="201"/>
        <location filename="../common/ModbusDataHelper.cpp" line="237"/>
        <source>Expected quantity must be greater than zero</source>
        <translation>期望数量必须大于零</translation>
    </message>
    <message>
        <location filename="../common/ModbusDataHelper.cpp" line="241"/>
        <source>Unsupported radix (only 10, 16, and 2 supported)</source>
        <translation>不支持的进制（仅支持 10、16 和 2）</translation>
    </message>
    <message>
        <location filename="../common/ModbusDataHelper.cpp" line="210"/>
        <source>Invalid bit character: only &apos;0&apos; and &apos;1&apos; are allowed</source>
        <translation>非法的位字符：仅允许 &apos;0&apos; 和 &apos;1&apos;</translation>
    </message>
    <message>
        <location filename="../common/ModbusDataHelper.cpp" line="216"/>
        <source>Bit count (%1) does not match expected quantity (%2)</source>
        <translation>位数量 (%1) 与期望数量 (%2) 不匹配</translation>
    </message>
    <message>
        <location filename="../common/ModbusDataHelper.cpp" line="247"/>
        <source>Word count (%1) does not match expected quantity (%2)</source>
        <translation>字数量 (%1) 与期望数量 (%2) 不匹配</translation>
    </message>
    <message>
        <location filename="../common/ModbusDataHelper.cpp" line="273"/>
        <source>Invalid register value: &apos;%1&apos;</source>
        <translation>非法的寄存器数值：&apos;%1&apos;</translation>
    </message>
</context>
<context>
    <name>ui::common::UpdateChecker</name>
    <message>
        <location filename="../common/UpdateChecker.cpp" line="114"/>
        <source>Invalid release metadata JSON: %1</source>
        <translation>无效的版本元数据 JSON：%1</translation>
    </message>
    <message>
        <location filename="../common/UpdateChecker.cpp" line="115"/>
        <source>Invalid release metadata format</source>
        <translation>无效的版本元数据格式</translation>
    </message>
    <message>
        <location filename="../common/UpdateChecker.cpp" line="137"/>
        <source>Release tag is missing</source>
        <translation>发布标签缺失</translation>
    </message>
</context>
<context>
    <name>ui::common::connection_alert</name>
    <message>
        <location filename="../common/ConnectionAlert.h" line="29"/>
        <source>Not Connected</source>
        <translation>未连接</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="30"/>
        <source>Please connect first.</source>
        <translation>请先连接设备。</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="39"/>
        <source>Connection Lost</source>
        <translation>连接中断</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="40"/>
        <source>Connection was closed.</source>
        <translation>连接已关闭。</translation>
    </message>
</context>
<context>
    <name>ui::shell::NavigationController</name>
    <message>
        <location filename="../shell/NavigationController.cpp" line="145"/>
        <source>Expand Navigation</source>
        <translation>展开导航</translation>
    </message>
    <message>
        <location filename="../shell/NavigationController.cpp" line="146"/>
        <source>Collapse Navigation</source>
        <translation>折叠导航</translation>
    </message>
</context>
<context>
    <name>ui::views::analyzer::FrameAnalyzerView</name>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="174"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1108"/>
        <source>Frame Input</source>
        <translation>报文输入</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="180"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1109"/>
        <source>Protocol:</source>
        <translation>协议：</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="183"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1111"/>
        <source>Auto Detect</source>
        <translation>自动检测</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="184"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1112"/>
        <source>Modbus TCP</source>
        <translation>Modbus TCP</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="185"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1113"/>
        <source>Modbus RTU</source>
        <translation>Modbus RTU</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="186"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1114"/>
        <source>Modbus ASCII</source>
        <translation>Modbus ASCII</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="226"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1177"/>
        <source>Paste &amp; Parse</source>
        <translation>粘贴并解析</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1178"/>
        <source>Paste clipboard content and parse immediately (Ctrl+Shift+V)</source>
        <translation>粘贴剪贴板内容并立即解析 (Ctrl+Shift+V)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1174"/>
        <source>Parse current input (Ctrl+Enter)</source>
        <translation>解析当前输入 (Ctrl+Enter)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="551"/>
        <source>Clipboard is empty</source>
        <translation>剪贴板为空</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="219"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1173"/>
        <source>Parse</source>
        <translation>解析</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="213"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1171"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="240"/>
        <source>Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)</source>
        <translation>输入十六进制字符串（如 01 03 00 00 00 01 84 0A）</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1182"/>
        <source>Enter Hex string (e.g., RTU: 01 03 00 00 00 01 84 0A, ASCII bytes: 3A 30 31 30 33 ... 0D 0A)</source>
        <translation>输入十六进制字符串（如 RTU: 01 03 00 00 00 01 84 0A，ASCII 字节: 3A 30 31 30 33 ... 0D 0A）</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="251"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1185"/>
        <source>Analysis Result</source>
        <translation>分析结果</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="267"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1146"/>
        <source>Status:</source>
        <translation>状态：</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="271"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="958"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1164"/>
        <source>Ready</source>
        <translation>就绪</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="280"/>
        <source>Tip: &quot;Pause&quot; to edit description</source>
        <translation>提示：&quot;暂停&quot;以编辑描述</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="297"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="969"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1035"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1144"/>
        <source>Pause Refresh</source>
        <translation>暂停刷新</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1035"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1144"/>
        <source>Resume Refresh</source>
        <translation>恢复刷新</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="305"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1142"/>
        <source>Stop Link</source>
        <translation>停止联动</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="668"/>
        <source>Import Config</source>
        <translation>导入配置</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="651"/>
        <source>Export Config</source>
        <translation>导出配置</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="405"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1193"/>
        <source>Field</source>
        <translation>字段</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="405"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1194"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="405"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1195"/>
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="409"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="883"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="982"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1188"/>
        <source>Structure</source>
        <translation>结构</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1118"/>
        <source>Start Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>起始地址 (0-65535)。支持 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="421"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="473"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1186"/>
        <source>History</source>
        <translation>历史记录</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="425"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1200"/>
        <source>Clear History</source>
        <translation>清空历史</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="651"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="668"/>
        <source>JSON Files (*.json)</source>
        <translation>JSON 文件 (*.json)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="662"/>
        <source>Export Failed</source>
        <translation>导出失败</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="489"/>
        <source>OK</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="489"/>
        <source>ERR</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="416"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1189"/>
        <source>Decoded Data</source>
        <translation>解析数据</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="603"/>
        <source>Invalid Address (0-65535): %1</source>
        <translation>无效地址 (0-65535): %1</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="632"/>
        <source>Parse Failed</source>
        <translation>解析失败</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="673"/>
        <source>Import Failed</source>
        <translation>导入失败</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1009"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1153"/>
        <source>LIVE: %1</source>
        <translation>联动中: %1</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1013"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1155"/>
        <source>Live Data Received at %1</source>
        <translation>实时数据接收于 %1</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="595"/>
        <source>Error: Empty input</source>
        <translation>错误：输入为空</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="192"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1116"/>
        <source>Start Addr:</source>
        <translation>起始地址：</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="193"/>
        <source>Start Address (for Response)</source>
        <translation>起始地址（用于响应帧解析）</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="207"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1170"/>
        <source>Format</source>
        <translation>格式化</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="208"/>
        <source>Format Hex String (Ctrl+Shift+F)</source>
        <translation>格式化十六进制字符串 (Ctrl+Shift+F)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="214"/>
        <source>Clear Input and Results</source>
        <translation>清空输入与解析结果</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="220"/>
        <source>Parse Input (Ctrl+Enter)</source>
        <translation>解析输入 (Ctrl+Enter)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="227"/>
        <source>Paste Clipboard and Parse (Ctrl+Shift+V)</source>
        <translation>粘贴剪贴板并解析 (Ctrl+Shift+V)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="317"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1120"/>
        <source>Decode:</source>
        <translation>解码：</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="318"/>
        <source>Decode Mode</source>
        <translation>解码类型</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="320"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1122"/>
        <source>UInt16</source>
        <translation>UInt16</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="321"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1123"/>
        <source>Int16</source>
        <translation>Int16</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="322"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1124"/>
        <source>Float32</source>
        <translation>Float32</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="323"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1125"/>
        <source>Int32</source>
        <translation>Int32</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="324"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1126"/>
        <source>UInt32</source>
        <translation>UInt32</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="325"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1127"/>
        <source>Float64</source>
        <translation>Float64</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="339"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1129"/>
        <source>Order:</source>
        <translation>字节序：</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="340"/>
        <source>Byte Order</source>
        <translation>字节序</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="342"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1131"/>
        <source>ABCD</source>
        <translation>ABCD</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="359"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="719"/>
        <source>Reset</source>
        <translation>重置</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="371"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1136"/>
        <source>Import</source>
        <translation>导入</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="372"/>
        <source>Import Register Config (JSON)</source>
        <translation>导入寄存器配置 (JSON)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="374"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1137"/>
        <source>Export</source>
        <translation>导出</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="375"/>
        <source>Export Register Config (JSON)</source>
        <translation>导出寄存器配置 (JSON)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="377"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1138"/>
        <source>CSV</source>
        <translation>CSV</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="378"/>
        <source>Export Decoded Data to CSV</source>
        <translation>导出解码数据为 CSV</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="473"/>
        <source>History ◂</source>
        <translation>历史记录 ◂</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="609"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1161"/>
        <source>Parsing...</source>
        <translation>解析中...</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="715"/>
        <source>Reset (%1)</source>
        <translation>重置 (%1)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="914"/>
        <source>Parse Failed: %1</source>
        <translation>解析失败：%1</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="920"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1149"/>
        <source>TCP</source>
        <translation>TCP</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="921"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1150"/>
        <source>RTU</source>
        <translation>RTU</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="896"/>
        <source>Frame</source>
        <translation>帧</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="897"/>
        <source>%1 bytes</source>
        <translation>%1 字节</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="800"/>
        <source>MBAP Header</source>
        <translation>MBAP 头</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="802"/>
        <source>Transaction ID</source>
        <translation>事务 ID</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="805"/>
        <source>Protocol ID</source>
        <translation>协议 ID</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="808"/>
        <source>Length</source>
        <translation>长度</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="811"/>
        <source>Unit ID</source>
        <translation>单元 ID</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="815"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="824"/>
        <source>Slave ID</source>
        <translation>从站 ID</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="847"/>
        <source>PDU</source>
        <translation>PDU</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="849"/>
        <source>Function Code</source>
        <translation>功能码</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="854"/>
        <source>Exception Code</source>
        <translation>异常码</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="863"/>
        <source>CRC16</source>
        <translation>CRC16</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="474"/>
        <source>Show History</source>
        <translation>显示历史</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="474"/>
        <source>Hide History</source>
        <translation>隐藏历史</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="496"/>
        <source>Local time %1</source>
        <translation>Local time %1</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="362"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="720"/>
        <source>Reset all custom register types to Default</source>
        <translation>重置所有自定义寄存器类型为默认</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="716"/>
        <source>Reset selected %1 register type(s) to Default (Delete)</source>
        <translation>重置所选 %1 个寄存器类型为默认 (Delete)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="922"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1151"/>
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="884"/>
        <source>(Unavailable in Live Mode)</source>
        <translation>(Unavailable in Live Mode)</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="885"/>
        <source>Logical parsing is disabled for high-frequency linkage</source>
        <translation>高频联动下已禁用逻辑解析</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="900"/>
        <source>Frame Bytes</source>
        <translation>帧字节</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="900"/>
        <source>Complete raw frame</source>
        <translation>完整原始帧</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="801"/>
        <source>Transaction + Protocol + Length + Unit ID</source>
        <translation>事务 + 协议 + 长度 + 单元 ID</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="804"/>
        <source>Request/response correlation ID</source>
        <translation>请求/响应关联 ID</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="807"/>
        <source>Modbus TCP protocol identifier</source>
        <translation>Modbus TCP 协议标识符</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="810"/>
        <source>Remaining bytes after this field</source>
        <translation>此字段之后的剩余字节数</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="813"/>
        <source>Target slave / unit address</source>
        <translation>目标从站 / 单元地址</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="817"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="826"/>
        <source>Target slave address</source>
        <translation>目标从站地址</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="818"/>
        <source>ASCII Start</source>
        <translation>ASCII Start</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="819"/>
        <source>Start delimiter &apos;:&apos;</source>
        <translation>Start delimiter &apos;:&apos;</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="820"/>
        <source>ASCII Payload</source>
        <translation>ASCII Payload</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="822"/>
        <source>ASCII hex payload before CRLF</source>
        <translation>ASCII hex payload before CRLF</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="847"/>
        <source>Function code + payload</source>
        <translation>功能码 + 负载</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="850"/>
        <source>Normal response</source>
        <translation>正常响应</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="850"/>
        <source>Exception response</source>
        <translation>异常响应</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="851"/>
        <source>Payload</source>
        <translation>负载</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="852"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="856"/>
        <source>Exception detail payload</source>
        <translation>异常详情负载</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="852"/>
        <source>Application data payload</source>
        <translation>应用数据负载</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="859"/>
        <source>CRC valid</source>
        <translation>CRC 有效</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="859"/>
        <source>CRC invalid</source>
        <translation>CRC 无效</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="861"/>
        <source>Expected 0x%1</source>
        <translation>预期 0x%1</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="867"/>
        <source>LRC</source>
        <translation>LRC</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="870"/>
        <source>LRC valid</source>
        <translation>LRC valid</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="870"/>
        <source>LRC invalid</source>
        <translation>LRC invalid</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="871"/>
        <source>CRLF</source>
        <translation>CRLF</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="871"/>
        <source>ASCII frame terminator</source>
        <translation>ASCII frame terminator</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="943"/>
        <source>Success (%1)</source>
        <translation>成功（%1）</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="945"/>
        <source>Forced Parsing</source>
        <translation>强制解析</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="948"/>
        <source>Warnings</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="989"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1188"/>
        <source>Structure (Unavailable in Live Mode)</source>
        <translation>结构（联动模式不可用）</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="924"/>
        <source>Request</source>
        <translation>请求</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="925"/>
        <source>Response</source>
        <translation>响应</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="926"/>
        <source>Exception</source>
        <translation>异常</translation>
    </message>
    <message>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="922"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="926"/>
        <location filename="../views/analyzer/FrameAnalyzerView.cpp" line="1152"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
</context>
<context>
    <name>ui::views::converter::Ieee754ConverterWidget</name>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="107"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="285"/>
        <source>Precision</source>
        <translation>精度</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="113"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="286"/>
        <source>Float32 (32-bit, 2 Regs)</source>
        <translation>Float32 (单精度 32位 / 2寄存器)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="114"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="287"/>
        <source>Float64 (64-bit, 4 Regs)</source>
        <translation>Float64 (双精度 64位 / 4寄存器)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="127"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="288"/>
        <source>Quick Presets</source>
        <translation>快捷预置</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="158"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="289"/>
        <source>Max Normal</source>
        <translation>最大规格化数</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="167"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="290"/>
        <source>+Infinity</source>
        <translation>+无穷大</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="168"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="291"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="768"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="818"/>
        <source>NaN</source>
        <translation>非数 (NaN)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="177"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="292"/>
        <source>Interactive Inputs (Two-Way Live Sync)</source>
        <translation>交互输入区 (双向实时联动)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="183"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="293"/>
        <source>Floating Point:</source>
        <translation>浮点数值:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="185"/>
        <source>e.g. 123.456, -1.0, 1e-5, NaN, Inf</source>
        <translation>例如 123.456, -1.0, 1e-5, NaN, Inf</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="189"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="294"/>
        <source>Hex (Big-Endian):</source>
        <translation>十六进制 (大端序):</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="191"/>
        <source>e.g. 42 F6 E9 79 or 42F6E979</source>
        <translation>例如 42 F6 E9 79 或 42F6E979</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="195"/>
        <source>Signed Integer:</source>
        <translation>有符号整数:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="200"/>
        <source>Unsigned Integer:</source>
        <translation>无符号整数:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="207"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="297"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="722"/>
        <source>Editing any field instantly derives all others in real-time.</source>
        <translation>编辑任意输入字段，其他字段与字节序矩阵均即时自动推导。</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="218"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="298"/>
        <source>Four-Endian Live Matrix (Modbus Permutations)</source>
        <translation>四种 Modbus 字节序换算矩阵</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="237"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="299"/>
        <source>IEEE 754 Binary Bitfield Breakdown</source>
        <translation>IEEE 754 二进制位域剖析</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="295"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="342"/>
        <source>Signed Int32:</source>
        <translation>有符号 Int32:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="295"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="342"/>
        <source>Signed Int64:</source>
        <translation>有符号 Int64:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="296"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="343"/>
        <source>Unsigned UInt32:</source>
        <translation>无符号 UInt32:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="296"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="343"/>
        <source>Unsigned UInt64:</source>
        <translation>无符号 UInt64:</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="303"/>
        <source>Endian Order</source>
        <translation>字节序模式</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="304"/>
        <source>Raw Hex</source>
        <translation>原始十六进制</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="305"/>
        <source>Modbus Register Words</source>
        <translation>Modbus 寄存器字</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="306"/>
        <source>Floating Value</source>
        <translation>浮点解析值</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="307"/>
        <source>Signed Int</source>
        <translation>有符号整数</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="308"/>
        <source>Unsigned Int</source>
        <translation>无符号整数</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="309"/>
        <source>Actions</source>
        <translation>操作</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="386"/>
        <source>Invalid floating-point value (use finite number, nan, inf, or -inf).</source>
        <translation>无效的浮点数值（请输入有限数值、nan、inf 或 -inf）。</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="439"/>
        <source>Invalid hex value (expected up to 8 hex digits).</source>
        <translation>无效的十六进制值（最多允许 8 位十六进制字符）。</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="440"/>
        <source>Invalid hex value (expected up to 16 hex digits).</source>
        <translation>无效的十六进制值（最多允许 16 位十六进制字符）。</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="477"/>
        <source>Invalid signed 32-bit integer.</source>
        <translation>无效的 32 位有符号整数。</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="478"/>
        <source>Invalid signed 64-bit integer.</source>
        <translation>无效的 64 位有符号整数。</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="511"/>
        <source>Invalid unsigned 32-bit integer.</source>
        <translation>无效的 32 位无符号整数。</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="512"/>
        <source>Invalid unsigned 64-bit integer.</source>
        <translation>无效的 64 位无符号整数。</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="593"/>
        <source>Big-Endian (Standard / High-Word First)</source>
        <translation>大端序 (标准 / 高字在前)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="593"/>
        <source>Big-Endian</source>
        <translation>大端序 (Big-Endian)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="594"/>
        <source>Word Swap (Little-Endian Words / Middle-Endian)</source>
        <translation>字颠倒 (低字在前 / 中端序)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="594"/>
        <source>Word Swap</source>
        <translation>字交换 (Word Swap)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="595"/>
        <source>Byte Swap (Word-Level Endian Inversion)</source>
        <translation>字节颠倒 (字内字节反转)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="595"/>
        <source>Byte Swap</source>
        <translation>字节交换 (Byte Swap)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="596"/>
        <source>Little-Endian (True Little-Endian / Low-Word First)</source>
        <translation>小端序 (完全小端 / 低字在前)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="596"/>
        <source>Little-Endian</source>
        <translation>小端序 (Little-Endian)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="694"/>
        <source>Copy Hex</source>
        <translation>复制 Hex</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="697"/>
        <source>Copy Hex formatted bytes</source>
        <translation>复制十六进制字节串</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="701"/>
        <source>Copy Words</source>
        <translation>复制寄存器字</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="704"/>
        <source>Copy Modbus Register Words</source>
        <translation>复制 Modbus 寄存器字</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="765"/>
        <source>Sign [Bit 31]: %1 (%2)</source>
        <translation>符号位 [Bit 31]: %1 (%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="765"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="815"/>
        <source>Positive +</source>
        <translation>正数 (+)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="765"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="815"/>
        <source>Negative -</source>
        <translation>负数 (-)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="768"/>
        <source>Exp [Bits 30-23]: 0xFF (Special: %1)</source>
        <translation>阶码 [Bits 30-23]: 0xFF (特殊值: %1)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="768"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="818"/>
        <source>Infinity</source>
        <translation>无穷大</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="770"/>
        <source>Exp [Bits 30-23]: 0 (Subnormal, 2^-126)</source>
        <translation>阶码 [Bits 30-23]: 0 (非规格化数, 2^-126)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="772"/>
        <source>Exp [Bits 30-23]: %1 (Actual: %1 - 127 = %2, 2^%2)</source>
        <translation>阶码 [Bits 30-23]: %1 (真实指数: %1 - 127 = %2, 2^%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="782"/>
        <source>Mantissa [Bits 22-0]: %1 (Significand: %2)</source>
        <translation>尾数 [Bits 22-0]: %1 (有效数: %2)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="789"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="839"/>
        <source>Formula: (-1)^%1 × ∞ = %2</source>
        <translation>公式：(-1)^%1 × ∞ = %2</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="790"/>
        <source>Formula: NaN (Exp = 0xFF, Frac = 0x%1 ≠ 0)</source>
        <translation>公式：NaN (Exp = 0xFF, Frac = 0x%1 ≠ 0)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="793"/>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="843"/>
        <source>Formula: (-1)^%1 × 0 = %2</source>
        <translation>公式：(-1)^%1 × 0 = %2</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="794"/>
        <source>Formula: (-1)^%1 × 2^(-126) × (0 + 0x%2/2^23) = %3</source>
        <translation>公式：(-1)^%1 × 2^(-126) × (0 + 0x%2/2^23) = %3</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="796"/>
        <source>Formula: (-1)^%1 × 2^(%2 - 127) × (1 + 0x%3/2^23) = %4</source>
        <translation>计算公式: (-1)^%1 × 2^(%2 - 127) × (1 + 0x%3/2^23) = %4</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="815"/>
        <source>Sign [Bit 63]: %1 (%2)</source>
        <translation>符号位 [Bit 63]: %1 (%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="818"/>
        <source>Exp [Bits 62-52]: 0x7FF (Special: %1)</source>
        <translation>阶码 [Bits 62-52]: 0x7FF (特殊值: %1)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="820"/>
        <source>Exp [Bits 62-52]: 0 (Subnormal, 2^-1022)</source>
        <translation>阶码 [Bits 62-52]: 0 (非规格化数, 2^-1022)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="822"/>
        <source>Exp [Bits 62-52]: %1 (Actual: %1 - 1023 = %2, 2^%2)</source>
        <translation>阶码 [Bits 62-52]: %1 (真实指数: %1 - 1023 = %2, 2^%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="832"/>
        <source>Mantissa [Bits 51-0]: %1 (Significand: %2)</source>
        <translation>尾数 [Bits 51-0]: %1 (有效数: %2)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="840"/>
        <source>Formula: NaN (Exp = 0x7FF, Frac = 0x%1 ≠ 0)</source>
        <translation>公式：NaN (Exp = 0x7FF, Frac = 0x%1 ≠ 0)</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="844"/>
        <source>Formula: (-1)^%1 × 2^(-1022) × (0 + 0x%2/2^52) = %3</source>
        <translation>公式：(-1)^%1 × 2^(-1022) × (0 + 0x%2/2^52) = %3</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="846"/>
        <source>Formula: (-1)^%1 × 2^(%2 - 1023) × (1 + Frac) = %3</source>
        <translation>计算公式: (-1)^%1 × 2^(%2 - 1023) × (1 + Frac) = %3</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="856"/>
        <source>Copied Hex: %1</source>
        <translation>已复制十六进制: %1</translation>
    </message>
    <message>
        <location filename="../views/converter/Ieee754ConverterWidget.cpp" line="865"/>
        <source>Copied Register Words: %1</source>
        <translation>已复制寄存器字: %1</translation>
    </message>
</context>
<context>
    <name>ui::views::converter::ModbusFrameBuilderWidget</name>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="621"/>
        <source>Address range exceeds 65535.</source>
        <translation>地址范围超出 65535。</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="84"/>
        <source>Read Coils (0x01)</source>
        <translation>读线圈 (0x01)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="86"/>
        <source>Read Discrete Inputs (0x02)</source>
        <translation>读离散输入 (0x02)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="88"/>
        <source>Read Holding Registers (0x03)</source>
        <translation>读保持寄存器 (0x03)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="90"/>
        <source>Read Input Registers (0x04)</source>
        <translation>读输入寄存器 (0x04)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="92"/>
        <source>Write Single Coil (0x05)</source>
        <translation>写单线圈 (0x05)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="94"/>
        <source>Write Single Register (0x06)</source>
        <translation>写单寄存器 (0x06)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="96"/>
        <source>Write Multiple Coils (0x0F)</source>
        <translation>写多线圈 (0x0F)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="98"/>
        <source>Write Multiple Registers (0x10)</source>
        <translation>写多寄存器 (0x10)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="218"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="410"/>
        <source>Frame Parameters</source>
        <translation>帧参数</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="224"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="411"/>
        <source>Protocol:</source>
        <translation>协议:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="232"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="488"/>
        <source>Slave ID:</source>
        <translation>从机地址:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="235"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="491"/>
        <source>1 - 247</source>
        <translation>1 - 247</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="240"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="412"/>
        <source>Function Code:</source>
        <translation>功能码:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="261"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="414"/>
        <source>Address Base:</source>
        <translation>地址基准:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="263"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="432"/>
        <source>0-Based (PDU: 0-65535)</source>
        <translation>0 基准 (PDU: 0-65535)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="264"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="433"/>
        <source>1-Based (PLC: 1-65536)</source>
        <translation>1 基准 (PLC: 1-65536)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="269"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="415"/>
        <source>Start Address:</source>
        <translation>起始地址:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="272"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="413"/>
        <source>e.g. 0, 0x0000, 40001</source>
        <translation>例如 0, 0x0000, 40001</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="276"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="416"/>
        <source>Quantity:</source>
        <translation>数量:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="287"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="417"/>
        <source>Write Data:</source>
        <translation>写入数据:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="289"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="537"/>
        <source>e.g. 0001 0002 or 1, 2</source>
        <translation>例如 0001 0002 或 1, 2</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="293"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="418"/>
        <source>Data Format:</source>
        <translation>数据格式:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="304"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="419"/>
        <source>Generated Frame</source>
        <translation>生成报文</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="311"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="420"/>
        <source>Full Frame Hex:</source>
        <translation>完整报文 Hex:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="330"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="427"/>
        <source>Field</source>
        <translation>字段</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="295"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="330"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="427"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="438"/>
        <source>Hex</source>
        <translation>Hex</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="151"/>
        <source>Invalid coil value. Use ON/OFF, 1/0, or FF00/0000 in the selected format.</source>
        <translation>线圈值无效。请在所选格式下输入 ON/OFF、1/0 或 FF00/0000。</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="162"/>
        <source>Invalid register data or count. Use Hex bytes, Decimal values (0-65535), or 16 MSB-first bits per register.</source>
        <translation>寄存器数据或数量无效。请输入十六进制字节、十进制数值 (0-65535) 或每个寄存器 16 位高位优先二进制。</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="173"/>
        <source>Invalid coil data or count. Use packed Hex bytes or one Decimal/Binary 0 or 1 per coil.</source>
        <translation>线圈数据或数量无效。请输入打包的十六进制字节，或每个线圈一个十进制/二进制 0 或 1。</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="296"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="439"/>
        <source>Decimal</source>
        <translation>十进制</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="297"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="440"/>
        <source>Binary</source>
        <translation>二进制</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="330"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="427"/>
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="344"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="421"/>
        <source>Copy Hex (spaced)</source>
        <translation>复制 Hex (空格分隔)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="345"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="422"/>
        <source>Copy Hex (compact)</source>
        <translation>复制 Hex (紧凑)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="346"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="423"/>
        <source>Copy C array</source>
        <translation>复制 C 数组</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="347"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="424"/>
        <source>Inspect in Frame Analyzer</source>
        <translation>在报文分析器中验证</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="488"/>
        <source>Unit ID:</source>
        <translation>单元标识:</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="491"/>
        <source>1 - 255</source>
        <translation>1 - 255</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="531"/>
        <source>e.g. ON, OFF, 1, 0, or FF00</source>
        <translation>例如 ON, OFF, 1, 0, 或 FF00</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="533"/>
        <source>e.g. 1234 or 0x1234</source>
        <translation>例如 1234 或 0x1234</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="535"/>
        <source>e.g. 1 0 1 1 (matching quantity)</source>
        <translation>例如 1 0 1 1 (与数量匹配)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="587"/>
        <source>Invalid %1 (%2). Valid range: 1 - %3.</source>
        <translation>无效的 %1 (%2)。有效范围: 1 - %3。</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="588"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="711"/>
        <source>Unit ID</source>
        <translation>单元标识</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="588"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="640"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="770"/>
        <source>Slave ID</source>
        <translation>从机地址</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="596"/>
        <source>Invalid start address: %1</source>
        <translation>无效的起始地址: %1</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="616"/>
        <source>Failed to build Modbus PDU.</source>
        <translation>构建 Modbus PDU 失败。</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="642"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="772"/>
        <source>Device address %1 (0x%2)</source>
        <translation>设备地址 %1 (0x%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="645"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="716"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="774"/>
        <source>Function Code</source>
        <translation>功能码</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="654"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="678"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="725"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="749"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="782"/>
        <source>Start Address</source>
        <translation>起始地址</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="656"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="663"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="671"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="680"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="727"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="734"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="742"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="751"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="784"/>
        <source>Address %1 (0x%2)</source>
        <translation>地址 %1 (0x%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="657"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="681"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="728"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="752"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="785"/>
        <source>Quantity</source>
        <translation>数量</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="659"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="730"/>
        <source>%1 %2 (0x%3)</source>
        <translation>%1 %2 (0x%3)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="659"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="683"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="730"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="754"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="787"/>
        <source>coils</source>
        <translation>线圈</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="659"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="683"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="730"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="754"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="787"/>
        <source>registers</source>
        <translation>寄存器</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="661"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="732"/>
        <source>Coil Address</source>
        <translation>线圈地址</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="665"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="736"/>
        <source>Output Value</source>
        <translation>输出值</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="667"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="738"/>
        <source>Coil ON (0xFF00)</source>
        <translation>线圈接通 (0xFF00)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="667"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="738"/>
        <source>Coil OFF (0x0000)</source>
        <translation>线圈断开 (0x0000)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="669"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="740"/>
        <source>Register Address</source>
        <translation>寄存器地址</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="673"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="744"/>
        <source>Register Value</source>
        <translation>寄存器数值</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="675"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="746"/>
        <source>Value: %1 (0x%2)</source>
        <translation>数值: %1 (0x%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="683"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="754"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="787"/>
        <source>%1 %2</source>
        <translation>%1 %2</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="684"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="755"/>
        <source>Byte Count</source>
        <translation>字节数</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="686"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="757"/>
        <source>%1 bytes</source>
        <translation>%1 字节</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="688"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="759"/>
        <source>Write Payload</source>
        <translation>写入负载</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="690"/>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="761"/>
        <source>Packed output data (%1 bytes)</source>
        <translation>打包输出数据 (%1 字节)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="694"/>
        <source>CRC-16</source>
        <translation>CRC-16</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="696"/>
        <source>CRC-16/MODBUS (LE: Low byte first)</source>
        <translation>CRC-16/MODBUS (小端: 低字节在前)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="701"/>
        <source>Transaction ID</source>
        <translation>事务 ID</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="703"/>
        <source>Transaction Identifier (0x0000)</source>
        <translation>事务标识符 (0x0000)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="704"/>
        <source>Protocol ID</source>
        <translation>协议 ID</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="706"/>
        <source>0x0000 = Modbus Protocol</source>
        <translation>0x0000 = Modbus 协议</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="708"/>
        <source>Length</source>
        <translation>长度</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="710"/>
        <source>%1 bytes following</source>
        <translation>后续 %1 字节</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="713"/>
        <source>Unit identifier %1 (0x%2)</source>
        <translation>单元标识符 %1 (0x%2)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="766"/>
        <source>Start Delimiter</source>
        <translation>起始定界符</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="768"/>
        <source>ASCII Colon delimiter</source>
        <translation>ASCII 冒号定界符</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="790"/>
        <source>PDU Payload</source>
        <translation>PDU 负载</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="792"/>
        <source>ASCII encoded PDU data</source>
        <translation>ASCII 编码 PDU 数据</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="795"/>
        <source>LRC Checksum</source>
        <translation>LRC 校验和</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="797"/>
        <source>Longitudinal Redundancy Check</source>
        <translation>纵向冗余校验 (LRC)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="799"/>
        <source>End Delimiter</source>
        <translation>结束定界符</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="801"/>
        <source>Carriage Return &amp; Line Feed</source>
        <translation>回车换行符 (CRLF)</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="811"/>
        <source>Spaced Hex copied to clipboard</source>
        <translation>已复制空格分隔 Hex 到剪贴板</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="820"/>
        <source>Compact Hex copied to clipboard</source>
        <translation>已复制紧凑 Hex 到剪贴板</translation>
    </message>
    <message>
        <location filename="../views/converter/ModbusFrameBuilderWidget.cpp" line="843"/>
        <source>C array declaration copied to clipboard</source>
        <translation>已复制 C 数组声明到剪贴板</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus::ModbusPage</name>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="349"/>
        <source>Protocol:</source>
        <translation>协议:</translation>
    </message>
</context>
<context>
    <name>ui::views::network::NetworkDebuggerView</name>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="305"/>
        <source>Connecting to %1:%2...</source>
        <translation>正在连接 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="323"/>
        <source>Starting TCP server on %1:%2...</source>
        <translation>正在启动 TCP 服务端 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="340"/>
        <source>Stopping TCP server...</source>
        <translation>正在停止 TCP 服务端...</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="355"/>
        <source>Binding UDP %1:%2 -&gt; %3:%4...</source>
        <translation>正在绑定 UDP %1:%2 -&gt; %3:%4...</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="359"/>
        <source>Binding UDP %1:%2...</source>
        <translation>正在绑定 UDP %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="413"/>
        <source>Server mode: no connected clients available</source>
        <translation>服务端模式：当前无已连接客户端</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="414"/>
        <source>Server mode: select at least one client</source>
        <translation>服务端模式：请至少选择一个客户端</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="451"/>
        <source>Closed</source>
        <translation>已关闭</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="452"/>
        <source>Opening</source>
        <translation>正在打开</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="453"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="454"/>
        <source>Closing</source>
        <translation>正在关闭</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="455"/>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="590"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="456"/>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="591"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="498"/>
        <source>State changed: %1</source>
        <translation>状态已变更：%1</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="516"/>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="631"/>
        <source>Connection failed</source>
        <translation>连接失败</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="517"/>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="632"/>
        <source>Connection timeout</source>
        <translation>连接超时</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="518"/>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="633"/>
        <source>Write failed</source>
        <translation>写入失败</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="519"/>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="634"/>
        <source>Read failed</source>
        <translation>读取失败</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="520"/>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="635"/>
        <source>Port not found</source>
        <translation>找不到端口</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="521"/>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="636"/>
        <source>Permission denied</source>
        <translation>权限被拒绝</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="522"/>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="637"/>
        <source>Connection reset by peer</source>
        <translation>连接被对端重置</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="523"/>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="638"/>
        <source>Unknown error</source>
        <translation>未知错误</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="525"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="548"/>
        <source>Client #%1 connected: %2</source>
        <translation>客户端 #%1 已连接：%2</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="555"/>
        <source>Client #%1 disconnected</source>
        <translation>客户端 #%1 已断开</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="588"/>
        <source>Listening</source>
        <translation>监听中</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="589"/>
        <source>Stopped</source>
        <translation>已停止</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="620"/>
        <source>Server state: %1</source>
        <translation>服务端状态：%1</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="640"/>
        <source>Server Error: %1</source>
        <translation>服务端错误：%1</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="678"/>
        <source>Send Data</source>
        <translation>发送数据</translation>
    </message>
</context>
<context>
    <name>ui::views::network::Protocol</name>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="56"/>
        <source>TCP Client</source>
        <translation>TCP 客户端</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="57"/>
        <source>TCP Server</source>
        <translation>TCP 服务端</translation>
    </message>
    <message>
        <location filename="../views/network/NetworkDebuggerView.cpp" line="58"/>
        <source>UDP</source>
        <translation>UDP</translation>
    </message>
</context>
<context>
    <name>ui::views::serial::SerialDebuggerView</name>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="152"/>
        <source>Opening %1...</source>
        <translation>正在打开 %1...</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="29"/>
        <source>Closed</source>
        <translation>已关闭</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="31"/>
        <source>Opening</source>
        <translation>正在打开</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="33"/>
        <source>Open</source>
        <translation>已打开</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="35"/>
        <source>Closing</source>
        <translation>正在关闭</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="37"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="39"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="211"/>
        <source>State changed: %1</source>
        <translation>状态已变更：%1</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="235"/>
        <source>Connection failed</source>
        <translation>连接失败</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="236"/>
        <source>Connection timeout</source>
        <translation>连接超时</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="237"/>
        <source>Write failed</source>
        <translation>写入失败</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="238"/>
        <source>Read failed</source>
        <translation>读取失败</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="239"/>
        <source>Port not found</source>
        <translation>找不到端口</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="240"/>
        <source>Permission denied</source>
        <translation>权限被拒绝</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="241"/>
        <source>Connection reset</source>
        <translation>连接重置</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="242"/>
        <source>Unknown error</source>
        <translation>未知错误</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="247"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="282"/>
        <source>Control</source>
        <translation>控制</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="283"/>
        <source>Send Data</source>
        <translation>发送数据</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="284"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../views/serial/SerialDebuggerView.cpp" line="285"/>
        <source>RTS</source>
        <translation>RTS</translation>
    </message>
</context>
<context>
    <name>ui::views::tools::UtilitiesView</name>
    <message>
        <location filename="../views/tools/UtilitiesView.cpp" line="28"/>
        <location filename="../views/tools/UtilitiesView.cpp" line="48"/>
        <source>IEEE 754 Converter</source>
        <translation>IEEE 754 转换器</translation>
    </message>
    <message>
        <location filename="../views/tools/UtilitiesView.cpp" line="31"/>
        <location filename="../views/tools/UtilitiesView.cpp" line="49"/>
        <source>Frame Builder</source>
        <translation>帧构建器</translation>
    </message>
</context>
<context>
    <name>ui::widgets::BaseConnectionWidget</name>
    <message>
        <location filename="../widgets/BaseConnectionWidget.cpp" line="77"/>
        <source>Connection Settings</source>
        <translation>连接设置</translation>
    </message>
</context>
<context>
    <name>ui::widgets::ByteMonitorWidget</name>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="168"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="347"/>
        <source>Cannot write file: %1 (%2)</source>
        <translation>无法写入文件：%1 (%2)</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="385"/>
        <source>[%1] [TX] %2</source>
        <translation>[%1] [TX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="392"/>
        <source>[%1] [RX] %2</source>
        <translation>[%1] [RX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="397"/>
        <source>[INFO] %1</source>
        <translation>[INFO] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="398"/>
        <source>[%1] [INFO] %2</source>
        <translation>[%1] [信息] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="407"/>
        <source>[ERROR] %1</source>
        <translation>[ERROR] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="408"/>
        <source>[%1] [ERROR] %2</source>
        <translation>[%1] [ERROR] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="402"/>
        <source>[WARN] %1</source>
        <translation>[WARN] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="403"/>
        <source>[%1] [WARN] %2</source>
        <translation>[%1] [警告] %2</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="333"/>
        <source>Text Files (*.txt);;All Files (*)</source>
        <translation>文本文件 (*.txt);;所有文件 (*)</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="332"/>
        <source>Save Log</source>
        <translation>保存日志</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="384"/>
        <source>[TX] %1</source>
        <translation>[TX] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="391"/>
        <source>[RX] %1</source>
        <translation>[RX] %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="448"/>
        <source>Paused overflow: dropped %1 buffered log entries</source>
        <translation>暂停缓冲溢出：已丢弃 %1 条缓冲日志</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="505"/>
        <source>TX: %1</source>
        <translation>TX: %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="508"/>
        <source>RX: %1</source>
        <translation>RX: %1</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="597"/>
        <source>HEX</source>
        <translation>十六进制 (HEX)</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="600"/>
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="606"/>
        <source>Absolute</source>
        <translation>Absolute</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="607"/>
        <source>Relative</source>
        <translation>Relative</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="608"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="611"/>
        <source>TX</source>
        <translation>发送</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="612"/>
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="613"/>
        <source>Auto Scroll</source>
        <translation>自动滚动</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="615"/>
        <source>Resume</source>
        <translation>Resume</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="615"/>
        <source>Pause</source>
        <translation>暂停</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="617"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="618"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
</context>
<context>
    <name>ui::widgets::CollapsibleSection</name>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="94"/>
        <source>Collapse</source>
        <translation>折叠</translation>
    </message>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="94"/>
        <source>Expand</source>
        <translation>展开</translation>
    </message>
</context>
<context>
    <name>ui::widgets::DisclaimerDialog</name>
    <message>
        <location filename="../widgets/DisclaimerDialog.cpp" line="19"/>
        <source>Language:</source>
        <translation>语言：</translation>
    </message>
    <message>
        <location filename="../widgets/DisclaimerDialog.cpp" line="20"/>
        <source>This tool is provided &quot;AS IS&quot; for testing and analysis only.

Improper operations may cause equipment damage or data loss.

You are solely responsible for all operations.</source>
        <translation>本工具按“原样”提供，仅供测试与分析使用。

操作不当可能导致设备损坏或数据丢失。

您需对所有操作自行承担全部责任。</translation>
    </message>
    <message>
        <location filename="../widgets/DisclaimerDialog.cpp" line="25"/>
        <source>📄 Learn more</source>
        <translation>📄 了解更多</translation>
    </message>
    <message>
        <location filename="../widgets/DisclaimerDialog.cpp" line="26"/>
        <source>✅ Agree</source>
        <translation>✅ 同意并继续</translation>
    </message>
    <message>
        <location filename="../widgets/DisclaimerDialog.cpp" line="27"/>
        <source>❌ Exit</source>
        <translation>❌ 退出</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FrameDecodedTableView</name>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="58"/>
        <source>Address</source>
        <translation>地址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="59"/>
        <source>Hex</source>
        <translation>16进制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="60"/>
        <source>Decimal</source>
        <translation>十进制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="61"/>
        <source>Binary</source>
        <translation>二进制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="62"/>
        <source>Type</source>
        <translation>数据类型</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="63"/>
        <source>Scale</source>
        <translation>缩放比例</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="64"/>
        <source>Value</source>
        <translation>数值</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="65"/>
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="183"/>
        <source>[%1 Low-Word]</source>
        <translation>[%1 低字]</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="184"/>
        <source>[%1 W%2]</source>
        <translation>[%1 字%2]</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="204"/>
        <source>(Subordinate word of address %1)</source>
        <translation>(地址 %1 的从属字)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="234"/>
        <source>&lt;Incomplete&gt;</source>
        <translation>&lt;数据不完整&gt;</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="235"/>
        <source>Incomplete register bytes for %1</source>
        <translation>寄存器字节不足以解析 %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="248"/>
        <source>Default (%1)</source>
        <translation>默认 (%1)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="467"/>
        <source>Reset Selected Type to Default (%1)	Delete</source>
        <translation>重置选中类型为默认 (%1)	Delete</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="468"/>
        <source>Reset Selected Type to Default	Delete</source>
        <translation>重置选中类型为默认	Delete</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="474"/>
        <source>Batch Set Selected Type to...</source>
        <translation>批量设置选中类型为...</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="484"/>
        <source>UInt16 (1 Reg)</source>
        <translation>UInt16 (单寄存器 16位)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="485"/>
        <source>Int16 (1 Reg)</source>
        <translation>Int16 (单寄存器 16位)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="486"/>
        <source>Float32 (Real, 2 Regs)</source>
        <translation>Float32 (实数, 2寄存器)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="487"/>
        <source>Int32 (DInt, 2 Regs)</source>
        <translation>Int32 (双整型, 2寄存器)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="488"/>
        <source>UInt32 (UDInt, 2 Regs)</source>
        <translation>UInt32 (无符号双整型, 2寄存器)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="489"/>
        <source>Float64 (Double, 4 Regs)</source>
        <translation>Float64 (双精度, 4寄存器)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="493"/>
        <source>Reset All Custom Types to Default</source>
        <translation>重置所有自定义类型为默认</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="549"/>
        <source>No Data</source>
        <translation>无数据</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="521"/>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="549"/>
        <source>There is no data to export.</source>
        <translation>当前没有可导出的数据。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="553"/>
        <source>Export CSV</source>
        <translation>导出 CSV</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="555"/>
        <source>CSV Files (*.csv)</source>
        <translation>CSV 文件 (*.csv)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameDecodedTableView.cpp" line="560"/>
        <source>Export Failed</source>
        <translation>导出失败</translation>
    </message>
</context>
<context>
    <name>ui::widgets::GenericInputWidget</name>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="218"/>
        <source>Invalid HEX input: enter an even number of hex digits (0-9, A-F).</source>
        <translation>无效的 HEX 输入：请输入偶数个十六进制字符 (0-9, A-F)。</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="387"/>
        <source>Enter data to send...</source>
        <translation>输入要发送的数据...</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="390"/>
        <source>HEX</source>
        <translation>十六进制 (HEX)</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="393"/>
        <location filename="../widgets/GenericInputWidget.cpp" line="422"/>
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="396"/>
        <source>Auto Send</source>
        <translation>自动发送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="397"/>
        <source>Send repeatedly at fixed interval</source>
        <translation>按固定时间间隔循环发送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="400"/>
        <source> ms</source>
        <translation> 毫秒</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="401"/>
        <source>Auto-send interval (ms)</source>
        <translation>自动发送间隔 (ms)</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="404"/>
        <source>Send</source>
        <translation>发送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="415"/>
        <source>Line ending appended on send</source>
        <translation>发送时追加的行尾结束符</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="420"/>
        <source>UTF-8</source>
        <translation>UTF-8</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="421"/>
        <source>Latin-1</source>
        <translation>Latin-1</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="424"/>
        <source>Text character encoding</source>
        <translation>文本字符编码</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="427"/>
        <source>Recent sent messages</source>
        <translation>最近发送的报文记录</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="122"/>
        <location filename="../widgets/GenericInputWidget.cpp" line="410"/>
        <source>No LF</source>
        <translation>无换行</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="123"/>
        <location filename="../widgets/GenericInputWidget.cpp" line="411"/>
        <source>CR (\r)</source>
        <translation>CR (\r)</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="124"/>
        <location filename="../widgets/GenericInputWidget.cpp" line="412"/>
        <source>LF (\n)</source>
        <translation>LF (\n)</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="125"/>
        <location filename="../widgets/GenericInputWidget.cpp" line="413"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r\n)</translation>
    </message>
</context>
<context>
    <name>ui::widgets::NetworkConnectionWidget</name>
    <message>
        <location filename="../widgets/NetworkConnectionWidget.cpp" line="120"/>
        <source>Port:</source>
        <translation>端口：</translation>
    </message>
</context>
<context>
    <name>ui::widgets::RegisterTypeDelegate</name>
    <message>
        <location filename="../widgets/RegisterTypeDelegate.cpp" line="25"/>
        <location filename="../widgets/RegisterTypeDelegate.cpp" line="69"/>
        <source>Default</source>
        <translation>默认</translation>
    </message>
</context>
<context>
    <name>ui::widgets::SerialConnectionWidget</name>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="35"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="36"/>
        <source>Even</source>
        <translation>偶校验</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="37"/>
        <source>Odd</source>
        <translation>奇校验</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="38"/>
        <source>Space</source>
        <translation>空格校验</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="39"/>
        <source>Mark</source>
        <translation>标记校验</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="40"/>
        <source>RTS/CTS</source>
        <translation>RTS/CTS</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="41"/>
        <source>XON/XOFF</source>
        <translation>XON/XOFF</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="150"/>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="152"/>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="154"/>
        <source>Disconnect</source>
        <translation>断开连接</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="150"/>
        <source>Connecting</source>
        <translation>Connecting</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="152"/>
        <source>Transport Connected</source>
        <translation>Transport Connected</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="154"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="148"/>
        <source>Connect</source>
        <translation>连接</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="148"/>
        <source>Disconnected</source>
        <translation>已断开</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="156"/>
        <source>Disconnecting</source>
        <translation>Disconnecting</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="189"/>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="458"/>
        <source>%1 (Offline)</source>
        <translation>%1（已离线）</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="243"/>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="451"/>
        <source>Refresh Ports</source>
        <translation>刷新端口</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="431"/>
        <source>Port:</source>
        <translation>端口：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="434"/>
        <source>Baud:</source>
        <translation>波特率：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="437"/>
        <source>Data:</source>
        <translation>数据位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="440"/>
        <source>Parity:</source>
        <translation>校验：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="444"/>
        <source>Stop:</source>
        <translation>停止位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="447"/>
        <source>Flow:</source>
        <translation>Flow:</translation>
    </message>
</context>
<context>
    <name>ui::widgets::ServerClientPanel</name>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="204"/>
        <source>Select one or more clients for directed send, or enable broadcast.</source>
        <translation>选择一个或多个客户端进行定向发送，或启用广播。</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="207"/>
        <source>Broadcast To All Clients</source>
        <translation>向所有客户端广播</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="210"/>
        <source>Disconnect Selected</source>
        <translation>断开选中的客户端</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="213"/>
        <source>Disconnect All Clients</source>
        <translation>断开所有客户端</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="231"/>
        <source>all clients</source>
        <translation>all clients</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="232"/>
        <source>%1 selected</source>
        <translation>%1 selected</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="233"/>
        <source>Connected Clients: %1 | Send Target: %2</source>
        <translation>已连接客户端: %1 | 发送目标: %2</translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="265"/>
        <source>Client #%1 - %2</source>
        <translation>Client #%1 - %2</translation>
    </message>
</context>
<context>
    <name>ui::widgets::TcpConnectionWidget</name>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="25"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="115"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="126"/>
        <source>Host:</source>
        <translation>主机：</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="26"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="58"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="116"/>
        <source>Connect</source>
        <translation>连接</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="58"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="73"/>
        <source>Disconnected</source>
        <translation>已断开</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="60"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="62"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="64"/>
        <source>Disconnect</source>
        <translation>断开连接</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="60"/>
        <source>Connecting</source>
        <translation>Connecting</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="62"/>
        <source>Transport Connected</source>
        <translation>Transport Connected</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="64"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="66"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="79"/>
        <source>Disconnecting</source>
        <translation>Disconnecting</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="28"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="118"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="126"/>
        <source>Listen:</source>
        <translation>Listen:</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="29"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="73"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="119"/>
        <source>Start Listen</source>
        <translation>Start Listen</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="75"/>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="77"/>
        <source>Stop</source>
        <translation>Stop</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="75"/>
        <source>Starting</source>
        <translation>Starting</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="77"/>
        <source>Listening</source>
        <translation>Listening</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="79"/>
        <source>Stopping</source>
        <translation>Stopping</translation>
    </message>
</context>
<context>
    <name>ui::widgets::UdpConnectionWidget</name>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="70"/>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="134"/>
        <source>Local:</source>
        <translation>Local:</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="71"/>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="96"/>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="128"/>
        <source>Bind</source>
        <translation>Bind</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="72"/>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="135"/>
        <source>Remote:</source>
        <translation>Remote:</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="73"/>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="136"/>
        <source>Remote Port:</source>
        <translation>Remote Port:</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="96"/>
        <source>Disconnected</source>
        <translation>已断开</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="98"/>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="100"/>
        <source>Unbind</source>
        <translation>Unbind</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="98"/>
        <source>Binding</source>
        <translation>Binding</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="100"/>
        <source>Bound</source>
        <translation>Bound</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="102"/>
        <source>Unbinding</source>
        <translation>Unbinding</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="102"/>
        <source>Disconnecting</source>
        <translation>Disconnecting</translation>
    </message>
</context>
<context>
    <name>ui::widgets::modbus::ModbusTelemetryWidget</name>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="87"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="544"/>
        <source>All</source>
        <translation>全部</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="87"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="545"/>
        <source>TX</source>
        <translation>发送</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="87"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="546"/>
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="87"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="547"/>
        <source>SYS</source>
        <translation>系统</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="87"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="548"/>
        <source>Errors</source>
        <translation>仅错误</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="90"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="540"/>
        <source>Filter:</source>
        <translation>过滤:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="94"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="552"/>
        <source>Raw Frames</source>
        <translation>原始帧</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="98"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="283"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="555"/>
        <source>Auto-Scroll: ON</source>
        <translation>自动滚屏: 开启</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="104"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="558"/>
        <source>Clear (Ctrl+L)</source>
        <translation>清空 (Ctrl+L)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="108"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="561"/>
        <source>Export...</source>
        <translation>导出...</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="153"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="283"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="555"/>
        <source>Auto-Scroll: OFF</source>
        <translation>自动滚屏: 关闭</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="167"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="298"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="359"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="566"/>
        <source>Frame Details</source>
        <translation>帧详细信息</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="175"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="576"/>
        <source>Select a frame to inspect details</source>
        <translation>选择帧以查看详细解析</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="192"/>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="579"/>
        <source>Link to Frame Analyzer</source>
        <translation>联动帧分析仪</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="207"/>
        <source>Queue overflow: %1 dropped</source>
        <translation>队列溢出：%1 条丢弃</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="263"/>
        <source>TX: %1 | RX: %2 | Error: %3 | RTT: %4 ms</source>
        <translation>TX: %1 | RX: %2 | Error: %3 | RTT: %4 ms</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="329"/>
        <source>[TID: 0x%1]  [PID: 0x%2]  [MBAP Len: %3 B]  [UID: %4]  [FC: 0x%5 (%6)]
</source>
        <translation>[事务ID: 0x%1]  [协议ID: 0x%2]  [MBAP长度: %3 B]  [单元ID: %4]  [功能码: 0x%5 (%6)]
</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="332"/>
        <source>[Slave: %1]  [ADU Len: %2 B]  [FC: 0x%3 (%4)]
</source>
        <translation>[从站: %1]  [ADU长度: %2 B]  [功能码: 0x%3 (%4)]
</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="339"/>
        <source>[Exception Code: 0x%1 (%2)]
</source>
        <translation>[异常码: 0x%1 (%2)]
</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="352"/>
        <source>[Error: %1]
</source>
        <translation>[错误: %1]
</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="365"/>
        <source>[SYS] %1</source>
        <translation>[SYS] %1</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="371"/>
        <source>[#%1] %2 | %3 | %4 bytes | %5</source>
        <translation>[#%1] %2 | %3 | %4 字节 | %5</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="471"/>
        <source>Export Failed</source>
        <translation>导出失败</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="472"/>
        <source>Failed to export telemetry data: %1</source>
        <translation>导出遥测数据失败：%1</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="463"/>
        <source>Export Telemetry</source>
        <translation>导出遥测记录</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusTelemetryWidget.cpp" line="464"/>
        <source>CSV Files (*.csv);;JSON Files (*.json)</source>
        <translation>CSV 文件 (*.csv);;JSON 文件 (*.json)</translation>
    </message>
</context>
<context>
    <name>ui::widgets::modbus::ModbusWorkbenchWidget</name>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1186"/>
        <source>Stop Polling (Esc)</source>
        <translation>停止轮询 (Esc)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1194"/>
        <source>Execute (F5)</source>
        <translation>执行 (F5)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1111"/>
        <source>Standard</source>
        <translation>标准模式</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1112"/>
        <source>Raw Frame</source>
        <translation>原始报文</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1113"/>
        <source>Addressing &amp; Parameters</source>
        <translation>寻址与参数</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1115"/>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1149"/>
        <source>DEC</source>
        <translation>十进制 (DEC)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1116"/>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1150"/>
        <source>HEX</source>
        <translation>十六进制 (HEX)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1125"/>
        <source>Function Code:</source>
        <translation>功能码:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1130"/>
        <source>0x01 Read Coils</source>
        <translation>0x01 读线圈</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1130"/>
        <source>0x02 Read Discrete Inputs</source>
        <translation>0x02 读离散输入</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1131"/>
        <source>0x03 Read Holding Registers</source>
        <translation>0x03 读保持寄存器</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1131"/>
        <source>0x04 Read Input Registers</source>
        <translation>0x04 读输入寄存器</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1132"/>
        <source>0x05 Write Single Coil</source>
        <translation>0x05 写单线圈</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1132"/>
        <source>0x06 Write Single Register</source>
        <translation>0x06 写单寄存器</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1133"/>
        <source>0x0F Write Multiple Coils</source>
        <translation>0x0F 写多线圈</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1133"/>
        <source>0x10 Write Multiple Registers</source>
        <translation>0x10 写多寄存器</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1141"/>
        <source>Slave ID (0-247):</source>
        <translation>从站号 (0-247):</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1142"/>
        <source>Start Address:</source>
        <translation>起始地址:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1143"/>
        <source>Quantity:</source>
        <translation>数量:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1155"/>
        <source>ON (0xFF00)</source>
        <translation>通 ON (0xFF00)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1156"/>
        <source>OFF (0x0000)</source>
        <translation>断 OFF (0x0000)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1168"/>
        <source>Execution &amp; Polling</source>
        <translation>执行与轮询</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1170"/>
        <source>Periodic Polling</source>
        <translation>周期轮询</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1171"/>
        <source>Interval:</source>
        <translation>间隔:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1173"/>
        <source>[!] Write Polling Active</source>
        <translation>[!] 写入轮询运行中</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1158"/>
        <source>Raw Frame Dispatch (HEX)</source>
        <translation>原始报文发送 (HEX)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="146"/>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="752"/>
        <source>Polling automatically stopped due to address change</source>
        <translation>起始地址已变更，轮询已自动停止</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="494"/>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="763"/>
        <source>Polling automatically stopped due to payload change</source>
        <translation>写入负载已变更，轮询已自动停止</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="718"/>
        <source>Polling automatically stopped due to function code change</source>
        <translation>功能码已变更，轮询已自动停止</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="743"/>
        <source>Polling automatically stopped due to Slave ID change</source>
        <translation>从站地址已变更，轮询已自动停止</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="757"/>
        <source>Polling automatically stopped due to quantity change</source>
        <translation>数量已变更，轮询已自动停止</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1159"/>
        <source>Enter HEX bytes (e.g. 01 03 00 00 00 0A)</source>
        <translation>输入十六进制字节 (如 01 03 00 00 00 0A)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1160"/>
        <source>+ MBAP</source>
        <translation>+ MBAP</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1161"/>
        <source>+ CRC16</source>
        <translation>+ CRC16</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1162"/>
        <source>+ LRC</source>
        <translation>+ LRC</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1163"/>
        <source>Format</source>
        <translation>格式化</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1164"/>
        <source>Send Raw Frame</source>
        <translation>发送原始报文</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="720"/>
        <source>Polling automatically stopped due to write operation switch</source>
        <translation>切换至写操作，已自动停止轮询</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="839"/>
        <source>High-Risk Operation Warning</source>
        <translation>高危操作警告</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="840"/>
        <source>Warning: The current function code is a write operation [FC 0x%1]. Periodic polling will continuously write physical data to the slave, which may cause frequent slave state changes or hardware damage. Are you sure you want to start write polling?</source>
        <translation>警告：当前功能码为写操作 [FC 0x%1]，周期轮询将持续向从站写入物理数据，可能导致从站状态频繁变动或设备损坏。确定要启动写入轮询吗？</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="909"/>
        <source>Address overflow: Start (%1) + Qty (%2) &gt; 65536</source>
        <translation>地址溢出：起始 (%1) + 数量 (%2) &gt; 65536</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="956"/>
        <source>Broadcast read (Slave ID 0) is forbidden per IEC 61158</source>
        <translation>依据 IEC 61158 严禁广播读取 (从站号 0)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="696"/>
        <source>Register Values (required: %1 words):</source>
        <translation>寄存器数值 (需 %1 字):</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1005"/>
        <source>Broadcast (Slave ID 0) does not support periodic polling</source>
        <translation>广播 (从站号 0) 不支持周期轮询</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1118"/>
        <source>Address Base:</source>
        <translation>地址基准:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1122"/>
        <source>0-Based</source>
        <translation>0-Based (偏移寻址)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1123"/>
        <source>1-Based</source>
        <translation>1-Based (常规寻址)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="889"/>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="905"/>
        <source>PDU: 0x%1 (%2)</source>
        <translation>PDU: 0x%1 (%2)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="899"/>
        <source>PDU: --</source>
        <translation>PDU: --</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1190"/>
        <source>Write (F5)</source>
        <translation>写入 (F5)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1147"/>
        <source>Request Data</source>
        <translation>请求数据</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1148"/>
        <source>Data Radix:</source>
        <translation>数据进制:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1114"/>
        <source>Address Radix:</source>
        <translation>地址进制:</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1151"/>
        <source>BIN</source>
        <translation>二进制 (BIN)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="1153"/>
        <source>Read operation requires no request payload</source>
        <translation>读操作无请求数据</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="706"/>
        <source>Register Value (0 ~ 65535):</source>
        <translation>寄存器数值 (0 ~ 65535):</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="700"/>
        <source>Register Value (0x0000 ~ 0xFFFF):</source>
        <translation>寄存器数值 (0x0000 ~ 0xFFFF):</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="703"/>
        <source>Register Value (16-bit 0/1):</source>
        <translation>寄存器数值 (16 位 0/1):</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/ModbusWorkbenchWidget.cpp" line="693"/>
        <source>Coil Bits (LSB-first: left = start address, %1 bits):</source>
        <translation>线圈状态 (低位在前: 左侧=起始地址, 需 %1 位):</translation>
    </message>
</context>
<context>
    <name>ui::widgets::modbus::TelemetryTableModel</name>
    <message>
        <location filename="../widgets/modbus/TelemetryTableModel.cpp" line="32"/>
        <source>Timestamp (ms)</source>
        <translation>时间戳 (ms)</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/TelemetryTableModel.cpp" line="33"/>
        <source>Direction</source>
        <translation>方向</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/TelemetryTableModel.cpp" line="34"/>
        <source>Length</source>
        <translation>长度</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/TelemetryTableModel.cpp" line="35"/>
        <source>Message HEX / Summary</source>
        <translation>报文 HEX / 摘要</translation>
    </message>
    <message>
        <location filename="../widgets/modbus/TelemetryTableModel.cpp" line="36"/>
        <source>Status</source>
        <translation>状态</translation>
    </message>
</context>
</TS>
