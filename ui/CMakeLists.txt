set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_library(ui STATIC)

target_sources(ui PRIVATE
    common/Theme.cpp
    MainWindow.cpp
    views/tcp/TcpView.cpp
    views/rtu/RtuView.cpp
    widgets/TcpConnectionWidget.cpp
    widgets/SerialConnectionWidget.cpp
    widgets/FunctionWidget.cpp
    widgets/ControlWidget.cpp
    widgets/TrafficMonitorWidget.cpp
    widgets/QuickCommandWidget.cpp
    widgets/QuickCommandWidget.h
    widgets/GenericInputWidget.cpp
    widgets/GenericInputWidget.h
)

target_link_libraries(ui PUBLIC
    Qt6::Widgets
    Qt6::Charts
    core
)

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
