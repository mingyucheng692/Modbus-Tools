<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>CommandEditDialog</name>
    <message>
        <source>Command Editor</source>
        <translation type="vanished">命令编辑器</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation type="vanished">名称：</translation>
    </message>
    <message>
        <source>Data:</source>
        <translation type="vanished">数据：</translation>
    </message>
    <message>
        <source>Hex Mode</source>
        <translation type="vanished">十六进制模式</translation>
    </message>
</context>
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
        <source>TCP connection was closed.</source>
        <translation>TCP 连接已关闭。</translation>
    </message>
</context>
<context>
    <name>ModbusFrameParser</name>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="15"/>
        <source>Illegal Function</source>
        <translation>非法功能码</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="17"/>
        <source>Illegal Data Address</source>
        <translation>非法数据地址</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="19"/>
        <source>Illegal Data Value</source>
        <translation>非法数据值</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="21"/>
        <source>Server Device Failure</source>
        <translation>服务器设备故障</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="23"/>
        <source>Acknowledge</source>
        <translation>确认</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="25"/>
        <source>Server Device Busy</source>
        <translation>服务器设备忙</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="27"/>
        <source>Memory Parity Error</source>
        <translation>内存奇偶校验错误</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="29"/>
        <source>Gateway Path Unavailable</source>
        <translation>网关路径不可用</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="31"/>
        <source>Gateway Target Device Failed To Respond</source>
        <translation>网关目标设备未响应</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="33"/>
        <source>Unknown Exception</source>
        <translation>未知异常</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="44"/>
        <source>Empty frame data</source>
        <translation>帧数据为空</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="56"/>
        <source>Unable to identify protocol (neither valid TCP nor RTU)</source>
        <translation>无法识别协议（既非有效 TCP 也非 RTU）</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="110"/>
        <source>Frame too short for Modbus TCP</source>
        <translation>Modbus TCP 帧过短</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="126"/>
        <source>Incomplete TCP frame. Expected %1 bytes, got %2</source>
        <translation>TCP 帧不完整。预期 %1 字节，实际 %2 字节</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="169"/>
        <source>CRC Mismatch. Expected %1, Got %2</source>
        <translation>CRC 校验失败。预期 %1，实际 %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="231"/>
        <source>Response PDU too short</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="239"/>
        <source>Byte count does not match payload length</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="248"/>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="340"/>
        <source>Register byte count must be even</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="333"/>
        <source>Write request byte count exceeds payload</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Request: Start Address %1, Quantity %2</source>
        <translation type="vanished">请求：起始地址 %1，数量 %2</translation>
    </message>
    <message>
        <source>Register %1</source>
        <translation type="vanished">寄存器 %1</translation>
    </message>
    <message>
        <source>Coil %1: %2</source>
        <translation type="vanished">线圈 %1: %2</translation>
    </message>
    <message>
        <source>Write Coil ON</source>
        <translation type="vanished">写线圈 ON</translation>
    </message>
    <message>
        <source>Write Coil OFF</source>
        <translation type="vanished">写线圈 OFF</translation>
    </message>
    <message>
        <source>Write Register %1</source>
        <translation type="vanished">写寄存器 %1</translation>
    </message>
    <message>
        <source>Response: Written %1 items starting at %2</source>
        <translation type="vanished">响应：从 %2 开始写入 %1 项</translation>
    </message>
    <message>
        <source>Write Coil %1: %2</source>
        <translation type="vanished">写线圈 %1: %2</translation>
    </message>
    <message>
        <location filename="../../core/modbus/parser/ModbusFrameParser.cpp" line="383"/>
        <source>Unsupported Function Code for deep parsing</source>
        <translation>不支持深度解析的功能码</translation>
    </message>
</context>
<context>
    <name>ui::MainWindow</name>
    <message>
        <location filename="../MainWindow.cpp" line="195"/>
        <location filename="../MainWindow.cpp" line="968"/>
        <source>Modbus Tools</source>
        <translation>Modbus 工具</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="281"/>
        <location filename="../MainWindow.cpp" line="970"/>
        <location filename="../MainWindow.cpp" line="976"/>
        <source>Modbus TCP</source>
        <translation>Modbus TCP</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="282"/>
        <location filename="../MainWindow.cpp" line="971"/>
        <location filename="../MainWindow.cpp" line="977"/>
        <source>Modbus RTU</source>
        <translation>Modbus RTU</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="283"/>
        <location filename="../MainWindow.cpp" line="972"/>
        <location filename="../MainWindow.cpp" line="978"/>
        <source>TCP Client</source>
        <translation>TCP 客户端</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="284"/>
        <location filename="../MainWindow.cpp" line="973"/>
        <location filename="../MainWindow.cpp" line="979"/>
        <source>Serial Port</source>
        <translation>串口</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="285"/>
        <location filename="../MainWindow.cpp" line="974"/>
        <location filename="../MainWindow.cpp" line="980"/>
        <source>Frame Analyzer</source>
        <translation>报文分析</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="336"/>
        <location filename="../MainWindow.cpp" line="995"/>
        <source>Language</source>
        <translation>语言</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="328"/>
        <location filename="../MainWindow.cpp" line="998"/>
        <source>Settings</source>
        <translation>设置</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="324"/>
        <source>Expand Navigation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="324"/>
        <source>Collapse Navigation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="329"/>
        <location filename="../MainWindow.cpp" line="414"/>
        <location filename="../MainWindow.cpp" line="1004"/>
        <source>Modbus Settings</source>
        <translation>Modbus设置</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="331"/>
        <location filename="../MainWindow.cpp" line="1007"/>
        <source>Update Settings</source>
        <translation>更新设置</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="340"/>
        <location filename="../MainWindow.cpp" line="1016"/>
        <source>English (US)</source>
        <translation>英语（美国）</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="345"/>
        <location filename="../MainWindow.cpp" line="1020"/>
        <source>简体中文</source>
        <translation>简体中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="350"/>
        <location filename="../MainWindow.cpp" line="1024"/>
        <source>繁體中文</source>
        <translation>繁體中文</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="362"/>
        <location filename="../MainWindow.cpp" line="557"/>
        <location filename="../MainWindow.cpp" line="1010"/>
        <source>Check for Updates</source>
        <translation>检查更新</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="421"/>
        <source>Request Timeout (ms):</source>
        <translation>请求超时(ms)：</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="425"/>
        <source>Enable Retry:</source>
        <translation>启用重试：</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="430"/>
        <source>Retry Count:</source>
        <translation>重试次数：</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="436"/>
        <source>Retry Interval (ms):</source>
        <translation>重试间隔(ms)：</translation>
    </message>
    <message>
        <source>Current version: v%1
Latest version: v%2

Open download page now?</source>
        <translation type="vanished">当前版本：v%1
最新版本：v%2

立即打开下载页面吗？</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="836"/>
        <location filename="../MainWindow.cpp" line="845"/>
        <source>Update Available</source>
        <translation>发现新版本</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="907"/>
        <source>No Updates</source>
        <translation>无可用更新</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="908"/>
        <source>You are already using the latest version: v%1</source>
        <translation>当前已是最新版本：v%1</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="772"/>
        <location filename="../MainWindow.cpp" line="781"/>
        <location filename="../MainWindow.cpp" line="788"/>
        <location filename="../MainWindow.cpp" line="795"/>
        <location filename="../MainWindow.cpp" line="804"/>
        <location filename="../MainWindow.cpp" line="812"/>
        <location filename="../MainWindow.cpp" line="819"/>
        <location filename="../MainWindow.cpp" line="857"/>
        <location filename="../MainWindow.cpp" line="917"/>
        <source>Update Check Failed</source>
        <translation>更新检查失败</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="579"/>
        <source>Invalid update URL</source>
        <translation>更新链接无效</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="586"/>
        <source>Failed to create update directory</source>
        <translation>创建更新目录失败</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="592"/>
        <source>Failed to write update file</source>
        <translation>写入更新文件失败</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="603"/>
        <source>Downloading %1</source>
        <translation>正在下载 %1</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="604"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="608"/>
        <source>Downloading Update</source>
        <translation>正在下载更新</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="636"/>
        <source>Download canceled</source>
        <translation>下载已取消</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="645"/>
        <source>Downloaded file is empty</source>
        <translation>下载文件为空</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="702"/>
        <source>Failed to create update task file</source>
        <translation>创建更新任务文件失败</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="708"/>
        <source>Failed to write update task file</source>
        <translation>写入更新任务文件失败</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="717"/>
        <source>Updater not found</source>
        <translation>未找到更新器</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="735"/>
        <location filename="../MainWindow.cpp" line="745"/>
        <source>Failed to launch updater</source>
        <translation>启动更新器失败</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="781"/>
        <source>Missing update checksum</source>
        <translation>缺少更新校验值</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="795"/>
        <source>Unable to resolve update checksum</source>
        <translation>无法解析更新校验值</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="804"/>
        <source>Update file verification failed</source>
        <translation>更新文件校验失败</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="831"/>
        <source>Current version: v%1
Latest version: v%2</source>
        <translation>当前版本：v%1
最新版本：v%2</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="835"/>
        <source>

UpdateOnly package is unavailable for this release.
Open download page now?</source>
        <translation>

当前版本未提供 UpdateOnly 增量包。
是否立即打开下载页面？</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="846"/>
        <source>

Choose update method:</source>
        <translation>

请选择更新方式：</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="847"/>
        <source>Update Main Program</source>
        <translation>更新主程序</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="850"/>
        <source>Download Full Package</source>
        <translation>下载完整安装包</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="857"/>
        <source>Main program update failed</source>
        <translation>主程序更新失败</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="361"/>
        <location filename="../MainWindow.cpp" line="365"/>
        <location filename="../MainWindow.cpp" line="478"/>
        <location filename="../MainWindow.cpp" line="556"/>
        <location filename="../MainWindow.cpp" line="1001"/>
        <location filename="../MainWindow.cpp" line="1013"/>
        <source>About</source>
        <translation>关于</translation>
    </message>
    <message>
        <source>Every Startup</source>
        <translation type="vanished">每次启动</translation>
    </message>
    <message>
        <source>Weekly</source>
        <translation type="vanished">每周</translation>
    </message>
    <message>
        <source>Monthly</source>
        <translation type="vanished">每月</translation>
    </message>
    <message>
        <source>Disable Update Check</source>
        <translation type="vanished">关闭更新检测</translation>
    </message>
    <message>
        <source>Update Check Frequency:</source>
        <translation type="vanished">更新检测频率：</translation>
    </message>
    <message>
        <location filename="../MainWindow.cpp" line="487"/>
        <source>Welcome to Modbus-Tools&lt;br&gt;Version: v%1&lt;br&gt;&lt;br&gt;An open-source Modbus communication debugging assistant.&lt;br&gt;Developer: mingyucheng692&lt;br&gt;License: MIT License&lt;br&gt;&lt;br&gt;This project is developed in spare time, completely free and open-source.&lt;br&gt;Feel free to star on GitHub or submit issues.&lt;br&gt;Your feedback keeps the project improving!&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 Visit GitHub Repository&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 Issue Tracker&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;This software is provided &amp;quot;as is&amp;quot; without warranty of any kind.</source>
        <translation>欢迎使用 Modbus-Tools&lt;br&gt;版本：v%1&lt;br&gt;&lt;br&gt;一款开源的 Modbus 通讯调试助手。&lt;br&gt;开发者：mingyucheng692&lt;br&gt;许可协议：MIT License&lt;br&gt;&lt;br&gt;本项目由个人业余开发，完全免费且开源。&lt;br&gt;欢迎在 GitHub 上 Star ⭐ 或提交 Issue。&lt;br&gt;您的反馈是项目持续改进的动力！&lt;br&gt;&lt;br&gt;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools&quot;&gt;🌐 访问 GitHub 仓库&lt;/a&gt;&amp;nbsp;&amp;nbsp;&amp;nbsp;&lt;a href=&quot;https://github.com/mingyucheng692/Modbus-Tools/issues&quot;&gt;🐛 问题反馈&lt;/a&gt;&lt;br&gt;&lt;br&gt;--------------------------&lt;br&gt;本软件按&amp;quot;原样&amp;quot;提供，无任何形式的保证。</translation>
    </message>
</context>
<context>
    <name>ui::common::UpdateChecker</name>
    <message>
        <source>Invalid release response</source>
        <translation type="vanished">发布信息响应无效</translation>
    </message>
    <message>
        <location filename="../common/updatechecker.cpp" line="94"/>
        <source>Release tag is missing</source>
        <translation>发布标签缺失</translation>
    </message>
</context>
<context>
    <name>ui::views::generic_serial::GenericSerialView</name>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="120"/>
        <source>Opening %1...</source>
        <translation>正在打开 %1...</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="151"/>
        <source>Closed</source>
        <translation>已关闭</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="152"/>
        <source>Opening</source>
        <translation>正在打开</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="153"/>
        <source>Open</source>
        <translation>已打开</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="154"/>
        <source>Closing</source>
        <translation>正在关闭</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="155"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="156"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="159"/>
        <source>State changed: %1</source>
        <translation>状态已变更：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="163"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="190"/>
        <source>Control</source>
        <translation>控制</translation>
    </message>
    <message>
        <source>Quick Commands</source>
        <translation type="vanished">快捷指令</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="191"/>
        <source>Send Data</source>
        <translation>发送数据</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="192"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../views/generic_serial/GenericSerialView.cpp" line="193"/>
        <source>RTS</source>
        <translation>RTS</translation>
    </message>
</context>
<context>
    <name>ui::views::generic_tcp::GenericTcpView</name>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="104"/>
        <source>Connecting to %1:%2...</source>
        <translation>正在连接 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="142"/>
        <source>Closed</source>
        <translation>已关闭</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="143"/>
        <source>Opening</source>
        <translation>正在打开</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="144"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="145"/>
        <source>Closing</source>
        <translation>正在关闭</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="146"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="147"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="150"/>
        <source>State changed: %1</source>
        <translation>状态已变更：%1</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="158"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <source>Quick Commands</source>
        <translation type="vanished">快捷指令</translation>
    </message>
    <message>
        <location filename="../views/generic_tcp/GenericTcpView.cpp" line="171"/>
        <source>Send Data</source>
        <translation>发送数据</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus_rtu::ModbusRtuView</name>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="117"/>
        <source>Opening %1...</source>
        <translation>正在打开 %1...</translation>
    </message>
    <message>
        <source>Port Opened</source>
        <translation type="vanished">串口已打开</translation>
    </message>
    <message>
        <source>Failed to open port</source>
        <translation type="vanished">打开串口失败</translation>
    </message>
    <message>
        <source>Closed</source>
        <translation type="vanished">已关闭</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="241"/>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation>发送读请求 FC:%1 地址:%2 数量:%3 从站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="199"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="136"/>
        <source>Failed to create Modbus stack</source>
        <translation>更新设置</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="166"/>
        <source>Connected</source>
        <translation type="unfinished">已连接</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="169"/>
        <source>Connection failed: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="188"/>
        <source>Success: Response received</source>
        <translation>成功：收到响应</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="214"/>
        <source>Disconnected</source>
        <translation type="unfinished">已断开</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="297"/>
        <source>Error: Invalid decimal value for 0x05</source>
        <translation>错误：0x05 十进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="304"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="310"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="317"/>
        <source>Error: Invalid hex value for 0x05</source>
        <translation>错误：0x05 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="327"/>
        <source>Error: Empty value for 0x06</source>
        <translation>错误：0x06 值为空</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="334"/>
        <source>Error: Invalid decimal value for 0x06</source>
        <translation>错误：0x06 十进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="342"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="352"/>
        <source>Error: Invalid hex value for 0x06</source>
        <translation>错误：0x06 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="358"/>
        <source>Error: 0x0F requires Hex data</source>
        <translation>错误：0x0F 需要十六进制数据</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="363"/>
        <source>Error: Invalid hex value for 0x0F</source>
        <translation>错误：0x0F 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="368"/>
        <source>Error: Invalid quantity for 0x0F</source>
        <translation>错误：0x0F 数量无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="373"/>
        <source>Error: Coil data length mismatch for 0x0F</source>
        <translation>错误：0x0F 线圈数据长度不匹配</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="382"/>
        <source>Error: Empty value for 0x10</source>
        <translation>错误：0x10 值为空</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="387"/>
        <source>Error: Invalid quantity for 0x10</source>
        <translation>错误：0x10 数量无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="396"/>
        <source>Error: Invalid decimal list for 0x10</source>
        <translation>错误：0x10 十进制列表无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="407"/>
        <source>Error: Invalid hex value for 0x10</source>
        <translation>错误：0x10 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="413"/>
        <source>Error: Quantity does not match data length for 0x10</source>
        <translation>错误：0x10 数量与数据长度不匹配</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="421"/>
        <source>Error: Unsupported write function code</source>
        <translation>错误：不支持的写功能码</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="427"/>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation>发送写请求 FC:%1 地址:%2 数据:%3 从站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="190"/>
        <source>Success: Write confirmed</source>
        <translation>成功：写入确认</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="443"/>
        <source>Sending Raw Data: %1</source>
        <translation>发送原始数据：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="197"/>
        <source>Poll Error: %1</source>
        <translation>轮询错误：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="549"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="559"/>
        <source>[%1] %2</source>
        <translation>[%1] %2</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="549"/>
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="559"/>
        <source>TX</source>
        <translation>发送</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="563"/>
        <source>Data Monitor</source>
        <translation>数据监视</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="564"/>
        <source>Receive Data</source>
        <translation>接收数据</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="565"/>
        <source>Send Data</source>
        <translation>发送数据</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="566"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="567"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="568"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="569"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="570"/>
        <location filename="../views/modbus_rtu/ModbusRtuView.cpp" line="571"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
</context>
<context>
    <name>ui::views::modbus_tcp::ModbusTcpView</name>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="118"/>
        <source>Connecting to %1:%2...</source>
        <translation>正在连接 %1:%2...</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="180"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <source>Connection failed</source>
        <translation type="vanished">连接失败</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="231"/>
        <source>Disconnected</source>
        <translation>已断开</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="257"/>
        <source>Sending Read Request FC:%1 Addr:%2 Qty:%3 Slave:%4</source>
        <translation>发送读请求 FC:%1 地址:%2 数量:%3 从站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="213"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="134"/>
        <source>Failed to create Modbus stack</source>
        <translation type="unfinished">更新设置</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="183"/>
        <source>Connection failed: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="202"/>
        <source>Success: Response received</source>
        <translation>成功：收到响应</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="314"/>
        <source>Error: Invalid decimal value for 0x05</source>
        <translation>错误：0x05 十进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="321"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="327"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="334"/>
        <source>Error: Invalid hex value for 0x05</source>
        <translation>错误：0x05 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="344"/>
        <source>Error: Empty value for 0x06</source>
        <translation>错误：0x06 值为空</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="351"/>
        <source>Error: Invalid decimal value for 0x06</source>
        <translation>错误：0x06 十进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="359"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="369"/>
        <source>Error: Invalid hex value for 0x06</source>
        <translation>错误：0x06 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="375"/>
        <source>Error: 0x0F requires Hex data</source>
        <translation>错误：0x0F 需要十六进制数据</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="380"/>
        <source>Error: Invalid hex value for 0x0F</source>
        <translation>错误：0x0F 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="385"/>
        <source>Error: Invalid quantity for 0x0F</source>
        <translation>错误：0x0F 数量无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="390"/>
        <source>Error: Coil data length mismatch for 0x0F</source>
        <translation>错误：0x0F 线圈数据长度不匹配</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="399"/>
        <source>Error: Empty value for 0x10</source>
        <translation>错误：0x10 值为空</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="404"/>
        <source>Error: Invalid quantity for 0x10</source>
        <translation>错误：0x10 数量无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="413"/>
        <source>Error: Invalid decimal list for 0x10</source>
        <translation>错误：0x10 十进制列表无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="424"/>
        <source>Error: Invalid hex value for 0x10</source>
        <translation>错误：0x10 十六进制值无效</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="430"/>
        <source>Error: Quantity does not match data length for 0x10</source>
        <translation>错误：0x10 数量与数据长度不匹配</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="438"/>
        <source>Error: Unsupported write function code</source>
        <translation>错误：不支持的写功能码</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="444"/>
        <source>Sending Write Request FC:%1 Addr:%2 Data:%3 Slave:%4</source>
        <translation>发送写请求 FC:%1 地址:%2 数据:%3 从站:%4</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="204"/>
        <source>Success: Write confirmed</source>
        <translation>成功：写入确认</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="461"/>
        <source>Sending Raw Data: %1</source>
        <translation>发送原始数据：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="211"/>
        <source>Poll Error: %1</source>
        <translation>轮询错误：%1</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="570"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="580"/>
        <source>[%1] %2</source>
        <translation>[%1] %2</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="570"/>
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="580"/>
        <source>TX</source>
        <translation>发送</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="584"/>
        <source>Data Monitor</source>
        <translation>数据监视</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="585"/>
        <source>Receive Data</source>
        <translation>接收数据</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="586"/>
        <source>Send Data</source>
        <translation>发送数据</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="587"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="588"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="589"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="590"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="591"/>
        <location filename="../views/modbus_tcp/ModbusTcpView.cpp" line="592"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
</context>
<context>
    <name>ui::widgets::CollapsibleSection</name>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="89"/>
        <source>Collapse</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/CollapsibleSection.cpp" line="89"/>
        <source>Expand</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ui::widgets::ControlWidget</name>
    <message>
        <source>TX: %1 | RX: %2 | Avg RTT: %3 ms</source>
        <translation type="vanished">TX:%1 | RX:%2 | 平均RTT:%3 ms</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="85"/>
        <source>TX: %1 | RX: %2 | FAIL: %3 | RTT: %4 ms</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="189"/>
        <source>Enable Polling</source>
        <translation>启用轮询</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="192"/>
        <source>Interval(ms):</source>
        <translation>间隔(ms)：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="195"/>
        <source>FC:</source>
        <translation>功能码：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="198"/>
        <source>01-Read Coils</source>
        <translation>01-读线圈</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="199"/>
        <source>02-Read Discrete</source>
        <translation>02-读离散输入</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="200"/>
        <source>03-Read Holding</source>
        <translation>03-读保持寄存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="201"/>
        <source>04-Read Input</source>
        <translation>04-读输入寄存器</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="204"/>
        <source>Addr:</source>
        <translation>地址：</translation>
    </message>
    <message>
        <location filename="../widgets/ControlWidget.cpp" line="207"/>
        <source>Qty:</source>
        <translation>数量：</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FrameAnalyzerWidget</name>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="115"/>
        <source>Description: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="121"/>
        <source>Raw: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="122"/>
        <source>Scale: %1</source>
        <translation>倍率：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="123"/>
        <source>Scaled: %1</source>
        <translation>换算值：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="237"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="669"/>
        <source>Frame Input</source>
        <translation>报文输入</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="243"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="672"/>
        <source>Protocol:</source>
        <translation>协议：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="246"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="675"/>
        <source>Auto Detect</source>
        <translation>自动检测</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="247"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="676"/>
        <source>Modbus TCP</source>
        <translation>Modbus TCP</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="248"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="677"/>
        <source>Modbus RTU</source>
        <translation>Modbus RTU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="252"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="680"/>
        <source>Start Address (for Response):</source>
        <translation>起始地址（用于响应）：</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="261"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="690"/>
        <source>Format Hex</source>
        <translation>格式化 Hex</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="265"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="699"/>
        <source>Parse</source>
        <translation>解析</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="269"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="702"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="277"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="705"/>
        <source>Enter Hex string (e.g., 01 03 00 00 00 01 84 0A)</source>
        <translation>输入十六进制字符串（如 01 03 00 00 00 01 84 0A）</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="288"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="709"/>
        <source>Analysis Result</source>
        <translation>分析结果</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="292"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="469"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="726"/>
        <source>Ready</source>
        <translation>就绪</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="296"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="683"/>
        <source>Decode Mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="299"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="686"/>
        <source>Unsigned</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="300"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="687"/>
        <source>Signed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="303"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="693"/>
        <source>Import JSON</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="305"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="696"/>
        <source>Export JSON</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="313"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="717"/>
        <source>Field</source>
        <translation>字段</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="313"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="321"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="718"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="722"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="313"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="321"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="719"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="722"/>
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="316"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="712"/>
        <source>Structure</source>
        <translation>结构</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="321"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="722"/>
        <source>Address</source>
        <translation>地址</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="321"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="722"/>
        <source>Hex</source>
        <translation>十六进制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="321"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="722"/>
        <source>Decimal</source>
        <translation>十进制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="321"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="722"/>
        <source>Binary</source>
        <translation>二进制</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="321"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="722"/>
        <source>Scale</source>
        <translation>倍率</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="333"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="713"/>
        <source>Data Details</source>
        <translation>数据详情</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="379"/>
        <source>Export Frame Metadata</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="379"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="388"/>
        <source>JSON Files (*.json)</source>
        <translation type="unfinished">JSON 文件 (*.json)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="388"/>
        <source>Import Frame Metadata</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="413"/>
        <source>Export Failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="413"/>
        <source>Cannot write file: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="424"/>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="430"/>
        <source>Import Failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="424"/>
        <source>Cannot open file: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="430"/>
        <source>Invalid JSON format.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="527"/>
        <source>Error: Empty input</source>
        <translation>错误：输入为空</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="562"/>
        <source>Parse Failed: %1</source>
        <translation>解析失败：%1</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="567"/>
        <source>Success (%1)</source>
        <translation>成功 (%1)</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="567"/>
        <source>TCP</source>
        <translation>TCP</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="567"/>
        <source>RTU</source>
        <translation>RTU</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="572"/>
        <source>Frame</source>
        <translation>帧</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="573"/>
        <source>%1 bytes</source>
        <translation>%1 字节</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="577"/>
        <source>Transaction ID</source>
        <translation>事务 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="578"/>
        <source>Protocol ID</source>
        <translation>协议 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="579"/>
        <source>Length</source>
        <translation>长度</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="580"/>
        <source>Unit ID</source>
        <translation>单元 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="582"/>
        <source>Slave ID</source>
        <translation>从站 ID</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="586"/>
        <source>Function Code</source>
        <translation>功能码</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="586"/>
        <source>Exception</source>
        <translation>异常</translation>
    </message>
    <message>
        <location filename="../widgets/FrameAnalyzerWidget.cpp" line="586"/>
        <source>Normal</source>
        <translation>正常</translation>
    </message>
</context>
<context>
    <name>ui::widgets::FunctionWidget</name>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="257"/>
        <source>Standard</source>
        <translation>标准</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="260"/>
        <source>Raw</source>
        <translation>原始</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="263"/>
        <source>Slave ID:</source>
        <translation>从站ID：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="266"/>
        <source>Start Addr:</source>
        <translation>起始地址：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="269"/>
        <source>Quantity:</source>
        <translation>数量：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="272"/>
        <source>Write Data:</source>
        <translation>写入数据：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="275"/>
        <source>Space separated hex (e.g., 01 02) or values</source>
        <translation>空格分隔的十六进制(如 01 02)或数值</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="278"/>
        <source>Format:</source>
        <translation>格式：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="281"/>
        <source>Hex</source>
        <translation>十六进制</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="282"/>
        <source>Decimal</source>
        <translation>十进制</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="285"/>
        <source>Read Coils (0x01)</source>
        <translation>读线圈 (0x01)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="288"/>
        <source>Read Discrete (0x02)</source>
        <translation>读离散输入 (0x02)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="291"/>
        <source>Read Holding (0x03)</source>
        <translation>读保持寄存器 (0x03)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="294"/>
        <source>Read Input (0x04)</source>
        <translation>读输入寄存器 (0x04)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="297"/>
        <source>Write Coil (0x05)</source>
        <translation>写单线圈 (0x05)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="300"/>
        <source>Write Reg (0x06)</source>
        <translation>写单寄存器 (0x06)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="303"/>
        <source>Write Multi Coils (0x0F)</source>
        <translation>写多线圈 (0x0F)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="306"/>
        <source>Write Multi Regs (0x10)</source>
        <translation>写多寄存器 (0x10)</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="309"/>
        <source>Raw Hex Data (e.g., 01 03 00 00 00 01 84 0A):</source>
        <translation>原始十六进制数据(如 01 03 00 00 00 01 84 0A)：</translation>
    </message>
    <message>
        <location filename="../widgets/FunctionWidget.cpp" line="312"/>
        <source>Send Raw</source>
        <translation>发送原始数据</translation>
    </message>
</context>
<context>
    <name>ui::widgets::GenericInputWidget</name>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="199"/>
        <source>Select File to Send</source>
        <translation>选择要发送的文件</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="204"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="204"/>
        <source>Cannot open file</source>
        <translation>无法打开文件</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="222"/>
        <source>Enter data to send...</source>
        <translation>输入要发送的数据...</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="225"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="228"/>
        <source>ASCII</source>
        <translation>ASCII</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="231"/>
        <source>Auto Send</source>
        <translation>自动发送</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="234"/>
        <source> ms</source>
        <translation> 毫秒</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="237"/>
        <source>Send File</source>
        <translation>发送文件</translation>
    </message>
    <message>
        <location filename="../widgets/GenericInputWidget.cpp" line="240"/>
        <source>Send</source>
        <translation>发送</translation>
    </message>
</context>
<context>
    <name>ui::widgets::QuickCommandWidget</name>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>ASCII</source>
        <translation type="vanished">ASCII</translation>
    </message>
    <message>
        <source>Export Commands</source>
        <translation type="vanished">导出指令</translation>
    </message>
    <message>
        <source>JSON Files (*.json)</source>
        <translation type="vanished">JSON 文件 (*.json)</translation>
    </message>
    <message>
        <source>Import Commands</source>
        <translation type="vanished">导入指令</translation>
    </message>
    <message>
        <source>Add</source>
        <translation type="vanished">添加</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="vanished">编辑</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation type="vanished">删除</translation>
    </message>
    <message>
        <source>Send</source>
        <translation type="vanished">发送</translation>
    </message>
    <message>
        <source>Export</source>
        <translation type="vanished">导出</translation>
    </message>
    <message>
        <source>Import</source>
        <translation type="vanished">导入</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="vanished">名称</translation>
    </message>
    <message>
        <source>Data</source>
        <translation type="vanished">数据</translation>
    </message>
    <message>
        <source>Format</source>
        <translation type="vanished">格式</translation>
    </message>
</context>
<context>
    <name>ui::widgets::SerialConnectionWidget</name>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="56"/>
        <source>Disconnect</source>
        <translation>断开连接</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="57"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="60"/>
        <source>Connect</source>
        <translation>连接</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="61"/>
        <source>Disconnected</source>
        <translation>已断开</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="117"/>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="279"/>
        <source>Refresh Ports</source>
        <translation>刷新端口</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="261"/>
        <source>Connection Settings</source>
        <translation>连接设置</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="264"/>
        <source>Port:</source>
        <translation>端口：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="267"/>
        <source>Baud:</source>
        <translation>波特率：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="270"/>
        <source>Data:</source>
        <translation>数据位：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="273"/>
        <source>Parity:</source>
        <translation>校验：</translation>
    </message>
    <message>
        <location filename="../widgets/SerialConnectionWidget.cpp" line="276"/>
        <source>Stop:</source>
        <translation>停止位：</translation>
    </message>
</context>
<context>
    <name>ui::widgets::TcpConnectionWidget</name>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="133"/>
        <source>Disconnect</source>
        <translation>断开连接</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="134"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="139"/>
        <source>Connect</source>
        <translation>连接</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="140"/>
        <source>Disconnected</source>
        <translation>已断开</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="149"/>
        <source>Connection Settings</source>
        <translation>连接设置</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="152"/>
        <source>Host:</source>
        <translation>主机：</translation>
    </message>
    <message>
        <location filename="../widgets/TcpConnectionWidget.cpp" line="155"/>
        <source>Port:</source>
        <translation>端口：</translation>
    </message>
</context>
<context>
    <name>ui::widgets::TrafficMonitorWidget</name>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="83"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="95"/>
        <source>[%1] [TX] %2</source>
        <translation>[%1] [TX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="111"/>
        <source>[%1] [RX] %2</source>
        <translation>[%1] [RX] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="124"/>
        <source>[%1] [INFO] %2</source>
        <translation>[%1] [信息] %2</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="192"/>
        <source>Save Log</source>
        <translation>保存日志</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="192"/>
        <source>Text Files (*.txt);;All Files (*)</source>
        <translation>文本文件 (*.txt);;所有文件 (*)</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="213"/>
        <source>Traffic Monitor</source>
        <translation>通信监视</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="214"/>
        <source>Auto Scroll</source>
        <translation>自动滚动</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="215"/>
        <source>TX</source>
        <translation>发送</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="216"/>
        <source>RX</source>
        <translation>接收</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="217"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../widgets/TrafficMonitorWidget.cpp" line="218"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
</context>
<context>
    <name>ui::widgets::UpdateSettingsDialog</name>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="21"/>
        <source>Update Settings</source>
        <translation type="unfinished">更新设置</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="26"/>
        <source>Every Startup</source>
        <translation>每次启动</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="27"/>
        <source>Weekly</source>
        <translation>每周</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="28"/>
        <source>Monthly</source>
        <translation>每月</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="29"/>
        <source>Disable Update Check</source>
        <translation>关闭更新检测</translation>
    </message>
    <message>
        <location filename="../widgets/UpdateSettingsDialog.cpp" line="30"/>
        <source>Update Check Frequency:</source>
        <translation>更新检测频率：</translation>
    </message>
</context>
</TS>
