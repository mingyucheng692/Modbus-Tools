#include "application/AnalyzerLinkCoordinator.h"

#include "views/modbus_tcp/ModbusTcpPage.h"
#include "views/modbus_rtu/ModbusRtuPage.h"
#include "widgets/FrameAnalyzerWidget.h"

namespace ui::application {

AnalyzerLinkCoordinator::AnalyzerLinkCoordinator(QObject* parent)
    : QObject(parent) {}

AnalyzerLinkCoordinator::~AnalyzerLinkCoordinator() = default;

void AnalyzerLinkCoordinator::bind(views::modbus_tcp::ModbusTcpPage* tcpView,
                                   views::modbus_rtu::ModbusRtuPage* rtuView,
                                   widgets::FrameAnalyzerWidget* frameAnalyzer) {
    tcpView_ = tcpView;
    rtuView_ = rtuView;
    frameAnalyzer_ = frameAnalyzer;
    applyState(State{});

    QObject::connect(tcpView, &views::modbus_tcp::ModbusTcpPage::linkageToggled,
                     this, &AnalyzerLinkCoordinator::handleTcpLinkageToggled);
    QObject::connect(rtuView, &views::modbus_rtu::ModbusRtuPage::linkageToggled,
                     this, &AnalyzerLinkCoordinator::handleRtuLinkageToggled);
    QObject::connect(tcpView, &views::modbus_tcp::ModbusTcpPage::linkageDataReceived,
                     this, &AnalyzerLinkCoordinator::handleLinkageData);
    QObject::connect(rtuView, &views::modbus_rtu::ModbusRtuPage::linkageDataReceived,
                     this, &AnalyzerLinkCoordinator::handleLinkageData);
    QObject::connect(tcpView, &views::modbus_tcp::ModbusTcpPage::linkageSourceDisconnected,
                     this, &AnalyzerLinkCoordinator::handleTcpSourceDisconnected);
    QObject::connect(rtuView, &views::modbus_rtu::ModbusRtuPage::linkageSourceDisconnected,
                     this, &AnalyzerLinkCoordinator::handleRtuSourceDisconnected);
    QObject::connect(frameAnalyzer, &widgets::FrameAnalyzerWidget::linkagePauseToggled,
                     this, &AnalyzerLinkCoordinator::handleLinkagePauseToggled);
    QObject::connect(frameAnalyzer, &widgets::FrameAnalyzerWidget::linkageStopRequested,
                     this, &AnalyzerLinkCoordinator::handleLinkageStopRequested);
}

AnalyzerLinkCoordinator::State AnalyzerLinkCoordinator::state() const {
    return state_;
}

// --- Signal handlers ---

void AnalyzerLinkCoordinator::handleTcpLinkageToggled(bool active) {
    requestLinkToggle(LinkSource::Tcp, active);
}

void AnalyzerLinkCoordinator::handleRtuLinkageToggled(bool active) {
    requestLinkToggle(LinkSource::Rtu, active);
}

void AnalyzerLinkCoordinator::handleLinkageStopRequested() {
    requestStop();
}

void AnalyzerLinkCoordinator::handleLinkagePauseToggled(bool paused) {
    requestPause(paused);
}

void AnalyzerLinkCoordinator::handleLinkageData(const ::modbus::base::Pdu& pdu,
                                                ::modbus::core::parser::ProtocolType protocol,
                                                uint16_t addr) {
    handleLiveData(pdu, protocol, addr);
}

void AnalyzerLinkCoordinator::handleTcpSourceDisconnected() {
    handleSourceDisconnected(LinkSource::Tcp);
}

void AnalyzerLinkCoordinator::handleRtuSourceDisconnected() {
    handleSourceDisconnected(LinkSource::Rtu);
}

// --- State machine core ---

void AnalyzerLinkCoordinator::requestLinkToggle(LinkSource source, bool active) {
    if (!active) {
        if (state_.source == source) {
            stopInternal();
        }
        return;
    }

    transitionTo(LinkState::Live, source);
}

void AnalyzerLinkCoordinator::requestStop() {
    stopInternal();
}

void AnalyzerLinkCoordinator::requestPause(bool paused) {
    if (state_.state == LinkState::Idle || state_.source == LinkSource::None) {
        return;
    }

    transitionTo(paused ? LinkState::Paused : LinkState::Live, state_.source);
    if (!paused) {
        replayBufferedLiveDataIfNeeded();
    }
}

void AnalyzerLinkCoordinator::handleLiveData(const ::modbus::base::Pdu& pdu,
                                             ::modbus::core::parser::ProtocolType protocol,
                                             uint16_t addr) {
    const LinkSource dataSource = sourceFromProtocol(protocol);
    if (state_.state == LinkState::Idle || state_.source == LinkSource::None || dataSource != state_.source) {
        return;
    }

    if (state_.state == LinkState::Paused) {
        bufferedLiveData_ = BufferedLiveData{pdu, protocol, addr};
        return;
    }

    if (frameAnalyzer_) {
        frameAnalyzer_->processLivePdu(pdu, protocol, addr);
        frameAnalyzer_->setLivePaused(false);
    }
    clearBufferedLiveData();
}

void AnalyzerLinkCoordinator::handleSourceDisconnected(LinkSource source) {
    if (state_.source == source) {
        stopInternal();
    }
}

AnalyzerLinkCoordinator::LinkSource AnalyzerLinkCoordinator::sourceFromProtocol(::modbus::core::parser::ProtocolType protocol) {
    switch (protocol) {
    case ::modbus::core::parser::ProtocolType::Tcp:
        return LinkSource::Tcp;
    case ::modbus::core::parser::ProtocolType::Rtu:
        return LinkSource::Rtu;
    default:
        return LinkSource::None;
    }
}

void AnalyzerLinkCoordinator::transitionTo(LinkState state, LinkSource source) {
    const State previousState = state_;
    state_.state = state;
    state_.source = source;
    if (state_.state == LinkState::Idle || previousState.source != state_.source) {
        clearBufferedLiveData();
    }
    applyState(previousState);
}

void AnalyzerLinkCoordinator::applyState(const State& previousState) {
    const bool tcpLinked = state_.source == LinkSource::Tcp && state_.state != LinkState::Idle;
    const bool rtuLinked = state_.source == LinkSource::Rtu && state_.state != LinkState::Idle;
    const bool paused = state_.state == LinkState::Paused;

    if (tcpView_) {
        tcpView_->setLinked(tcpLinked);
    }
    if (rtuView_) {
        rtuView_->setLinked(rtuLinked);
    }

    if (!frameAnalyzer_) {
        return;
    }

    const bool sourceChanged = previousState.source != LinkSource::None && previousState.source != state_.source;
    if (sourceChanged || state_.state == LinkState::Idle) {
        frameAnalyzer_->exitLiveMode();
    }

    if (state_.state == LinkState::Idle) {
        return;
    }

    frameAnalyzer_->setLivePaused(paused);
}

void AnalyzerLinkCoordinator::clearBufferedLiveData() {
    bufferedLiveData_.reset();
}

void AnalyzerLinkCoordinator::replayBufferedLiveDataIfNeeded() {
    if (!bufferedLiveData_ || !frameAnalyzer_ || state_.state != LinkState::Live) {
        return;
    }

    if (sourceFromProtocol(bufferedLiveData_->protocol) != state_.source) {
        clearBufferedLiveData();
        return;
    }

    frameAnalyzer_->processLivePdu(bufferedLiveData_->pdu, bufferedLiveData_->protocol, bufferedLiveData_->addr);
    frameAnalyzer_->setLivePaused(false);
    clearBufferedLiveData();
}

void AnalyzerLinkCoordinator::stopInternal() {
    if (state_.state == LinkState::Idle && state_.source == LinkSource::None) {
        applyState(state_);
        return;
    }

    transitionTo(LinkState::Idle, LinkSource::None);
}

} // namespace ui::application
