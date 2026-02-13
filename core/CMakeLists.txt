add_library(core STATIC)

target_sources(core PRIVATE
    logging/Logger.cpp
    io/ChannelBase.cpp
    # modbus base
    modbus/base/ModbusTypes.h
    modbus/base/ModbusFrame.h
    modbus/base/ModbusConfig.h
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
