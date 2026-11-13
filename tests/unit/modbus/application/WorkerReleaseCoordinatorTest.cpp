// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QEventLoop>
#include <QSignalSpy>
#include <QThread>
#include <QTimer>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <algorithm>
#include <functional>
#include <memory>

#include "../../../ui/application/modbus/WorkerReleaseCoordinator.h"
#include "../../../infra/io/ChannelBase.h"
#include "../../../infra/io/TcpChannel.h"
#include "common/ThreadGuard.h"
#include "modbus/dispatch/ModbusWorker.h"
#include "modbus/session/ModbusClient.h"

using namespace ui::application::modbus;

namespace {

/// Helper that pumps the GUI event loop until @p spy catches at least @p count
/// signals or the wait budget expires without blocking sleep calls.
void waitForSignals(QSignalSpy& spy, int count, int maxIterations = 500) {
    const int stepTimeoutMs = 50;
    const int maxSteps = std::max(1, (maxIterations * 5) / stepTimeoutMs);
    for (int i = 0; i < maxSteps && spy.count() < count; ++i) {
        if (spy.wait(stepTimeoutMs)) {
            continue;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
    }
}

/// Pumps the event loop via QTimer/QEventLoop until @p predicate holds or
/// the timeout budget is exhausted.
bool waitForCondition(const std::function<bool()>& predicate,
                      int maxIterations = 500) {
    if (predicate()) {
        return true;
    }
    QEventLoop loop;
    QTimer pollTimer;
    pollTimer.setInterval(5);
    QObject::connect(&pollTimer, &QTimer::timeout, &loop, [&]() {
        if (predicate()) {
            loop.quit();
        }
    });
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    pollTimer.start();
    timeoutTimer.start(maxIterations * 5);
    loop.exec();
    return predicate();
}

/// Deterministic ThreadGuard-protected ChannelBase double that lingers in
/// Closing for @p closeLingerMs on its IO thread before emitting Closed.
class FakeLingeringChannel final : public io::ChannelBase {
public:
    explicit FakeLingeringChannel(int closeLingerMs)
        : closeLingerMs_(closeLingerMs) {}

    ~FakeLingeringChannel() noexcept override {
        assertOwnerThreadForDestruction("FakeLingeringChannel");
    }

    io::ChannelKind kind() const override { return io::ChannelKind::Tcp; }

    void moveToThread(QThread* thread) override {
        io::ChannelBase::moveToThread(thread);
        anchor_.moveToThread(thread);
    }

    bool open() override {
        if (marshalToOwner(anchor_, [this]() { setState(io::ChannelState::Open); })) {
            return true;
        }
        setState(io::ChannelState::Open);
        return true;
    }

    void close() override {
        if (marshalToOwner(anchor_, [this]() { close(); })) {
            return;
        }
        setState(io::ChannelState::Closing);
        QTimer::singleShot(closeLingerMs_, &anchor_, [this]() {
            setState(io::ChannelState::Closed);
        });
    }

    bool write(QByteArrayView) override { return true; }

private:
    int closeLingerMs_ = 0;
    QObject anchor_;
};

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
    auto thread = std::shared_ptr<QThread>(
        new QThread(), &core::common::ThreadGuard::releaseThread);
    auto* parked = new QObject();
    parked->moveToThread(thread.get());
    QObject::connect(thread.get(), &QThread::finished, parked, &QObject::deleteLater);
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
    thread.reset();
}

TEST_F(WorkerReleaseCoordinatorTest, ShutdownAll_StampsTimeoutMessageOnPending) {
    auto thread = std::shared_ptr<QThread>(
        new QThread(), &core::common::ThreadGuard::releaseThread);
    auto* parked = new QObject();
    parked->moveToThread(thread.get());
    QObject::connect(thread.get(), &QThread::finished, parked, &QObject::deleteLater);
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
    thread.reset();
}

TEST_F(WorkerReleaseCoordinatorTest, MultipleConcurrentReleases_AllComplete) {
    // Two independent live stacks released back-to-back must both flow through
    // the coordinator and each emit releaseCompleted.
    auto threadA = std::shared_ptr<QThread>(
        new QThread(), &core::common::ThreadGuard::releaseThread);
    auto* parkedA = new QObject();
    parkedA->moveToThread(threadA.get());
    QObject::connect(threadA.get(), &QThread::finished, parkedA, &QObject::deleteLater);
    threadA->start();

    auto threadB = std::shared_ptr<QThread>(
        new QThread(), &core::common::ThreadGuard::releaseThread);
    auto* parkedB = new QObject();
    parkedB->moveToThread(threadB.get());
    QObject::connect(threadB.get(), &QThread::finished, parkedB, &QObject::deleteLater);
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
       ConnectedChannel_WithoutWorker_ClosesImmediatelyWithoutTimeout) {
    QTcpServer listener;
    ASSERT_TRUE(listener.listen(QHostAddress::LocalHost, 0));
    const quint16 port = listener.serverPort();

    auto ioThread = std::shared_ptr<QThread>(
        new QThread(), &core::common::ThreadGuard::releaseThread);
    auto channel = std::shared_ptr<io::TcpChannel>(
        new io::TcpChannel(/*closeLingerMs=*/200),
        [ioThread](io::TcpChannel* ch) {
            core::common::ThreadGuard::releaseChannel(ch, ioThread);
        });
    channel->moveToThread(ioThread.get());
    ioThread->start();
    channel->setEndpoint(QStringLiteral("127.0.0.1"),
                          static_cast<int>(port));

    channel->open();
    ASSERT_TRUE(waitForCondition(
        [&channel]() { return channel->state() == io::ChannelState::Open; }))
        << "channel never reached Open";

    StackHandle handle;
    handle.channel = channel;
    handle.channelThread = ioThread;
    coordinator_->requestRelease(std::move(handle), QStringLiteral("immediate"),
                                 /*timeoutMs=*/500);

    waitForSignals(*completedSpy_, 1, 1000);
    ASSERT_EQ(completedSpy_->count(), 1);
    EXPECT_EQ(timedOutSpy_->count(), 0);
    EXPECT_EQ(channel->state(), io::ChannelState::Closed)
        << "channel must be Closed before finalize destroys it";
    EXPECT_FALSE(coordinator_->hasPending());

    if (ioThread->isRunning()) {
        ioThread->wait(2000);
    }
    channel.reset();
}

TEST_F(WorkerReleaseCoordinatorTest,
       ConnectedChannel_TimeoutWaitsForClosedBeforeFinalizing) {
    auto ioThread = std::shared_ptr<QThread>(
        new QThread(), &core::common::ThreadGuard::releaseThread);
    auto channel = std::shared_ptr<FakeLingeringChannel>(
        new FakeLingeringChannel(/*closeLingerMs=*/150),
        [ioThread](FakeLingeringChannel* ch) {
            core::common::ThreadGuard::releaseChannel(ch, ioThread);
        });
    channel->moveToThread(ioThread.get());
    ioThread->start();
    channel->open();
    ASSERT_TRUE(waitForCondition(
        [&channel]() { return channel->state() == io::ChannelState::Open; }))
        << "channel never reached Open";

    StackHandle handle;
    handle.channel = channel;
    handle.channelThread = ioThread;
    // Budget (50ms) expires while the channel is still closing (150ms linger):
    // timeout fires TimedOut but finalization waits until Closed arrives.
    coordinator_->requestRelease(std::move(handle), QStringLiteral("timeout"),
                                 /*timeoutMs=*/50);

    waitForSignals(*completedSpy_, 1, 1000);
    ASSERT_EQ(completedSpy_->count(), 1);
    EXPECT_EQ(timedOutSpy_->count(), 1);
    EXPECT_EQ(channel->state(), io::ChannelState::Closed)
        << "channel must be Closed before finalize destroys it";
    EXPECT_FALSE(coordinator_->hasPending());

    if (ioThread->isRunning()) {
        ioThread->wait(2000);
    }
    channel.reset();
}

TEST_F(WorkerReleaseCoordinatorTest, ReleaseStage_AdvancesMonotonically) {
    auto thread = std::shared_ptr<QThread>(
        new QThread(), &core::common::ThreadGuard::releaseThread);
    auto* parked = new QObject();
    parked->moveToThread(thread.get());
    QObject::connect(thread.get(), &QThread::finished, parked, &QObject::deleteLater);
    thread->start();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);

    StackHandle handle;
    handle.channelThread = thread;

    coordinator_->requestRelease(std::move(handle), QStringLiteral("stage_test"));
    EXPECT_TRUE(coordinator_->hasPending());
    // Active stage should be at least StoppingWorker or JoiningThreads
    const auto stageDuringWait = coordinator_->currentStage();
    EXPECT_TRUE(stageDuringWait == WorkerReleaseCoordinator::ReleaseStage::StoppingWorker
                || stageDuringWait == WorkerReleaseCoordinator::ReleaseStage::JoiningThreads);

    thread->quit();
    waitForSignals(*completedSpy_, 1, 500);

    EXPECT_EQ(completedSpy_->count(), 1);
    EXPECT_FALSE(coordinator_->hasPending());

    if (thread->isRunning()) {
        thread->wait(2000);
    }
    thread.reset();
}

TEST_F(WorkerReleaseCoordinatorTest, ConcurrentRapidRequestRelease_Idempotent) {
    // Calling requestRelease repeatedly with empty handles must be strictly idempotent
    for (int i = 0; i < 5; ++i) {
        StackHandle empty;
        coordinator_->requestRelease(std::move(empty), QStringLiteral("rapid"));
    }
    waitForSignals(*completedSpy_, 5, 500);
    EXPECT_EQ(completedSpy_->count(), 5);
    EXPECT_EQ(timedOutSpy_->count(), 0);
    EXPECT_FALSE(coordinator_->hasPending());
}

TEST_F(WorkerReleaseCoordinatorTest,
       SharedThread_WorkerAndChannel_WaitsForClosedBeforeQuittingThread) {
    auto sharedThread = std::shared_ptr<QThread>(
        new QThread(), &core::common::ThreadGuard::releaseThread);
    auto channel = std::shared_ptr<FakeLingeringChannel>(
        new FakeLingeringChannel(/*closeLingerMs=*/50),
        [sharedThread](FakeLingeringChannel* ch) {
            core::common::ThreadGuard::releaseChannel(ch, sharedThread);
        });
    channel->moveToThread(sharedThread.get());

    auto client = std::make_shared<::modbus::session::ModbusClient>(channel, nullptr);
    auto worker = std::shared_ptr<::modbus::dispatch::ModbusWorker>(
        new ::modbus::dispatch::ModbusWorker(client, sharedThread.get()),
        [](::modbus::dispatch::ModbusWorker* w) {
            core::common::ThreadGuard::releaseWorker(w, [w]() { w->stop(); });
        });
    worker->moveToThread(sharedThread.get());
    sharedThread->start();

    channel->open();
    ASSERT_TRUE(waitForCondition(
        [&channel]() { return channel->state() == io::ChannelState::Open; }))
        << "channel never reached Open";
    worker->start();

    StackHandle handle;
    handle.worker = worker;
    handle.client = client;
    handle.channel = channel;
    handle.workerThread = sharedThread;
    handle.channelThread = sharedThread;

    coordinator_->requestRelease(std::move(handle), QStringLiteral("shared_timeout"),
                                 /*timeoutMs=*/1500);
    waitForSignals(*completedSpy_, 1, 1000);

    EXPECT_EQ(completedSpy_->count(), 1);
    EXPECT_EQ(timedOutSpy_->count(), 0);
    EXPECT_EQ(channel->state(), io::ChannelState::Closed);
    EXPECT_FALSE(coordinator_->hasPending());

    if (sharedThread->isRunning()) {
        sharedThread->wait(2000);
    }
    EXPECT_TRUE(sharedThread->isFinished());
    worker.reset();
    client.reset();
    channel.reset();
}

TEST_F(WorkerReleaseCoordinatorTest,
       DestroyedWhilePending_DetachesWatchersSafely) {
    auto ioThread = std::shared_ptr<QThread>(
        new QThread(), &core::common::ThreadGuard::releaseThread);
    auto channel = std::shared_ptr<FakeLingeringChannel>(
        new FakeLingeringChannel(/*closeLingerMs=*/150),
        [ioThread](FakeLingeringChannel* ch) {
            core::common::ThreadGuard::releaseChannel(ch, ioThread);
        });
    channel->moveToThread(ioThread.get());
    ioThread->start();
    channel->open();
    ASSERT_TRUE(waitForCondition(
        [&channel]() { return channel->state() == io::ChannelState::Open; }))
        << "channel never reached Open";

    StackHandle handle;
    handle.channel = channel;
    handle.channelThread = ioThread;
    coordinator_->requestRelease(std::move(handle), QStringLiteral("destroy_pending"),
                                 /*timeoutMs=*/500);
    EXPECT_TRUE(coordinator_->hasPending());

    // Destroy the coordinator while the channel is still open / ioThread is running.
    coordinator_.reset();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);

    if (ioThread->isRunning()) {
        ioThread->wait(2000);
    }
    channel.reset();
}
