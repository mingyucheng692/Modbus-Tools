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
    applyState();

    QObject::connect(modbusView, &views::modbus::ModbusPage::linkageToggled,
                     this, &AnalyzerLinkCoordinator::handleLinkageToggled);
    QObject::connect(modbusView, &views::modbus::ModbusPage::linkageDataReceived,
                     this, &AnalyzerLinkCoordinator::handleLinkageData);
    QObject::connect(modbusView, &views::modbus::ModbusPage::linkageSourceDisconnected,
                     this, &AnalyzerLinkCoordinator::handleSourceDisconnected);
    QObject::connect(frameAnalyzer, &widgets::FrameAnalyzerWidget::linkagePauseToggled,
                     this, &AnalyzerLinkCoordinator::handleLinkagePauseToggled);
    QObject::connect(frameAnalyzer, &widgets::FrameAnalyzerWidget::linkageStopRequested,
                     this, &AnalyzerLinkCoordinator::handleLinkageStopRequested);
}

AnalyzerLinkCoordinator::LinkState AnalyzerLinkCoordinator::state() const {
    return state_;
}

// --- Signal handlers ---

void AnalyzerLinkCoordinator::handleLinkageToggled(bool active) {
    requestLinkToggle(active);
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
    if (state_ == LinkState::Idle) {
        return;
    }
    stopInternal();
}

// --- State machine core ---

void AnalyzerLinkCoordinator::requestLinkToggle(bool active) {
    if (!active) {
        stopInternal();
        return;
    }
    transitionTo(LinkState::Live);
}

void AnalyzerLinkCoordinator::requestStop() {
    stopInternal();
}

void AnalyzerLinkCoordinator::requestPause(bool paused) {
    if (state_ == LinkState::Idle) {
        return;
    }

    transitionTo(paused ? LinkState::Paused : LinkState::Live);
    if (!paused) {
        replayBufferedLiveDataIfNeeded();
    }
}

void AnalyzerLinkCoordinator::handleLiveData(const ::modbus::base::Pdu& pdu,
                                             ::modbus::core::parser::ProtocolType protocol,
                                             uint16_t addr) {
    if (state_ == LinkState::Idle) {
        return;
    }

    if (state_ == LinkState::Paused) {
        bufferedLiveData_ = BufferedLiveData{pdu, protocol, addr};
        return;
    }

    if (frameAnalyzer_) {
        frameAnalyzer_->processLivePdu(pdu, protocol, addr);
        frameAnalyzer_->setLivePaused(false);
    }
    clearBufferedLiveData();
}

void AnalyzerLinkCoordinator::transitionTo(LinkState state) {
    state_ = state;
    if (state_ == LinkState::Idle) {
        clearBufferedLiveData();
    }
    applyState();
}

void AnalyzerLinkCoordinator::applyState() {
    const bool linked = state_ != LinkState::Idle;
    const bool paused = state_ == LinkState::Paused;

    if (modbusView_) {
        modbusView_->setLinked(linked);
    }

    if (!frameAnalyzer_) {
        return;
    }

    if (state_ == LinkState::Idle) {
        frameAnalyzer_->exitLiveMode();
        return;
    }

    frameAnalyzer_->setLivePaused(paused);
}

void AnalyzerLinkCoordinator::clearBufferedLiveData() {
    bufferedLiveData_.reset();
}

void AnalyzerLinkCoordinator::replayBufferedLiveDataIfNeeded() {
    if (!bufferedLiveData_ || !frameAnalyzer_ || state_ != LinkState::Live) {
        return;
    }

    frameAnalyzer_->processLivePdu(bufferedLiveData_->pdu, bufferedLiveData_->protocol, bufferedLiveData_->addr);
    frameAnalyzer_->setLivePaused(false);
    clearBufferedLiveData();
}

void AnalyzerLinkCoordinator::stopInternal() {
    if (state_ == LinkState::Idle) {
        applyState();
        return;
    }
    transitionTo(LinkState::Idle);
}

} // namespace ui::application
