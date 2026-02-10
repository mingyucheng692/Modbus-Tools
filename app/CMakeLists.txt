add_executable(ModbusTools
    main.cpp
)

target_link_libraries(ModbusTools PRIVATE
    Qt6::Widgets
    core
    ui
)
