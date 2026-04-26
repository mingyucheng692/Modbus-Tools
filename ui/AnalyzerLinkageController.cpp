#include "AnalyzerLinkageController.h"
#include "views/modbus_tcp/ModbusTcpView.h"
#include "views/modbus_rtu/ModbusRtuView.h"
#include "widgets/FrameAnalyzerWidget.h"

namespace ui {

AnalyzerLinkageController::AnalyzerLinkageController(QObject* parent)
    : QObject(parent) {}

void AnalyzerLinkageController::bind(views::modbus_tcp::ModbusTcpView* tcpView,
                                     views::modbus_rtu::ModbusRtuView* rtuView,
                                     widgets::FrameAnalyzerWidget* frameAnalyzer) {
    tcpView_ = tcpView;
    rtuView_ = rtuView;
    frameAnalyzer_ = frameAnalyzer;
    applyState(State{});
}

AnalyzerLinkageController::State AnalyzerLinkageController::state() const {
    return state_;
}

void AnalyzerLinkageController::requestLinkToggle(LinkSource source, bool active) {
    if (!active) {
        if (state_.source == source) {
            stopInternal();
        }
        return;
    }

    transitionTo(LinkState::Live, source);
}

void AnalyzerLinkageController::requestStop() {
    stopInternal();
}

void AnalyzerLinkageController::requestPause(bool paused) {
    if (state_.state == LinkState::Idle || state_.source == LinkSource::None) {
        return;
    }

    transitionTo(paused ? LinkState::Paused : LinkState::Live, state_.source);
}

void AnalyzerLinkageController::handleLiveData(const modbus::base::Pdu& pdu,
                                               modbus::core::parser::ProtocolType protocol,
                                               uint16_t addr) {
    const LinkSource dataSource = sourceFromProtocol(protocol);
    if (state_.state == LinkState::Idle || state_.source == LinkSource::None || dataSource != state_.source) {
        return;
    }

    if (state_.state == LinkState::Paused) {
        return;
    }

    if (frameAnalyzer_) {
        frameAnalyzer_->processLivePdu(pdu, protocol, addr);
        frameAnalyzer_->setLivePaused(false);
    }
}

void AnalyzerLinkageController::handleSourceDisconnected(LinkSource source) {
    if (state_.source == source) {
        stopInternal();
    }
}

AnalyzerLinkageController::LinkSource AnalyzerLinkageController::sourceFromProtocol(modbus::core::parser::ProtocolType protocol) {
    switch (protocol) {
    case modbus::core::parser::ProtocolType::Tcp:
        return LinkSource::Tcp;
    case modbus::core::parser::ProtocolType::Rtu:
        return LinkSource::Rtu;
    default:
        return LinkSource::None;
    }
}

void AnalyzerLinkageController::transitionTo(LinkState state, LinkSource source) {
    const State previousState = state_;
    state_.state = state;
    state_.source = source;
    applyState(previousState);
}

void AnalyzerLinkageController::applyState(const State& previousState) {
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

void AnalyzerLinkageController::stopInternal() {
    if (state_.state == LinkState::Idle && state_.source == LinkSource::None) {
        applyState(state_);
        return;
    }

    transitionTo(LinkState::Idle, LinkSource::None);
}

} // namespace ui
