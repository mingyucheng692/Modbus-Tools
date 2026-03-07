add_executable(ModbusTools WIN32
    app.rc
    main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    core
    ui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)
