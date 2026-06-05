set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_library(ui STATIC)

target_sources(ui PRIVATE
    common/AppConfig.cpp
    common/AppConfig.h
    common/CompactMenuBarStyle.cpp
    common/CompactMenuBarStyle.h
    common/IAppConfig.h
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
    application/modbus/RequestSubmissionService.cpp
    application/modbus/RequestSubmissionService.h
    application/modbus/PollingController.cpp
    application/modbus/PollingController.h
    application/modbus/TrafficLogController.cpp
    application/modbus/TrafficLogController.h
    application/modbus/ModbusSessionPresenter.cpp
    application/modbus/ModbusSessionPresenter.h
    application/modbus/ModbusTypes.h
    logging/LogBridge.cpp
    logging/LogBridge.h
    assets.qrc
    AnalyzerLinkageController.cpp
    AnalyzerLinkageController.h
    MainWindow.cpp
    application/AnalyzerLinkCoordinator.cpp
    application/AnalyzerLinkCoordinator.h
    application/IMainWindowPresenter.h
    shell/NavigationController.cpp
    shell/NavigationController.h
    shell/MainWindowPageBuilder.cpp
    shell/MainWindowPageBuilder.h
    views/modbus_tcp/ModbusTcpPage.cpp
    views/modbus_tcp/ModbusTcpPage.h
    views/modbus_rtu/ModbusRtuPage.cpp
    views/modbus_rtu/ModbusRtuPage.h
    views/BaseModbusPage.cpp
    views/BaseModbusPage.h
    views/GenericChannelViewBase.cpp
    views/GenericChannelViewBase.h
    views/generic_tcp/GenericTcpView.cpp
    views/generic_tcp/GenericTcpView.h
    views/generic_serial/GenericSerialView.cpp
    views/generic_serial/GenericSerialView.h
    widgets/BaseConnectionWidget.cpp
    widgets/BaseConnectionWidget.h
    widgets/TcpConnectionWidget.cpp
    widgets/SerialConnectionWidget.cpp
    widgets/FunctionWidget.cpp
    widgets/ControlWidget.cpp
    widgets/TrafficMonitorWidget.cpp
    widgets/LogListModel.cpp
    widgets/LogListModel.h
    widgets/LogRecorder.cpp
    widgets/LogRecorder.h
    widgets/ByteMonitorWidget.cpp
    widgets/ByteMonitorWidget.h
    widgets/ServerClientPanel.cpp
    widgets/ServerClientPanel.h
    widgets/TrafficStats.h
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

set(ui_translation_qm_files "")
foreach(ts_file IN LISTS ui_translation_ts)
    get_filename_component(ts_basename "${ts_file}" NAME_WE)
    set(qm_file "${ui_translation_output_dir}/${ts_basename}.qm")
    add_custom_command(
        OUTPUT "${qm_file}"
        COMMAND Qt6::lrelease
                -silent
                "${ts_file}"
                -qm "${qm_file}"
        DEPENDS "${ts_file}"
        COMMENT "Generating translation: ${ts_basename}.qm"
        VERBATIM
    )
    list(APPEND ui_translation_qm_files "${qm_file}")
endforeach()

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
    DEPENDS ${ui_translation_qm_files} "${ui_embedded_i18n_qm_list_file}"
    COMMENT "Generating embedded i18n resource manifest"
    VERBATIM
)

target_sources(ui PRIVATE
    "${ui_i18n_resource_file}"
)

set(modbus_tools_release_bundle_dependencies Modbus-Tools)
if(MODBUS_TOOLS_UPDATER_ENABLED)
    list(APPEND modbus_tools_release_bundle_dependencies updater)
endif()

add_custom_target(release_bundle
    DEPENDS ${modbus_tools_release_bundle_dependencies}
    COMMENT "Building release bundle artifacts"
)

target_link_libraries(ui PUBLIC
    Qt6::Widgets
    Qt6::Charts
    Qt6::Concurrent
    Qt6::Network
    Qt6::StateMachine
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
