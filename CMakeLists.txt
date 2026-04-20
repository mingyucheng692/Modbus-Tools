cmake_minimum_required(VERSION 3.16)

project(ModbusTools VERSION 1.0.6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MODBUS_TOOLS_ENABLE_PARALLEL_COMPILE "Enable parallel compile for MSVC" ON)
option(MODBUS_TOOLS_ENABLE_PCH "Enable precompiled headers" ON)
option(MODBUS_TOOLS_INCLUDE_PRERELEASE "Include pre-release when checking updates" OFF)
option(MODBUS_TOOLS_BUILD_TESTS "Build unit tests" OFF)
option(MODBUS_TOOLS_ENABLE_ASAN "Enable AddressSanitizer (ASan) for memory safety" OFF)

if(MSVC)
    if(MODBUS_TOOLS_ENABLE_PARALLEL_COMPILE)
        add_compile_options(/utf-8 /MP)
    else()
        add_compile_options(/utf-8)
    endif()

    if(MODBUS_TOOLS_ENABLE_ASAN)
        message(STATUS "AddressSanitizer (ASan) enabled for MSVC")
        # ASan requires debug info and non-incremental linking.
        # We'll apply these globally for MSVC to ensure consistency.
        add_compile_options(/fsanitize:address /Zi)
        add_link_options(/DEBUG /INCREMENTAL:NO)
        
        # Note: If LNK4044 persists, it typically means LINK.exe is used instead of CL.exe for linking.
        # In modern CMake, /fsanitize=address is the correct flag for both.
    endif()
endif()

# Global output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/app)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/app)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Network SerialPort Widgets Charts LinguistTools Test)

# Add spdlog
# Check if spdlog exists, if not, we might need to rely on system installed or header only
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/CMakeLists.txt")
    add_subdirectory(third_party/spdlog)
else()
    message(WARNING "spdlog submodule not found at third_party/spdlog")
endif()

if(MODBUS_TOOLS_BUILD_TESTS)
    enable_testing()
    include(FetchContent)
    
    # GTest CRT compatibility for MSVC
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Check for local source fallback
    if(DEFINED ENV{GTEST_SOURCE_DIR})
        set(GTEST_SOURCE_DIR $ENV{GTEST_SOURCE_DIR})
        message(STATUS "Using local GTest source: ${GTEST_SOURCE_DIR}")
        add_subdirectory(${GTEST_SOURCE_DIR} ${CMAKE_BINARY_DIR}/googletest)
    else()
        FetchContent_Declare(
          googletest
          URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        )
        set(FETCHCONTENT_QUIET OFF)
        FetchContent_MakeAvailable(googletest)
    endif()
endif()

# Add subdirectories
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(app)
add_subdirectory(updater)

if(MODBUS_TOOLS_BUILD_TESTS)
    add_subdirectory(tests)
endif()


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
