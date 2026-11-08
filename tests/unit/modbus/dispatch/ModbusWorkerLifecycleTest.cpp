// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QSignalSpy>
#include <QThread>
#include <QtTest>
#include <QtCore/QCoreApplication>

#include "mocks/MockChannel.h"
#include "mocks/MockTransport.h"
#include "modbus/dispatch/ModbusWorker.h"
#include "modbus/session/ModbusClient.h"
#include "helpers/ModbusTestHelpers.h"

// T03/T04 acceptance: lifecycle entry points must never drive ModbusClient
// from a thread other than the worker thread, and a stop→start cycle must
// leave the request pipeline fully functional.

using modbus::base::ModbusConfig;
using modbus::base::ModbusMode;
using modbus::dispatch::ModbusWorker;
using modbus::session::ModbusClient;
using namespace testing;

namespace {

class ModbusWorkerLifecycleTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockChannel_ = std::make_shared<NiceMock<io::MockChannel>>();
        mockTransport_ = std::make_shared<NiceMock<modbus::transport::MockTransport>>();

        ON_CALL(*mockChannel_, state()).WillByDefault(ReturnPointee(&currentState_));
        ON_CALL(*mockChannel_, isOpen()).WillByDefault(testing::Invoke([this] {
            return currentState_ == io::ChannelState::Open;
        }));
        ON_CALL(*mockChannel_, addStateHandler(_))
            .WillByDefault(DoAll(SaveArg<0>(&stateHandler_), testing::Return(1)));
        ON_CALL(*mockChannel_, setReadHandler(_))
            .WillByDefault(SaveArg<0>(&readHandler_));

        client_ = std::make_shared<ModbusClient>(mockChannel_, mockTransport_);
        thread_ = std::make_unique<QThread>();
        worker_ = std::make_unique<ModbusWorker>(client_, thread_.get());
    }

    void TearDown() override {
        if (worker_) {
            worker_->stop();
        }
        if (thread_ && thread_->isRunning()) {
            thread_->quit();
            EXPECT_TRUE(thread_->wait(2000));
        }
        worker_.reset();
        client_.reset();
        mockChannel_.reset();
        mockTransport_.reset();
    }

    // Arms the mock so client_->connect() succeeds from the worker thread.
    void expectChannelOpens() {
        EXPECT_CALL(*mockChannel_, open()).WillOnce(testing::Invoke([this] {
            currentState_ = io::ChannelState::Open;
            if (stateHandler_) stateHandler_(io::ChannelState::Open);
            return true;
        }));
    }

    ModbusConfig fastConfig() {
        auto cfg = modbus::test::MakeModbusConfig(ModbusMode::RTU, 150, 0);
        cfg.retryIntervalMs = 10;
        cfg.retryBackoffFactor = 1.0;
        cfg.maxRetryIntervalMs = 10;
        cfg.retryJitterPercent = 0;
        return cfg;
    }

    io::ChannelState currentState_ = io::ChannelState::Closed;
    std::function<void(io::ChannelState)> stateHandler_;
    std::function<void(QByteArrayView)> readHandler_;
    std::shared_ptr<NiceMock<io::MockChannel>> mockChannel_;
    std::shared_ptr<NiceMock<modbus::transport::MockTransport>> mockTransport_;
    std::shared_ptr<ModbusClient> client_;
    std::unique_ptr<QThread> thread_;
    std::unique_ptr<ModbusWorker> worker_;
};

// T03: with a never-started worker thread, requestDisconnect/updateConfig/stop
// must complete without driving the channel from the calling thread.
TEST_F(ModbusWorkerLifecycleTest, EntriesBeforeThreadStart_DoNotTouchChannel) {
    EXPECT_CALL(*mockChannel_, open()).Times(0);
    EXPECT_CALL(*mockChannel_, close()).Times(0);
    EXPECT_CALL(*mockChannel_, write(_)).Times(0);

    QSignalSpy disconnectSpy(worker_.get(), &ModbusWorker::disconnectFinished);
    worker_->requestDisconnect();
    QCoreApplication::processEvents();
    EXPECT_EQ(disconnectSpy.count(), 1);

    // Cached for later start(); must not touch the client cross-thread.
    worker_->updateConfig(fastConfig());

    QSignalSpy stoppedSpy(worker_.get(), &ModbusWorker::stopped);
    worker_->stop();
    QCoreApplication::processEvents();
    EXPECT_EQ(stoppedSpy.count(), 1);

    // Detach expectations before teardown: client/channel destruction is
    // outside this test's scope and may legally call channel methods.
    testing::Mock::VerifyAndClearExpectations(mockChannel_.get());
}

// T03: config applied before start() is cached and reaches the client on the
// worker thread instead of being driven cross-thread or dropped.
TEST_F(ModbusWorkerLifecycleTest, UpdateConfigBeforeStart_IsAppliedOnWorkerThread) {
    const ModbusConfig cfg = fastConfig();
    worker_->updateConfig(cfg); // thread not running -> cached

    expectChannelOpens();
    worker_->start();

    QSignalSpy connectSpy(worker_.get(), &ModbusWorker::connectFinished);
    worker_->requestConnect();
    ASSERT_TRUE(connectSpy.wait(2000)) << "connect did not finish on worker thread";
    ASSERT_EQ(connectSpy.count(), 1);
    EXPECT_TRUE(connectSpy.takeFirst().at(0).toBool());

    // While connected, a follow-up config propagates timeouts to the channel
    // (deterministic proof the queued path drives the client on the worker).
    const int updatedTimeoutMs = cfg.timeoutMs + 100;
    ModbusConfig updated = cfg;
    updated.timeoutMs = updatedTimeoutMs;
    io::Timeouts captured{};
    EXPECT_CALL(*mockChannel_, setTimeouts(_)).WillOnce(SaveArg<0>(&captured));
    worker_->updateConfig(updated);
    QTRY_COMPARE(captured.readMs, updatedTimeoutMs);
    EXPECT_EQ(captured.writeMs, updatedTimeoutMs);
}

// T04 acceptance: stop→start→submit must restore the request pipeline; a
// leaked isProcessing_ flag would permanently short-circuit processQueue()
// and the second submit would never produce a response.
TEST_F(ModbusWorkerLifecycleTest, SubmitAfterStopStartCycle_CompletesNormally) {
    worker_->start();

    QSignalSpy finishedSpy(worker_.get(), &ModbusWorker::requestFinished);

    // Channel stays closed -> the request fails fast with a connection error
    // (no timeout machinery involved, fully deterministic).
    worker_->submit(modbus::test::MakeReadHoldingRegistersPdu(0, 2), 1, 1);
    ASSERT_TRUE(finishedSpy.wait(2000)) << "first submit did not complete";
    EXPECT_EQ(finishedSpy.takeFirst().at(0).toInt(), 1);

    QSignalSpy stoppedSpy(worker_.get(), &ModbusWorker::stopped);
    worker_->stop();
    ASSERT_TRUE(stoppedSpy.wait(2000)) << "worker did not stop";

    worker_->start();
    worker_->submit(modbus::test::MakeReadHoldingRegistersPdu(0, 2), 1, 2);
    ASSERT_TRUE(finishedSpy.wait(2000))
        << "submit after stop/start cycle never completed (pipeline stuck?)";
    EXPECT_EQ(finishedSpy.takeFirst().at(0).toInt(), 2);
}

} // namespace
