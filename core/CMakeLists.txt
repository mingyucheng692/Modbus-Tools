add_library(core STATIC)

target_sources(core PRIVATE
    logging/Logger.cpp
    io/ChannelBase.h
    io/ChannelBase.cpp
    io/SerialChannel.h
    io/SerialChannel.cpp
    io/TcpChannel.h
    io/TcpChannel.cpp
    io/ChannelOperationWorker.h
    io/ChannelOperationWorker.cpp
    # modbus base
    modbus/base/ModbusTypes.h
    modbus/base/ModbusFrame.h
    modbus/base/ModbusConfig.h
    modbus/base/ModbusProtocolChecks.h
    modbus/base/ModbusProtocolChecks.cpp
    modbus/base/ModbusDataHelper.h
    modbus/base/ModbusDataHelper.cpp
    modbus/base/ModbusPduBuilder.h
    modbus/base/ModbusPduBuilder.cpp
    # modbus transport
    modbus/transport/ITransport.h
    modbus/transport/ModbusRtuTransport.h
    modbus/transport/ModbusRtuTransport.cpp
    modbus/transport/ModbusTcpTransport.h
    modbus/transport/ModbusTcpTransport.cpp
    # modbus session
    modbus/session/IModbusClient.h
    modbus/session/ModbusClient.h
    modbus/session/ModbusClient.cpp
    # modbus dispatch
    modbus/dispatch/ModbusWorker.h
    modbus/dispatch/ModbusWorker.cpp
    # modbus factory
    modbus/factory/IModbusFactory.h
    modbus/factory/ModbusFactory.h
    modbus/factory/ModbusFactory.cpp
    # modbus parser
    modbus/parser/ModbusFrameParser.h
    modbus/parser/ModbusFrameParser.cpp
    update/UpdateManager.h
    update/UpdateManager.cpp
    common/SettingsController.h
    common/SettingsController.cpp
)

target_link_libraries(core PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::SerialPort
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
        <QtSerialPort/QSerialPort>
        <spdlog/spdlog.h>
        <vector>
        <string>
        <memory>
    )
endif()

set_property(TARGET core PROPERTY AUTOMOC ON)
