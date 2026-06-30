add_library(core STATIC)

target_sources(core PRIVATE
    Config.h
    # reconnect
    ReconnectPolicy.h
    ReconnectPolicy.cpp
    # modbus base
    modbus/base/ModbusTypes.h
    modbus/base/ModbusFrame.h
    modbus/base/ModbusConfig.h
    modbus/base/ModbusEndianCodec.h
    modbus/base/ModbusProtocolChecks.h
    modbus/base/ModbusProtocolChecks.cpp
    modbus/base/ModbusLrc.h
    modbus/base/ModbusLrc.cpp
    modbus/base/ModbusDataHelper.h
    modbus/base/ModbusDataHelper.cpp
    modbus/base/ModbusPduBuilder.h
    modbus/base/ModbusPduBuilder.cpp
    # modbus transport
    modbus/transport/ITransport.h
    modbus/transport/ModbusRtuTransport.h
    modbus/transport/ModbusRtuTransport.cpp
    modbus/transport/ModbusAsciiTransport.h
    modbus/transport/ModbusAsciiTransport.cpp
    modbus/transport/ModbusTcpTransport.h
    modbus/transport/ModbusTcpTransport.cpp
    # modbus session
    modbus/session/IModbusClient.h
    modbus/session/ModbusClient.h
    modbus/session/ModbusClient.cpp
    modbus/session/ExceptionInterpreter.h
    modbus/session/ExceptionInterpreter.cpp
    modbus/session/RequestValidator.h
    modbus/session/RequestValidator.cpp
    modbus/session/FrameExtractor.h
    modbus/session/FrameExtractor.cpp
    modbus/session/BackoffCalculator.h
    modbus/session/BackoffCalculator.cpp
    modbus/session/RetryStrategy.h
    modbus/session/RetryStrategy.cpp
    modbus/session/ConnectionStateMachine.h
    modbus/session/ConnectionStateMachine.cpp
    modbus/session/RequestStateMachine.h
    modbus/session/RequestStateMachine.cpp
    modbus/session/FlowController.h
    modbus/session/FlowController.cpp
    modbus/session/TimeoutController.h
    modbus/session/TimeoutController.cpp
    modbus/session/ConnectionManager.h
    modbus/session/ConnectionManager.cpp
    modbus/session/RequestExecutor.h
    modbus/session/RequestExecutor.cpp
    # modbus dispatch
    modbus/dispatch/ModbusWorker.h
    modbus/dispatch/ModbusWorker.cpp
    # modbus factory
    modbus/factory/IModbusFactory.h
    modbus/factory/ModbusFactory.h
    modbus/factory/ModbusFactory.cpp
    # modbus request
    modbus/request/ModbusRequestFactory.h
    modbus/request/ModbusRequestFactory.cpp
    # modbus parser
    modbus/parser/ModbusFrameParser.h
    modbus/parser/ModbusFrameParser.cpp
    modbus/parser/FrameParseWorker.h
    modbus/parser/FrameParseWorker.cpp
    # analyzer common & exporter
    analyzer/AnalyzerCommon.h
    analyzer/AnalyzerExporter.h
    analyzer/AnalyzerExporter.cpp
    analyzer/ValueFormatter.cpp
    update/PlatformReleaseAssetStrategy.h
    update/PlatformReleaseAssetStrategy.cpp
    update/PlatformUpdateInstallStrategy.h
    update/PlatformUpdateInstallStrategy.cpp
    update/UpdateManager.h
    update/UpdateManager.cpp
    update/ReleaseParser.h
    update/ReleaseParser.cpp
    polling/PollingStrategy.h
    polling/PollingStrategy.cpp
    common/SettingsController.h
    common/SettingsController.cpp
)

target_link_libraries(core PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::SerialPort
    infra
    spdlog::spdlog
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MODBUS_TOOLS_ENABLE_PCH AND COMMAND target_precompile_headers)
    target_precompile_headers(core PRIVATE
        <QtCore/QObject>
        <QtCore/QByteArray>
        <QtCore/QString>
        <QtCore/QDateTime>
        <QtCore/QVector>
        <QtCore/QMutex>
        <QtCore/QSharedPointer>
        <QtNetwork/QTcpSocket>
        <QtNetwork/QHostAddress>
        <QtNetwork/QUdpSocket>
        <QtNetwork/QTcpServer>
        <QtSerialPort/QSerialPort>
        <spdlog/spdlog.h>
        <vector>
        <string>
        <memory>
    )
endif()

set_property(TARGET core PROPERTY AUTOMOC ON)
