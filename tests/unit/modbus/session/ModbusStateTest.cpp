#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "modbus/session/ModbusClient.h"
#include "../../../mocks/MockChannel.h"
#include "../../../mocks/MockTransport.h"
#include <algorithm>
#include <future>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

using namespace modbus::session;
using namespace modbus::transport;
using namespace modbus::base;
using namespace io;
using namespace testing;

namespace {

// Appends a sink to spdlog's default logger for the lifetime of the object
// and exposes the captured (raw, unformatted) message payloads. Used to
// assert that the connection FSM never performs an invalid transition: the
// FSM logs "invalid transition" at ERROR level on every rejected edge, so
// any occurrence means a caller bypassed the transition table's contract.
class CapturingLogSink final : public spdlog::sinks::base_sink<std::mutex> {
public:
    std::vector<std::string> lines() const {
        std::lock_guard<std::mutex> lock(linesMutex_);
        return lines_;
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        std::lock_guard<std::mutex> lock(linesMutex_);
        lines_.emplace_back(msg.payload.data(), msg.payload.size());
    }

    void flush_() override {}

private:
    mutable std::mutex linesMutex_;
    std::vector<std::string> lines_;
};

class ScopedLogCapture {
public:
    ScopedLogCapture() : sink_(std::make_shared<CapturingLogSink>()) {
        spdlog::default_logger()->sinks().push_back(sink_);
    }

    ~ScopedLogCapture() {
        auto& sinks = spdlog::default_logger()->sinks();
        sinks.erase(std::remove(sinks.begin(), sinks.end(), sink_), sinks.end());
    }

    ScopedLogCapture(const ScopedLogCapture&) = delete;
    ScopedLogCapture& operator=(const ScopedLogCapture&) = delete;

    [[nodiscard]] bool contains(std::string_view needle) const {
        const auto lines = sink_->lines();
        return std::any_of(lines.begin(), lines.end(),
                          [needle](const std::string& line) {
                              return line.find(needle) != std::string::npos;
                          });
    }

    [[nodiscard]] size_t count(std::string_view needle) const {
        const auto lines = sink_->lines();
        return static_cast<size_t>(std::count_if(lines.begin(), lines.end(),
                                  [needle](const std::string& line) {
                                      return line.find(needle) != std::string::npos;
                                  }));
    }

private:
    std::shared_ptr<CapturingLogSink> sink_;
};

} // namespace

class ModbusStateTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockChannel_ = std::make_shared<NiceMock<MockChannel>>();
        mockTransport_ = std::make_shared<NiceMock<MockTransport>>();

        ON_CALL(*mockChannel_, addStateHandler(_))
            .WillByDefault(DoAll(SaveArg<0>(&stateHandler_), Return(1)));
        ON_CALL(*mockChannel_, setReadHandler(_))
            .WillByDefault(SaveArg<0>(&readHandler_));
        ON_CALL(*mockChannel_, setErrorHandler(_))
            .WillByDefault(SaveArg<0>(&errorHandler_));
        ON_CALL(*mockChannel_, setWriteDrainedHandler(_))
            .WillByDefault(SaveArg<0>(&writeDrainedHandler_));

        client_ = std::make_unique<ModbusClient>(mockChannel_, mockTransport_);
        
        modbus::base::ModbusConfig config;
        config.timeoutMs = 500;
        client_->setConfig(config);

        // Default behavior for mocks to simulate state
        ON_CALL(*mockChannel_, state()).WillByDefault(ReturnPointee(&currentState_));
        ON_CALL(*mockChannel_, isOpen()).WillByDefault(Invoke([this](){
            return currentState_ == ChannelState::Open;
        }));
    }

    void TearDown() override {
        client_.reset();
        mockChannel_.reset();
        mockTransport_.reset();
    }

    ChannelState currentState_ = ChannelState::Closed;
    std::function<void(ChannelState)> stateHandler_;
    std::function<void(QByteArrayView)> readHandler_;
    std::function<void(const io::ChannelError&)> errorHandler_;
    std::function<void()> writeDrainedHandler_;
    std::shared_ptr<NiceMock<MockChannel>> mockChannel_;
    std::shared_ptr<NiceMock<MockTransport>> mockTransport_;
    std::unique_ptr<ModbusClient> client_;
};

TEST_F(ModbusStateTest, InitialState) {
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Disconnected);
    EXPECT_EQ(client_->requestState(), ModbusClient::RequestState::Idle);
}

TEST_F(ModbusStateTest, ConnectionTransition) {
    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&](){
        currentState_ = ChannelState::Open;
        if (stateHandler_) stateHandler_(ChannelState::Open);
        return true;
    }));

    bool ok = client_->connect();
    EXPECT_TRUE(ok);
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);
}

TEST_F(ModbusStateTest, ConnectionFailure) {
    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&](){
        currentState_ = ChannelState::Error;
        return false;
    }));
    bool ok = client_->connect();
    EXPECT_FALSE(ok);
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);
}

TEST_F(ModbusStateTest, ManualDisconnect) {
    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&](){
        currentState_ = ChannelState::Open;
        if (stateHandler_) stateHandler_(ChannelState::Open);
        return true;
    }));
    
    client_->connect();
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);
    
    EXPECT_CALL(*mockChannel_, close()).WillOnce(Invoke([&](){
        currentState_ = ChannelState::Closed;
        if (stateHandler_) stateHandler_(ChannelState::Closed);
    }));
    
    client_->disconnect();
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Disconnected);
}

TEST_F(ModbusStateTest, ConnectWaitsForAsyncStateHandlerWithoutProcessEvents) {
    std::promise<void> openCalled;
    auto openObserved = openCalled.get_future();

    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&]() {
        openCalled.set_value();
        return true;
    }));

    auto connectFuture = std::async(std::launch::async, [this]() {
        return client_->connect();
    });

    ASSERT_EQ(openObserved.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    currentState_ = ChannelState::Open;
    ASSERT_TRUE(static_cast<bool>(stateHandler_));
    stateHandler_(ChannelState::Open);

    EXPECT_TRUE(connectFuture.get());
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);
}

TEST_F(ModbusStateTest, AbortUnblocksConnectWaitWithoutProcessEvents) {
    std::promise<void> openCalled;
    auto openObserved = openCalled.get_future();

    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&]() {
        openCalled.set_value();
        return true;
    }));

    auto connectFuture = std::async(std::launch::async, [this]() {
        return client_->connect();
    });

    ASSERT_EQ(openObserved.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    client_->abort();

    EXPECT_FALSE(connectFuture.get());
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);
}

// --- P1: passive channel-loss wiring (T1.1) ------------------------------

// A passive channel loss (peer RST / cable pull / driver error) observed as
// Closed while the FSM reports Connected must drive Connected -> Failed.
// Before T1.1 the state handler was a no-op and the FSM stayed Connected
// forever, so every UI derivation kept reporting a live session.
TEST_F(ModbusStateTest, PassiveChannelLoss_ClosedDrivesConnectedToFailed) {
    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&]() {
        currentState_ = ChannelState::Open;
        if (stateHandler_) stateHandler_(ChannelState::Open);
        return true;
    }));
    ASSERT_TRUE(client_->connect());
    ASSERT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);

    currentState_ = ChannelState::Closed;
    stateHandler_(ChannelState::Closed);

    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);
    EXPECT_FALSE(client_->isConnected());
}

// Same contract for the Error notification (serial driver error, socket
// error before the Closed notification).
TEST_F(ModbusStateTest, PassiveChannelLoss_ErrorDrivesConnectedToFailed) {
    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&]() {
        currentState_ = ChannelState::Open;
        if (stateHandler_) stateHandler_(ChannelState::Open);
        return true;
    }));
    ASSERT_TRUE(client_->connect());

    currentState_ = ChannelState::Error;
    stateHandler_(ChannelState::Error);

    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);
}

// The channel-lost handler must not hijack an in-flight user disconnect:
// during disconnect() the FSM is already in Disconnecting, where the Closed
// notification must be ignored so the terminal Disconnected edge succeeds
// without forceReset().
TEST_F(ModbusStateTest, ChannelErrorDuringDisconnect_DoesNotBlockDisconnected) {
    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&]() {
        currentState_ = ChannelState::Open;
        if (stateHandler_) stateHandler_(ChannelState::Open);
        return true;
    }));
    ASSERT_TRUE(client_->connect());

    EXPECT_CALL(*mockChannel_, close()).WillOnce(Invoke([&]() {
        currentState_ = ChannelState::Error;
        // Driver error fires BEFORE the terminal Closed notification.
        stateHandler_(ChannelState::Error);
        currentState_ = ChannelState::Closed;
        stateHandler_(ChannelState::Closed);
    }));

    client_->disconnect();
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Disconnected);
}

// --- P1: retry-after-failure normalization (T1.2) -------------------------

// Reconnecting from Failed must start legally (Failed -> Connecting edge)
// and land Connected. Before T1.2 the second connect() logged an invalid
// transition and, on success, left the FSM stuck in Failed while the channel
// was open.
TEST_F(ModbusStateTest, ReconnectAfterFailure_LandsConnectedWithoutInvalidTransition) {
    ScopedLogCapture logCapture;

    modbus::base::ModbusConfig config;
    config.timeoutMs = 500;
    config.retries = 0;
    config.autoReconnect = false;
    client_->setConfig(config);

    int openCalls = 0;
    EXPECT_CALL(*mockChannel_, open()).WillRepeatedly(Invoke([&]() {
        ++openCalls;
        if (openCalls == 1) {
            // First attempt: dispatch failure -> Connecting -> Failed.
            currentState_ = ChannelState::Error;
            return false;
        }
        currentState_ = ChannelState::Open;
        if (stateHandler_) stateHandler_(ChannelState::Open);
        return true;
    }));

    ASSERT_FALSE(client_->connect());
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);

    EXPECT_TRUE(client_->connect());
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);

    // Gate: no FSM edge may be rejected across the whole failure-retry cycle.
    EXPECT_FALSE(logCapture.contains("invalid transition"))
        << "connection FSM performed a rejected transition during retry";
}

// A channel that is already open while the FSM sits in Failed (stale state,
// e.g. a passive loss reported after the failure was already handled) must
// be walked to Connected through legal edges on the already-open fast path.
TEST_F(ModbusStateTest, EnsureConnected_AlreadyOpenChannelFromFailedLandsConnected) {
    ScopedLogCapture logCapture;

    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&]() {
        currentState_ = ChannelState::Open;
        if (stateHandler_) stateHandler_(ChannelState::Open);
        return true;
    }));
    ASSERT_TRUE(client_->connect());

    // Passive loss drives Connected -> Failed...
    currentState_ = ChannelState::Closed;
    stateHandler_(ChannelState::Closed);
    ASSERT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);

    // ...but the channel comes back open before the next ensureConnected()
    // observes it (e.g. automatic transport-level rebind): the fast path
    // must resume the FSM to Connected without a rejected edge.
    currentState_ = ChannelState::Open;

    EXPECT_TRUE(client_->connect());
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);
    EXPECT_FALSE(logCapture.contains("invalid transition"))
        << "already-open fast path performed a rejected transition";
}

// Full lifecycle gate: connect -> passive loss -> user disconnect must never
// log a rejected FSM edge.
TEST_F(ModbusStateTest, FullLifecycle_NoInvalidTransitionLogs) {
    ScopedLogCapture logCapture;

    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&]() {
        currentState_ = ChannelState::Open;
        if (stateHandler_) stateHandler_(ChannelState::Open);
        return true;
    }));
    ASSERT_TRUE(client_->connect());
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);

    currentState_ = ChannelState::Closed;
    stateHandler_(ChannelState::Closed);
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);

    client_->disconnect();
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Disconnected);

    EXPECT_FALSE(logCapture.contains("invalid transition"))
        << "connection FSM performed a rejected transition across the lifecycle";
}

TEST_F(ModbusStateTest, AbortGuaranteesTerminalState) {
    // In Connecting state, abort() must force terminal Failed state.
    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&]() {
        EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connecting);
        client_->abort();
        EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);
        return false;
    }));
    EXPECT_FALSE(client_->connect());
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);

    // Verify clearRuntimeState() / subsequent abort while in terminal state remains safe
    client_->abort();
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);
}

TEST_F(ModbusStateTest, FastPathConsistencyUnderOpenChannel) {
    ScopedLogCapture logCapture;

    // Case 1: Channel is already open while FSM is Disconnected
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Disconnected);
    currentState_ = ChannelState::Open;
    EXPECT_TRUE(client_->connect());
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);
    EXPECT_FALSE(logCapture.contains("invalid transition"));

    // Case 2: Channel is already open while FSM is Failed
    currentState_ = ChannelState::Closed;
    stateHandler_(ChannelState::Closed);
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);

    currentState_ = ChannelState::Open;
    EXPECT_TRUE(client_->connect());
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);
    EXPECT_FALSE(logCapture.contains("invalid transition"));
}

TEST_F(ModbusStateTest, FailureLatchSilencesRepetitiveWarnings) {
    ScopedLogCapture logCapture;
    EXPECT_CALL(*mockChannel_, open()).WillRepeatedly(Return(false));
    currentState_ = ChannelState::Error;

    // 10 consecutive failures should only log 1 warning due to Failure Latch
    for (int i = 0; i < 10; ++i) {
        EXPECT_FALSE(client_->connect());
        EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);
    }
    EXPECT_EQ(logCapture.count("ModbusClient: connect failed"), 1u);

    // Connecting successfully resets the latch
    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&]() {
        currentState_ = ChannelState::Open;
        if (stateHandler_) stateHandler_(ChannelState::Open);
        return true;
    }));
    EXPECT_TRUE(client_->connect());
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);

    // Passive loss causes failure
    currentState_ = ChannelState::Closed;
    stateHandler_(ChannelState::Closed);
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);

    // Next connection attempt fails: latch was reset, so 1 new warning is logged (total 2)
    EXPECT_CALL(*mockChannel_, open()).WillRepeatedly(Return(false));
    EXPECT_FALSE(client_->connect());
    EXPECT_EQ(logCapture.count("ModbusClient: connect failed"), 2u);

    // Subsequent failure is silenced again
    EXPECT_FALSE(client_->connect());
    EXPECT_EQ(logCapture.count("ModbusClient: connect failed"), 2u);
}

TEST_F(ModbusStateTest, AssertCleanEntryStateLogsErrorOnStaleState) {
    // Connect successfully first
    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&]() {
        currentState_ = ChannelState::Open;
        if (stateHandler_) stateHandler_(ChannelState::Open);
        return true;
    }));
    ASSERT_TRUE(client_->connect());
    ASSERT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);

    // Channel becomes closed silently without stateHandler notification
    // (simulating missed channel-lost callback, leaving FSM in stale Connected state)
    currentState_ = ChannelState::Closed;

    ScopedLogCapture logCapture;
    EXPECT_CALL(*mockChannel_, open()).WillRepeatedly(Return(false));

    // Calling ensureConnected() finds channel closed while FSM is Connected.
    // assertCleanEntryState() detects stale Connected state, logs SPDLOG_ERROR,
    // and self-heals into Failed before the connect attempt without crashing.
    EXPECT_FALSE(client_->connect());
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);
    EXPECT_TRUE(logCapture.contains("assertCleanEntryState"));
    EXPECT_TRUE(logCapture.contains("channel closed but state is Connected"));
}

#if !defined(NDEBUG)
namespace {

// Death-test child hygiene (Windows/MSVC debug CRT only): without this,
// abort() raises an invisible modal error dialog inside the re-spawned
// child process and the death test hangs forever. Routes CRT reports to
// the debugger stream and drops the WER/abort popup. No-op elsewhere.
void suppressFatalDialogsForDeathTest()
{
#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    _set_abort_behavior(0, _CALL_REPORTFAULT);
#else
    (void)0;
#endif
}

} // namespace

// Negative test: once ownership is claimed (as ModbusWorker does on
// its thread), a session-driving call from a foreign thread must trip the
// Debug affinity guard (Q_ASSERT → qFatal → process death). Runs as a death
// test so the fatal exit happens in a re-spawned child process, not the test
// binary. The match regex is ".*" because Qt routes the qFatal text through
// its message handler, which may not reach the captured stderr pipe before
// abort() — the death itself is the assertion.
TEST_F(ModbusStateTest, SessionCallFromForeignThreadAfterClaimTripsDebugAssert) {
    EXPECT_DEATH({
        suppressFatalDialogsForDeathTest();
        client_->claimSessionOwnershipForCurrentThread(); // claim on this thread
        std::thread foreign([this]() {
            client_->connect(); // foreign thread → affinity assert fires
        });
        foreign.join();
    }, ".*");
}
#endif
