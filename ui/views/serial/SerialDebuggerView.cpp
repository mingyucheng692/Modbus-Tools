// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "SerialDebuggerView.h"
#include "Config.h"
#include "core/common/SettingsKeys.h"
#include "infra/config/ISettingsService.h"
#include "../../common/ConnectionAlert.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/ByteMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../../infra/io/ChannelOperationWorker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMetaObject>
#include <QCheckBox>
#include <QEvent>
#include <QSignalBlocker>
#include <spdlog/spdlog.h>

namespace ui::views::serial {

namespace {

QString channelStateText(io::ChannelState state) {
    switch (state) {
    case io::ChannelState::Closed:
        return SerialDebuggerView::tr("Closed");
    case io::ChannelState::Opening:
        return SerialDebuggerView::tr("Opening");
    case io::ChannelState::Open:
        return SerialDebuggerView::tr("Open");
    case io::ChannelState::Closing:
        return SerialDebuggerView::tr("Closing");
    case io::ChannelState::Error:
        return SerialDebuggerView::tr("Error");
    default:
        return SerialDebuggerView::tr("Unknown");
    }
}

} // namespace

SerialDebuggerView::SerialDebuggerView(infra::config::ISettingsService* settingsService, QWidget *parent)
    : GenericChannelViewBase(settingsService, parent),
      channelCtrl_(this),
      disconnectAlertHandler_([this]() { ui::common::connection_alert::showDisconnected(this); }) {
    channelController_ = &channelCtrl_;
    setupUi();
    startWorker();
}

void SerialDebuggerView::setDisconnectAlertHandler(DisconnectAlertHandler handler) noexcept {
    disconnectAlertHandler_ = std::move(handler);
}

SerialDebuggerView::~SerialDebuggerView() noexcept {
    // channelCtrl_ is a value member, destroyed automatically.
    // channelController_ pointer in base class is already null when this destructor runs.
}

void SerialDebuggerView::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(4);

    // 1. Connection Section (Top)
    auto topLayout = new QHBoxLayout();
    connectionWidget_ = new widgets::SerialConnectionWidget(settingsService_, this);
    connectionWidget_->setSettingsGroup(QStringLiteral("serial_debugger"));
    topLayout->addWidget(connectionWidget_);

    // Serial Controls (DTR/RTS)
    controlGroup_ = new QGroupBox(this);
    auto controlLayout = new QHBoxLayout(controlGroup_);
    controlLayout->setContentsMargins(5, 5, 5, 5);

    dtrCheck_ = new QCheckBox(this);
    rtsCheck_ = new QCheckBox(this);
    controlLayout->addWidget(dtrCheck_);
    controlLayout->addWidget(rtsCheck_);

    topLayout->addWidget(controlGroup_);
    topLayout->addStretch();

    mainLayout->addLayout(topLayout);

    // 2. Central Area (Traffic Monitor)
    monitor_ = new widgets::ByteMonitorWidget(settingsService_, this);
    monitor_->setSettingsGroup(QStringLiteral("serial_debugger/traffic"));
    mainLayout->addWidget(monitor_);

    // 3. Input Section (Bottom)
    inputSection_ = new widgets::CollapsibleSection(settingsService_, this);
    inputSection_->setSettingsKey(QStringLiteral("serial_debugger/ui/inputCollapsed"));
    auto inputLayout = new QVBoxLayout(inputSection_->contentWidget());
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputWidget_ = new widgets::GenericInputWidget(settingsService_, inputSection_->contentWidget());
    inputWidget_->setSettingsGroup(QStringLiteral("serial_debugger/input"));
    inputLayout->addWidget(inputWidget_);
    mainLayout->addWidget(inputSection_);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);
    mainLayout->setStretch(2, 0);

    // Connections
    connect(connectionWidget_, &widgets::SerialConnectionWidget::connectClicked,
            this, &SerialDebuggerView::onConnectClicked);
    connect(connectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked,
            this, &SerialDebuggerView::onDisconnectClicked);

    bindInputWidget(inputWidget_);

    connect(dtrCheck_, &QCheckBox::toggled, this, &SerialDebuggerView::onDtrChanged);
    connect(rtsCheck_, &QCheckBox::toggled, this, &SerialDebuggerView::onRtsChanged);

    // Disable controls initially
    dtrCheck_->setEnabled(false);
    rtsCheck_->setEnabled(false);
    if (settingsService_) {
        const QSignalBlocker dtrBlocker(dtrCheck_);
        const QSignalBlocker rtsBlocker(rtsCheck_);
        dtrCheck_->setChecked(
            settingsService_->value(core::common::settings_keys::kSerialDebuggerDtr).toBool());
        rtsCheck_->setChecked(
            settingsService_->value(core::common::settings_keys::kSerialDebuggerRts).toBool());
    }

    retranslateUi();
}

void SerialDebuggerView::startWorker() {
    auto* worker = channelCtrl_.createWorker();
    connect(worker, &io::ChannelOperationWorker::channelErrorOccurred,
            this, &SerialDebuggerView::onWorkerError);
    connect(worker, &io::ChannelOperationWorker::monitor,
            this, &SerialDebuggerView::onWorkerMonitor);
    connect(worker, &io::ChannelOperationWorker::stateChangedWithGeneration,
            this, &SerialDebuggerView::onWorkerStateChanged);
}

void SerialDebuggerView::onConnectClicked(const io::SerialConfig& config) {
    auto* worker = channelCtrl_.worker();
    if (!worker) return;

    manualDisconnectRequested_ = false;
    const quint64 generation = ++connectionGeneration_;

    SPDLOG_INFO("SerialDebugger: Connecting to {}", config.portName.toStdString());
    if (monitor_) {
        monitor_->appendInfo(tr("Opening %1...").arg(config.portName));
    }
    connectionWidget_->setDisplayState(widgets::SerialConnectionWidget::DisplayState::Connecting);

    QMetaObject::invokeMethod(worker, "openSerial",
                              Qt::QueuedConnection,
                              Q_ARG(io::SerialConfig, config),
                              Q_ARG(quint64, generation));
}

void SerialDebuggerView::applyConnectionWidgetState(io::ChannelState state) {
    if (!connectionWidget_) {
        return;
    }
    switch (state) {
    case io::ChannelState::Opening:
        connectionWidget_->setDisplayState(widgets::SerialConnectionWidget::DisplayState::Connecting);
        break;
    case io::ChannelState::Open:
        connectionWidget_->setConnected(true);
        break;
    case io::ChannelState::Closing:
        connectionWidget_->setDisplayState(widgets::SerialConnectionWidget::DisplayState::Disconnecting);
        break;
    case io::ChannelState::Closed:
    case io::ChannelState::Error:
        connectionWidget_->setConnected(false);
        break;
    }
}

void SerialDebuggerView::applySerialControlLines() {
    const bool dtr = dtrCheck_ && dtrCheck_->isChecked();
    const bool rts = rtsCheck_ && rtsCheck_->isChecked();
    if (auto* worker = channelCtrl_.worker()) {
        QMetaObject::invokeMethod(worker, "setDtr", Qt::QueuedConnection, Q_ARG(bool, dtr));
        QMetaObject::invokeMethod(worker, "setRts", Qt::QueuedConnection, Q_ARG(bool, rts));
    }
    emit serialControlLinesApplied(dtr, rts);
}

void SerialDebuggerView::onWorkerStateChanged(io::ChannelState state, quint64 generation) {
    if (generation != connectionGeneration_) {
        return;
    }

    const bool wasConnected = isConnected_;
    isConnected_ = (state == io::ChannelState::Open);
    if (!isConnected_ && channelController_ && channelController_->isAutoSendActive()) {
        channelController_->stopAutoSend();
        if (inputWidget_) {
            inputWidget_->setAutoSendEnabled(false);
        }
    }
    dtrCheck_->setEnabled(isConnected_);
    rtsCheck_->setEnabled(isConnected_);
    applyConnectionWidgetState(state);

    if (monitor_) {
        monitor_->appendInfo(tr("State changed: %1").arg(channelStateText(state)));
    }

    if (isConnected_) {
        manualDisconnectRequested_ = false;
        applySerialControlLines();
        return;
    }

    const bool isPassiveDisconnect = wasConnected
        && !manualDisconnectRequested_
        && (state == io::ChannelState::Closed || state == io::ChannelState::Error);
    if (isPassiveDisconnect) {
        emit unexpectedDisconnectDetected();
        if (disconnectAlertHandler_) {
            disconnectAlertHandler_();
        }
    }
}

void SerialDebuggerView::onWorkerError(const QString& deviceHint, io::ChannelErrorCode code, const QString& error) {
    if (monitor_) {
        QString localizedMsg;
        switch (code) {
            case io::ChannelErrorCode::ConnectionFailed: localizedMsg = tr("Connection failed"); break;
            case io::ChannelErrorCode::Timeout: localizedMsg = tr("Connection timeout"); break;
            case io::ChannelErrorCode::WriteFailed: localizedMsg = tr("Write failed"); break;
            case io::ChannelErrorCode::ReadFailed: localizedMsg = tr("Read failed"); break;
            case io::ChannelErrorCode::PortNotFound: localizedMsg = tr("Port not found"); break;
            case io::ChannelErrorCode::PermissionDenied: localizedMsg = tr("Permission denied"); break;
            case io::ChannelErrorCode::ConnectionReset: localizedMsg = tr("Connection reset"); break;
            default: localizedMsg = tr("Unknown error"); break;
        }
        const QString displayMsg = (error.isEmpty() || error == localizedMsg)
            ? localizedMsg
            : QStringLiteral("%1 (%2)").arg(localizedMsg, error);
        monitor_->appendError(tr("Error: %1").arg(displayMsg));
    }
    const QString hint = deviceHint.isEmpty() ? QStringLiteral("Serial Worker") : deviceHint;
    SPDLOG_ERROR("{} Error (code={}): {}", hint.toStdString(), static_cast<int>(code), error.toStdString());
}

void SerialDebuggerView::onWorkerMonitor(bool isTx, const QByteArray& data) {
    if (monitor_) {
        monitor_->appendMessage(isTx, data);
    }
}

void SerialDebuggerView::onDtrChanged(bool checked) {
    if (settingsService_) {
        settingsService_->setValue(core::common::settings_keys::kSerialDebuggerDtr, checked);
    }
    auto* worker = channelCtrl_.worker();
    if (!worker) return;
    QMetaObject::invokeMethod(worker, "setDtr",
                              Qt::QueuedConnection,
                              Q_ARG(bool, checked));
}

void SerialDebuggerView::onRtsChanged(bool checked) {
    if (settingsService_) {
        settingsService_->setValue(core::common::settings_keys::kSerialDebuggerRts, checked);
    }
    auto* worker = channelCtrl_.worker();
    if (!worker) return;
    QMetaObject::invokeMethod(worker, "setRts",
                              Qt::QueuedConnection,
                              Q_ARG(bool, checked));
}

void SerialDebuggerView::retranslateUi() {
    if (controlGroup_) controlGroup_->setTitle(tr("Control"));
    if (inputSection_) inputSection_->setTitle(tr("Send Data"));
    if (dtrCheck_) dtrCheck_->setText(tr("DTR"));
    if (rtsCheck_) rtsCheck_->setText(tr("RTS"));
}


} // namespace ui::views::serial
