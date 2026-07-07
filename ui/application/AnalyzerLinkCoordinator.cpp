#include "application/AnalyzerLinkCoordinator.h"

#include "views/modbus/ModbusPage.h"
#include "widgets/FrameAnalyzerWidget.h"

namespace ui::application {

AnalyzerLinkCoordinator::AnalyzerLinkCoordinator(QObject* parent)
    : QObject(parent) {}

AnalyzerLinkCoordinator::~AnalyzerLinkCoordinator() = default;

void AnalyzerLinkCoordinator::bind(views::modbus::ModbusPage* modbusView,
                                   widgets::FrameAnalyzerWidget* frameAnalyzer) {
    modbusView_ = modbusView;
    frameAnalyzer_ = frameAnalyzer;
    applyState(State{});

    QObject::connect(modbusView, &views::modbus::ModbusPage::linkageToggled,
                     this, &AnalyzerLinkCoordinator::handleLinkageToggled);
    QObject::connect(modbusView, &views::modbus::ModbusPage::linkageDataReceived,
                     this, &AnalyzerLinkCoordinator::handleLinkageData);
    QObject::connect(modbusView, &views::modbus::ModbusPage::linkageSourceDisconnected,
                     this, static_cast<void (AnalyzerLinkCoordinator::*)()>(&AnalyzerLinkCoordinator::handleSourceDisconnected));
    QObject::connect(frameAnalyzer, &widgets::FrameAnalyzerWidget::linkagePauseToggled,
                     this, &AnalyzerLinkCoordinator::handleLinkagePauseToggled);
    QObject::connect(frameAnalyzer, &widgets::FrameAnalyzerWidget::linkageStopRequested,
                     this, &AnalyzerLinkCoordinator::handleLinkageStopRequested);
}

AnalyzerLinkCoordinator::State AnalyzerLinkCoordinator::state() const {
    return state_;
}

// --- Signal handlers ---

void AnalyzerLinkCoordinator::handleLinkageToggled(bool active) {
    const LinkSource source = modbusView_
        ? sourceFromMode(modbusView_->currentMode())
        : LinkSource::None;
    requestLinkToggle(source, active);
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

void AnalyzerLinkCoordinator::handleSourceDisconnected() {
    const LinkSource source = modbusView_
        ? sourceFromMode(modbusView_->currentMode())
        : LinkSource::None;
    handleSourceDisconnected(source);
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
    case ::modbus::core::parser::ProtocolType::Ascii:
        return LinkSource::Ascii;
    default:
        return LinkSource::None;
    }
}

AnalyzerLinkCoordinator::LinkSource AnalyzerLinkCoordinator::sourceFromMode(::ui::application::modbus::SessionMode mode) {
    switch (mode) {
    case ::ui::application::modbus::SessionMode::Tcp:
        return LinkSource::Tcp;
    case ::ui::application::modbus::SessionMode::Rtu:
        return LinkSource::Rtu;
    case ::ui::application::modbus::SessionMode::Ascii:
        return LinkSource::Ascii;
    }
    return LinkSource::None;
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
    const bool linked = state_.state != LinkState::Idle;
    const bool paused = state_.state == LinkState::Paused;

    if (modbusView_) {
        modbusView_->setLinked(linked);
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
