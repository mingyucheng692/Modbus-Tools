set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_library(ui STATIC)

target_sources(ui PRIVATE
    common/CompactMenuBarStyle.cpp
    common/CompactMenuBarStyle.h
    common/ISettingsService.h
    common/QtThemeRuntime.cpp
    common/QtThemeRuntime.h
    common/SettingsKeys.h
    common/SettingsService.cpp
    common/SettingsService.h
    common/Theme.cpp
    common/ThemeController.cpp
    common/ThemeController.h
    common/ThemeRuntime.h
    common/ThemeUiHelpers.cpp
    common/ThemeUiHelpers.h
    common/ThemeVisuals.cpp
    common/ThemeVisuals.h
    common/UpdateChecker.cpp
    common/UpdateChecker.h
    assets.qrc
    MainWindow.cpp
    views/modbus_tcp/ModbusTcpView.cpp
    views/modbus_tcp/ModbusTcpView.h
    views/modbus_rtu/ModbusRtuView.cpp
    views/modbus_rtu/ModbusRtuView.h
    views/generic_tcp/GenericTcpView.cpp
    views/generic_tcp/GenericTcpView.h
    views/generic_serial/GenericSerialView.cpp
    views/generic_serial/GenericSerialView.h
    widgets/TcpConnectionWidget.cpp
    widgets/SerialConnectionWidget.cpp
    widgets/FunctionWidget.cpp
    widgets/ControlWidget.cpp
    widgets/TrafficMonitorWidget.cpp
    widgets/GenericInputWidget.cpp
    widgets/GenericInputWidget.h
    widgets/CollapsibleSection.cpp
    widgets/CollapsibleSection.h
    widgets/DisclaimerDialog.cpp
    widgets/DisclaimerDialog.h
    widgets/UpdateSettingsDialog.cpp
    widgets/UpdateSettingsDialog.h
    widgets/FrameAnalyzerWidget.cpp
    widgets/FrameAnalyzerWidget.h
)

set(ui_translation_ts
    ${CMAKE_CURRENT_SOURCE_DIR}/i18n/Modbus-Tools_zh_CN.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/i18n/Modbus-Tools_zh_TW.ts
)

qt_add_translations(ui
    TS_FILES ${ui_translation_ts}
    QM_FILES_OUTPUT_VARIABLE ui_translation_qm_files
)

qt_add_resources(ui i18n
    PREFIX "/i18n"
    BASE "${CMAKE_CURRENT_BINARY_DIR}"
    FILES ${ui_translation_qm_files}
)

target_link_libraries(ui PUBLIC
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
    core
)

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MODBUS_TOOLS_ENABLE_PCH AND COMMAND target_precompile_headers)
    target_precompile_headers(ui PRIVATE
        <QtCore/QObject>
        <QtCore/QString>
        <QtCore/QTimer>
        <QtGui/QIcon>
        <QtWidgets/QWidget>
        <QtWidgets/QMainWindow>
        <QtWidgets/QLayout>
        <QtWidgets/QDialog>
        <QtWidgets/QLabel>
        <QtWidgets/QMenuBar>
        <QtWidgets/QListWidget>
        <QtCharts/QChartView>
        <vector>
        <memory>
    )
endif()
