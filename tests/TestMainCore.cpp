#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QLoggingCategory>
#include <QtCore/QStandardPaths>

#include "infra/logging/Logger.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\nqt.*.debug=false"));
    spdlog::set_level(spdlog::level::warn);

    testing::InitGoogleMock(&argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
