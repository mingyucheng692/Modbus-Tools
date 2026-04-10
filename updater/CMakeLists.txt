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
