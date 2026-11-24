// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "SerialDebuggerView.h"
#include "core/common/SettingsKeys.h"
#include "infra/config/ISettingsService.h"
#include "../../common/ConnectionAlert.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/ByteMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QEvent>
#include <QSignalBlocker>

namespace ui::views::serial {

SerialDebuggerView::SerialDebuggerView(infra::config::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent)
    , sessionCtrl_(this)
    , disconnectAlertHandler_([this]() { ui::common::connection_alert::showDisconnected(this); })
    , settingsService_(settingsService)
{
    setupUi();
}

SerialDebuggerView::~SerialDebuggerView() noexcept = default;

void SerialDebuggerView::setDisconnectAlertHandler(DisconnectAlertHandler handler) noexcept {
    disconnectAlertHandler_ = std::move(handler);
}

bool SerialDebuggerView::isConnected() const noexcept {
    return sessionCtrl_.state() == io::ChannelState::Open;
}

void SerialDebuggerView::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(4);

    // 1. Connection Section & Serial Controls Section (Top Bar)
    auto* topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(4);

    connectionWidget_ = new widgets::SerialConnectionWidget(settingsService_, this);
    connectionWidget_->setSettingsGroup(QStringLiteral("serial_debugger"));
    topLayout->addWidget(connectionWidget_, 1);

    controlSection_ = new widgets::CollapsibleSection(settingsService_, this);
    controlSection_->setCollapsible(false);
    auto* controlLayout = new QHBoxLayout(controlSection_->contentWidget());
    controlLayout->setContentsMargins(4, 2, 4, 2);
    controlLayout->setSpacing(6);

    dtrCheck_ = new QCheckBox(this);
    rtsCheck_ = new QCheckBox(this);
    controlLayout->addWidget(dtrCheck_);
    controlLayout->addWidget(rtsCheck_);

    topLayout->addWidget(controlSection_, 0);
    mainLayout->addLayout(topLayout);

    // Synchronize collapsible folding between connection settings and control section
    connect(connectionWidget_, &widgets::SerialConnectionWidget::sectionExpandedChanged,
            controlSection_, &widgets::CollapsibleSection::setExpanded);

    // 2. Central Area (Traffic Monitor)
    monitor_ = new widgets::ByteMonitorWidget(settingsService_, this);
    monitor_->setSettingsGroup(QStringLiteral("serial_debugger/traffic"));
    mainLayout->addWidget(monitor_, 1);

    // 3. Input Section (Bottom)
    inputSection_ = new widgets::CollapsibleSection(settingsService_, this);
    inputSection_->setSettingsKey(QStringLiteral("serial_debugger/ui/inputCollapsed"));
    auto* inputLayout = new QVBoxLayout(inputSection_->contentWidget());
    inputLayout->setContentsMargins(0, 0, 0, 0);

    inputWidget_ = new widgets::GenericInputWidget(settingsService_, inputSection_->contentWidget());
    inputWidget_->setSettingsGroup(QStringLiteral("serial_debugger/input"));
    inputLayout->addWidget(inputWidget_);
    mainLayout->addWidget(inputSection_, 0);

    // Initial state: input controls disabled via snapshot
    onSessionSnapshotChanged(sessionCtrl_.currentSnapshot());

    // Connect user interactions
    connect(connectionWidget_, &widgets::SerialConnectionWidget::connectClicked,
            this, &SerialDebuggerView::onConnectClicked);
    connect(connectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked,
            this, &SerialDebuggerView::onDisconnectClicked);

    connect(inputWidget_, &widgets::GenericInputWidget::sendRequested,
            this, [this](const QByteArray& data) { sessionCtrl_.write(data); });
    connect(inputWidget_, &widgets::GenericInputWidget::autoSendToggled,
            this, [this](bool checked, int intervalMs) {
        sessionCtrl_.setAutoSend(checked, intervalMs, [this]() {
            return inputWidget_ ? inputWidget_->getData() : QByteArray();
        });
    });
    connect(inputWidget_, &widgets::GenericInputWidget::autoSendIntervalChanged,
            this, [this](int intervalMs) { sessionCtrl_.setAutoSendInterval(intervalMs); });
    connect(&sessionCtrl_, &application::ChannelSessionController::autoSendStateChanged,
            inputWidget_, &widgets::GenericInputWidget::setAutoSendEnabled);

    // Session controller signals
    connect(&sessionCtrl_, &application::ChannelSessionController::snapshotChanged,
            this, &SerialDebuggerView::onSessionSnapshotChanged);
    connect(&sessionCtrl_, &application::ChannelSessionController::trafficOccurred,
            monitor_, &widgets::ByteMonitorWidget::appendMessage);
    connect(&sessionCtrl_, &application::ChannelSessionController::errorOccurred,
            this, &SerialDebuggerView::onWorkerError);
    connect(&sessionCtrl_, &application::ChannelSessionController::unexpectedDisconnectDetected,
            this, [this]() {
        emit unexpectedDisconnectDetected();
        if (disconnectAlertHandler_) {
            disconnectAlertHandler_();
        }
    });

    connect(dtrCheck_, &QCheckBox::toggled, this, &SerialDebuggerView::onDtrChanged);
    connect(rtsCheck_, &QCheckBox::toggled, this, &SerialDebuggerView::onRtsChanged);

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

void SerialDebuggerView::onConnectClicked(const io::SerialConfig& config) {
    if (monitor_) {
        monitor_->appendInfo(tr("Opening %1...").arg(config.portName));
    }
    sessionCtrl_.openSerial(config);
}

void SerialDebuggerView::onDisconnectClicked() {
    sessionCtrl_.closeChannel();
}

void SerialDebuggerView::onSessionSnapshotChanged(const application::SessionUiSnapshot& snap) {
    if (connectionWidget_) {
        connectionWidget_->applyChannelState(snap.state);
    }
    if (dtrCheck_) {
        dtrCheck_->setEnabled(snap.serialControlLinesEnabled);
    }
    if (rtsCheck_) {
        rtsCheck_->setEnabled(snap.serialControlLinesEnabled);
    }
    if (inputWidget_) {
        inputWidget_->setSendControlsEnabled(snap.sendControlsEnabled, snap.sendDisabledTooltip);
    }

    if (snap.stateChanged && monitor_) {
        monitor_->appendInfo(tr("State changed: %1").arg(snap.stateText));
    }

    if (snap.state == io::ChannelState::Open && snap.stateChanged) {
        applySerialControlLines();
    }
}

void SerialDebuggerView::onWorkerStateChanged(io::ChannelState state, quint64 generation) {
    sessionCtrl_.onWorkerStateChanged(state, generation);
}

void SerialDebuggerView::onWorkerError(const QString& deviceHint, io::ChannelErrorCode code, const QString& error) {
    if (monitor_) {
        const QString displayMsg = application::ChannelSessionController::formatChannelError(code, error);
        monitor_->appendError(tr("Error: %1").arg(displayMsg));
    }
}

void SerialDebuggerView::applySerialControlLines() {
    const bool dtr = dtrCheck_ && dtrCheck_->isChecked();
    const bool rts = rtsCheck_ && rtsCheck_->isChecked();
    sessionCtrl_.setDtr(dtr);
    sessionCtrl_.setRts(rts);
    emit serialControlLinesApplied(dtr, rts);
}

void SerialDebuggerView::onDtrChanged(bool checked) {
    if (settingsService_) {
        settingsService_->setValue(core::common::settings_keys::kSerialDebuggerDtr, checked);
    }
    sessionCtrl_.setDtr(checked);
}

void SerialDebuggerView::onRtsChanged(bool checked) {
    if (settingsService_) {
        settingsService_->setValue(core::common::settings_keys::kSerialDebuggerRts, checked);
    }
    sessionCtrl_.setRts(checked);
}

void SerialDebuggerView::retranslateUi() {
    if (controlSection_) controlSection_->setTitle(tr("Control"));
    if (inputSection_) inputSection_->setTitle(tr("Send Data"));
    if (dtrCheck_) dtrCheck_->setText(tr("DTR"));
    if (rtsCheck_) rtsCheck_->setText(tr("RTS"));
}

void SerialDebuggerView::changeEvent(QEvent* event) {
    if (event && event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::views::serial
