file(GLOB_RECURSE SOURCES
    "App/*.cpp" "App/*.h"
    "Core/*.cpp" "Core/*.h"
    "HAL/*.cpp" "HAL/*.h"
    "UI/*.cpp" "UI/*.h"
)

add_executable(ModbusTools ${SOURCES})

target_link_libraries(ModbusTools PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::SerialPort
    Qt6::Charts
    spdlog::spdlog
)
