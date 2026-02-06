cmake_minimum_required(VERSION 3.16)

project(Modbus-Tools VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(MODBUSTOOLS_ENABLE_PCH "Enable precompiled headers" ON)

if(MSVC)
    add_compile_options(/MP)
endif()

# Qt 6
find_package(Qt6 REQUIRED COMPONENTS Widgets Network SerialPort Charts)

# spdlog
if(MODBUSTOOLS_ENABLE_PCH)
    set(SPDLOG_ENABLE_PCH ON CACHE BOOL "Enable spdlog precompiled headers" FORCE)
endif()
add_subdirectory(third_party/spdlog)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Includes
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party/spdlog/include
)

add_subdirectory(src)
