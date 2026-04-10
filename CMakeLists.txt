cmake_minimum_required(VERSION 3.16)

project(ModbusTools VERSION 1.0.5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MODBUS_TOOLS_ENABLE_PARALLEL_COMPILE "Enable parallel compile for MSVC" ON)
option(MODBUS_TOOLS_ENABLE_PCH "Enable precompiled headers" ON)
option(MODBUS_TOOLS_INCLUDE_PRERELEASE "Include pre-release when checking updates" OFF)

if(MSVC)
    if(MODBUS_TOOLS_ENABLE_PARALLEL_COMPILE)
        add_compile_options(/utf-8 /MP)
    else()
        add_compile_options(/utf-8)
    endif()
endif()

# Global output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/app)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/app)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Network SerialPort Widgets Charts LinguistTools)

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
add_subdirectory(updater)

if(MODBUS_TOOLS_INCLUDE_PRERELEASE)
    set(MODBUS_TOOLS_RELEASES_API_URL "https://api.github.com/repos/mingyucheng692/Modbus-Tools/releases")
    set(MODBUS_TOOLS_INCLUDE_PRERELEASE_VALUE 1)
else()
    set(MODBUS_TOOLS_RELEASES_API_URL "https://api.github.com/repos/mingyucheng692/Modbus-Tools/releases/latest")
    set(MODBUS_TOOLS_INCLUDE_PRERELEASE_VALUE 0)
endif()
set(MODBUS_TOOLS_RELEASES_PAGE_URL "https://github.com/mingyucheng692/Modbus-Tools/releases")
set(MODBUS_TOOLS_PLATFORM "windows-x86_64")

target_compile_definitions(core PUBLIC
    MODBUS_TOOLS_APP_VERSION="${PROJECT_VERSION}"
    MODBUS_TOOLS_RELEASES_API_URL="${MODBUS_TOOLS_RELEASES_API_URL}"
    MODBUS_TOOLS_RELEASES_PAGE_URL="${MODBUS_TOOLS_RELEASES_PAGE_URL}"
    MODBUS_TOOLS_PLATFORM="${MODBUS_TOOLS_PLATFORM}"
    MODBUS_TOOLS_INCLUDE_PRERELEASE=${MODBUS_TOOLS_INCLUDE_PRERELEASE_VALUE}
)

target_compile_definitions(ui PUBLIC
    MODBUS_TOOLS_APP_VERSION="${PROJECT_VERSION}"
    MODBUS_TOOLS_RELEASES_API_URL="${MODBUS_TOOLS_RELEASES_API_URL}"
    MODBUS_TOOLS_RELEASES_PAGE_URL="${MODBUS_TOOLS_RELEASES_PAGE_URL}"
    MODBUS_TOOLS_PLATFORM="${MODBUS_TOOLS_PLATFORM}"
    MODBUS_TOOLS_INCLUDE_PRERELEASE=${MODBUS_TOOLS_INCLUDE_PRERELEASE_VALUE}
)

target_compile_definitions(Modbus-Tools PRIVATE
    MODBUS_TOOLS_APP_VERSION="${PROJECT_VERSION}"
    MODBUS_TOOLS_RELEASES_API_URL="${MODBUS_TOOLS_RELEASES_API_URL}"
    MODBUS_TOOLS_RELEASES_PAGE_URL="${MODBUS_TOOLS_RELEASES_PAGE_URL}"
    MODBUS_TOOLS_PLATFORM="${MODBUS_TOOLS_PLATFORM}"
    MODBUS_TOOLS_INCLUDE_PRERELEASE=${MODBUS_TOOLS_INCLUDE_PRERELEASE_VALUE}
)

# Ensure updater is built along with the main application
add_dependencies(Modbus-Tools updater)
