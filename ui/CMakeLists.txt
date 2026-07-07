set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

function(modbus_target_embed_translations target_name)
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "Target '${target_name}' must exist before embedding translations")
    endif()

    cmake_parse_arguments(I18N "" "OUTPUT_DIR" "TS_FILES" ${ARGN})
    if(NOT I18N_TS_FILES)
        return()
    endif()

    if(NOT I18N_OUTPUT_DIR)
        set(I18N_OUTPUT_DIR "${CMAKE_BINARY_DIR}/translations")
    endif()

    set(qm_files "")
    foreach(ts_file IN LISTS I18N_TS_FILES)
        get_filename_component(ts_basename "${ts_file}" NAME_WE)
        set(qm_file "${I18N_OUTPUT_DIR}/${ts_basename}.qm")
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
        list(APPEND qm_files "${qm_file}")
    endforeach()

    set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
    set(resource_file "${generated_dir}/${target_name}_i18n.qrc")
    set(qm_list_file "${generated_dir}/${target_name}_i18n_qm_files.cmake")
    file(MAKE_DIRECTORY "${generated_dir}")

    set(qm_list_content "set(QM_FILES\n")
    foreach(qm_file IN LISTS qm_files)
        file(TO_CMAKE_PATH "${qm_file}" qm_file_path)
        string(APPEND qm_list_content "    \"${qm_file_path}\"\n")
    endforeach()
    string(APPEND qm_list_content ")\n")
    file(GENERATE OUTPUT "${qm_list_file}" CONTENT "${qm_list_content}")

    add_custom_command(
        OUTPUT "${resource_file}"
        COMMAND ${CMAKE_COMMAND}
                -DOUTPUT_FILE=${resource_file}
                -DQM_FILES_CMAKE=${qm_list_file}
                -P "${CMAKE_SOURCE_DIR}/cmake/GenerateEmbeddedI18nQrc.cmake"
        DEPENDS ${qm_files} "${qm_list_file}"
        COMMENT "Generating embedded i18n resource manifest for ${target_name}"
        VERBATIM
    )

    target_sources(${target_name} PRIVATE "${resource_file}")
endfunction()

add_library(ui STATIC)

target_sources(ui PRIVATE
    common/AppConfig.cpp
    common/AppConfig.h
    common/CompactMenuBarStyle.cpp
    common/CompactMenuBarStyle.h
    common/IAppConfig.h
    common/SettingsKeys.h
    common/SettingsService.cpp
    common/SettingsService.h
    common/Theme.cpp
    common/ThemeController.cpp
    common/ThemeController.h
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
    application/modbus/RequestCoordinator.cpp
    application/modbus/RequestCoordinator.h
    application/modbus/RequestSubmissionService.cpp
    application/modbus/RequestSubmissionService.h
    application/modbus/PollingController.cpp
    application/modbus/PollingController.h
    application/modbus/TrafficLogController.cpp
    application/modbus/TrafficLogController.h
    application/modbus/ModbusSessionPresenter.cpp
    application/modbus/ModbusSessionPresenter.h
    application/modbus/ModbusPagePresenter.cpp
    application/modbus/ModbusPagePresenter.h
    application/modbus/ModbusTypes.h
    application/modbus/WorkerReleaseCoordinator.cpp
    application/modbus/WorkerReleaseCoordinator.h
    application/modbus/SessionConnectionStateMachine.cpp
    application/modbus/SessionConnectionStateMachine.h
    logging/LogBridge.cpp
    logging/LogBridge.h
    assets.qrc
    MainWindow.cpp
    UpdateInteractionView.h
    UpdateInteractionView.cpp
    application/AnalyzerLinkCoordinator.cpp
    application/AnalyzerLinkCoordinator.h
    application/IMainWindowPresenter.h
    shell/NavigationController.cpp
    shell/NavigationController.h
    shell/MainWindowPageBuilder.cpp
    shell/MainWindowPageBuilder.h
    views/modbus/ModbusPage.cpp
    views/modbus/ModbusPage.h
    views/GenericChannelViewBase.cpp
    views/GenericChannelViewBase.h
    views/generic_tcp/GenericTcpView.cpp
    views/generic_tcp/GenericTcpView.h
    views/generic_serial/GenericSerialView.cpp
    views/generic_serial/GenericSerialView.h
    widgets/BaseConnectionWidget.cpp
    widgets/BaseConnectionWidget.h
    widgets/NetworkConnectionWidget.cpp
    widgets/TcpClientConnectionWidget.cpp
    widgets/TcpServerConnectionWidget.cpp
    widgets/UdpConnectionWidget.cpp
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

modbus_target_embed_translations(ui
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/translations"
    TS_FILES
        ${ui_translation_ts}
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
