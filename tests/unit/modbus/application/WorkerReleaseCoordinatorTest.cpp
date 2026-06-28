#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QThread>
#include <QObject>
#include <memory>

#include "../../../ui/application/modbus/WorkerReleaseCoordinator.h"

using namespace ui::application::modbus;

namespace {

/// Helper that pumps the GUI event loop until @p spy catches at least @p count
/// signals or @p maxIterations processEvents rounds elapse. Each iteration
/// sleeps briefly so wall-clock timers (QTimer::singleShot) can actually fire.
void waitForSignals(QSignalSpy& spy, int count, int maxIterations = 500) {
    for (int i = 0; i < maxIterations && spy.count() < count; ++i) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
        QThread::msleep(5);
    }
}

} // namespace

class WorkerReleaseCoordinatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        coordinator_ = std::make_unique<WorkerReleaseCoordinator>();
        completedSpy_ = std::make_unique<QSignalSpy>(
            coordinator_.get(), &WorkerReleaseCoordinator::releaseCompleted);
        timedOutSpy_ = std::make_unique<QSignalSpy>(
            coordinator_.get(), &WorkerReleaseCoordinator::releaseTimedOut);
    }

    void TearDown() override {
        // Drain any pending events so timers do not fire into a destroyed
        // coordinator during teardown.
        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
    }

    std::unique_ptr<WorkerReleaseCoordinator> coordinator_;
    std::unique_ptr<QSignalSpy> completedSpy_;
    std::unique_ptr<QSignalSpy> timedOutSpy_;
};

TEST_F(WorkerReleaseCoordinatorTest, Initially_HasNoPending) {
    EXPECT_FALSE(coordinator_->hasPending());
}

TEST_F(WorkerReleaseCoordinatorTest, EmptyHandle_EmitsReleaseCompletedQueued) {
    StackHandle empty;
    coordinator_->requestRelease(std::move(empty), QStringLiteral("timeout"));

    // Empty handles complete via a queued invocation; hasPending() must be
    // false immediately because nothing was pushed to the pending vector.
    EXPECT_FALSE(coordinator_->hasPending());

    waitForSignals(*completedSpy_, 1);
    EXPECT_EQ(completedSpy_->count(), 1);
    EXPECT_EQ(timedOutSpy_->count(), 0);
    EXPECT_FALSE(coordinator_->hasPending());
}

TEST_F(WorkerReleaseCoordinatorTest, EmptyHandle_NeverEmitsTimedOut) {
    StackHandle empty;
    coordinator_->requestRelease(std::move(empty), QStringLiteral("never"),
                                 /*timeoutMs=*/10);

    waitForSignals(*completedSpy_, 1);
    // Even with a tiny 10ms budget the empty path short-circuits, so the
    // timeout signal must never fire.
    EXPECT_EQ(completedSpy_->count(), 1);
    EXPECT_EQ(timedOutSpy_->count(), 0);
}

TEST_F(WorkerReleaseCoordinatorTest, ThreadOnlyHandle_CompletesWhenThreadJoins) {
    // A QThread running an event loop (kept alive by a parked QObject so
    // quit() is meaningful). The coordinator observes QThread::finished and
    // finalizes the release.
    auto thread = std::make_shared<QThread>();
    auto* parked = new QObject();
    parked->moveToThread(thread.get());
    thread->start();
    // Give the worker event loop a moment to spin up.
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);

    StackHandle handle;
    handle.channelThread = thread; // worker/client/channel/workerThread null

    coordinator_->requestRelease(std::move(handle), QStringLiteral("timeout"));
    EXPECT_TRUE(coordinator_->hasPending());

    // quit() is thread-safe and asks the worker event loop to exit. The
    // coordinator's queued QThread::finished connection is then delivered on
    // the main thread while we pump events below.
    thread->quit();

    waitForSignals(*completedSpy_, 1, 500);
    ASSERT_EQ(completedSpy_->count(), 1);
    EXPECT_EQ(timedOutSpy_->count(), 0);
    EXPECT_FALSE(coordinator_->hasPending());

    if (thread->isRunning()) {
        thread->wait(2000);
    }
    delete parked;
    thread.reset();
}

TEST_F(WorkerReleaseCoordinatorTest, ShutdownAll_StampsTimeoutMessageOnPending) {
    auto thread = std::make_shared<QThread>();
    auto* parked = new QObject();
    parked->moveToThread(thread.get());
    thread->start();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);

    StackHandle handle;
    handle.channelThread = thread;

    coordinator_->requestRelease(std::move(handle), QStringLiteral("original"),
                                 /*timeoutMs=*/10000);
    ASSERT_TRUE(coordinator_->hasPending());

    coordinator_->shutdownAll(QStringLiteral("overridden"));
    EXPECT_TRUE(coordinator_->hasPending());

    thread->quit();
    waitForSignals(*completedSpy_, 1, 500);

    if (thread->isRunning()) {
        thread->wait(2000);
    }
    delete parked;
    thread.reset();
}

TEST_F(WorkerReleaseCoordinatorTest, MultipleConcurrentReleases_AllComplete) {
    // Two independent live stacks released back-to-back must both flow through
    // the coordinator and each emit releaseCompleted.
    auto threadA = std::make_shared<QThread>();
    auto* parkedA = new QObject();
    parkedA->moveToThread(threadA.get());
    threadA->start();

    auto threadB = std::make_shared<QThread>();
    auto* parkedB = new QObject();
    parkedB->moveToThread(threadB.get());
    threadB->start();

    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);

    StackHandle handleA;
    handleA.channelThread = threadA;
    coordinator_->requestRelease(std::move(handleA), QStringLiteral("a"));
    EXPECT_TRUE(coordinator_->hasPending());

    StackHandle handleB;
    handleB.channelThread = threadB;
    coordinator_->requestRelease(std::move(handleB), QStringLiteral("b"));
    EXPECT_TRUE(coordinator_->hasPending());

    threadA->quit();
    threadB->quit();

    waitForSignals(*completedSpy_, 2, 500);
    EXPECT_EQ(completedSpy_->count(), 2);
    EXPECT_EQ(timedOutSpy_->count(), 0);
    EXPECT_FALSE(coordinator_->hasPending());

    if (threadA->isRunning()) threadA->wait(2000);
    if (threadB->isRunning()) threadB->wait(2000);
    delete parkedA;
    delete parkedB;
    threadA.reset();
    threadB.reset();
}
