// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QCoreApplication>

#include "../../../infra/io/ChannelOperationWorker.h"
#include "../../../infra/io/SerialConfig.h"
#include "../../helpers/ModbusTestHelpers.h"

using namespace io;

namespace {

class ChannelOperationWorkerTest : public ::testing::Test {
protected:
    void SetUp() override {
        worker_ = std::make_unique<ChannelOperationWorker>();
    }

    void TearDown() override {
        // Ensure channel cleanup before destroying worker
        if (worker_) {
            worker_->close();
        }
        worker_.reset();
    }

    std::unique_ptr<ChannelOperationWorker> worker_;
};

// Construction / Destruction
TEST_F(ChannelOperationWorkerTest, Construct_DestructNoCrash) {
    EXPECT_NE(worker_, nullptr);
}

// Open TCP — verify the attempt does not crash even if destroyed while connectToHost
// is in flight (half-open ConnectingState must abort safely before socket teardown).
TEST_F(ChannelOperationWorkerTest, OpenTcp_NoCrash) {
    QSignalSpy stateSpy(worker_.get(), &ChannelOperationWorker::stateChangedWithGeneration);

    worker_->openTcp(QStringLiteral("127.0.0.1"), 1502, 7);
    ASSERT_GE(stateSpy.count(), 1);
    EXPECT_EQ(stateSpy.at(0).at(1).toULongLong(), 7u);

    worker_->close();
    worker_.reset();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    SUCCEED();
}

// Open Serial — verify error path works
TEST_F(ChannelOperationWorkerTest, OpenSerial_InvalidPort_EmitsError) {
    QSignalSpy errorSpy(worker_.get(), &ChannelOperationWorker::channelErrorOccurred);

    SerialConfig cfg;
    cfg.portName = QStringLiteral("COM_NONEXISTENT_99999");
    worker_->openSerial(cfg, 1);

    // Serial open failure emits error synchronously (or very quickly)
    // Allow some async processing
    for (int i = 0; i < 10 && errorSpy.count() == 0; ++i) {
        QCoreApplication::processEvents();
    }
    // Pump one more turn so any late queued duplicate QSerialPort::errorOccurred would surface.
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);

    EXPECT_EQ(errorSpy.count(), 1);
}

TEST_F(ChannelOperationWorkerTest, OpenSerial_EmptyPortName_EmitsError) {
    QSignalSpy errorSpy(worker_.get(), &ChannelOperationWorker::channelErrorOccurred);

    SerialConfig cfg;
    cfg.portName.clear();
    worker_->openSerial(cfg, 1);

    for (int i = 0; i < 10 && errorSpy.count() == 0; ++i) {
        QCoreApplication::processEvents();
    }
    // Pump one more turn so any late queued duplicate QSerialPort::errorOccurred would surface.
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);

    EXPECT_EQ(errorSpy.count(), 1);
}

// Generation plumbing: openSerial carries the same monotonic
// attempt counter as openTcp, and the channel state handler is detached
// BEFORE close during cleanup so a torn-down channel cannot emit stale
// generation-tagged states.
TEST_F(ChannelOperationWorkerTest, OpenSerial_Failure_StatesCarryGeneration) {
    QSignalSpy spy(worker_.get(), &ChannelOperationWorker::stateChangedWithGeneration);

    SerialConfig cfg;
    cfg.portName = QStringLiteral("COM_NONEXISTENT_99999");
    worker_->openSerial(cfg, 42);

    for (int i = 0; i < 10 && spy.count() == 0; ++i) {
        QCoreApplication::processEvents();
    }

    ASSERT_GT(spy.count(), 0);
    for (const auto& args : spy) {
        EXPECT_EQ(args.at(1).toULongLong(), 42u);
    }
}

TEST_F(ChannelOperationWorkerTest, OpenSerial_SecondAttempt_NoStaleGenerationEvents) {
    QSignalSpy spy(worker_.get(), &ChannelOperationWorker::stateChangedWithGeneration);

    SerialConfig cfg;
    cfg.portName = QStringLiteral("COM_NONEXISTENT_99999");
    worker_->openSerial(cfg, 1);
    for (int i = 0; i < 10; ++i) {
        QCoreApplication::processEvents();
    }
    spy.clear();

    // Attempt 2: cleanupChannel() detaches the old state handler before
    // close(), so the torn-down channel must not emit Closing/Closed
    // tagged with the OLD generation (stale event suppression).
    worker_->openSerial(cfg, 2);
    for (int i = 0; i < 10; ++i) {
        QCoreApplication::processEvents();
    }

    for (const auto& args : spy) {
        EXPECT_EQ(args.at(1).toULongLong(), 2u)
            << "stale generation-1 event leaked from the torn-down channel";
    }
}

// Write without open channel
TEST_F(ChannelOperationWorkerTest, Write_ChannelNotOpen_EmitsError) {
    QSignalSpy errorSpy(worker_.get(), &ChannelOperationWorker::channelErrorOccurred);

    worker_->write(QByteArrayLiteral("test data"));

    EXPECT_GE(errorSpy.count(), 1);
}

// Close with no channel should not crash
TEST_F(ChannelOperationWorkerTest, Close_NoChannel_DoesNotCrash) {
    EXPECT_NO_FATAL_FAILURE(worker_->close());
}

// DTR / RTS on non-serial channel (silently ignored)
TEST_F(ChannelOperationWorkerTest, DtrRts_NoChannel_DoesNotCrash) {
    EXPECT_NO_FATAL_FAILURE(worker_->setDtr(true));
    EXPECT_NO_FATAL_FAILURE(worker_->setRts(false));
}

// Repeated writes
TEST_F(ChannelOperationWorkerTest, RepeatedWrites_WithoutChannel_EmitsMultipleErrors) {
    QSignalSpy errorSpy(worker_.get(), &ChannelOperationWorker::channelErrorOccurred);

    worker_->write(QByteArrayLiteral("data1"));
    worker_->write(QByteArrayLiteral("data2"));

    EXPECT_GE(errorSpy.count(), 2);
}

TEST_F(ChannelOperationWorkerTest, OpenUdp_GenerationPlumbing_AttachesGenerationToStates) {
    QSignalSpy spy(worker_.get(), &ChannelOperationWorker::stateChangedWithGeneration);

    worker_->openUdp(QStringLiteral("127.0.0.1"), 0, QString(), 0, 99);
    ASSERT_GE(spy.count(), 1);
    for (const auto& args : spy) {
        EXPECT_EQ(args.at(1).toULongLong(), 99u);
    }
}

} // namespace
