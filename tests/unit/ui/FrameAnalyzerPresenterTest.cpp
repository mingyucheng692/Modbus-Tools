// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTimer>
#include <memory>

#include "application/analyzer/FrameAnalyzerPresenter.h"

using modbus::base::FunctionCode;
using modbus::base::RegisterOrder;
using modbus::parser::ParseResult;
using modbus::parser::ProtocolType;
using ui::application::analyzer::FrameAnalyzerPresenter;

namespace {

bool waitForSpyCount(QSignalSpy& spy, int expectedCount, int timeoutMs = 2000) {
    if (spy.count() >= expectedCount) {
        return true;
    }
    QEventLoop loop;
    QTimer pollTimer;
    pollTimer.setInterval(5);
    QObject::connect(&pollTimer, &QTimer::timeout, &loop, [&]() {
        if (spy.count() >= expectedCount) {
            loop.quit();
        }
    });
    QTimer budgetTimer;
    budgetTimer.setSingleShot(true);
    QObject::connect(&budgetTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    pollTimer.start();
    budgetTimer.start(timeoutMs);
    loop.exec();
    return spy.count() >= expectedCount;
}

} // namespace

TEST(FrameAnalyzerPresenterTest, EnqueueParse_EmitsParsedResultOnGuiThread) {
    FrameAnalyzerPresenter presenter;
    QSignalSpy finishedSpy(&presenter, &FrameAnalyzerPresenter::parseFinished);
    ASSERT_TRUE(finishedSpy.isValid());

    presenter.enqueueParse(QStringLiteral("01 03 00 00 00 02 C4 0B"),
                           ProtocolType::Rtu, 0, RegisterOrder::ABCD, 42);

    ASSERT_TRUE(waitForSpyCount(finishedSpy, 1));
    const QList<QVariant> args = finishedSpy.takeFirst();
    const auto result = args.at(0).value<ParseResult>();
    const auto requestId = args.at(1).toULongLong();

    EXPECT_EQ(requestId, 42ULL);
    EXPECT_TRUE(result.isValid);
    EXPECT_EQ(result.slaveId, 1);
    EXPECT_EQ(result.functionCode, FunctionCode::ReadHoldingRegisters);
}

TEST(FrameAnalyzerPresenterTest, DestroyImmediatelyAfterEnqueue_NoUseAfterFree) {
    // Regression: enqueueParse previously captured raw `this` in the lambda
    // dispatched onto parseWorker_->thread(). Destroying the presenter before
    // the background worker runs must not touch a dangling presenter pointer.
    for (int iteration = 0; iteration < 20; ++iteration) {
        auto presenter = std::make_unique<FrameAnalyzerPresenter>();
        presenter->enqueueParse(QStringLiteral("01 03 00 00 00 02 C4 0B"),
                                ProtocolType::Rtu, 0, RegisterOrder::ABCD,
                                static_cast<quint64>(iteration + 1));
        presenter->enqueueParse(QStringLiteral("01 03 00 0A 00 01 A4 08"),
                                ProtocolType::Rtu, 10, RegisterOrder::ABCD,
                                static_cast<quint64>(iteration + 101));
        presenter.reset();
        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
    }
    SUCCEED();
}
