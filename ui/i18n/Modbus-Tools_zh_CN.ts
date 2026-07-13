<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>ConnectionAlert</name>
    <message>
        <location filename="../common/ConnectionAlert.h" line="23"/>
        <source>Not Connected</source>
        <translation>未连接</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="23"/>
        <source>Please connect first.</source>
        <translation>请先连接设备</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="27"/>
        <source>Connection Lost</source>
        <translation>连接中断</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="27"/>
        <source>Connection was closed.</source>
        <translation>连接已关闭。</translation>
    </message>
</context>
<context>
    <name>FrameParseWorker</name>
    <message>
        <location filename="../../core/modbus/parser/FrameParseWorker.cpp" line="105"/>
        <source>Error: Empty input</source>
        <translation>错误：输入为空</translation>
    </message>
</context>
<context>
    <name>ModbusFrameParser</name>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="42"/>
        <source>Illegal Function</source>
        <translation>非法功能码</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="44"/>
        <source>Illegal Data Address</source>
        <translation>非法数据地址</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="46"/>
        <source>Illegal Data Value</source>
        <translation>非法数据值</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="48"/>
        <source>Server Device Failure</source>
        <translation>服务器设备故障</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="50"/>
        <source>Acknowledge</source>
        <translation>确认</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="52"/>
        <source>Server Device Busy</source>
        <translation>服务器设备忙</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="54"/>
        <source>Negative Acknowledge</source>
        <translation>否定确认</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="56"/>
        <source>Memory Parity Error</source>
        <translation>内存奇偶校验错误</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="58"/>
        <source>Gateway Path Unavailable</source>
        <translation>网关路径不可用</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="60"/>
        <source>Gateway Target Device Failed To Respond</source>
        <translation>网关目标设备未响应</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="62"/>
        <source>Unknown Exception</source>
        <translation>未知异常</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="81"/>
        <source>Empty frame data</source>
        <translation>帧数据为空</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="95"/>
        <source>Unable to identify protocol. Frame length: %1 bytes, data: %2</source>
        <translation>无法识别协议。帧长度：%1 字节，数据：%2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="153"/>
        <source>Frame too short for Modbus TCP. Expected a complete MBAP + PDU, got %1 bytes</source>
        <translation>Modbus TCP 帧太短。预期完整的 MBAP + PDU，实际 %1 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="162"/>
        <source>Invalid TCP MBAP header or length</source>
        <translation>无效的 TCP MBAP 报头或长度</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="168"/>
        <source>TCP frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>TCP 帧包含多余字节。预期 %1 字节，实际 %2 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="176"/>
        <source>Frame too short for TCP MBAP</source>
        <translation>帧长度对于 TCP MBAP 来说太短</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="180"/>
        <source>Warning: Invalid TCP MBAP header or length (Forced)</source>
        <translation>警告：TCP MBAP 报头或长度无效（强制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="183"/>
        <source>Warning: TCP frame contains trailing bytes (Forced)</source>
        <translation>警告：TCP 帧包含多余字节（强制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="223"/>
        <source>Frame too short for Modbus RTU. Expected at least 5 bytes, got %1</source>
        <translation>Modbus RTU 帧太短。预期至少 5 字节，实际 %1 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="247"/>
        <source>RTU frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>RTU 帧包含多余字节。预期 %1 字节，实际 %2 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="255"/>
        <source>Frame too short for RTU</source>
        <translation>帧长度对于 RTU 来说太短</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="259"/>
        <source>Warning: CRC Mismatch. Expected %1, Got %2 (Forced)</source>
        <translation>警告：CRC 校验不匹配。预期 %1，实际 %2（强制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="265"/>
        <source>Warning: RTU frame contains trailing bytes (Forced)</source>
        <translation>警告：RTU 帧包含多余字节（强制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="239"/>
        <source>CRC Mismatch. Expected %1, Got %2</source>
        <translation>CRC 校验失败。预期 %1，实际 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="196"/>
        <source>Invalid TCP PDU length. MBAP length field is %1, so PDU length is %2</source>
        <translation>无效的 TCP PDU 长度。MBAP 长度字段为 %1，因此 PDU 长度为 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="296"/>
        <source>Frame incomplete for Modbus ASCII. Missing CRLF terminator or full payload</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="304"/>
        <source>Invalid Modbus ASCII frame or LRC mismatch</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="312"/>
        <source>ASCII frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="320"/>
        <source>Warning: Invalid Modbus ASCII frame or LRC mismatch (Forced)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="344"/>
        <source>Empty PDU. Function code is missing from the frame</source>
        <translation>PDU 为空。帧中缺少功能码</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="358"/>
        <source>Exception PDU too short for function 0x%1. Expected 2 bytes, got %2</source>
        <translation>功能码 0x%1 的异常 PDU 过短。预期 2 字节，实际 %2 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="367"/>
        <source>Modbus exception: %1 (code %2)</source>
        <translation>Modbus 异常：%1（代码 %2）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="405"/>
        <source>Response PDU too short for function 0x%1. Expected at least 2 bytes, got %2</source>
        <translation>功能码 0x%1 的响应 PDU 过短。预期至少 2 字节，实际 %2 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="417"/>
        <source>Byte count mismatch for function 0x%1. Declared %2, actual %3</source>
        <translation>功能码 0x%1 的字节数不匹配。声明 %2，实际 %3</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="431"/>
        <source>Register response byte count does not match expected quantity. Declared %1, expected %2</source>
        <translation>寄存器响应字节数与预期数量不符。声明 %1，预期 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="440"/>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="574"/>
        <source>Register byte count must be even, got %1</source>
        <translation>寄存器字节数必须为偶数，实际 %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="472"/>
        <source>Coil response bit count does not match expected quantity. Byte count %1 cannot represent %2 bits</source>
        <translation>线圈响应位计数与预期数量不符。字节数 %1 无法表示 %2 位</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="507"/>
        <source>Write single PDU length mismatch for function 0x%1. Expected 4 bytes, got %2</source>
        <translation>功能码 0x%1 的单写 PDU 长度不匹配。预期 4 字节，实际 %2 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="554"/>
        <source>Write request byte count mismatch. Declared %1, actual %2</source>
        <translation>写请求字节数不匹配。声明 %1，实际 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="565"/>
        <source>Register write byte count does not match quantity. Declared %1, expected %2</source>
        <translation>寄存器写字节数与数量不符。声明 %1，预期 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="599"/>
        <source>Coil write byte count does not match quantity. Declared %1, expected %2</source>
        <translation>线圈写字节数与数量不符。声明 %1，预期 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="631"/>
        <source>Unsupported function code 0x%1 for deep parsing</source>
        <translation>不支持对功能码 0x%1 进行深度解析</translation>
    </message>
</context>
<context>
    <name>ModbusPduBuilder</name>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="29"/>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="45"/>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="59"/>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="72"/>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="105"/>
        <source>Invalid start address</source>
        <translation>无效的起始地址</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="32"/>
        <source>Invalid quantity</source>
        <translation>无效的数量</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="75"/>
        <source>Invalid quantity for 0x0F</source>
        <translation>0x0F 数量无效</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="80"/>
        <source>Data length mismatch for 0x0F</source>
        <translation>0x0F 数据长度不匹配</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="108"/>
        <source>Invalid quantity for 0x10</source>
        <translation>0x10 数量无效</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusPduBuilder.cpp" line="112"/>
        <source>Data length mismatch for 0x10</source>
        <translation>0x10 数据长度不匹配</translation>
    </message>
</context>
<context>
    <name>ModbusProtocolChecks</name>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="25"/>
        <source>Exception</source>
        <translation>异常</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="26"/>
        <source>Bit read</source>
        <translation>位读取</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="27"/>
        <source>Register read</source>
        <translation>寄存器读取</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="28"/>
        <source>Write single</source>
        <translation>单写</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="29"/>
        <source>Write multiple</source>
        <translation>多写</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="30"/>
        <source>%1 response payload length mismatch: expected %2, got %3</source>
        <translation>%1 响应负载长度不匹配。预期 %2，实际 %3</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="31"/>
        <source>Exception function code does not match request</source>
        <translation>异常功能码与请求不匹配</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="32"/>
        <source>Response function code does not match request</source>
        <translation>响应功能码与请求不匹配</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="33"/>
        <source>Request quantity missing for bit-read validation</source>
        <translation>位读取校验缺少请求数量</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="34"/>
        <source>Bit-read response byte count does not match payload length</source>
        <translation>位读取响应字节数与负载长度不匹配</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="35"/>
        <source>Bit-read response byte count does not match requested quantity</source>
        <translation>位读取响应字节数与请求数量不匹配</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="36"/>
        <source>Request quantity missing for register-read validation</source>
        <translation>寄存器读取校验缺少请求数量</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="37"/>
        <source>Register-read response byte count does not match payload length</source>
        <translation>寄存器读取响应字节数与负载长度不匹配</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="38"/>
        <source>Register-read response byte count does not match requested quantity</source>
        <translation>寄存器读取响应字节数与请求数量不匹配</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="39"/>
        <source>Register-read response byte count must be even</source>
        <translation>寄存器读取响应字节数必须为偶数</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="40"/>
        <source>Write-single response echo does not match request</source>
        <translation>单写响应回显与请求不匹配</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="41"/>
        <source>Request echo fields missing for write-multiple validation</source>
        <translation>多写校验缺少请求回显字段</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="42"/>
        <source>Write-multiple response echo fields are incomplete</source>
        <translation>多写响应回显字段不完整</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="43"/>
        <source>Write-multiple response echo does not match request</source>
        <translation>多写响应回显与请求不匹配</translation>
    </message>
    <message>
        <location filename="../../core/modbus/base/ModbusProtocolChecks.cpp" line="44"/>
        <source>Unsupported function code for response validation</source>
        <translation>响应校验不支持该功能码</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>Cannot write file: %1</source>
        <translation type="vanished">无法写入文件：%1</translation>
    </message>
    <message>
        <source>Expand Navigation</source>
        <translation type="obsolete">展开导航</translation>
    </message>
    <message>
        <source>Collapse Navigation</source>
        <translation type="obsolete">折叠导航</translation>
    </message>
</context>
<context>
    <name>ReconnectPolicy</name>
    <message>
        <location filename="../../core/ReconnectPolicy.cpp" line="84"/>
        <source>Attempt %1/%2</source>
        <translation>重连尝试 %1/%2</translation>
    </message>
</context>
<context>
    <name>RequestSubmissionService</name>
    <message>
        <source>Unsupported function code</source>
        <translation type="obsolete">不支持的功能码</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="64"/>
        <source>Invalid decimal value for 0x05</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="74"/>
        <source>Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="83"/>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="91"/>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="100"/>
        <source>Invalid hex value for 0x05</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="114"/>
        <source>Empty value for 0x06</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="123"/>
        <source>Invalid decimal value for 0x06</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="131"/>
        <source>Binary format not supported for registers (0x06)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="138"/>
        <source>Invalid hex value for 0x06</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="147"/>
        <source>Invalid quantity for 0x0F</source>
        <translation type="unfinished">0x0F 数量无效</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="157"/>
        <source>Binary bit count (%1) does not match Quantity (%2)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="167"/>
        <source>0x0F requires Hex or Binary data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="174"/>
        <source>Empty value for 0x10</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="180"/>
        <source>Invalid quantity for 0x10</source>
        <translation type="unfinished">0x10 数量无效</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="189"/>
        <source>Invalid decimal list for 0x10</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="197"/>
        <source>Invalid hex value for 0x10</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestSubmissionService.cpp" line="230"/>
        <source>Raw data is empty</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SerialConfig</name>
    <message>
        <location filename="../../infra/io/SerialConfig.cpp" line="25"/>
        <source>Port name is empty</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../infra/io/SerialConfig.cpp" line="33"/>
        <source>Unsupported baud rate: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../infra/io/SerialConfig.cpp" line="41"/>
        <source>Invalid data bits: %1 (expected 5, 6, 7, or 8)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../infra/io/SerialConfig.cpp" line="54"/>
        <source>Invalid stop bits: %1 (expected 1, 1.5, or 2)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>TcpChannel</name>
    <message>
        <location filename="../../infra/io/TcpChannel.cpp" line="98"/>
        <source>Invalid IP address: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../infra/io/TcpChannel.cpp" line="106"/>
        <source>Invalid port: %1 (expected 1-65535)</source>
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
        <translation>换算值：%1</translation>
    </message>
</context>
<context>
    <name>core::update::ChecksumWorker</name>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="61"/>
        <source>Missing or invalid expected checksum</source>
        <translation>缺少或无效的预期校验值</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="75"/>
        <source>Failed to calculate file checksum</source>
        <translation>计算文件校验值失败</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="80"/>
        <source>Checksum mismatch. Expected: %1, Actual: %2</source>
        <translation>校验值不匹配。预期：%1，实际：%2</translation>
    </message>
</context>
<context>
    <name>core::update::UpdateManager</name>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="128"/>
        <source>Failed to access system temporary directory</source>
        <translation>无法访问系统临时目录</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="134"/>
        <source>Failed to create update directory</source>
        <translation>创建更新目录失败</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="140"/>
        <source>Invalid update URL</source>
        <translation>更新链接无效</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="187"/>
        <source>Failed to open local file for writing</source>
        <translation>无法打开本地文件进行写入</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="251"/>
        <location filename="../../core/update/UpdateManager.cpp" line="286"/>
        <source>No update install strategy available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="52"/>
        <source>Failed to create update task file</source>
        <translation>创建更新任务文件失败</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="69"/>
        <source>Automatic update is not supported on Windows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="76"/>
        <source>Updater not found</source>
        <translation>未找到更新器</translation>
    </message>
    <message>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="106"/>
        <location filename="../../core/update/PlatformUpdateInstallStrategy.cpp" line="120"/>
        <source>Automatic update is not supported on %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Failed to launch updater (Access Denied or System Error)</source>
        <translation type="vanished">启动更新器失败（拒绝访问或系统错误）</translation>
    </message>
    <message>
        <source>Automatic update is only supported on Windows</source>
        <translation type="vanished">自动更新仅支持 Windows 系统</translation>
    </message>
</context>
<context>
    <name>logging</name>
    <message>
        <location filename="../../infra/logging/Logger.cpp" line="75"/>
        <source>Log directory path is empty.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../infra/logging/Logger.cpp" line="83"/>
        <source>Failed to create log directory: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../infra/logging/Logger.cpp" line="92"/>
        <source>Log directory is not writable: %1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>main</name>
    <message>
        <location filename="../../app/main.cpp" line="39"/>
        <source>Startup Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../app/main.cpp" line="41"/>
        <source>Failed to initialize application logging.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../app/main.cpp" line="42"/>
        <source>Failed to initialize application logging.
%1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>modbus::session::RequestValidator</name>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="35"/>
        <source>Invalid RTU slave id: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="40"/>
        <source>PDU data too long: %1 bytes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="55"/>
        <source>Invalid read request payload length: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="58"/>
        <source>Read quantity must be at least %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="62"/>
        <source>Read address range exceeds limit: start=%1 quantity=%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="69"/>
        <source>Read bit quantity exceeds limit: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="74"/>
        <source>Read register quantity exceeds limit: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="82"/>
        <source>Invalid write-single request payload length: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="85"/>
        <source>Write address out of range: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="89"/>
        <source>Invalid single coil value: 0x%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="99"/>
        <source>Invalid write-multiple request payload length: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="103"/>
        <source>Write quantity must be at least %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="107"/>
        <source>Write address range exceeds limit: start=%1 quantity=%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="112"/>
        <source>Write request byte count mismatch: declared %1, actual %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="118"/>
        <source>Write coil quantity exceeds limit: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="122"/>
        <source>Write coil byte count mismatch: declared %1, expected %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="128"/>
        <source>Write register quantity exceeds limit: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="132"/>
        <source>Write register byte count mismatch: declared %1, expected %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="139"/>
        <source>Unsupported function code: 0x%1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="146"/>
        <source>TCP MBAP length exceeds limit: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/session/RequestValidator.cpp" line="151"/>
        <source>RTU ADU length exceeds limit: %1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::MainWindow</name>
    <message>
        <location filename="../MainWindow.cpp" line="135"/>
        <location filename="../MainWindow.cpp" line="374"/>
        <source>Modbus Tools</source>
        <translation>Modbus 工具</translation>
    </message>
    <message>
        <source>Modbus TCP</source>
        <translation type="vanished">Modbus TCP</translation>
    </message>
    <message>
        <source>Modbus RTU</source>
        <translation type="vanished">Modbus RTU</translation>
    </message>
    <message>
        <source>Modbus ASCII</source>
        <translation type="vanished">Modbus ASCII</translation>
    </message>
    <message>
        <source>TCP Client</source>
        <translation type="vanished">TCP 客户端</translation>
    </message>
    <message>
        <source>Serial Port</source>
        <translation type="vanished">串口</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="200"/>
        <location filename="../MainWindow.cpp" line="379"/>
        <source>Frame Analyzer</source>
        <translation>报文分析</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="268"/>
        <location filename="../MainWindow.cpp" line="384"/>
        <source>Language</source>
        <translation>语言</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="248"/>
        <location filename="../MainWindow.cpp" line="385"/>
        <source>Settings</source>
        <translation>设置</translation>
    </message>
    <message>
        <source>You are using the latest version: v%1</source>
        <translation type="vanished">您正在使用最新版本：v%1</translation>
    </message>
    <message>
        <source>Update Failed</source>
        <translation type="vanished">更新失败</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="382"/>
        <source>Expand Navigation</source>
        <translation>展开导航</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="382"/>
        <source>Collapse Navigation</source>
        <translation>折叠导航</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="254"/>
        <location filename="../MainWindow.cpp" line="387"/>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="29"/>
        <source>Modbus Settings</source>
        <translation>Modbus设置</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="200"/>
        <location filename="../MainWindow.cpp" line="377"/>
        <source>TCP/UDP Tool</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="200"/>
        <location filename="../MainWindow.cpp" line="378"/>
        <source>Serial Debugger</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="200"/>
        <location filename="../MainWindow.cpp" line="376"/>
        <source>Modbus</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="257"/>
        <location filename="../MainWindow.cpp" line="388"/>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="33"/>
        <source>Update Settings</source>
        <translation>更新设置</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="261"/>
        <location filename="../MainWindow.cpp" line="389"/>
        <source>Open Log Folder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="282"/>
        <location filename="../MainWindow.cpp" line="393"/>
        <source>English (US)</source>
        <translation>英语（美国）</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="283"/>
        <location filename="../MainWindow.cpp" line="394"/>
        <source>简体中文</source>
        <translation>简体中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="284"/>
        <location filename="../MainWindow.cpp" line="395"/>
        <source>繁體中文</source>
        <translation>繁體中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="303"/>
        <location filename="../MainWindow.cpp" line="390"/>
        <source>Check for Updates</source>
        <translation>检查更新</translation>
    </message>
    <message>
        <source>New version v%1 available. Open download page?</source>
        <translation type="vanished">发现新版本 v%1。是否打开下载页面？</translation>
    </message>
    <message>
        <source>Current: v%1, Latest: v%2
Choose update method:</source>
        <translation type="vanished">当前版本：v%1，最新版本：v%2
请选择更新方式：</translation>
    </message>
    <message>
        <source>Downloading Update...</source>
        <translation type="vanished">正在下载更新...</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="37"/>
        <source>Request Timeout (ms):</source>
        <translation>请求超时(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="41"/>
        <source>Enable Retry:</source>
        <translation>启用重试：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="46"/>
        <source>Retry Count:</source>
        <translation>重试次数：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="52"/>
        <source>Retry Interval (ms):</source>
        <translation>重试间隔(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="62"/>
        <source>Debug</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="63"/>
        <source>Info</source>
        <translation type="unfinished">信息</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="64"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="65"/>
        <source>Error</source>
        <translation type="unfinished">错误</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="68"/>
        <source>Log Level:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Update Available</source>
        <translation type="vanished">发现新版本</translation>
    </message>
    <message>
        <source>No Updates</source>
        <translation type="vanished">无可用更新</translation>
    </message>
    <message>
        <source>Update Check Failed</source>
        <translation type="vanished">更新检查失败</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation type="vanished">取消</translation>
    </message>
    <message>
        <source>Update Main Program</source>
        <translation type="vanished">更新主程序</translation>
    </message>
    <message>
        <source>Download Full Package</source>
        <translation type="vanished">下载完整安装包</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="297"/>
        <location filename="../MainWindow.cpp" line="307"/>
        <location filename="../MainWindow.cpp" line="386"/>
        <location filename="../MainWindow.cpp" line="391"/>
        <location filename="../widgets/AboutDialog.cpp" line="24"/>
        <source>About</source>
        <translation>关于</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="329"/>
        <source>Theme: Auto</source>
        <translation>主题：自动</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="330"/>
        <source>Theme: Light</source>
        <translation>主题：浅色</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="330"/>
        <source>Theme: Dark</source>
        <translation>主题：深色</translation>
    </message>
    <message>
        <location filename="../widgets/AboutDialog.cpp" line="33"/>
        <source>Welcome to Modbus-Tools&lt;br&gt;Version: v%1&lt;br&gt;&lt;br&gt;An open-source Modbus communication debugging assistant.&lt;br&gt;Developer: mingyucheng692&lt;br&gt;License: MIT License&lt;br&gt;&lt;br&gt;This project is developed in spare time, completely free and open-source.&lt;br&gt;Feel free to star on GitHub or submit issues.&lt;br&gt;Your feedback keeps the project improving!&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 Visit GitHub Repository&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 Issue Tracker&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;This software is provided &amp;quot;as is&amp;quot; without warranty of any kind.</source>
        <translation>欢迎使用 Modbus-Tools&lt;br&gt;版本：v%1&lt;br&gt;&lt;br&gt;一款开源的 Modbus 通讯调试助手。&lt;br&gt;开发者：mingyucheng692&lt;br&gt;许可协议：MIT License&lt;br&gt;&lt;br&gt;本项目由个人业余开发，完全免费且开源。&lt;br&gt;欢迎在 GitHub 上 Star ⭐ 或提交 Issue。&lt;br&gt;您的反馈是项目持续改进的动力！&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 访问 GitHub 仓库&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 问题反馈&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;本软件按&amp;quot;原样&amp;quot;提供，无任何形式的保证。</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="38"/>
        <source>Every Startup</source>
        <translation>每次启动</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="40"/>
        <source>Weekly</source>
        <translation>每周</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="42"/>
        <source>Monthly</source>
        <translation>每月</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="44"/>
        <source>Disable Update Check</source>
        <translation>关闭更新检测</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="46"/>
        <source>Update Check Frequency:</source>
        <translation>更新检测频率：</translation>
    </message>
</context>
<context>
    <name>ui::UpdateInteractionView</name>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="41"/>
        <source>About</source>
        <translation type="unfinished">关于</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="44"/>
        <source>Check for Updates</source>
        <translation type="unfinished">检查更新</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="63"/>
        <location filename="../UpdateInteractionView.cpp" line="71"/>
        <source>Update Available</source>
        <translation type="unfinished">发现新版本</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="64"/>
        <source>New version v%1 available. Open download page?</source>
        <translation type="unfinished">发现新版本 v%1。是否打开下载页面？</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="72"/>
        <source>Current: v%1, Latest: v%2
Choose update method:</source>
        <translation type="unfinished">当前版本：v%1，最新版本：v%2
请选择更新方式：</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="73"/>
        <source>Update Main Program</source>
        <translation type="unfinished">更新主程序</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="74"/>
        <source>Download Full Package</source>
        <translation type="unfinished">下载完整安装包</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="91"/>
        <source>Downloading Update...</source>
        <translation type="unfinished">正在下载更新...</translation>
    </message>
    <message>
        <location filename="../UpdateInteractionView.cpp" line="91"/>
        <source>Cancel</source>
        <translation type="unfinished">取消</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::ModbusSessionPresenter</name>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="73"/>
        <source>Connecting to %1:%2...</source>
        <translation type="unfinished">正在连接 %1:%2...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="131"/>
        <source>Opening %1...</source>
        <translation type="unfinished">正在打开 %1...</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="172"/>
        <source>Disconnecting...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="179"/>
        <source>Shutdown timed out; restart recommended</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="185"/>
        <source>Release timed out; restart recommended</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="511"/>
        <source>Transport connected, validating session...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="531"/>
        <source>Disconnected</source>
        <translation type="unfinished">已断开</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="79"/>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="138"/>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="431"/>
        <source>Failed to create Modbus stack</source>
        <translation type="unfinished">创建 Modbus 栈失败</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="566"/>
        <source>Connected</source>
        <translation type="unfinished">已连接</translation>
    </message>
    <message>
        <location filename="../application/modbus/ModbusSessionPresenter.cpp" line="557"/>
        <source>Connection failed: %1</source>
        <translation type="unfinished">连接失败：%1</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::PollingController</name>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="79"/>
        <source>Error: %1</source>
        <translation type="unfinished">错误：%1</translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="117"/>
        <source>Poll recovered after %1 consecutive failures</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="171"/>
        <source>Poll Error: Connection unavailable during polling (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="174"/>
        <source>Poll Error escalated after %1 consecutive failures: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="178"/>
        <source>Poll Error persists (%1 consecutive failures): %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../application/modbus/PollingController.cpp" line="195"/>
        <source>Poll Warning: %1 consecutive failure(s): %2</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::RequestCoordinator</name>
    <message>
        <location filename="../application/modbus/RequestCoordinator.cpp" line="50"/>
        <location filename="../application/modbus/RequestCoordinator.cpp" line="84"/>
        <source>Error: Request service not available</source>
        <translation type="unfinished">错误：请求服务不可用</translation>
    </message>
    <message>
        <location filename="../application/modbus/RequestCoordinator.cpp" line="64"/>
        <location filename="../application/modbus/RequestCoordinator.cpp" line="92"/>
        <source>Error: %1</source>
        <translation type="unfinished">错误：%1</translation>
    </message>
</context>
<context>
    <name>ui::application::modbus::TrafficLogController</name>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="58"/>
        <source>Success: Response received</source>
        <translation type="unfinished">成功：收到响应</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="64"/>
        <source>Success: Write confirmed</source>
        <translation type="unfinished">成功：写入确认</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="70"/>
        <source>Success: Broadcast write sent, no response expected</source>
        <translation type="unfinished">成功：广播写已发送，预期无响应</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="80"/>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation type="unfinished">发送读请求 FC:%1 地址:%2 数量:%3 从站:%4</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="87"/>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation type="unfinished">发送写请求 FC:%1 地址:%2 数据:%3 从站:%4</translation>
    </message>
    <message>
        <location filename="../application/modbus/TrafficLogController.cpp" line="93"/>
        <source>Sending Raw Data: %1</source>
        <translation type="unfinished">发送原始数据：%1</translation>
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
        <translation type="unfinished">错误：%1</translation>
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
        <location filename="../common/UpdateChecker.cpp" line="89"/>
        <source>Release tag is missing</source>
        <translation>发布标签缺失</translation>
    </message>
</context>
<context>
    <name>ui::shell::NavigationController</name>
    <message>
        <location filename="../shell/NavigationController.cpp" line="142"/>
        <source>Expand Navigation</source>
        <translation type="unfinished">展开导航</translation>
    </message>
    <message>
        <location filename="../shell/NavigationController.cpp" line="143"/>
        <source>Collapse Navigation</source>
        <translation type="unfinished">折叠导航</translation>
    </message>
</context>
<context>
    <name>ui::views::BaseModbusPage</name>
    <message>
        <source>Error: Request service not available</source>
        <translation type="obsolete">错误：请求服务不可用</translation>
    </message>
    <message>
        <source>Error: %1</source>
        <translation type="obsolete">错误：%1</translation>
    </message>
    <message>
        <source>[%1] %2</source>
        <translation type="obsolete">[%1] %2</translation>
    </message>
    <message>
        <source>RX</source>
        <translation type="obsolete">接收</translation>
    </message>
    <message>
        <source>TX</source>
        <translation type="obsolete">发送</translation>
    </message>
    <message>
        <source>Data Monitor</source>
        <translation type="obsolete">数据监视</translation>
    </message>
    <message>
        <source>Receive Data</source>
        <translation type="obsolete">接收数据</translation>
    </message>
    <message>
        <source>Send Data</source>
        <translation type="obsolete">发送数据</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="obsolete">复制</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="obsolete">清除</translation>
    </message>
</context>
<context>
    <name>ui::views::GenericChannelViewBase</name>
    <message>
        <location filename="../views/GenericChannelViewBase.cpp" line="72"/>
        <source>Error: %1</source>
        <translation type="unfinished">错误：%1</translation>
    </message>
    <message>
        <location filename="../views/GenericChannelViewBase.cpp" line="152"/>
        <source>File transfer started: %1 (%2 bytes)</source>
        <translation type="unfinished">文件传输已开始：%1 (%2 字节)</translation>
    </message>
    <message>
        <location filename="../views/GenericChannelViewBase.cpp" line="165"/>
        <source>File transfer progress: %1 %2/%3</source>
        <translation type="unfinished">文件传输进度：%1 %2/%3</translation>
    </message>
    <message>
        <location filename="../views/GenericChannelViewBase.cpp" line="173"/>
        <source>File transfer finished: %1</source>
        <translation type="unfinished">文件传输完成：%1</translation>
    </message>
    <message>
        <location filename="../views/GenericChannelViewBase.cpp" line="178"/>
        <source>File transfer failed: %1 (%2)</source>
        <translation type="unfinished">文件传输失败：%1 (%2)</translation>
    </message>
    <message>
        <location filename="../views/GenericChannelViewBase.cpp" line="183"/>
        <source>File transfer canceled: %1</source>
        <translation type="unfinished">文件传输已取消：%1</translation>
    </message>
    <message>
        <location filename="../views/GenericChannelViewBase.cpp" line="195"/>
        <source>Auto-reconnect exhausted (%1 attempts)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/GenericChannelViewBase.cpp" line="207"/>
        <source>Auto-reconnect in %1ms (%2)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::views::generic_serial::GenericSerialView</name>
    <message>
        <source>File transfer started: %1 (%2 bytes)</source>
        <translation type="vanished">文件传输已开始：%1 (%2 字节)</translation>
    </message>
    <message>
        <source>File transfer progress: %1 %2/%3</source>
        <translation type="vanished">文件传输进度：%1 %2/%3</translation>
    </message>
    <message>
        <source>File transfer finished: %1</source>
        <translation type="vanished">文件传输完成：%1</translation>
    </message>
    <message>
        <source>File transfer failed: %1 (%2)</source>
        <translation type="vanished">文件传输失败：%1 (%2)</translation>
    </message>
    <message>
        <source>File transfer canceled: %1</source>
        <translation type="vanished">文件传输已取消：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="129"/>
        <source>Opening %1...</source>
        <translation>正在打开 %1...</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="147"/>
        <source>Closed</source>
        <translation>已关闭</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="148"/>
        <source>Opening</source>
        <translation>正在打开</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="149"/>
        <source>Open</source>
        <translation>已打开</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="150"/>
        <source>Closing</source>
        <translation>正在关闭</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="151"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="152"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="172"/>
        <source>State changed: %1</source>
        <translation>状态已变更：%1</translation>
    </message>
    <message>
        <source>Error: %1</source>
        <translation type="vanished">错误：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="205"/>
        <source>Control</source>
        <translation>控制</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="206"/>
        <source>Send Data</source>
        <translation>发送数据</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="207"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="208"/>
        <source>RTS</source>
        <translation>RTS</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="229"/>
        <source>Reconnecting to %1...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::views::generic_tcp::GenericTcpView</name>
    <message>
        <source>File transfer started: %1 (%2 bytes)</source>
        <translation type="vanished">文件传输已开始：%1 (%2 字节)</translation>
    </message>
    <message>
        <source>File transfer progress: %1 %2/%3</source>
        <translation type="vanished">文件传输进度：%1 %2/%3</translation>
    </message>
    <message>
        <source>File transfer finished: %1</source>
        <translation type="vanished">文件传输完成：%1</translation>
    </message>
    <message>
        <source>File transfer failed: %1 (%2)</source>
        <translation type="vanished">文件传输失败：%1 (%2)</translation>
    </message>
    <message>
        <source>File transfer canceled: %1</source>
        <translation type="vanished">文件传输已取消：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="273"/>
        <source>Connecting to %1:%2...</source>
        <translation>正在连接 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="289"/>
        <source>Starting TCP server on %1:%2...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="304"/>
        <source>Stopping TCP server...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="317"/>
        <source>Binding UDP %1:%2 -&gt; %3:%4...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="321"/>
        <source>Binding UDP %1:%2...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="366"/>
        <source>Server mode: no connected clients available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="367"/>
        <source>Server mode: select at least one client</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="396"/>
        <source>Server mode: file transfer not supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="420"/>
        <source>Closed</source>
        <translation>已关闭</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="421"/>
        <source>Opening</source>
        <translation>正在打开</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="422"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="423"/>
        <source>Closing</source>
        <translation>正在关闭</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="424"/>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="515"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="425"/>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="516"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="463"/>
        <source>State changed: %1</source>
        <translation>状态已变更：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="606"/>
        <source>Auto-reconnecting to %1:%2 (attempt %3)...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error: %1</source>
        <translation type="vanished">错误：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="491"/>
        <source>Client #%1 connected: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="498"/>
        <source>Client #%1 disconnected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="513"/>
        <source>Listening</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="514"/>
        <source>Stopped</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="541"/>
        <source>Server state: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="548"/>
        <source>Server Error: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="581"/>
        <source>Send Data</source>
        <translation>发送数据</translation>
    </message>
</context>
<context>
    <name>ui::views::generic_tcp::Protocol</name>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="41"/>
        <source>TCP Client</source>
        <translation type="unfinished">TCP 客户端</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="42"/>
        <source>TCP Server</source>
        <translation type="unfinished">TCP 服务端</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="43"/>
        <source>UDP</source>
        <translation type="unfinished">UDP</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus::ModbusPage</name>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="83"/>
        <source>TCP</source>
        <translation type="unfinished">TCP</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="85"/>
        <source>RTU</source>
        <translation type="unfinished">RTU</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="87"/>
        <source>ASCII</source>
        <translation type="unfinished">ASCII</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="407"/>
        <location filename="../views/modbus/ModbusPage.cpp" line="419"/>
        <source>[%1] %2</source>
        <translation type="unfinished">[%1] %2</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="408"/>
        <source>RX</source>
        <translation type="unfinished">接收</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="420"/>
        <source>TX</source>
        <translation type="unfinished">发送</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="425"/>
        <source>Data Monitor</source>
        <translation type="unfinished">数据监视</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="426"/>
        <source>Receive Data</source>
        <translation type="unfinished">接收数据</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="427"/>
        <source>Send Data</source>
        <translation type="unfinished">发送数据</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="428"/>
        <location filename="../views/modbus/ModbusPage.cpp" line="429"/>
        <source>Copy</source>
        <translation type="unfinished">复制</translation>
    </message>
    <message>
        <location filename="../views/modbus/ModbusPage.cpp" line="430"/>
        <location filename="../views/modbus/ModbusPage.cpp" line="431"/>
        <source>Clear</source>
        <translation type="unfinished">清除</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus_rtu::ModbusRtuPage</name>
    <message>
        <source>Opening %1...</source>
        <translation type="vanished">正在打开 %1...</translation>
    </message>
    <message>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation type="vanished">发送读请求 FC:%1 地址:%2 数量:%3 从站:%4</translation>
    </message>
    <message>
        <source>Error: %1</source>
        <translation type="vanished">错误：%1</translation>
    </message>
    <message>
        <source>Failed to create Modbus stack</source>
        <translation type="vanished">创建 Modbus 栈失败</translation>
    </message>
    <message>
        <source>Connected</source>
        <translation type="vanished">已连接</translation>
    </message>
    <message>
        <source>Connection failed: %1</source>
        <translation type="vanished">连接失败：%1</translation>
    </message>
    <message>
        <source>Success: Broadcast write sent, no response expected</source>
        <translation type="vanished">成功：广播写已发送，预期无响应</translation>
    </message>
    <message>
        <source>Success: Response received</source>
        <translation type="vanished">成功：收到响应</translation>
    </message>
    <message>
        <source>Unsupported function code</source>
        <translation type="vanished">不支持的功能码</translation>
    </message>
    <message>
        <source>Error: Invalid decimal value for 0x05</source>
        <translation type="vanished">错误：0x05 十进制值无效</translation>
    </message>
    <message>
        <source>Error: Invalid hex value for 0x05</source>
        <translation type="vanished">错误：0x05 十六进制值无效</translation>
    </message>
    <message>
        <source>Error: Empty value for 0x06</source>
        <translation type="vanished">错误：0x06 值为空</translation>
    </message>
    <message>
        <source>Error: Invalid decimal value for 0x06</source>
        <translation type="vanished">错误：0x06 十进制值无效</translation>
    </message>
    <message>
        <source>Error: Invalid hex value for 0x06</source>
        <translation type="vanished">错误：0x06 十六进制值无效</translation>
    </message>
    <message>
        <source>Error: Binary bit count (%1) does not match Quantity (%2)</source>
        <translation type="vanished">错误：二进制位数 (%1) 与数量 (%2) 不匹配</translation>
    </message>
    <message>
        <source>Error: Binary format not supported for registers (0x06)</source>
        <translation type="vanished">错误：寄存器操作 (0x06) 不支持二进制格式</translation>
    </message>
    <message>
        <source>Error: Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation type="vanished">错误：0x05 二进制值无效（预期为 0 或 1）</translation>
    </message>
    <message>
        <source>Error: Invalid quantity for 0x0F</source>
        <translation type="vanished">错误：0x0F 数量无效</translation>
    </message>
    <message>
        <source>Error: Empty value for 0x10</source>
        <translation type="vanished">错误：0x10 值为空</translation>
    </message>
    <message>
        <source>Error: Invalid quantity for 0x10</source>
        <translation type="vanished">错误：0x10 数量无效</translation>
    </message>
    <message>
        <source>Error: Invalid decimal list for 0x10</source>
        <translation type="vanished">错误：0x10 十进制列表无效</translation>
    </message>
    <message>
        <source>Error: Invalid hex value for 0x10</source>
        <translation type="vanished">错误：0x10 十六进制值无效</translation>
    </message>
    <message>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation type="vanished">发送写请求 FC:%1 地址:%2 数据:%3 从站:%4</translation>
    </message>
    <message>
        <source>Success: Write confirmed</source>
        <translation type="vanished">成功：写入确认</translation>
    </message>
    <message>
        <source>Sending Raw Data: %1</source>
        <translation type="vanished">发送原始数据：%1</translation>
    </message>
    <message>
        <source>Poll Error: %1</source>
        <translation type="vanished">轮询错误：%1</translation>
    </message>
    <message>
        <source>Disconnected</source>
        <translation type="vanished">已断开</translation>
    </message>
    <message>
        <source>Error: 0x0F requires Hex or Binary data</source>
        <translation type="vanished">错误：0x0F 需要十六进制或二进制数据</translation>
    </message>
    <message>
        <source>Error: Request service not available</source>
        <translation type="vanished">错误：请求服务不可用</translation>
    </message>
    <message>
        <source>[%1] %2</source>
        <translation type="vanished">[%1] %2</translation>
    </message>
    <message>
        <source>RX</source>
        <translation type="vanished">接收</translation>
    </message>
    <message>
        <source>TX</source>
        <translation type="vanished">发送</translation>
    </message>
    <message>
        <source>Data Monitor</source>
        <translation type="vanished">数据监视</translation>
    </message>
    <message>
        <source>Receive Data</source>
        <translation type="vanished">接收数据</translation>
    </message>
    <message>
        <source>Send Data</source>
        <translation type="vanished">发送数据</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="vanished">复制</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">清除</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus_tcp::ModbusTcpPage</name>
    <message>
        <source>Connecting to %1:%2...</source>
        <translation type="vanished">正在连接 %1:%2...</translation>
    </message>
    <message>
        <source>Connected</source>
        <translation type="vanished">已连接</translation>
    </message>
    <message>
        <source>Disconnected</source>
        <translation type="vanished">已断开</translation>
    </message>
    <message>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation type="vanished">发送读请求 FC:%1 地址:%2 数量:%3 从站:%4</translation>
    </message>
    <message>
        <source>Error: %1</source>
        <translation type="vanished">错误：%1</translation>
    </message>
    <message>
        <source>Failed to create Modbus stack</source>
        <translation type="vanished">创建 Modbus 栈失败</translation>
    </message>
    <message>
        <source>Connection failed: %1</source>
        <translation type="vanished">连接失败：%1</translation>
    </message>
    <message>
        <source>Success: Response received</source>
        <translation type="vanished">成功：收到响应</translation>
    </message>
    <message>
        <source>Unsupported function code</source>
        <translation type="vanished">不支持的功能码</translation>
    </message>
    <message>
        <source>Error: Invalid decimal value for 0x05</source>
        <translation type="vanished">错误：0x05 十进制值无效</translation>
    </message>
    <message>
        <source>Error: Invalid hex value for 0x05</source>
        <translation type="vanished">错误：0x05 十六进制值无效</translation>
    </message>
    <message>
        <source>Error: Empty value for 0x06</source>
        <translation type="vanished">错误：0x06 值为空</translation>
    </message>
    <message>
        <source>Error: Invalid decimal value for 0x06</source>
        <translation type="vanished">错误：0x06 十进制值无效</translation>
    </message>
    <message>
        <source>Error: Invalid hex value for 0x06</source>
        <translation type="vanished">错误：0x06 十六进制值无效</translation>
    </message>
    <message>
        <source>Error: Binary format not supported for registers (0x06)</source>
        <translation type="vanished">错误：寄存器操作 (0x06) 不支持二进制格式</translation>
    </message>
    <message>
        <source>Error: Binary bit count (%1) does not match Quantity (%2)</source>
        <translation type="vanished">错误：二进制位数 (%1) 与数量 (%2) 不匹配</translation>
    </message>
    <message>
        <source>Error: Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation type="vanished">错误：0x05 二进制值无效（预期为 0 或 1）</translation>
    </message>
    <message>
        <source>Error: Invalid quantity for 0x0F</source>
        <translation type="vanished">错误：0x0F 数量无效</translation>
    </message>
    <message>
        <source>Error: Empty value for 0x10</source>
        <translation type="vanished">错误：0x10 值为空</translation>
    </message>
    <message>
        <source>Error: Invalid quantity for 0x10</source>
        <translation type="vanished">错误：0x10 数量无效</translation>
    </message>
    <message>
        <source>Error: Invalid decimal list for 0x10</source>
        <translation type="vanished">错误：0x10 十进制列表无效</translation>
    </message>
    <message>
        <source>Error: Invalid hex value for 0x10</source>
        <translation type="vanished">错误：0x10 十六进制值无效</translation>
    </message>
    <message>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation type="vanished">发送写请求 FC:%1 地址:%2 数据:%3 从站:%4</translation>
    </message>
    <message>
        <source>Success: Write confirmed</source>
        <translation type="vanished">成功：写入确认</translation>
    </message>
    <message>
        <source>Sending Raw Data: %1</source>
        <translation type="vanished">发送原始数据：%1</translation>
    </message>
    <message>
        <source>Poll Error: %1</source>
        <translation type="vanished">轮询错误：%1</translation>
    </message>
    <message>
        <source>Error: 0x0F requires Hex or Binary data</source>
        <translation type="vanished">错误：0x0F 需要十六进制或二进制数据</translation>
    </message>
    <message>
        <source>Error: Request service not available</source>
        <translation type="vanished">错误：请求服务不可用</translation>
    </message>
    <message>
        <source>[%1] %2</source>
        <translation type="vanished">[%1] %2</translation>
    </message>
    <message>
        <source>RX</source>
        <translation type="vanished">接收</translation>
    </message>
    <message>
        <source>TX</source>
        <translation type="vanished">发送</translation>
    </message>
    <message>
        <source>Data Monitor</source>
        <translation type="vanished">数据监视</translation>
    </message>
    <message>
        <source>Receive Data</source>
        <translation type="vanished">接收数据</translation>
    </message>
    <message>
        <source>Send Data</source>
        <translation type="vanished">发送数据</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="vanished">复制</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">清除</translation>
    </message>
</context>
<context>
    <name>ui::widgets::BaseConnectionWidget</name>
    <message>
        <location filename="../widgets/BaseConnectionWidget.cpp" line="108"/>
        <source>Connection Settings</source>
        <translation type="unfinished">连接设置</translation>
    </message>
    <message>
        <location filename="../widgets/BaseConnectionWidget.cpp" line="111"/>
        <source>Auto Reconnect</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::widgets::ByteMonitorWidget</name>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="185"/>
        <source>Copy</source>
        <translation type="unfinished">复制</translation>
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
        <translation type="unfinished">[%1] [信息] %2</translation>
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
        <translation type="unfinished">文本文件 (*.txt);;所有文件 (*)</translation>
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
        <translation type="unfinished">保存日志</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="355"/>
        <source>Cannot write file: %1</source>
        <translation type="unfinished">无法写入文件：%1</translation>
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
        <translation type="unfinished">无</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="577"/>
        <source>TX</source>
        <translation type="unfinished">发送</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="578"/>
        <source>RX</source>
        <translation type="unfinished">接收</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="579"/>
        <source>Auto Scroll</source>
        <translation type="unfinished">自动滚动</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="581"/>
        <source>Resume</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="581"/>
        <source>Pause</source>
        <translation type="unfinished">暂停</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="583"/>
        <source>Clear</source>
        <translation type="unfinished">清除</translation>
    </message>
    <message>
        <location filename="../widgets/ByteMonitorWidget.cpp" line="584"/>
        <source>Save</source>
        <translation type="unfinished">保存</translation>
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
        <translation>折叠</translation>
    </message>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="101"/>
        <source>Expand</source>
        <translation>展开</translation>
    </message>
</context>
<context>
    <name>ui::widgets::ControlWidget</name>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="112"/>
        <source>Invalid Address format or range (0-65535): %1</source>
        <translation>无效的地址格式或范围 (0-65535): %1</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="124"/>
        <source>Confirm Address</source>
        <translation>确认地址</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="125"/>
        <source>The polling address is set to 0. Are you sure you want to continue?</source>
        <translation>轮流地址设置为 0。确定要继续吗？</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="129"/>
        <source>Do not show this again</source>
        <translation>不再显示</translation>
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
        <translation>启用轮询</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="354"/>
        <source>Interval(ms):</source>
        <translation>间隔(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="357"/>
        <source>FC:</source>
        <translation>功能码：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="360"/>
        <source>01-Read Coils</source>
        <translation>01-读线圈</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="361"/>
        <source>02-Read Discrete</source>
        <translation>02-读离散输入</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="362"/>
        <source>03-Read Holding</source>
        <translation>03-读保持寄存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="363"/>
        <source>04-Read Input</source>
        <translation>04-读输入寄存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="366"/>
        <source>Addr:</source>
        <translation>地址：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="369"/>
        <source>Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>地址 (0-65535)。支持 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="372"/>
        <source>Qty:</source>
        <translation>数量：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="389"/>
        <source>Last Success RTT</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="351"/>
        <source>Link to Analyzer</source>
        <translation>联动分析仪</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FrameAnalyzerWidget</name>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="358"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1167"/>
        <source>Frame Input</source>
        <translation>报文输入</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="364"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1168"/>
        <source>Protocol:</source>
        <translation>协议：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="367"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1170"/>
        <source>Auto Detect</source>
        <translation>自动检测</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="368"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1171"/>
        <source>Modbus TCP</source>
        <translation>Modbus TCP</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="369"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1172"/>
        <source>Modbus RTU</source>
        <translation>Modbus RTU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="370"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1173"/>
        <source>Modbus ASCII</source>
        <translation>Modbus ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="374"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1175"/>
        <source>Start Address (for Response):</source>
        <translation>起始地址（用于响应）：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="388"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1191"/>
        <source>Format Hex</source>
        <translation>格式化 Hex</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="393"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1218"/>
        <source>Parse</source>
        <translation>解析</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="398"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1219"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="408"/>
        <source>Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)</source>
        <translation>输入十六进制字符串（如 01 03 00 00 00 01 84 0A）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1222"/>
        <source>Enter Hex string (e.g., RTU: 01 03 00 00 00 01 84 0A, ASCII bytes: 3A 30 31 30 33 ... 0D 0A)</source>
        <translation>输入十六进制字符串（如 RTU: 01 03 00 00 00 01 84 0A，ASCII 字节: 3A 30 31 30 33 ... 0D 0A）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="419"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1225"/>
        <source>Analysis Result</source>
        <translation>分析结果</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="435"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1202"/>
        <source>Status:</source>
        <translation>状态：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="439"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1032"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1215"/>
        <source>Ready</source>
        <translation>就绪</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="448"/>
        <source>Tip: &quot;Pause&quot; to edit description</source>
        <translation>提示：&quot;暂停&quot;以编辑描述</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="465"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1044"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1107"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1200"/>
        <source>Pause Refresh</source>
        <translation>暂停刷新</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1107"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1200"/>
        <source>Resume Refresh</source>
        <translation>恢复刷新</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="473"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1198"/>
        <source>Stop Link</source>
        <translation>停止联动</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="485"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1179"/>
        <source>Decode Mode:</source>
        <translation>解码模式：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="487"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1181"/>
        <source>Unsigned</source>
        <translation>无符号</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="488"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1182"/>
        <source>Signed</source>
        <translation>有符号</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="522"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="717"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1192"/>
        <source>Import Config</source>
        <translation>导入配置</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="524"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="700"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1193"/>
        <source>Export Config</source>
        <translation>导出配置</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="526"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="748"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1194"/>
        <source>Export CSV</source>
        <translation>导出 CSV</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="550"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1233"/>
        <source>Field</source>
        <translation>字段</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="550"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="558"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1234"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1238"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="550"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="558"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1235"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1238"/>
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="554"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="840"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1053"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1228"/>
        <source>Structure</source>
        <translation>结构</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="558"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1238"/>
        <source>Address</source>
        <translation>地址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="558"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1238"/>
        <source>Hex</source>
        <translation>十六进制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="558"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1238"/>
        <source>Decimal</source>
        <translation>十进制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="558"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1238"/>
        <source>Binary</source>
        <translation>二进制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="558"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1238"/>
        <source>Scale</source>
        <translation>倍率</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="744"/>
        <source>There is no data to export.</source>
        <translation>没有可导出的数据。</translation>
    </message>
    <message>
        <source>Raw Hex</source>
        <translation type="vanished">原始十六进制</translation>
    </message>
    <message>
        <source>Valid</source>
        <translation type="vanished">有效</translation>
    </message>
    <message>
        <source>Invalid (Expected 0x%1)</source>
        <translation type="vanished">无效 (预期 0x%1)</translation>
    </message>
    <message>
        <source>Success</source>
        <translation type="vanished">成功</translation>
    </message>
    <message>
        <source>Success with warnings</source>
        <translation type="vanished">成功（带警告）</translation>
    </message>
    <message>
        <source>Structure (Live Mode)</source>
        <translation type="vanished">结构（联动模式）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1177"/>
        <source>Start Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>起始地址 (0-65535)。支持 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="589"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1226"/>
        <source>History</source>
        <translation>历史记录</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="593"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1240"/>
        <source>Clear History</source>
        <translation>清空历史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="700"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="717"/>
        <source>JSON Files (*.json)</source>
        <translation>JSON 文件 (*.json)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="744"/>
        <source>No Data</source>
        <translation>无数据</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="750"/>
        <source>CSV Files (*.csv)</source>
        <translation>CSV 文件 (*.csv)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="710"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="770"/>
        <source>Export Failed</source>
        <translation>导出失败</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="270"/>
        <source>OK</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="270"/>
        <source>ERR</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="584"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1229"/>
        <source>Decoded Data</source>
        <translation>解析数据</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="648"/>
        <source>Invalid Address (0-65535): %1</source>
        <translation>无效地址 (0-65535): %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="677"/>
        <source>Parse Failed</source>
        <translation>解析失败</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="722"/>
        <source>Import Failed</source>
        <translation>导入失败</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1080"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1209"/>
        <source>LIVE: %1</source>
        <translation>联动中: %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1084"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1211"/>
        <source>Live Data Received at %1</source>
        <translation>实时数据接收于 %1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="639"/>
        <source>Error: Empty input</source>
        <translation>错误：输入为空</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="655"/>
        <source>Parsing...</source>
        <translation>解析中...</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="789"/>
        <source>Parse Failed: %1</source>
        <translation>解析失败：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="796"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1205"/>
        <source>TCP</source>
        <translation>TCP</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="797"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1206"/>
        <source>RTU</source>
        <translation>RTU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="846"/>
        <source>Frame</source>
        <translation>帧</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="847"/>
        <source>%1 bytes</source>
        <translation>%1 字节</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="855"/>
        <source>MBAP Header</source>
        <translation>MBAP 头</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="860"/>
        <source>Transaction ID</source>
        <translation>事务 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="865"/>
        <source>Protocol ID</source>
        <translation>协议 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="870"/>
        <source>Length</source>
        <translation>长度</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="875"/>
        <source>Unit ID</source>
        <translation>单元 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="881"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="897"/>
        <source>Slave ID</source>
        <translation>从站 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="918"/>
        <source>PDU</source>
        <translation>PDU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="924"/>
        <source>Function Code</source>
        <translation>功能码</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="936"/>
        <source>Exception Code</source>
        <translation>异常码</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="951"/>
        <source>CRC16</source>
        <translation>CRC16</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="255"/>
        <source>Show History</source>
        <translation>显示历史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="255"/>
        <source>Hide History</source>
        <translation>隐藏历史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="277"/>
        <source>Local time %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="501"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1184"/>
        <source>Byte Order:</source>
        <translation>字节序：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="503"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1186"/>
        <source>ABCD(default)</source>
        <translation>ABCD(默认)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="798"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1207"/>
        <source>ASCII</source>
        <translation type="unfinished">ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="841"/>
        <source>(Unavailable in Live Mode)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="842"/>
        <source>Logical parsing is disabled for high-frequency linkage</source>
        <translation>高频联动下已禁用逻辑解析</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="850"/>
        <source>Frame Bytes</source>
        <translation>帧字节</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="850"/>
        <source>Complete raw frame</source>
        <translation>完整原始帧</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="857"/>
        <source>Transaction + Protocol + Length + Unit ID</source>
        <translation>事务 + 协议 + 长度 + 单元 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="862"/>
        <source>Request/response correlation ID</source>
        <translation>请求/响应关联 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="867"/>
        <source>Modbus TCP protocol identifier</source>
        <translation>Modbus TCP 协议标识符</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="872"/>
        <source>Remaining bytes after this field</source>
        <translation>此字段之后的剩余字节数</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="877"/>
        <source>Target slave / unit address</source>
        <translation>目标从站 / 单元地址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="883"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="899"/>
        <source>Target slave address</source>
        <translation>目标从站地址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="886"/>
        <source>ASCII Start</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="888"/>
        <source>Start delimiter &apos;:&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="891"/>
        <source>ASCII Payload</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="893"/>
        <source>ASCII hex payload before CRLF</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="920"/>
        <source>Function code + payload</source>
        <translation>功能码 + 负载</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="926"/>
        <source>Normal response</source>
        <translation>正常响应</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="926"/>
        <source>Exception response</source>
        <translation>异常响应</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="929"/>
        <source>Payload</source>
        <translation>负载</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="931"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="938"/>
        <source>Exception detail payload</source>
        <translation>异常详情负载</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="931"/>
        <source>Application data payload</source>
        <translation>应用数据负载</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="942"/>
        <source>CRC valid</source>
        <translation>CRC 有效</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="942"/>
        <source>CRC invalid</source>
        <translation>CRC 无效</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="946"/>
        <source>Expected 0x%1</source>
        <translation>预期 0x%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="960"/>
        <source>LRC</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="964"/>
        <source>LRC valid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="964"/>
        <source>LRC invalid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="967"/>
        <source>CRLF</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="969"/>
        <source>ASCII frame terminator</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1015"/>
        <source>Success (%1)</source>
        <translation>成功（%1）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1017"/>
        <source>Forced Parsing</source>
        <translation>强制解析</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1020"/>
        <source>Warnings</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1060"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1228"/>
        <source>Structure (Unavailable in Live Mode)</source>
        <translation>结构（联动模式不可用）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="801"/>
        <source>Request</source>
        <translation>请求</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="802"/>
        <source>Response</source>
        <translation>响应</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="803"/>
        <source>Exception</source>
        <translation>异常</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="799"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="804"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1208"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FunctionWidget</name>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="408"/>
        <source>Standard</source>
        <translation>标准</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="411"/>
        <source>Raw</source>
        <translation>原始</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="414"/>
        <source>Slave ID:</source>
        <translation>从站ID：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="417"/>
        <source>Start Addr:</source>
        <translation>起始地址：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="428"/>
        <source>Quantity:</source>
        <translation>数量：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="430"/>
        <source>Write Data:</source>
        <translation>写入数据：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="431"/>
        <source>Format:</source>
        <translation>格式：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="437"/>
        <source>Hex</source>
        <translation>十六进制</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="439"/>
        <source>Binary</source>
        <translation>二进制</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="385"/>
        <source>Space separated hex (e.g., 01 02)</source>
        <translation>空格分隔的十六进制 (如 01 02)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="335"/>
        <location filename="../widgets/FunctionWidget.cpp" line="354"/>
        <source>Invalid Unit ID format or range (0-255): %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="336"/>
        <location filename="../widgets/FunctionWidget.cpp" line="355"/>
        <source>Invalid Slave ID format or range (0-255): %1</source>
        <translation>无效的 Slave ID 格式或范围 (0-255): %1</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="340"/>
        <location filename="../widgets/FunctionWidget.cpp" line="359"/>
        <source>Invalid Address format or range (0-65535): %1</source>
        <translation>无效的地址格式或范围 (0-65535): %1</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="387"/>
        <source>Space separated decimal (e.g., 100 200)</source>
        <translation>空格分隔的十进制 (如 100 200)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="389"/>
        <source>Bit string (e.g., 1 1 0 1)</source>
        <translation>二进制位字符串 (如 1 1 0 1)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="470"/>
        <source>Raw Hex Data (ASCII bytes, e.g., 3A 30 31 30 33 30 30 30 30 30 30 30 31 46 42 0D 0A):</source>
        <translation>原始十六进制数据（ASCII 字节，例如 3A 30 31 30 33 30 30 30 30 30 30 30 31 46 42 0D 0A）：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="479"/>
        <source>Add LRC + Encode ASCII</source>
        <translation>添加 LRC 并编码为 ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="414"/>
        <source>Unit ID:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="421"/>
        <source>Unit ID (0-255). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="422"/>
        <source>Slave ID (0-255). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>Slave ID (0-255)。支持 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="425"/>
        <source>Start Address (0-65535). Supports HEX (0x10 or 10H) and DEC (16).</source>
        <translation>起始地址 (0-65535)。支持 HEX (0x10 或 10H) 和 DEC (16)。</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="438"/>
        <source>Decimal</source>
        <translation>十进制</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="442"/>
        <source>Read Coils (0x01)</source>
        <translation>读线圈 (0x01)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="445"/>
        <source>Read Discrete Inputs (0x02)</source>
        <translation>读离散输入 (0x02)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="448"/>
        <source>Read Holding Registers (0x03)</source>
        <translation>读保持寄存器 (0x03)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="451"/>
        <source>Read Input Registers (0x04)</source>
        <translation>读输入寄存器 (0x04)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="454"/>
        <source>Write Single Coil (0x05)</source>
        <translation>写单线圈 (0x05)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="457"/>
        <source>Write Single Register (0x06)</source>
        <translation>写单寄存器 (0x06)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="460"/>
        <source>Write Multiple Coils (0x0F)</source>
        <translation>写多线圈 (0x0F)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="463"/>
        <source>Write Multiple Registers (0x10)</source>
        <translation>写多寄存器 (0x10)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="468"/>
        <source>Raw Hex Data (e.g., 00 00 00 00 00 06 01 03 00 00 00 01):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="471"/>
        <source>Raw Hex Data (e.g., 01 03 00 00 00 01 84 0A):</source>
        <translation>原始十六进制数据(如 01 03 00 00 00 01 84 0A)：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="474"/>
        <source>Send Raw</source>
        <translation>发送原始数据</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="480"/>
        <source>Append CRC</source>
        <translation>计算并追加 CRC</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="483"/>
        <source>Add MBAP</source>
        <translation>添加 MBAP 头</translation>
    </message>
</context>
<context>
    <name>ui::widgets::GenericInputWidget</name>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="301"/>
        <source>Select File to Send</source>
        <translation>选择要发送的文件</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="361"/>
        <source>Enter data to send...</source>
        <translation>输入要发送的数据...</translation>
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
        <translation>自动发送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="373"/>
        <source> ms</source>
        <translation> 毫秒</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="376"/>
        <source>Send File</source>
        <translation>发送文件</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="379"/>
        <source>Send</source>
        <translation>发送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="67"/>
        <location filename="../widgets/GenericInputWidget.cpp" line="385"/>
        <source>No LF</source>
        <translation>无换行</translation>
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
    <name>ui::widgets::NetworkConnectionWidget</name>
    <message>
        <location filename="../widgets/NetworkConnectionWidget.cpp" line="126"/>
        <source>Port:</source>
        <translation type="unfinished">端口：</translation>
    </message>
</context>
<context>
    <name>ui::widgets::SerialConnectionWidget</name>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="39"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="40"/>
        <source>Even</source>
        <translation>偶校验</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="41"/>
        <source>Odd</source>
        <translation>奇校验</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="42"/>
        <source>Space</source>
        <translation>空格校验</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="43"/>
        <source>Mark</source>
        <translation>标记校验</translation>
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
        <location filename="../widgets/SerialConnectionWidget.cpp" line="170"/>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="175"/>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="180"/>
        <source>Disconnect</source>
        <translation>断开连接</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="171"/>
        <source>Connecting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="176"/>
        <source>Transport Connected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="181"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="166"/>
        <source>Connect</source>
        <translation>连接</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="167"/>
        <source>Disconnected</source>
        <translation>已断开</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="185"/>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="186"/>
        <source>Disconnecting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="252"/>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="436"/>
        <source>Refresh Ports</source>
        <translation>刷新端口</translation>
    </message>
    <message>
        <source>Connection Settings</source>
        <translation type="vanished">连接设置</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="416"/>
        <source>Port:</source>
        <translation>端口：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="419"/>
        <source>Baud:</source>
        <translation>波特率：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="422"/>
        <source>Data:</source>
        <translation>数据位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="425"/>
        <source>Parity:</source>
        <translation>校验：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="429"/>
        <source>Stop:</source>
        <translation>停止位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="432"/>
        <source>Flow:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::widgets::ServerClientPanel</name>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="211"/>
        <source>Select one or more clients for directed send, or enable broadcast.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="214"/>
        <source>Broadcast To All Clients</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="217"/>
        <source>Disconnect Selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="220"/>
        <source>Disconnect All Clients</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="238"/>
        <source>all clients</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="239"/>
        <source>%1 selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="240"/>
        <source>Connected Clients: %1 | Send Target: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ServerClientPanel.cpp" line="272"/>
        <source>Client #%1 - %2</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::widgets::TcpClientConnectionWidget</name>
    <message>
        <location filename="../widgets/TcpClientConnectionWidget.cpp" line="31"/>
        <location filename="../widgets/TcpClientConnectionWidget.cpp" line="82"/>
        <source>Host:</source>
        <translation type="unfinished">主机：</translation>
    </message>
    <message>
        <location filename="../widgets/TcpClientConnectionWidget.cpp" line="32"/>
        <location filename="../widgets/TcpClientConnectionWidget.cpp" line="51"/>
        <location filename="../widgets/TcpClientConnectionWidget.cpp" line="83"/>
        <source>Connect</source>
        <translation type="unfinished">连接</translation>
    </message>
    <message>
        <location filename="../widgets/TcpClientConnectionWidget.cpp" line="51"/>
        <source>Disconnected</source>
        <translation type="unfinished">已断开</translation>
    </message>
    <message>
        <location filename="../widgets/TcpClientConnectionWidget.cpp" line="53"/>
        <location filename="../widgets/TcpClientConnectionWidget.cpp" line="55"/>
        <location filename="../widgets/TcpClientConnectionWidget.cpp" line="57"/>
        <source>Disconnect</source>
        <translation type="unfinished">断开连接</translation>
    </message>
    <message>
        <location filename="../widgets/TcpClientConnectionWidget.cpp" line="53"/>
        <source>Connecting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpClientConnectionWidget.cpp" line="55"/>
        <source>Transport Connected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpClientConnectionWidget.cpp" line="57"/>
        <source>Connected</source>
        <translation type="unfinished">已连接</translation>
    </message>
    <message>
        <location filename="../widgets/TcpClientConnectionWidget.cpp" line="59"/>
        <source>Disconnecting</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::widgets::TcpConnectionWidget</name>
    <message>
        <source>Disconnect</source>
        <translation type="vanished">断开连接</translation>
    </message>
    <message>
        <source>Connected</source>
        <translation type="vanished">已连接</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="vanished">连接</translation>
    </message>
    <message>
        <source>Disconnected</source>
        <translation type="vanished">已断开</translation>
    </message>
    <message>
        <source>Connection Settings</source>
        <translation type="vanished">连接设置</translation>
    </message>
    <message>
        <source>Host:</source>
        <translation type="vanished">主机：</translation>
    </message>
    <message>
        <source>TCP Client</source>
        <translation type="vanished">TCP 客户端</translation>
    </message>
    <message>
        <source>TCP Server</source>
        <translation type="vanished">TCP 服务端</translation>
    </message>
    <message>
        <source>UDP</source>
        <translation type="vanished">UDP</translation>
    </message>
    <message>
        <source>Port:</source>
        <translation type="vanished">端口：</translation>
    </message>
</context>
<context>
    <name>ui::widgets::TcpServerConnectionWidget</name>
    <message>
        <location filename="../widgets/TcpServerConnectionWidget.cpp" line="31"/>
        <location filename="../widgets/TcpServerConnectionWidget.cpp" line="81"/>
        <source>Listen:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpServerConnectionWidget.cpp" line="32"/>
        <location filename="../widgets/TcpServerConnectionWidget.cpp" line="53"/>
        <location filename="../widgets/TcpServerConnectionWidget.cpp" line="82"/>
        <source>Start Listen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpServerConnectionWidget.cpp" line="53"/>
        <source>Disconnected</source>
        <translation type="unfinished">已断开</translation>
    </message>
    <message>
        <location filename="../widgets/TcpServerConnectionWidget.cpp" line="55"/>
        <location filename="../widgets/TcpServerConnectionWidget.cpp" line="57"/>
        <source>Stop</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpServerConnectionWidget.cpp" line="55"/>
        <source>Starting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpServerConnectionWidget.cpp" line="57"/>
        <source>Listening</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpServerConnectionWidget.cpp" line="59"/>
        <source>Stopping</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/TcpServerConnectionWidget.cpp" line="59"/>
        <source>Disconnecting</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::widgets::TrafficMonitorWidget</name>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="185"/>
        <source>Copy</source>
        <translation>复制</translation>
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
        <translation>[%1] [信息] %2</translation>
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
        <translation>保存日志</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="480"/>
        <source>Text Files (*.txt);;All Files (*)</source>
        <translation>文本文件 (*.txt);;所有文件 (*)</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="505"/>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="527"/>
        <source>Save failed: %1</source>
        <translation>保存失败：%1</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="540"/>
        <source>Cannot write file: %1</source>
        <translation type="unfinished">无法写入文件：%1</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="634"/>
        <source>Traffic Monitor</source>
        <translation>通信监视</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="635"/>
        <source>Auto Scroll</source>
        <translation>自动滚动</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="636"/>
        <source>Pause</source>
        <translation>暂停</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="637"/>
        <source>Raw Frames</source>
        <translation>原始帧</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="642"/>
        <source>All</source>
        <translation>全部</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="643"/>
        <source>Info</source>
        <translation>信息</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="644"/>
        <source>Warn</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="645"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="647"/>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="648"/>
        <source>Log Level Filter</source>
        <translation>日志等级筛选</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="650"/>
        <source>TX</source>
        <translation>发送</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="651"/>
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <source>Raw Frames mode may reduce UI smoothness under high-frequency polling.</source>
        <translation type="vanished">高频轮询下，原始帧模式可能降低界面流畅度。</translation>
    </message>
    <message>
        <source>Paused (%1 new)</source>
        <translation type="vanished">已暂停（新增 %1）</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="654"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="655"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
</context>
<context>
    <name>ui::widgets::UdpConnectionWidget</name>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="75"/>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="131"/>
        <source>Local:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="76"/>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="103"/>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="132"/>
        <source>Bind</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="77"/>
        <source>Remote:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="78"/>
        <source>Remote Port:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="103"/>
        <source>Disconnected</source>
        <translation type="unfinished">已断开</translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="105"/>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="107"/>
        <source>Unbind</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="105"/>
        <source>Binding</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="107"/>
        <source>Bound</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="109"/>
        <source>Unbinding</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/UdpConnectionWidget.cpp" line="109"/>
        <source>Disconnecting</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
