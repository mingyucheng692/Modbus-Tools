/**
 * @file SettingsKeys.h
 * @brief Centralized settings key constants shared by core and ui layers.
 *
 * Lives in core/common so that core-layer controllers (e.g. SettingsController)
 * can use these constants without creating a reverse dependency from core to
 * ui. Pure constexpr char[] — no Qt includes, no link-time dependencies.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

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
inline constexpr char kModbusTcpStandardSlaveId[] = "modbus/tcp/standard/slaveId";
inline constexpr char kModbusTcpStandardStartAddr[] = "modbus/tcp/standard/startAddr";
inline constexpr char kModbusTcpStandardQuantity[] = "modbus/tcp/standard/quantity";
inline constexpr char kModbusTcpStandardFormatIndex[] = "modbus/tcp/standard/formatIndex";
inline constexpr char kModbusTcpStandardCollapsed[] = "modbus/tcp/standard/ui/standardCollapsed";
inline constexpr char kModbusTcpRawCollapsed[] = "modbus/tcp/standard/ui/rawCollapsed";
inline constexpr char kModbusTcpTrafficAutoScroll[] = "modbus/tcp/traffic/autoScroll";
inline constexpr char kModbusTcpTrafficShowTx[] = "modbus/tcp/traffic/showTx";
inline constexpr char kModbusTcpTrafficShowRx[] = "modbus/tcp/traffic/showRx";
inline constexpr char kModbusTcpTrafficCollapsed[] = "modbus/tcp/traffic/ui/trafficMonitorCollapsed";
inline constexpr char kModbusTcpControlEnablePoll[] = "modbus/tcp/control/enablePoll";
inline constexpr char kModbusTcpControlIntervalMs[] = "modbus/tcp/control/intervalMs";
inline constexpr char kModbusTcpControlFcIndex[] = "modbus/tcp/control/fcIndex";
inline constexpr char kModbusTcpControlAddr[] = "modbus/tcp/control/addr";
inline constexpr char kModbusTcpControlQty[] = "modbus/tcp/control/qty";
inline constexpr char kModbusTcpDataMonitorCollapsed[] = "modbus/tcp/ui/dataMonitorCollapsed";

inline constexpr char kModbusRtuBaudRate[] = "modbus/rtu/serial/baudRate";
inline constexpr char kModbusRtuDataBits[] = "modbus/rtu/serial/dataBits";
inline constexpr char kModbusRtuParity[] = "modbus/rtu/serial/parity";
inline constexpr char kModbusRtuStopBits[] = "modbus/rtu/serial/stopBits";
inline constexpr char kModbusRtuPortName[] = "modbus/rtu/serial/portName";
inline constexpr char kModbusRtuConnectionCollapsed[] = "modbus/rtu/serial/ui/connectionSettingsCollapsed";
inline constexpr char kModbusRtuStandardSlaveId[] = "modbus/rtu/standard/slaveId";
inline constexpr char kModbusRtuStandardStartAddr[] = "modbus/rtu/standard/startAddr";
inline constexpr char kModbusRtuStandardQuantity[] = "modbus/rtu/standard/quantity";
inline constexpr char kModbusRtuStandardFormatIndex[] = "modbus/rtu/standard/formatIndex";
inline constexpr char kModbusRtuStandardCollapsed[] = "modbus/rtu/standard/ui/standardCollapsed";
inline constexpr char kModbusRtuRawCollapsed[] = "modbus/rtu/standard/ui/rawCollapsed";
inline constexpr char kModbusRtuTrafficAutoScroll[] = "modbus/rtu/traffic/autoScroll";
inline constexpr char kModbusRtuTrafficShowTx[] = "modbus/rtu/traffic/showTx";
inline constexpr char kModbusRtuTrafficShowRx[] = "modbus/rtu/traffic/showRx";
inline constexpr char kModbusRtuTrafficCollapsed[] = "modbus/rtu/traffic/ui/trafficMonitorCollapsed";
inline constexpr char kModbusRtuControlEnablePoll[] = "modbus/rtu/control/enablePoll";
inline constexpr char kModbusRtuControlIntervalMs[] = "modbus/rtu/control/intervalMs";
inline constexpr char kModbusRtuControlFcIndex[] = "modbus/rtu/control/fcIndex";
inline constexpr char kModbusRtuControlAddr[] = "modbus/rtu/control/addr";
inline constexpr char kModbusRtuControlQty[] = "modbus/rtu/control/qty";
inline constexpr char kModbusRtuDataMonitorCollapsed[] = "modbus/rtu/ui/dataMonitorCollapsed";

// Network Debugger (TCP client mode) — canonical keys (Phase 9).
// NOTE: the tcp_server and udp traffic/input/ui/autoReconnect/reconnectDelay
// keys are composed dynamically by BaseConnectionWidget / ByteMonitorWidget /
// GenericInputWidget from the settings group, so they intentionally have no
// static constants here. The prefix-based migration in SettingsService covers
// them as whole families (network_debugger/server/, network_debugger/udp/).
inline constexpr char kNetworkDebuggerClientIp[] = "network_debugger/client/ip";
inline constexpr char kNetworkDebuggerClientPort[] = "network_debugger/client/port";
inline constexpr char kNetworkDebuggerClientConnectionCollapsed[] = "network_debugger/client/ui/connectionSettingsCollapsed";
inline constexpr char kNetworkDebuggerClientTrafficAutoScroll[] = "network_debugger/client/traffic/autoScroll";
inline constexpr char kNetworkDebuggerClientTrafficShowTx[] = "network_debugger/client/traffic/showTx";
inline constexpr char kNetworkDebuggerClientTrafficShowRx[] = "network_debugger/client/traffic/showRx";
inline constexpr char kNetworkDebuggerClientTrafficCollapsed[] = "network_debugger/client/traffic/ui/trafficMonitorCollapsed";
inline constexpr char kNetworkDebuggerClientInputFormat[] = "network_debugger/client/input/format";
inline constexpr char kNetworkDebuggerClientInputAutoSend[] = "network_debugger/client/input/autoSend";
inline constexpr char kNetworkDebuggerClientInputIntervalMs[] = "network_debugger/client/input/intervalMs";
inline constexpr char kNetworkDebuggerClientInputCollapsed[] = "network_debugger/client/ui/inputCollapsed";

// Serial Debugger — canonical keys (Phase 9).
inline constexpr char kSerialDebuggerBaudRate[] = "serial_debugger/baudRate";
inline constexpr char kSerialDebuggerDataBits[] = "serial_debugger/dataBits";
inline constexpr char kSerialDebuggerParity[] = "serial_debugger/parity";
inline constexpr char kSerialDebuggerStopBits[] = "serial_debugger/stopBits";
inline constexpr char kSerialDebuggerPortName[] = "serial_debugger/portName";
inline constexpr char kSerialDebuggerConnectionCollapsed[] = "serial_debugger/ui/connectionSettingsCollapsed";
inline constexpr char kSerialDebuggerTrafficAutoScroll[] = "serial_debugger/traffic/autoScroll";
inline constexpr char kSerialDebuggerTrafficShowTx[] = "serial_debugger/traffic/showTx";
inline constexpr char kSerialDebuggerTrafficShowRx[] = "serial_debugger/traffic/showRx";
inline constexpr char kSerialDebuggerTrafficCollapsed[] = "serial_debugger/traffic/ui/trafficMonitorCollapsed";
inline constexpr char kSerialDebuggerInputFormat[] = "serial_debugger/input/format";
inline constexpr char kSerialDebuggerInputAutoSend[] = "serial_debugger/input/autoSend";
inline constexpr char kSerialDebuggerInputIntervalMs[] = "serial_debugger/input/intervalMs";
inline constexpr char kSerialDebuggerInputCollapsed[] = "serial_debugger/ui/inputCollapsed";
inline constexpr char kSerialDebuggerDtr[] = "serial_debugger/dtr";
inline constexpr char kSerialDebuggerRts[] = "serial_debugger/rts";

inline constexpr char kModbusRtuFlowControl[] = "modbus/rtu/serial/flowControl";

// Network Debugger (UDP mode) — canonical keys (Phase 9).
inline constexpr char kNetworkDebuggerUdpRemoteIp[] = "network_debugger/udp/remoteIp";
inline constexpr char kNetworkDebuggerUdpRemotePort[] = "network_debugger/udp/remotePort";

}
