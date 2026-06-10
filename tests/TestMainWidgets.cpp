#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QtCore/QLoggingCategory>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QApplication>

#include "infra/logging/Logger.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\nqt.*.debug=false"));
    logging::SetLevel(spdlog::level::warn);

    testing::InitGoogleMock(&argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
