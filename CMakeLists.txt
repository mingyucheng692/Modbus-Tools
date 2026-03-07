cmake_minimum_required(VERSION 3.16)

project(ModbusTools VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MODBUS_TOOLS_ENABLE_PARALLEL_COMPILE "Enable parallel compile for MSVC" ON)
option(MODBUS_TOOLS_ENABLE_PCH "Enable precompiled headers" ON)

if(MSVC)
    if(MODBUS_TOOLS_ENABLE_PARALLEL_COMPILE)
        add_compile_options(/utf-8 /MP)
    else()
        add_compile_options(/utf-8)
    endif()
endif()

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Network SerialPort Widgets Charts)

# Add spdlog
# Check if spdlog exists, if not, we might need to rely on system installed or header only
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/CMakeLists.txt")
    add_subdirectory(third_party/spdlog)
else()
    message(WARNING "spdlog submodule not found at third_party/spdlog")
endif()

# Add subdirectories
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(app)
