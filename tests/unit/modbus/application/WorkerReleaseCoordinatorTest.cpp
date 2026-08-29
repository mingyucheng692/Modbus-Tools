#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QThread>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <functional>
#include <memory>

#include "../../../ui/application/modbus/WorkerReleaseCoordinator.h"
#include "../../../infra/io/TcpChannel.h"

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

/// Pumps the event loop until @p predicate holds or the spin budget is
/// exhausted.
bool waitForCondition(const std::function<bool()>& predicate,
                      int maxIterations = 500) {
    for (int i = 0; i < maxIterations; ++i) {
        if (predicate()) {
            return true;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
        QThread::msleep(5);
    }
    return predicate();
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

// Regression for the "unresponsive Modbus TCP peer" crash: requestRelease()
// used to quit() the IO thread immediately, so the channel's close() (and its
// linger fallback) never ran — the socket stayed CONNECTED. The timeout path
// then destroyed that live socket from the GUI thread, and QObject teardown
// tripped QCoreApplication::sendEvent's cross-thread assert inside
// QNativeSocketEngine's notifier children (ASSERT "Cannot send events to
// objects owned by a different thread").
//
// The fix keeps the IO thread alive until the channel actually reports
// Closed; releaseCompleted may only arrive after the channel is Closed and
// the thread has joined.
TEST_F(WorkerReleaseCoordinatorTest,
       ConnectedChannel_TimeoutWaitsForClosedBeforeFinalizing) {
    // Local listener so the channel reaches Open deterministically on
    // loopback (no external network dependency).
    QTcpServer listener;
    ASSERT_TRUE(listener.listen(QHostAddress::LocalHost, 0));
    const quint16 port = listener.serverPort();

    auto channel = std::make_shared<io::TcpChannel>(/*closeLingerMs=*/200);
    auto ioThread = std::make_shared<QThread>();
    channel->moveToThread(ioThread.get());
    ioThread->start();
    channel->setEndpoint(QStringLiteral("127.0.0.1"),
                          static_cast<int>(port));

    // open() from the test thread queues the real connect onto the IO
    // thread; pump until the channel reports Open.
    channel->open();
    ASSERT_TRUE(waitForCondition(
        [&channel]() { return channel->state() == io::ChannelState::Open; }))
        << "channel never reached Open";
    ASSERT_EQ(channel->state(), io::ChannelState::Open);

    StackHandle handle;
    handle.channel = channel;
    handle.channelThread = ioThread;
    // worker/workerThread are null; a tiny budget drives the timeout path
    // (the old code crashed in exactly this configuration).
    coordinator_->requestRelease(std::move(handle), QStringLiteral("timeout"),
                                 /*timeoutMs=*/50);

    // The release must still complete: the coordinator closes the channel
    // (graceful FIN on loopback), observes Closed, quits the IO thread and
    // only then finalizes. If the old immediate-quit/finalize behavior were
    // still present this process would have crashed with the sendEvent
    // assert instead of reaching this point.
    waitForSignals(*completedSpy_, 1, 1000);
    ASSERT_EQ(completedSpy_->count(), 1);
    EXPECT_EQ(timedOutSpy_->count(), 1);
    EXPECT_EQ(channel->state(), io::ChannelState::Closed)
        << "channel must be Closed before finalize destroys it";
    EXPECT_FALSE(coordinator_->hasPending());

    if (ioThread->isRunning()) {
        ioThread->wait(2000);
    }
    // Destroy the channel BEFORE the QThread object: ChannelBase keeps a
    // raw deviceThread_ pointer, so the thread object must outlive the
    // channel (otherwise ~TcpChannel reads freed memory in the destruction
    // guard). finalize() already released the coordinator's reference; this
    // reset drops the test's own reference in the correct order.
    channel.reset();
}
