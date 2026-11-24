// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

namespace core::common::settings_keys {

inline constexpr char kAppLanguage[] = "app/language";
inline constexpr char kAppThemeMode[] = "app/themeMode";
inline constexpr char kAppNavigationCollapsed[] = "app/navigationCollapsed";
inline constexpr char kAppUpdateCheckFrequency[] = "app/updateCheckFrequency";
inline constexpr char kAppUpdateLastCheckUtc[] = "app/updateLastCheckUtc";
inline constexpr char kAppDisclaimerAccepted[] = "app/disclaimerAccepted";
inline constexpr char kAppMainWindowGeometry[] = "app/mainWindowGeometry";
inline constexpr char kAppMainWindowState[] = "app/mainWindowState";
inline constexpr char kLoggingLevel[] = "Logging/Level";

inline constexpr char kModbusTimeoutMs[] = "modbus/timeoutMs";
inline constexpr char kModbusRetryCount[] = "modbus/retryCount";
inline constexpr char kModbusRetryIntervalMs[] = "modbus/retryIntervalMs";
inline constexpr char kModbusRetryEnabled[] = "modbus/retryEnabled";
inline constexpr char kModbusAddressBase[] = "modbus/addressBase";

inline constexpr char kFrameAnalyzerStartAddr[] = "frame_analyzer/startAddr";
inline constexpr char kFrameAnalyzerDecodeMode[] = "frame_analyzer/decodeMode";
inline constexpr char kFrameAnalyzerHistoryCollapsed[] = "frame_analyzer/historyCollapsed";

inline constexpr char kModbusTcpIp[] = "modbus/tcp/ip";
inline constexpr char kModbusTcpPort[] = "modbus/tcp/port";
inline constexpr char kModbusTcpConnectionCollapsed[] = "modbus/tcp/ui/connectionSettingsCollapsed";

inline constexpr char kModbusRtuBaudRate[] = "modbus/rtu/serial/baudRate";
inline constexpr char kModbusRtuDataBits[] = "modbus/rtu/serial/dataBits";
inline constexpr char kModbusRtuParity[] = "modbus/rtu/serial/parity";
inline constexpr char kModbusRtuStopBits[] = "modbus/rtu/serial/stopBits";
inline constexpr char kModbusRtuPortName[] = "modbus/rtu/serial/portName";
inline constexpr char kModbusRtuConnectionCollapsed[] = "modbus/rtu/serial/ui/connectionSettingsCollapsed";

// Network Page — canonical keys (Scheme A).
inline constexpr char kNetworkProtocol[] = "network/protocol";
inline constexpr char kNetworkClientIp[] = "network/client/ip";
inline constexpr char kNetworkClientPort[] = "network/client/port";
inline constexpr char kNetworkClientConnectionCollapsed[] = "network/client/ui/connectionSettingsCollapsed";
inline constexpr char kNetworkClientTrafficAutoScroll[] = "network/client/traffic/autoScroll";
inline constexpr char kNetworkClientTrafficShowTx[] = "network/client/traffic/showTx";
inline constexpr char kNetworkClientTrafficShowRx[] = "network/client/traffic/showRx";
inline constexpr char kNetworkClientInputFormat[] = "network/client/input/format";
inline constexpr char kNetworkClientInputIntervalMs[] = "network/client/input/intervalMs";
inline constexpr char kNetworkClientInputCollapsed[] = "network/client/ui/inputCollapsed";
inline constexpr char kNetworkUdpRemoteIp[] = "network/udp/remoteIp";
inline constexpr char kNetworkUdpRemotePort[] = "network/udp/remotePort";

// Serial Page — canonical keys (Scheme A).
inline constexpr char kSerialBaudRate[] = "serial/baudRate";
inline constexpr char kSerialDataBits[] = "serial/dataBits";
inline constexpr char kSerialParity[] = "serial/parity";
inline constexpr char kSerialStopBits[] = "serial/stopBits";
inline constexpr char kSerialPortName[] = "serial/portName";
inline constexpr char kSerialConnectionCollapsed[] = "serial/ui/connectionSettingsCollapsed";
inline constexpr char kSerialTrafficAutoScroll[] = "serial/traffic/autoScroll";
inline constexpr char kSerialTrafficShowTx[] = "serial/traffic/showTx";
inline constexpr char kSerialTrafficShowRx[] = "serial/traffic/showRx";
inline constexpr char kSerialInputFormat[] = "serial/input/format";
inline constexpr char kSerialInputIntervalMs[] = "serial/input/intervalMs";
inline constexpr char kSerialInputCollapsed[] = "serial/ui/inputCollapsed";
inline constexpr char kSerialDtr[] = "serial/dtr";
inline constexpr char kSerialRts[] = "serial/rts";

// Modbus ASCII mode — canonical keys (symmetric with RTU).
inline constexpr char kModbusAsciiBaudRate[] = "modbus/ascii/serial/baudRate";
inline constexpr char kModbusAsciiDataBits[] = "modbus/ascii/serial/dataBits";
inline constexpr char kModbusAsciiParity[] = "modbus/ascii/serial/parity";
inline constexpr char kModbusAsciiStopBits[] = "modbus/ascii/serial/stopBits";
inline constexpr char kModbusAsciiFlowControl[] = "modbus/ascii/serial/flowControl";
inline constexpr char kModbusAsciiPortName[] = "modbus/ascii/serial/portName";
inline constexpr char kModbusAsciiConnectionCollapsed[] = "modbus/ascii/serial/ui/connectionSettingsCollapsed";

// Modbus Workbench settings keys.
inline constexpr char kModbusWorkbenchMode[] = "modbus/workbench/mode";
inline constexpr char kModbusWorkbenchSlaveId[] = "modbus/workbench/slaveId";
inline constexpr char kModbusWorkbenchFunctionCode[] = "modbus/workbench/functionCode";
inline constexpr char kModbusWorkbenchStartAddress[] = "modbus/workbench/startAddress";
inline constexpr char kModbusWorkbenchQuantity[] = "modbus/workbench/quantity";
inline constexpr char kModbusWorkbenchRadix[] = "modbus/workbench/radix";
inline constexpr char kModbusWorkbenchDataRadix[] = "modbus/workbench/dataRadix";
inline constexpr char kModbusWorkbenchPollIntervalMs[] = "modbus/workbench/pollIntervalMs";
inline constexpr char kModbusWorkbenchFc05Value[] = "modbus/workbench/fc05Value";
inline constexpr char kModbusWorkbenchFc06Value[] = "modbus/workbench/fc06Value";
inline constexpr char kModbusWorkbenchFc0fValue[] = "modbus/workbench/fc0fValue";
inline constexpr char kModbusWorkbenchFc10Value[] = "modbus/workbench/fc10Value";
inline constexpr char kModbusWorkbenchRawText[] = "modbus/workbench/rawText";

inline constexpr char kModbusBuilderProtocol[] = "tools/builder/protocol";
inline constexpr char kModbusBuilderSlaveId[] = "tools/builder/slaveId";
inline constexpr char kModbusBuilderFunctionCode[] = "tools/builder/functionCode";
inline constexpr char kModbusBuilderAddressBase[] = "tools/builder/addressBase";
inline constexpr char kModbusBuilderStartAddress[] = "tools/builder/startAddress";
inline constexpr char kModbusBuilderQuantity[] = "tools/builder/quantity";
inline constexpr char kModbusBuilderWriteData[] = "tools/builder/writeData";
inline constexpr char kModbusBuilderDataFormat[] = "tools/builder/dataFormat";

}
