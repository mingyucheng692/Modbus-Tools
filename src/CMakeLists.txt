file(GLOB_RECURSE SOURCES
    "App/*.cpp" "App/*.h"
    "Core/*.cpp" "Core/*.h"
    "HAL/*.cpp" "HAL/*.h"
    "UI/*.cpp" "UI/*.h"
)

# Translations
find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

set(TS_FILES
    "${CMAKE_SOURCE_DIR}/resources/translations/modbustools_zh_CN.ts"
    "${CMAKE_SOURCE_DIR}/resources/translations/modbustools_zh_TW.ts"
)

set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_SOURCE_DIR}/resources/translations")

# Process .ts files to .qm
qt6_add_translation(QM_FILES ${TS_FILES})

# Add Resources
qt6_add_resources(RESOURCES "${CMAKE_SOURCE_DIR}/resources/resources.qrc")

add_executable(ModbusTools ${SOURCES} ${QM_FILES} ${RESOURCES})

target_link_libraries(ModbusTools PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::SerialPort
    Qt6::Charts
    spdlog::spdlog
)
