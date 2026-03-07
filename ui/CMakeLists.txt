set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_library(ui STATIC)

target_sources(ui PRIVATE
    common/Theme.cpp
    common/UpdateChecker.cpp
    common/UpdateChecker.h
    assets.qrc
    i18n/i18n.qrc
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
    widgets/DisclaimerDialog.cpp
    widgets/DisclaimerDialog.h
    widgets/FrameAnalyzerWidget.cpp
    widgets/FrameAnalyzerWidget.h
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
