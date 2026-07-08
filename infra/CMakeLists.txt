add_subdirectory(platform)

add_library(infra STATIC)
set_property(TARGET infra PROPERTY AUTOMOC ON)

target_sources(infra PRIVATE
    # io
    io/BufferedWritingChannel.h
    io/BufferedWritingChannel.cpp
    io/ChannelBase.h
    io/ChannelBase.cpp
    io/SerialChannel.h
    io/SerialChannel.cpp
    io/SerialConfig.h
    io/SerialConfig.cpp
    io/TcpChannel.h
    io/TcpChannel.cpp
    io/UdpChannel.h
    io/UdpChannel.cpp
    io/TcpServerHandle.h
    io/TcpServerHandle.cpp
    io/ChannelOperationWorker.h
    io/ChannelOperationWorker.cpp
    io/IChannel.h
    io/ServerChannelWorker.h
    io/ServerChannelWorker.cpp
    # logging
    logging/Logger.h
    logging/Logger.cpp
)

target_link_libraries(infra PUBLIC
    infra_platform
    Qt6::Core
    Qt6::Network
    Qt6::SerialPort
    spdlog::spdlog
)

target_include_directories(infra PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)