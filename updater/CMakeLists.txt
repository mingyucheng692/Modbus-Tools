# Updater is Windows-only for now.
# Linux (linux/) and macOS (macos/) implementations are placeholders for future OTA support.
# Each platform's CI builds its own updater independently.
if(NOT WIN32)
    message(STATUS "updater: skipped on ${CMAKE_SYSTEM_NAME} (Windows-only, Linux/macOS pending)")
    return()
endif()

add_executable(updater WIN32
    windows/main.cpp
    windows/Win32UpdateStrategy.cpp
    UpdateTask.cpp
)

set_target_properties(updater PROPERTIES OUTPUT_NAME "updater")

target_include_directories(updater PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(updater PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(updater PRIVATE
    bcrypt
)

if(MSVC)
    target_link_options(updater PRIVATE
        "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'"
    )
endif()

install(TARGETS updater
    RUNTIME DESTINATION .
    COMPONENT Production
)
