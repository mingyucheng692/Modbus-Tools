<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>ConnectionAlert</name>
    <message>
        <location filename="../common/ConnectionAlert.h" line="14"/>
        <source>Not Connected</source>
        <translation>未连接</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="14"/>
        <source>Please connect first.</source>
        <translation>请先连接设备</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="18"/>
        <source>Connection Lost</source>
        <translation>连接中断</translation>
    </message>
    <message>
        <location filename="../common/ConnectionAlert.h" line="18"/>
        <source>Connection was closed.</source>
        <translation>连接已关闭。</translation>
    </message>
</context>
<context>
    <name>FrameAnalyzerWidget</name>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="176"/>
        <source>Error: Empty input</source>
        <translation>错误：输入为空</translation>
    </message>
</context>
<context>
    <name>ModbusFrameParser</name>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="30"/>
        <source>Illegal Function</source>
        <translation>非法功能码</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="32"/>
        <source>Illegal Data Address</source>
        <translation>非法数据地址</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="34"/>
        <source>Illegal Data Value</source>
        <translation>非法数据值</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="36"/>
        <source>Server Device Failure</source>
        <translation>服务器设备故障</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="38"/>
        <source>Acknowledge</source>
        <translation>确认</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="40"/>
        <source>Server Device Busy</source>
        <translation>服务器设备忙</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="42"/>
        <source>Negative Acknowledge</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="44"/>
        <source>Memory Parity Error</source>
        <translation>内存奇偶校验错误</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="46"/>
        <source>Gateway Path Unavailable</source>
        <translation>网关路径不可用</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="48"/>
        <source>Gateway Target Device Failed To Respond</source>
        <translation>网关目标设备未响应</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="50"/>
        <source>Unknown Exception</source>
        <translation>未知异常</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="68"/>
        <source>Empty frame data</source>
        <translation>帧数据为空</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="80"/>
        <source>Unable to identify protocol. Frame length: %1 bytes, data: %2</source>
        <translation>无法识别协议。帧长度：%1 字节，数据：%2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="123"/>
        <source>Frame too short for Modbus TCP. Expected a complete MBAP + PDU, got %1 bytes</source>
        <translation>Modbus TCP 帧太短。预期完整的 MBAP + PDU，实际 %1 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="131"/>
        <source>Invalid TCP MBAP header or length</source>
        <translation>无效的 TCP MBAP 报头或长度</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="136"/>
        <source>TCP frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>TCP 帧包含多余字节。预期 %1 字节，实际 %2 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="144"/>
        <source>Frame too short for TCP MBAP</source>
        <translation>帧长度对于 TCP MBAP 来说太短</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="148"/>
        <source>Warning: Invalid TCP MBAP header or length (Forced)</source>
        <translation>警告：TCP MBAP 报头或长度无效（强制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="151"/>
        <source>Warning: TCP frame contains trailing bytes (Forced)</source>
        <translation>警告：TCP 帧包含多余字节（强制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="191"/>
        <source>Frame too short for Modbus RTU. Expected at least 5 bytes, got %1</source>
        <translation>Modbus RTU 帧太短。预期至少 5 字节，实际 %1 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="215"/>
        <source>RTU frame contains trailing bytes. Expected %1 bytes, got %2</source>
        <translation>RTU 帧包含多余字节。预期 %1 字节，实际 %2 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="223"/>
        <source>Frame too short for RTU</source>
        <translation>帧长度对于 RTU 来说太短</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="227"/>
        <source>Warning: CRC Mismatch. Expected %1, Got %2 (Forced)</source>
        <translation>警告：CRC 校验不匹配。预期 %1，实际 %2（强制解析）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="233"/>
        <source>Warning: RTU frame contains trailing bytes (Forced)</source>
        <translation>警告：RTU 帧包含多余字节（强制解析）</translation>
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
        <translation>CRC 校验失败。预期 %1，实际 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="164"/>
        <source>Invalid TCP PDU length. MBAP length field is %1, so PDU length is %2</source>
        <translation>无效的 TCP PDU 长度。MBAP 长度字段为 %1，因此 PDU 长度为 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="252"/>
        <source>Empty PDU. Function code is missing from the frame</source>
        <translation>PDU 为空。帧中缺少功能码</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="266"/>
        <source>Exception PDU too short for function 0x%1. Expected 2 bytes, got %2</source>
        <translation>功能码 0x%1 的异常 PDU 过短。预期 2 字节，实际 %2 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="275"/>
        <source>Modbus exception: %1 (code %2)</source>
        <translation>Modbus 异常：%1（代码 %2）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="313"/>
        <source>Response PDU too short for function 0x%1. Expected at least 2 bytes, got %2</source>
        <translation>功能码 0x%1 的响应 PDU 过短。预期至少 2 字节，实际 %2 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="325"/>
        <source>Byte count mismatch for function 0x%1. Declared %2, actual %3</source>
        <translation>功能码 0x%1 的字节数不匹配。声明 %2，实际 %3</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="339"/>
        <source>Register response byte count does not match expected quantity. Declared %1, expected %2</source>
        <translation>寄存器响应字节数与预期数量不符。声明 %1，预期 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="348"/>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="482"/>
        <source>Register byte count must be even, got %1</source>
        <translation>寄存器字节数必须为偶数，实际 %1</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="538"/>
        <source>Unsupported function code 0x%1 for deep parsing</source>
        <translation>不支持对功能码 0x%1 进行深度解析</translation>
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
        <translation type="unfinished">无法写入文件：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="781"/>
        <source>Cannot open file: %1</source>
        <translation type="unfinished">无法打开文件：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="786"/>
        <source>Invalid JSON format.</source>
        <translation type="unfinished">无效的 JSON 格式。</translation>
    </message>
</context>
<context>
    <name>core::update::ChecksumWorker</name>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="56"/>
        <source>Missing or invalid expected checksum</source>
        <translation>缺少或无效的预期校验值</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="70"/>
        <source>Failed to calculate file checksum</source>
        <translation>计算文件校验值失败</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="75"/>
        <source>Checksum mismatch. Expected: %1, Actual: %2</source>
        <translation>校验值不匹配。预期：%1，实际：%2</translation>
    </message>
</context>
<context>
    <name>core::update::UpdateManager</name>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="105"/>
        <source>Failed to access system temporary directory</source>
        <translation>无法访问系统临时目录</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="111"/>
        <source>Failed to create update directory</source>
        <translation>创建更新目录失败</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="117"/>
        <source>Invalid update URL</source>
        <translation>更新链接无效</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="163"/>
        <source>Failed to open local file for writing</source>
        <translation>无法打开本地文件进行写入</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="240"/>
        <source>Failed to create update task file</source>
        <translation>创建更新任务文件失败</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="260"/>
        <source>Updater not found</source>
        <translation>未找到更新器</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="280"/>
        <source>Failed to launch updater (Access Denied or System Error)</source>
        <translation>启动更新器失败（拒绝访问或系统错误）</translation>
    </message>
    <message>
        <location filename="../../core/update/UpdateManager.cpp" line="289"/>
        <source>Automatic update is only supported on Windows</source>
        <translation>自动更新仅支持 Windows 系统</translation>
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
        <translation>TCP 客户端</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="278"/>
        <location filename="../MainWindow.cpp" line="513"/>
        <source>Serial Port</source>
        <translation>串口</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="279"/>
        <location filename="../MainWindow.cpp" line="513"/>
        <source>Frame Analyzer</source>
        <translation>报文分析</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="315"/>
        <location filename="../MainWindow.cpp" line="519"/>
        <source>Language</source>
        <translation>语言</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="309"/>
        <location filename="../MainWindow.cpp" line="520"/>
        <source>Settings</source>
        <translation>设置</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="213"/>
        <source>You are using the latest version: v%1</source>
        <translation>您正在使用最新版本：v%1</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="225"/>
        <source>Update Failed</source>
        <translation>更新失败</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="305"/>
        <source>Expand Navigation</source>
        <translation>展开导航</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="305"/>
        <source>Collapse Navigation</source>
        <translation>折叠导航</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="310"/>
        <location filename="../MainWindow.cpp" line="522"/>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="19"/>
        <source>Modbus Settings</source>
        <translation>Modbus设置</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="311"/>
        <location filename="../MainWindow.cpp" line="523"/>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="21"/>
        <source>Update Settings</source>
        <translation>更新设置</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="329"/>
        <location filename="../MainWindow.cpp" line="527"/>
        <source>English (US)</source>
        <translation>英语（美国）</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="330"/>
        <location filename="../MainWindow.cpp" line="528"/>
        <source>简体中文</source>
        <translation>简体中文</translation>
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
        <translation>检查更新</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="445"/>
        <source>New version v%1 available. Open download page?</source>
        <translation>发现新版本 v%1。是否打开下载页面？</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="453"/>
        <source>Current: v%1, Latest: v%2
Choose update method:</source>
        <translation>当前版本：v%1，最新版本：v%2
请选择更新方式：</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="469"/>
        <source>Downloading Update...</source>
        <translation>正在下载更新...</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="27"/>
        <source>Request Timeout (ms):</source>
        <translation>请求超时(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="31"/>
        <source>Enable Retry:</source>
        <translation>启用重试：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="36"/>
        <source>Retry Count:</source>
        <translation>重试次数：</translation>
    </message>
    <message>
        <location filename="../widgets/ModbusSettingsDialog.cpp" line="42"/>
        <source>Retry Interval (ms):</source>
        <translation>重试间隔(ms)：</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="445"/>
        <location filename="../MainWindow.cpp" line="452"/>
        <source>Update Available</source>
        <translation>发现新版本</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="213"/>
        <source>No Updates</source>
        <translation>无可用更新</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="217"/>
        <source>Update Check Failed</source>
        <translation>更新检查失败</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="469"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="454"/>
        <source>Update Main Program</source>
        <translation>更新主程序</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="455"/>
        <source>Download Full Package</source>
        <translation>下载完整安装包</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="339"/>
        <location filename="../MainWindow.cpp" line="342"/>
        <location filename="../MainWindow.cpp" line="430"/>
        <location filename="../MainWindow.cpp" line="521"/>
        <location filename="../MainWindow.cpp" line="525"/>
        <location filename="../widgets/AboutDialog.cpp" line="15"/>
        <source>About</source>
        <translation>关于</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="362"/>
        <source>Theme: Auto</source>
        <translation>主题：自动</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="363"/>
        <source>Theme: Light</source>
        <translation>主题：浅色</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="363"/>
        <source>Theme: Dark</source>
        <translation>主题：深色</translation>
    </message>
    <message>
        <location filename="../widgets/AboutDialog.cpp" line="24"/>
        <source>Welcome to Modbus-Tools&lt;br&gt;Version: v%1&lt;br&gt;&lt;br&gt;An open-source Modbus communication debugging assistant.&lt;br&gt;Developer: mingyucheng692&lt;br&gt;License: MIT License&lt;br&gt;&lt;br&gt;This project is developed in spare time, completely free and open-source.&lt;br&gt;Feel free to star on GitHub or submit issues.&lt;br&gt;Your feedback keeps the project improving!&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 Visit GitHub Repository&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 Issue Tracker&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;This software is provided &amp;quot;as is&amp;quot; without warranty of any kind.</source>
        <translation>欢迎使用 Modbus-Tools&lt;br&gt;版本：v%1&lt;br&gt;&lt;br&gt;一款开源的 Modbus 通讯调试助手。&lt;br&gt;开发者：mingyucheng692&lt;br&gt;许可协议：MIT License&lt;br&gt;&lt;br&gt;本项目由个人业余开发，完全免费且开源。&lt;br&gt;欢迎在 GitHub 上 Star ⭐ 或提交 Issue。&lt;br&gt;您的反馈是项目持续改进的动力！&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 访问 GitHub 仓库&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 问题反馈&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;本软件按&amp;quot;原样&amp;quot;提供，无任何形式的保证。</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="26"/>
        <source>Every Startup</source>
        <translation type="unfinished">每次启动</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="27"/>
        <source>Weekly</source>
        <translation type="unfinished">每周</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="28"/>
        <source>Monthly</source>
        <translation type="unfinished">每月</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="29"/>
        <source>Disable Update Check</source>
        <translation type="unfinished">关闭更新检测</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="30"/>
        <source>Update Check Frequency:</source>
        <translation type="unfinished">更新检测频率：</translation>
    </message>
</context>
<context>
    <name>ui::common::UpdateChecker</name>
    <message>
        <location filename="../common/updatechecker.cpp" line="94"/>
        <source>Release tag is missing</source>
        <translation>发布标签缺失</translation>
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
        <translation>正在打开 %1...</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="220"/>
        <source>Closed</source>
        <translation>已关闭</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="221"/>
        <source>Opening</source>
        <translation>正在打开</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="222"/>
        <source>Open</source>
        <translation>已打开</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="223"/>
        <source>Closing</source>
        <translation>正在关闭</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="224"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="225"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="228"/>
        <source>State changed: %1</source>
        <translation>状态已变更：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="233"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="266"/>
        <source>Control</source>
        <translation>控制</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="267"/>
        <source>Send Data</source>
        <translation>发送数据</translation>
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
        <translation>正在连接 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="215"/>
        <source>Closed</source>
        <translation>已关闭</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="216"/>
        <source>Opening</source>
        <translation>正在打开</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="217"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="218"/>
        <source>Closing</source>
        <translation>正在关闭</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="219"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="220"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="223"/>
        <source>State changed: %1</source>
        <translation>状态已变更：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="232"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="245"/>
        <source>Send Data</source>
        <translation>发送数据</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus_rtu::ModbusRtuView</name>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="122"/>
        <source>Opening %1...</source>
        <translation>正在打开 %1...</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="263"/>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation>发送读请求 FC:%1 地址:%2 数量:%3 从站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="221"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="142"/>
        <source>Failed to create Modbus stack</source>
        <translation>创建 Modbus 栈失败</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="185"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="188"/>
        <source>Connection failed: %1</source>
        <translation>连接失败：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="204"/>
        <source>Success: Broadcast write sent, no response expected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="210"/>
        <source>Success: Response received</source>
        <translation>成功：收到响应</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="236"/>
        <source>Disconnected</source>
        <translation type="unfinished">已断开</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="319"/>
        <source>Error: Invalid decimal value for 0x05</source>
        <translation>错误：0x05 十进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="326"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="332"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="339"/>
        <source>Error: Invalid hex value for 0x05</source>
        <translation>错误：0x05 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="349"/>
        <source>Error: Empty value for 0x06</source>
        <translation>错误：0x06 值为空</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="356"/>
        <source>Error: Invalid decimal value for 0x06</source>
        <translation>错误：0x06 十进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="364"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="374"/>
        <source>Error: Invalid hex value for 0x06</source>
        <translation>错误：0x06 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="380"/>
        <source>Error: 0x0F requires Hex data</source>
        <translation>错误：0x0F 需要十六进制数据</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="385"/>
        <source>Error: Invalid hex value for 0x0F</source>
        <translation>错误：0x0F 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="385"/>
        <source>Error: Binary bit count (%1) does not match Quantity (%2)</source>
        <translation>错误：二进制位数 (%1) 与数量 (%2) 不匹配</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="371"/>
        <source>Error: Binary format not supported for registers (0x06)</source>
        <translation>错误：寄存器操作 (0x06) 不支持二进制格式</translation>
    </message>
     <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="338"/>
        <source>Error: Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation>错误：0x05 二进制值无效（预期为 0 或 1）</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="390"/>
        <source>Error: Invalid quantity for 0x0F</source>
        <translation>错误：0x0F 数量无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="395"/>
        <source>Error: Coil data length mismatch for 0x0F</source>
        <translation>错误：0x0F 线圈数据长度不匹配</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="404"/>
        <source>Error: Empty value for 0x10</source>
        <translation>错误：0x10 值为空</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="409"/>
        <source>Error: Invalid quantity for 0x10</source>
        <translation>错误：0x10 数量无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="418"/>
        <source>Error: Invalid decimal list for 0x10</source>
        <translation>错误：0x10 十进制列表无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="429"/>
        <source>Error: Invalid hex value for 0x10</source>
        <translation>错误：0x10 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="435"/>
        <source>Error: Quantity does not match data length for 0x10</source>
        <translation>错误：0x10 数量与数据长度不匹配</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="443"/>
        <source>Error: Unsupported write function code</source>
        <translation>错误：不支持的写功能码</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="449"/>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation>发送写请求 FC:%1 地址:%2 数据:%3 从站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="212"/>
        <source>Success: Write confirmed</source>
        <translation>成功：写入确认</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="465"/>
        <source>Sending Raw Data: %1</source>
        <translation>发送原始数据：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="219"/>
        <source>Poll Error: %1</source>
        <translation>轮询错误：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="606"/>
        <source>Data Monitor</source>
        <translation>数据监视</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="607"/>
        <source>Receive Data</source>
        <translation>接收数据</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="608"/>
        <source>Send Data</source>
        <translation>发送数据</translation>
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
        <translation>复制</translation>
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
        <translation>正在连接 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="33"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="221"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="37"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="274"/>
        <source>Disconnected</source>
        <translation>已断开</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="300"/>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation>发送读请求 FC:%1 地址:%2 数量:%3 从站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="256"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="151"/>
        <source>Failed to create Modbus stack</source>
        <translation>创建 Modbus 栈失败</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="225"/>
        <source>Connection failed: %1</source>
        <translation>连接失败：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="245"/>
        <source>Success: Response received</source>
        <translation>成功：收到响应</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="387"/>
        <source>Error: Empty value for 0x06</source>
        <translation>错误：0x06 值为空</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="394"/>
        <source>Error: Invalid decimal value for 0x06</source>
        <translation>错误：0x06 十进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="402"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="412"/>
        <source>Error: Invalid hex value for 0x06</source>
        <translation>错误：0x06 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="418"/>
        <source>Error: 0x0F requires Hex data</source>
        <translation>错误：0x0F 需要十六进制数据</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="423"/>
        <source>Error: Invalid hex value for 0x0F</source>
        <translation>错误：0x0F 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="414"/>
        <source>Error: Binary format not supported for registers (0x06)</source>
        <translation>错误：寄存器操作 (0x06) 不支持二进制格式</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="428"/>
        <source>Error: Binary bit count (%1) does not match Quantity (%2)</source>
        <translation>错误：二进制位数 (%1) 与数量 (%2) 不匹配</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="381"/>
        <source>Error: Invalid binary value for 0x05 (expected 0 or 1)</source>
        <translation>错误：0x05 二进制值无效（预期为 0 或 1）</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="423"/>
        <source>Error: Invalid quantity for 0x0F</source>
        <translation>错误：0x0F 数量无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="433"/>
        <source>Error: Coil data length mismatch for 0x0F</source>
        <translation>错误：0x0F 线圈数据长度不匹配</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="442"/>
        <source>Error: Empty value for 0x10</source>
        <translation>错误：0x10 值为空</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="447"/>
        <source>Error: Invalid quantity for 0x10</source>
        <translation>错误：0x10 数量无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="456"/>
        <source>Error: Invalid decimal list for 0x10</source>
        <translation>错误：0x10 十进制列表无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="467"/>
        <source>Error: Invalid hex value for 0x10</source>
        <translation>错误：0x10 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="473"/>
        <source>Error: Quantity does not match data length for 0x10</source>
        <translation>错误：0x10 数量与数据长度不匹配</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="481"/>
        <source>Error: Unsupported write function code</source>
        <translation>错误：不支持的写功能码</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="487"/>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation>发送写请求 FC:%1 地址:%2 数据:%3 从站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="247"/>
        <source>Success: Write confirmed</source>
        <translation>成功：写入确认</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="504"/>
        <source>Sending Raw Data: %1</source>
        <translation>发送原始数据：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="254"/>
        <source>Poll Error: %1</source>
        <translation>轮询错误：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="647"/>
        <source>Data Monitor</source>
        <translation>数据监视</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="648"/>
        <source>Receive Data</source>
        <translation>接收数据</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="649"/>
        <source>Send Data</source>
        <translation>发送数据</translation>
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
        <translation>复制</translation>
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
        <translation>折叠</translation>
    </message>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="92"/>
        <source>Expand</source>
        <translation>展开</translation>
    </message>
</context>
<context>
    <name>ui::widgets::ControlWidget</name>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="208"/>
        <source>Enable Polling</source>
        <translation>启用轮询</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="211"/>
        <source>Interval(ms):</source>
        <translation>间隔(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="214"/>
        <source>FC:</source>
        <translation>功能码：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="217"/>
        <source>01-Read Coils</source>
        <translation>01-读线圈</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="218"/>
        <source>02-Read Discrete</source>
        <translation>02-读离散输入</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="219"/>
        <source>03-Read Holding</source>
        <translation>03-读保持寄存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="220"/>
        <source>04-Read Input</source>
        <translation>04-读输入寄存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="223"/>
        <source>Addr:</source>
        <translation>地址：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="226"/>
        <source>Qty:</source>
        <translation>数量：</translation>
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
        <translation>换算值：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="420"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1325"/>
        <source>Frame Input</source>
        <translation>报文输入</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="426"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1328"/>
        <source>Protocol:</source>
        <translation>协议：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="429"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1331"/>
        <source>Auto Detect</source>
        <translation>自动检测</translation>
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
        <translation>起始地址（用于响应）：</translation>
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
        <translation>输入十六进制字符串（如 01 03 00 00 00 01 84 0A）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="482"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1377"/>
        <source>Analysis Result</source>
        <translation>分析结果</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="488"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="795"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1403"/>
        <source>Ready</source>
        <translation>就绪</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="492"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1339"/>
        <source>Decode Mode:</source>
        <translation>解码模式：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="495"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1342"/>
        <source>Unsigned</source>
        <translation>无符号</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="496"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1343"/>
        <source>Signed</source>
        <translation>有符号</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="502"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1350"/>
        <source>Import Config</source>
        <translation>导入配置</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="503"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1354"/>
        <source>Export Config</source>
        <translation>导出配置</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="504"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="687"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1358"/>
        <source>Export CSV</source>
        <translation>导出 CSV</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="530"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1388"/>
        <source>Field</source>
        <translation>字段</translation>
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
        <translation>结构</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="541"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1393"/>
        <source>Address</source>
        <translation>地址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="541"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1393"/>
        <source>Hex</source>
        <translation>十六进制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="541"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1393"/>
        <source>Decimal</source>
        <translation>十进制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="541"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1393"/>
        <source>Binary</source>
        <translation>二进制</translation>
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
        <translation>数据详情</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="573"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1380"/>
        <source>History</source>
        <translation>历史记录</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="583"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1396"/>
        <source>Clear History</source>
        <translation>清空历史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="662"/>
        <source>Export Frame Metadata</source>
        <translation>导出帧元数据</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="662"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="671"/>
        <source>JSON Files (*.json)</source>
        <translation>JSON 文件 (*.json)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="671"/>
        <source>Import Frame Metadata</source>
        <translation>导入帧元数据</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="681"/>
        <source>No Data</source>
        <translation>无数据</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="681"/>
        <source>There is no parsed data to export.</source>
        <translation>没有可导出的解析数据。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="689"/>
        <source>CSV Files (*.csv)</source>
        <translation>CSV 文件 (*.csv)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="717"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1137"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1173"/>
        <source>Export Failed</source>
        <translation>导出失败</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="740"/>
        <source>Import Failed</source>
        <translation>导入失败</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="852"/>
        <source>Error: Empty input</source>
        <translation>错误：输入为空</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="861"/>
        <source>Parsing...</source>
        <translation>解析中...</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="924"/>
        <source>Parse Failed: %1</source>
        <translation>解析失败：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="934"/>
        <source>Parse error</source>
        <translation>解析错误</translation>
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
        <translation>请求</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="919"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1239"/>
        <source>Response</source>
        <translation>响应</translation>
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
        <translation>提示：若自动识别失败，可手动选择 RTU 或 TCP 以尝试强制解析。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="932"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="966"/>
        <source>Frame</source>
        <translation>帧</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="933"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="967"/>
        <source>%1 bytes</source>
        <translation>%1 字节</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="937"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="983"/>
        <source>Frame Bytes</source>
        <translation>帧字节</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="937"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="983"/>
        <source>Complete raw frame</source>
        <translation>完整原始帧</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="939"/>
        <source>Protocol detected before parse failed</source>
        <translation>解析失败前检测到的协议</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="939"/>
        <source>Protocol</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="940"/>
        <source>Detailed parse failure reason</source>
        <translation>详细解析失败原因</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="940"/>
        <source>Error</source>
        <translation type="unfinished">错误</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="953"/>
        <source>Forced Parsing</source>
        <translation>强制解析</translation>
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
        <translation>强制解析过程中忽略的问题</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="977"/>
        <source>Issue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="986"/>
        <source>MBAP Header</source>
        <translation>MBAP 头</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="986"/>
        <source>Transaction + Protocol + Length + Unit ID</source>
        <translation>事务 ID + 协议 ID + 长度 + 单元 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="988"/>
        <source>Transaction ID</source>
        <translation>事务 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="990"/>
        <source>Request/response correlation ID</source>
        <translation>请求/响应关联 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="992"/>
        <source>Protocol ID</source>
        <translation>协议 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="994"/>
        <source>Modbus TCP protocol identifier</source>
        <translation>Modbus TCP 协议标识</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="996"/>
        <source>Length</source>
        <translation>长度</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="998"/>
        <source>Remaining bytes after this field</source>
        <translation>该字段之后的剩余字节数</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1000"/>
        <source>Unit ID</source>
        <translation>单元 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1002"/>
        <source>Target slave / unit address</source>
        <translation>目标从站 / 单元地址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1005"/>
        <source>Slave ID</source>
        <translation>从站 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1007"/>
        <source>Target slave address</source>
        <translation>目标从站地址</translation>
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
        <translation>功能码 + 载荷</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1015"/>
        <source>Function Code</source>
        <translation>功能码</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1017"/>
        <source>Exception response</source>
        <translation>异常响应</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1017"/>
        <source>Normal response</source>
        <translation>正常响应</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1021"/>
        <source>Payload</source>
        <translation>载荷</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1023"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1029"/>
        <source>Exception detail payload</source>
        <translation>异常详情载荷</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1023"/>
        <source>Application data payload</source>
        <translation>应用数据载荷</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1027"/>
        <source>Exception Code</source>
        <translation>异常码</translation>
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
        <translation>CRC 无效</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1038"/>
        <source>Calculated: 0x%1</source>
        <translation>计算值：0x%1</translation>
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
        <translation>显示历史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1305"/>
        <source>Hide History</source>
        <translation>隐藏历史</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1344"/>
        <source>Choose how parsed numeric values are displayed.</source>
        <translation>选择解析后的数值显示方式。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1351"/>
        <source>Import saved field scale and description settings from a JSON file.</source>
        <translation>从 JSON 文件导入已保存的字段缩放和描述配置。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1355"/>
        <source>Export current field scale and description settings to a JSON file.</source>
        <translation>将当前字段缩放和描述配置导出到 JSON 文件。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1359"/>
        <source>Export the parsed register data in the current table as a CSV file.</source>
        <translation>将当前表格中的解析寄存器数据导出为 CSV 文件。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1362"/>
        <source>Show or hide the parse history panel.</source>
        <translation>显示或隐藏解析历史面板。</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="920"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="1240"/>
        <source>Exception</source>
        <translation>异常</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FunctionWidget</name>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="278"/>
        <source>Standard</source>
        <translation>标准</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="281"/>
        <source>Raw</source>
        <translation>原始</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="284"/>
        <source>Slave ID:</source>
        <translation>从站ID：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="287"/>
        <source>Start Addr:</source>
        <translation>起始地址：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="290"/>
        <source>Quantity:</source>
        <translation>数量：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="293"/>
        <source>Write Data:</source>
        <translation>写入数据：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="296"/>
        <source>Space separated hex (e.g., 01 02) or values</source>
        <translation>空格分隔的十六进制(如 01 02)或数值</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="299"/>
        <source>Format:</source>
        <translation>格式：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="302"/>
        <source>Hex</source>
        <translation>十六进制</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="334"/>
        <source>Binary</source>
        <translation>二进制</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="281"/>
        <source>Space separated hex (e.g., 01 02)</source>
        <translation>空格分隔的十六进制 (如 01 02)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="283"/>
        <source>Space separated decimal (e.g., 100 200)</source>
        <translation>空格分隔的十进制 (如 100 200)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="285"/>
        <source>Bit string (e.g., 1 1 0 1)</source>
        <translation>二进制位字符串 (如 1 1 0 1)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="303"/>
        <source>Decimal</source>
        <translation>十进制</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="306"/>
        <source>Read Coils (0x01)</source>
        <translation>读线圈 (0x01)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="309"/>
        <source>Read Discrete Inputs (0x02)</source>
        <translation>读离散输入 (0x02)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="312"/>
        <source>Read Holding Registers (0x03)</source>
        <translation>读保持寄存器 (0x03)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="315"/>
        <source>Read Input Registers (0x04)</source>
        <translation>读输入寄存器 (0x04)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="318"/>
        <source>Write Single Coil (0x05)</source>
        <translation>写单线圈 (0x05)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="321"/>
        <source>Write Single Register (0x06)</source>
        <translation>写单寄存器 (0x06)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="324"/>
        <source>Write Multiple Coils (0x0F)</source>
        <translation>写多线圈 (0x0F)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="327"/>
        <source>Write Multiple Registers (0x10)</source>
        <translation>写多寄存器 (0x10)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="330"/>
        <source>Raw Hex Data (e.g., 01 03 00 00 00 01 84 0A):</source>
        <translation>原始十六进制数据(如 01 03 00 00 00 01 84 0A)：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="333"/>
        <source>Send Raw</source>
        <translation>发送原始数据</translation>
    </message>
</context>
<context>
    <name>ui::widgets::GenericInputWidget</name>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="208"/>
        <source>Select File to Send</source>
        <translation>选择要发送的文件</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="219"/>
        <source>Enter data to send...</source>
        <translation>输入要发送的数据...</translation>
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
        <translation>自动发送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="231"/>
        <source> ms</source>
        <translation> 毫秒</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="234"/>
        <source>Send File</source>
        <translation>发送文件</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="237"/>
        <source>Send</source>
        <translation>发送</translation>
    </message>
</context>
<context>
    <name>ui::widgets::SerialConnectionWidget</name>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="57"/>
        <source>Disconnect</source>
        <translation>断开连接</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="58"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="61"/>
        <source>Connect</source>
        <translation>连接</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="62"/>
        <source>Disconnected</source>
        <translation>已断开</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="118"/>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="282"/>
        <source>Refresh Ports</source>
        <translation>刷新端口</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="264"/>
        <source>Connection Settings</source>
        <translation>连接设置</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="267"/>
        <source>Port:</source>
        <translation>端口：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="270"/>
        <source>Baud:</source>
        <translation>波特率：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="273"/>
        <source>Data:</source>
        <translation>数据位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="276"/>
        <source>Parity:</source>
        <translation>校验：</translation>
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
        <translation>断开连接</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="143"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="148"/>
        <source>Connect</source>
        <translation>连接</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="149"/>
        <source>Disconnected</source>
        <translation>已断开</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="158"/>
        <source>Connection Settings</source>
        <translation>连接设置</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="161"/>
        <source>Host:</source>
        <translation>主机：</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="164"/>
        <source>Port:</source>
        <translation>端口：</translation>
    </message>
</context>
<context>
    <name>ui::widgets::TrafficMonitorWidget</name>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="94"/>
        <source>Copy</source>
        <translation>复制</translation>
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
        <translation>[%1] [信息] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="198"/>
        <source>Save Log</source>
        <translation>保存日志</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="198"/>
        <source>Text Files (*.txt);;All Files (*)</source>
        <translation>文本文件 (*.txt);;所有文件 (*)</translation>
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
        <translation>通信监视</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="272"/>
        <source>Auto Scroll</source>
        <translation>自动滚动</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="275"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="276"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
</context>
<context>
    <name>ui::widgets::UpdateSettingsDialog</name>
</context>
<context>
    <name>ModbusPduBuilder</name>
    <message>
        <source>Invalid start address</source>
        <translation>无效的起始地址</translation>
    </message>
    <message>
        <source>Invalid quantity</source>
        <translation>无效的数量</translation>
    </message>
    <message>
        <source>Invalid quantity for 0x0F</source>
        <translation>0x0F 数量无效</translation>
    </message>
    <message>
        <source>Data length mismatch for 0x0F</source>
        <translation>0x0F 数据长度不匹配</translation>
    </message>
    <message>
        <source>Invalid quantity for 0x10</source>
        <translation>0x10 数量无效</translation>
    </message>
    <message>
        <source>Data length mismatch for 0x10</source>
        <translation>0x10 数据长度不匹配</translation>
    </message>
</context>
<context>
    <name>ModbusClient</name>
    <message>
        <source>Modbus exception response. Slave=%1 FC=0x%2 Exception=0x%3 (%4)</source>
        <translation>Modbus 异常响应。站号=%1 功能码=0x%2 异常码=0x%3 (%4)</translation>
    </message>
    <message>
        <source>Illegal Function</source>
        <translation>非法功能码</translation>
    </message>
    <message>
        <source>Illegal Data Address</source>
        <translation>非法数据地址</translation>
    </message>
    <message>
        <source>Illegal Data Value</source>
        <translation>非法数据值</translation>
    </message>
    <message>
        <source>Server Device Failure</source>
        <translation>从站设备故障</translation>
    </message>
    <message>
        <source>Acknowledge</source>
        <translation>确认</translation>
    </message>
    <message>
        <source>Server Device Busy</source>
        <translation>从站设备忙</translation>
    </message>
    <message>
        <source>Negative Acknowledge</source>
        <translation>否定确认</translation>
    </message>
    <message>
        <source>Memory Parity Error</source>
        <translation>寄存器校验错误</translation>
    </message>
    <message>
        <source>Gateway Path Unavailable</source>
        <translation>网关路径不可用</translation>
    </message>
    <message>
        <source>Gateway Target Device Failed To Respond</source>
        <translation>网关目标设备响应失败</translation>
    </message>
    <message>
        <source>Unknown Exception</source>
        <translation>未知异常</translation>
    </message>
</context>
</TS>
