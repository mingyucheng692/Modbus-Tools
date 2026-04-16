#include <gtest/gtest.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QStandardPaths>
#include <QtCore/QLoggingCategory>
#include "logging/Logger.h" // Assuming this has access to spdlog or spdlog directly

int main(int argc, char** argv) {
    // 1. Initialize Qt Application Environment
    QCoreApplication app(argc, argv);
    
    // 2. Enable Test Mode for standard paths (Isolates file operations)
    QStandardPaths::setTestModeEnabled(true);
    
    // 3. Suppress verbose Qt/Business logs to keep test output clean
    QLoggingCategory::setFilterRules("*.debug=false\nqt.*.debug=false");
    
    // Assuming core uses spdlog, set it to warn level for tests
    logging::SetLevel(spdlog::level::warn);

    // 4. Initialize Google Test
    testing::InitGoogleTest(&argc, argv);
    
    // 5. Run tests
    return RUN_ALL_TESTS();
}
