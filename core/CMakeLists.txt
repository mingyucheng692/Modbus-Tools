add_library(core STATIC)

target_sources(core PRIVATE
    logging/Logger.cpp
    io/ChannelBase.h
    io/ChannelBase.cpp
    io/SerialChannel.h
    io/SerialChannel.cpp
    io/TcpChannel.h
    io/TcpChannel.cpp
    io/GenericIoWorker.h
    io/GenericIoWorker.cpp
    # modbus base
    modbus/base/ModbusTypes.h
    modbus/base/ModbusFrame.h
    modbus/base/ModbusConfig.h
    # modbus transport
    modbus/transport/ITransport.h
    modbus/transport/RtuTransport.h
    modbus/transport/RtuTransport.cpp
    modbus/transport/TcpTransport.h
    modbus/transport/TcpTransport.cpp
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

set_property(TARGET core PROPERTY AUTOMOC ON)
