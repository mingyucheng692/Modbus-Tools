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
    common/TrafficEvent.h
    common/UpdateChecker.cpp
    common/UpdateChecker.h
    application/AppLifecycleCoordinator.cpp
    application/AppLifecycleCoordinator.h
    application/IMainWindowView.h
    application/LanguageCoordinator.cpp
    application/LanguageCoordinator.h
    application/MainWindowPresenter.cpp
    application/MainWindowPresenter.h
    application/UpdateCoordinator.cpp
    application/UpdateCoordinator.h
    assets.qrc
    AnalyzerLinkageController.cpp
    AnalyzerLinkageController.h
    MainWindow.cpp
    shell/MainWindowPageBuilder.cpp
    shell/MainWindowPageBuilder.h
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
    widgets/ModbusSettingsDialog.cpp
    widgets/ModbusSettingsDialog.h
    widgets/AboutDialog.cpp
    widgets/AboutDialog.h
    widgets/FrameAnalyzerWidget.cpp
    widgets/FrameAnalyzerWidget.h
)

set(ui_translation_ts
    ${CMAKE_CURRENT_SOURCE_DIR}/i18n/Modbus-Tools_zh_CN.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/i18n/Modbus-Tools_zh_TW.ts
)

set(ui_translation_output_dir "${CMAKE_BINARY_DIR}/translations")

qt_add_lrelease(
    TS_FILES ${ui_translation_ts}
    LRELEASE_TARGET modbus_i18n
    EXCLUDE_FROM_ALL
    NO_GLOBAL_TARGET
    QM_OUTPUT_DIRECTORY "${ui_translation_output_dir}"
    QM_FILES_OUTPUT_VARIABLE ui_translation_qm_files
)

set(ui_i18n_resource_file "${CMAKE_CURRENT_BINARY_DIR}/generated/i18n.qrc")
set(ui_embedded_i18n_qm_list_file "${CMAKE_CURRENT_BINARY_DIR}/generated/i18n_qm_files.cmake")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(ui_embedded_i18n_qm_list_content "set(QM_FILES\n")
foreach(ui_translation_qm_file IN LISTS ui_translation_qm_files)
    file(TO_CMAKE_PATH "${ui_translation_qm_file}" ui_translation_qm_path)
    string(APPEND ui_embedded_i18n_qm_list_content "    \"${ui_translation_qm_path}\"\n")
endforeach()
string(APPEND ui_embedded_i18n_qm_list_content ")\n")
file(GENERATE OUTPUT "${ui_embedded_i18n_qm_list_file}" CONTENT "${ui_embedded_i18n_qm_list_content}")

add_custom_command(
    OUTPUT "${ui_i18n_resource_file}"
    COMMAND ${CMAKE_COMMAND}
            -DOUTPUT_FILE=${ui_i18n_resource_file}
            -DQM_FILES_CMAKE=${ui_embedded_i18n_qm_list_file}
            -P "${CMAKE_SOURCE_DIR}/cmake/GenerateEmbeddedI18nQrc.cmake"
    DEPENDS modbus_i18n ${ui_translation_qm_files} "${ui_embedded_i18n_qm_list_file}"
    COMMENT "Generating embedded i18n resource manifest"
    VERBATIM
)

add_dependencies(ui modbus_i18n)

target_sources(ui PRIVATE
    "${ui_i18n_resource_file}"
)

add_custom_target(release_bundle
    DEPENDS Modbus-Tools updater
    COMMENT "Building release bundle artifacts"
)

target_link_libraries(ui PUBLIC
    Qt6::Widgets
    Qt6::Charts
    Qt6::Concurrent
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
