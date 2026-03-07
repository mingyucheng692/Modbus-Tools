add_executable(Modbus-Tools WIN32
    app.rc
    main.cpp
)

set_target_properties(Modbus-Tools PROPERTIES OUTPUT_NAME "Modbus-Tools")

target_link_libraries(Modbus-Tools PRIVATE
    core
    ui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(MODBUS_TOOLS_ENABLE_PCH AND COMMAND target_precompile_headers)
    target_precompile_headers(Modbus-Tools PRIVATE
        <QtCore/QCoreApplication>
        <QtCore/QResource>
        <QtGui/QIcon>
        <QtWidgets/QApplication>
        <spdlog/spdlog.h>
        <memory>
    )
endif()
