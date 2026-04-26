add_executable(updater WIN32
    main.cpp
)

set_target_properties(updater PROPERTIES OUTPUT_NAME "updater")

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

if(WIN32)
    add_custom_command(TARGET updater PRE_LINK
        COMMAND ${CMAKE_COMMAND}
                -DTARGET_PATH="$<TARGET_FILE:updater>"
                -DTARGET_NAME=updater
                -P "${CMAKE_SOURCE_DIR}/cmake/CheckRuntimeOutputLock.cmake"
        VERBATIM
    )
endif()

install(TARGETS updater 
    RUNTIME DESTINATION . 
    COMPONENT Production
)
