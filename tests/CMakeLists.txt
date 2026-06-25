cmake_minimum_required(VERSION 3.16)

# ---------------------------------------------------------------------------
# Coverage instrumentation — controlled by ENABLE_COVERAGE option
# ---------------------------------------------------------------------------
option(ENABLE_COVERAGE "Enable code-coverage instrumentation for tests" OFF)

function(modbus_add_coverage target_name)
    if(NOT ENABLE_COVERAGE)
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE -O0 -g --coverage)
        target_link_options(${target_name} PRIVATE --coverage)
    elseif(MSVC)
        target_compile_options(${target_name} PRIVATE /Od)
        # /PROFILE enables POGO instrumentation; used here as the MSVC
        # nearest equivalent for line-based coverage tooling (OpenCppCoverage).
        target_link_options(${target_name} PRIVATE /PROFILE)
    endif()
endfunction()

include(GoogleTest)

function(modbus_add_test_target target_name main_source)
    cmake_parse_arguments(TEST "SKIP_DISCOVERY" "" "SOURCES;LIBS" ${ARGN})
    if(NOT TEST_SOURCES)
        return()
    endif()

    add_executable(${target_name}
        ${main_source}
        ${TEST_SOURCES}
    )

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /utf-8)
    endif()

    set_property(TARGET ${target_name} PROPERTY AUTOMOC ON)

    target_include_directories(${target_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/infra
    )

    target_link_libraries(${target_name} PRIVATE
        GTest::gtest
        GTest::gmock
        Qt6::Test
        ${TEST_LIBS}
    )

    set_target_properties(${target_name} PROPERTIES
        TIMEOUT 300
    )

    modbus_add_coverage(${target_name})

    if(NOT TEST_SKIP_DISCOVERY)
        gtest_discover_tests(${target_name}
            DISCOVERY_TIMEOUT 120
        )
    endif()

    install(TARGETS ${target_name}
        RUNTIME DESTINATION tests
        COMPONENT Test
    )
endfunction()

set(TEST_CORE_LOGIC_SOURCES
    unit/NoexceptContractTest.cpp
    unit/PathResolverTest.cpp
    unit/analyzer/ValueFormatterSafetyTest.cpp
    unit/analyzer/ValueFormatterTest.cpp
    unit/io/ChannelOperationWorkerTest.cpp
    unit/io/SerialChannelTest.cpp
    unit/io/TcpChannelTest.cpp
    unit/logging/LoggerInitTest.cpp
    unit/modbus/base/DataHelperTest.cpp
    unit/modbus/base/ModbusCrcTest.cpp
    unit/modbus/base/ModbusEndianCodecTest.cpp
    unit/modbus/base/ModbusPduTest.cpp
    unit/modbus/factory/ModbusFactoryThreadingTest.cpp
    unit/modbus/parser/ParserRobustnessTest.cpp
    unit/modbus/parser/ParserTest.cpp
    unit/modbus/session/ModbusClientBoundaryTest.cpp
    unit/modbus/session/ModbusResponseTest.cpp
    unit/modbus/session/FrameExtractorTest.cpp
    unit/modbus/session/ModbusStateTest.cpp
    unit/modbus/transport/TransportTest.cpp
    unit/update/UpdateManagerInstallStrategyTest.cpp
)

set(TEST_QT_CORE_SOURCES
    unit/modbus/application/ModbusSessionPresenterTest.cpp
    unit/modbus/application/PollingControllerTest.cpp
    unit/modbus/application/RequestSubmissionServiceTest.cpp
    unit/ui/AppLifecycleCoordinatorTest.cpp
    unit/ui/MainWindowPresenterTest.cpp
    unit/ui/SettingsServiceTest.cpp
    unit/ui/UpdateCheckerTest.cpp
    unit/ui/UpdateCoordinatorTest.cpp
)

set(TEST_UI_WIDGETS_SOURCES
    unit/modbus/application/TrafficLogControllerTest.cpp
    unit/ui/I18nWidgetTest.cpp
    unit/ui/NavigationControllerTest.cpp
    unit/ui/ServerClientPanelTest.cpp
)

set(TEST_INTEGRATION_SOURCES
    integration/session/ClientIntegrationTest.cpp
)

set(TEST_MODBUS_STABILITY_SOURCES
    unit/modbus/factory/ModbusFactoryThreadingTest.cpp
    unit/modbus/session/FrameExtractorTest.cpp
    unit/modbus/transport/TransportTest.cpp
)

set(TEST_MODBUS_CONCURRENCY_SOURCES
    unit/modbus/session/ModbusClientBoundaryTest.cpp
    unit/modbus/transport/TransportTest.cpp
)

set(TEST_MODBUS_FUZZ_SOURCES
    unit/modbus/fuzz/ModbusParsingFuzzTest.cpp
)

modbus_add_test_target(test_core_logic
    ${CMAKE_CURRENT_SOURCE_DIR}/TestMainCore.cpp
    SOURCES "${TEST_CORE_LOGIC_SOURCES}"
    LIBS core Qt6::Core
)

modbus_add_test_target(test_qt_core
    ${CMAKE_CURRENT_SOURCE_DIR}/TestMainCore.cpp
    SOURCES "${TEST_QT_CORE_SOURCES}"
    LIBS core ui Qt6::Core
)

modbus_add_test_target(test_ui_widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/TestMainWidgets.cpp
    SOURCES "${TEST_UI_WIDGETS_SOURCES}"
    LIBS core ui Qt6::Core Qt6::Widgets
)

modbus_add_test_target(test_integration
    ${CMAKE_CURRENT_SOURCE_DIR}/TestMainCore.cpp
    SKIP_DISCOVERY
    SOURCES "${TEST_INTEGRATION_SOURCES}"
    LIBS core Qt6::Core
)

modbus_add_test_target(test_modbus_stability
    ${CMAKE_CURRENT_SOURCE_DIR}/TestMainCore.cpp
    SOURCES "${TEST_MODBUS_STABILITY_SOURCES}"
    LIBS core Qt6::Core
)

modbus_add_test_target(test_modbus_concurrency
    ${CMAKE_CURRENT_SOURCE_DIR}/TestMainCore.cpp
    SOURCES "${TEST_MODBUS_CONCURRENCY_SOURCES}"
    LIBS core Qt6::Core
)

modbus_add_test_target(test_modbus_fuzz
    ${CMAKE_CURRENT_SOURCE_DIR}/TestMainCore.cpp
    SKIP_DISCOVERY
    SOURCES "${TEST_MODBUS_FUZZ_SOURCES}"
    LIBS core Qt6::Core
)
