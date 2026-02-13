add_library(core STATIC)

target_sources(core PRIVATE
    logging/Logger.cpp
    io/ChannelBase.cpp
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
    modbus/dispatch/ModbusCommand.h
    modbus/dispatch/CommandQueue.h
    modbus/dispatch/ModbusWorker.h
    modbus/dispatch/ModbusWorker.cpp
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
