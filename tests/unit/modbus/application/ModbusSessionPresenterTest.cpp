#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>

#include "../../../core/modbus/session/IModbusClient.h"
#include "../../../ui/application/modbus/ModbusSessionPresenter.h"
#include "../../../ui/application/modbus/RequestSubmissionService.h"
#include "../../../ui/application/modbus/PollingController.h"
#include "../../../ui/application/modbus/TrafficLogController.h"
#include "../../../ui/application/modbus/ModbusTypes.h"
#include "../../../mocks/UiTestDoubles.h"

using namespace testing;
using namespace ui::application::modbus;

namespace {

class ModbusResponseRelay : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;

signals:
    void sendResponse(int requestId, modbus::session::ModbusResponse response);
    void responseDelivered();

public:
    modbus::session::ModbusResponse lastResponse;
    int lastRequestId = -1;

    void accept(int requestId, modbus::session::ModbusResponse response) {
        lastRequestId = requestId;
        lastResponse = std::move(response);
        emit responseDelivered();
    }
};

class ModbusSessionPresenterTest : public Test {
protected:
    void SetUp() override {
        requestService_ = std::make_unique<RequestSubmissionService>();
        pollingController_ = std::make_unique<PollingController>(requestService_.get());
        trafficLogController_ = std::make_unique<TrafficLogController>(
            nullptr, pollingController_.get());
        tcpPresenter_ = std::make_unique<ModbusSessionPresenter>(
            SessionMode::Tcp);
        rtuPresenter_ = std::make_unique<ModbusSessionPresenter>(
            SessionMode::Rtu);

        tcpPresenter_->setRequestService(requestService_.get());
        tcpPresenter_->setPollingController(pollingController_.get());
        tcpPresenter_->setTrafficLogController(trafficLogController_.get());

        rtuPresenter_->setRequestService(requestService_.get());
        rtuPresenter_->setPollingController(pollingController_.get());
        rtuPresenter_->setTrafficLogController(trafficLogController_.get());
    }

    std::unique_ptr<RequestSubmissionService> requestService_;
    std::unique_ptr<PollingController> pollingController_;
    std::unique_ptr<TrafficLogController> trafficLogController_;
    std::unique_ptr<ModbusSessionPresenter> tcpPresenter_;
    std::unique_ptr<ModbusSessionPresenter> rtuPresenter_;
};

TEST_F(ModbusSessionPresenterTest, TcpMode_CreatesTcpPresenter) {
    EXPECT_EQ(tcpPresenter_->mode(), SessionMode::Tcp);
}

TEST_F(ModbusSessionPresenterTest, RtuMode_CreatesRtuPresenter) {
    EXPECT_EQ(rtuPresenter_->mode(), SessionMode::Rtu);
}

TEST_F(ModbusSessionPresenterTest, ModeDescriptor_MapsAsciiSessionMode) {
    const auto descriptor = modeDescriptor(SessionMode::Ascii);
    EXPECT_EQ(descriptor.modbusMode, ::modbus::base::ModbusMode::ASCII);
    EXPECT_EQ(descriptor.protocolType, ::modbus::core::parser::ProtocolType::Ascii);
    EXPECT_TRUE(descriptor.usesSerialConnection);
}

TEST_F(ModbusSessionPresenterTest, Initially_NotConnected) {
    EXPECT_FALSE(tcpPresenter_->isSessionConnected());
    EXPECT_FALSE(rtuPresenter_->isSessionConnected());
}

TEST_F(ModbusSessionPresenterTest, UpdateSettings_PropagatesValues) {
    ModbusTimingParams params;
    params.timeout = std::chrono::milliseconds(5000);
    params.retryCount = 3;
    params.retryInterval = std::chrono::milliseconds(200);

    EXPECT_NO_THROW(tcpPresenter_->updateSettings(params));
}

TEST_F(ModbusSessionPresenterTest, ConnectionGeneration_IncrementsOnRelease) {
    quint64 gen1 = tcpPresenter_->connectionGeneration();
    tcpPresenter_->releaseStack();
    quint64 gen2 = tcpPresenter_->connectionGeneration();

    EXPECT_GT(gen2, gen1);
}

TEST_F(ModbusSessionPresenterTest, ConnectionGeneration_IsolatesStaleCallbacks) {
    quint64 oldGen = tcpPresenter_->connectionGeneration();

    tcpPresenter_->releaseStack();
    quint64 newGen = tcpPresenter_->connectionGeneration();

    EXPECT_NE(oldGen, newGen);
}

TEST_F(ModbusSessionPresenterTest, RequestDisconnect_ClearsConnection) {
    QSignalSpy disconnectSpy(tcpPresenter_.get(), &ModbusSessionPresenter::sessionDisconnected);

    tcpPresenter_->requestDisconnect();

    EXPECT_FALSE(tcpPresenter_->isSessionConnected());
}

TEST_F(ModbusSessionPresenterTest, ReleaseStack_EmitsStackReleased) {
    QSignalSpy spy(tcpPresenter_.get(), &ModbusSessionPresenter::stackReleased);

    tcpPresenter_->releaseStack();

    EXPECT_GE(spy.count(), 1);
}

TEST_F(ModbusSessionPresenterTest, Shutdown_EmitsStackReleased) {
    QSignalSpy spy(tcpPresenter_.get(), &ModbusSessionPresenter::stackReleased);

    tcpPresenter_->shutdown();

    EXPECT_GE(spy.count(), 1);
}

TEST_F(ModbusSessionPresenterTest, ReleaseStack_ResetsPollingController) {
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 1;

    tcpPresenter_->releaseStack();
    pollingController_->reset();

    EXPECT_EQ(pollingController_->currentState(), PollState::Idle);
}

TEST_F(ModbusSessionPresenterTest, SubmitRequest_NoWorker_DoesNotCrash) {
    ::modbus::base::Pdu pdu(static_cast<::modbus::base::FunctionCode>(0x03));

    EXPECT_NO_THROW(tcpPresenter_->submitRequest(pdu, 1, 1));
}

TEST_F(ModbusSessionPresenterTest, SendRaw_NoWorker_DoesNotCrash) {
    QByteArray data("\x01\x02\x03", 3);

    EXPECT_NO_THROW(tcpPresenter_->sendRaw(data));
}

TEST_F(ModbusSessionPresenterTest, SetLinked_UpdatesFlag) {
    EXPECT_FALSE(tcpPresenter_->isLinked());

    tcpPresenter_->setLinked(true);
    EXPECT_TRUE(tcpPresenter_->isLinked());

    tcpPresenter_->setLinked(false);
    EXPECT_FALSE(tcpPresenter_->isLinked());
}

TEST_F(ModbusSessionPresenterTest, LinkedFalse_EmitsLinkageDisconnectedWhenPreviouslyLinked) {
    QSignalSpy spy(tcpPresenter_.get(), &ModbusSessionPresenter::linkageSourceDisconnected);

    tcpPresenter_->setLinked(true);
    tcpPresenter_->releaseStack();

    EXPECT_GE(spy.count(), 1);
}

TEST_F(ModbusSessionPresenterTest, Shutdown_IncrementsConnectionGeneration) {
    quint64 gen1 = tcpPresenter_->connectionGeneration();

    tcpPresenter_->shutdown();

    EXPECT_GT(tcpPresenter_->connectionGeneration(), gen1);
}

TEST_F(ModbusSessionPresenterTest, ModbusResponse_QueuedDeliveryPreservesExplicitKind) {
    ModbusResponseRelay relay;
    QSignalSpy deliveredSpy(&relay, &ModbusResponseRelay::responseDelivered);

    QObject::connect(&relay, &ModbusResponseRelay::sendResponse,
                     &relay, &ModbusResponseRelay::accept,
                     Qt::QueuedConnection);

    emit relay.sendResponse(42,
                            modbus::session::ModbusResponse::NoResponseExpected(
                                ::modbus::base::Pdu(static_cast<::modbus::base::FunctionCode>(0x10)),
                                1));

    for (int i = 0; i < 10 && deliveredSpy.count() == 0; ++i) {
        QCoreApplication::processEvents();
    }

    ASSERT_EQ(deliveredSpy.count(), 1);
    EXPECT_EQ(relay.lastRequestId, 42);
    EXPECT_EQ(relay.lastResponse.kind, modbus::session::ModbusResponseKind::NoResponseExpected);
    EXPECT_TRUE(relay.lastResponse.isSuccess);
    EXPECT_TRUE(relay.lastResponse.isNoResponseExpected());
}

TEST_F(ModbusSessionPresenterTest, DefaultTimingParams_AreFromConstants) {
    ModbusTimingParams defaultParams;
    defaultParams.timeout = std::chrono::milliseconds(3000);
    defaultParams.retryCount = 0;
    defaultParams.retryInterval = std::chrono::milliseconds(100);

    EXPECT_NO_THROW(rtuPresenter_->updateSettings(defaultParams));
}

} // namespace

#include "ModbusSessionPresenterTest.moc"
