add_library(ui STATIC)

target_sources(ui PRIVATE
    common/Theme.cpp
)

target_link_libraries(ui PUBLIC
    Qt6::Widgets
    Qt6::Charts
    core
)

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
