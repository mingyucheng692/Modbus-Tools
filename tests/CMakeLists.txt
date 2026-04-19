cmake_minimum_required(VERSION 3.16)

# Test target name
set(TEST_TARGET modbus_test)

# Collect test sources
# We start with TestMain.cpp and will add specific test cases
file(GLOB_RECURSE TEST_SOURCES 
    "*.cpp"
    "*.h"
)

add_executable(${TEST_TARGET} ${TEST_SOURCES})

# MSVC UTF-8 support
if(MSVC)
    target_compile_options(${TEST_TARGET} PRIVATE /utf-8)
endif()

# Enable MOC for tests (especially for Mock objects inheriting from QObject)
set_property(TARGET ${TEST_TARGET} PROPERTY AUTOMOC ON)

# Link dependencies
# We use GTest::gmock instead of gtest_main because we provide our own main in TestMain.cpp
target_link_libraries(${TEST_TARGET} PRIVATE
    core
    GTest::gtest
    GTest::gmock
    Qt6::Core
    Qt6::Test
)

# Global test properties
set_target_properties(${TEST_TARGET} PROPERTIES
    TIMEOUT 300
)

# Automatic test discovery
include(GoogleTest)
gtest_discover_tests(${TEST_TARGET}
    DISCOVERY_TIMEOUT 120
)

install(TARGETS ${TEST_TARGET}
    RUNTIME DESTINATION tests
    COMPONENT Test
)
