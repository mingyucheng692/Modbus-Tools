#pragma once

namespace ui::common::settings_keys {

inline constexpr char kAppLanguage[] = "app/language";
inline constexpr char kAppNavigationCollapsed[] = "app/navigationCollapsed";
inline constexpr char kAppUpdateCheckFrequency[] = "app/updateCheckFrequency";
inline constexpr char kAppUpdateLastCheckUtc[] = "app/updateLastCheckUtc";
inline constexpr char kAppDisclaimerAccepted[] = "app/disclaimerAccepted";
inline constexpr char kAppMainWindowGeometry[] = "app/mainWindowGeometry";
inline constexpr char kAppMainWindowState[] = "app/mainWindowState";

inline constexpr char kModbusTimeoutMs[] = "modbus/timeoutMs";
inline constexpr char kModbusRetryCount[] = "modbus/retryCount";
inline constexpr char kModbusRetryIntervalMs[] = "modbus/retryIntervalMs";
inline constexpr char kModbusRetryEnabled[] = "modbus/retryEnabled";

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

inline constexpr char kTcpClientIp[] = "tcp_client/ip";
inline constexpr char kTcpClientPort[] = "tcp_client/port";
inline constexpr char kTcpClientConnectionCollapsed[] = "tcp_client/ui/connectionSettingsCollapsed";
inline constexpr char kTcpClientTrafficAutoScroll[] = "tcp_client/traffic/autoScroll";
inline constexpr char kTcpClientTrafficShowTx[] = "tcp_client/traffic/showTx";
inline constexpr char kTcpClientTrafficShowRx[] = "tcp_client/traffic/showRx";
inline constexpr char kTcpClientTrafficCollapsed[] = "tcp_client/traffic/ui/trafficMonitorCollapsed";
inline constexpr char kTcpClientInputFormat[] = "tcp_client/input/format";
inline constexpr char kTcpClientInputAutoSend[] = "tcp_client/input/autoSend";
inline constexpr char kTcpClientInputIntervalMs[] = "tcp_client/input/intervalMs";
inline constexpr char kTcpClientInputCollapsed[] = "tcp_client/ui/inputCollapsed";

inline constexpr char kSerialPortBaudRate[] = "serial_port/baudRate";
inline constexpr char kSerialPortDataBits[] = "serial_port/dataBits";
inline constexpr char kSerialPortParity[] = "serial_port/parity";
inline constexpr char kSerialPortStopBits[] = "serial_port/stopBits";
inline constexpr char kSerialPortPortName[] = "serial_port/portName";
inline constexpr char kSerialPortConnectionCollapsed[] = "serial_port/ui/connectionSettingsCollapsed";
inline constexpr char kSerialPortTrafficAutoScroll[] = "serial_port/traffic/autoScroll";
inline constexpr char kSerialPortTrafficShowTx[] = "serial_port/traffic/showTx";
inline constexpr char kSerialPortTrafficShowRx[] = "serial_port/traffic/showRx";
inline constexpr char kSerialPortTrafficCollapsed[] = "serial_port/traffic/ui/trafficMonitorCollapsed";
inline constexpr char kSerialPortInputFormat[] = "serial_port/input/format";
inline constexpr char kSerialPortInputAutoSend[] = "serial_port/input/autoSend";
inline constexpr char kSerialPortInputIntervalMs[] = "serial_port/input/intervalMs";
inline constexpr char kSerialPortInputCollapsed[] = "serial_port/ui/inputCollapsed";
inline constexpr char kSerialPortDtr[] = "serial_port/dtr";
inline constexpr char kSerialPortRts[] = "serial_port/rts";

inline constexpr char kLegacySerialBaudRate[] = "serial/baudRate";

}
