// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "SerialPage.h"
#include "core/common/SettingsKeys.h"
#include "infra/config/ISettingsService.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/ByteMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../common/ConnectionAlert.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QEvent>
#include <QSignalBlocker>

namespace ui::views::serial {

SerialPage::SerialPage(infra::config::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService),
      sessionCtrl_(this),
      disconnectAlertHandler_([this]() { ui::common::connection_alert::showDisconnected(this); }) {
    setupUi();
}

SerialPage::~SerialPage() noexcept = default;

void SerialPage::setDisconnectAlertHandler(DisconnectAlertHandler handler) noexcept {
    disconnectAlertHandler_ = std::move(handler);
}

bool SerialPage::isConnected() const noexcept {
    return sessionCtrl_.state() == io::ChannelState::Open;
}

void SerialPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(4);

    // 1. Connection Section & Serial Controls Section (Top Bar)
    auto* topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(4);

    connectionWidget_ = new widgets::SerialConnectionWidget(settingsService_, this);
    connectionWidget_->setSettingsGroup(QStringLiteral("serial"));
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
    monitor_->setSettingsGroup(QStringLiteral("serial/traffic"));
    mainLayout->addWidget(monitor_, 1);

    // 3. Input Section (Bottom)
    inputSection_ = new widgets::CollapsibleSection(settingsService_, this);
    inputSection_->setSettingsKey(QStringLiteral("serial/ui/inputCollapsed"));
    auto* inputLayout = new QVBoxLayout(inputSection_->contentWidget());
    inputLayout->setContentsMargins(0, 0, 0, 0);

    inputWidget_ = new widgets::GenericInputWidget(settingsService_, inputSection_->contentWidget());
    inputWidget_->setSettingsGroup(QStringLiteral("serial/input"));
    inputLayout->addWidget(inputWidget_);
    mainLayout->addWidget(inputSection_, 0);

    // Initial state: input controls disabled via snapshot
    onSessionSnapshotChanged(sessionCtrl_.currentSnapshot());

    // Connect user interactions
    connect(connectionWidget_, &widgets::SerialConnectionWidget::connectClicked,
            this, &SerialPage::onConnectClicked);
    connect(connectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked,
            this, &SerialPage::onDisconnectClicked);

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
            this, &SerialPage::onSessionSnapshotChanged);
    connect(&sessionCtrl_, &application::ChannelSessionController::trafficOccurred,
            monitor_, &widgets::ByteMonitorWidget::appendMessage);
    connect(&sessionCtrl_, &application::ChannelSessionController::errorOccurred,
            this, &SerialPage::onWorkerError);
    connect(&sessionCtrl_, &application::ChannelSessionController::unexpectedDisconnectDetected,
            this, [this]() {
        emit unexpectedDisconnectDetected();
        if (disconnectAlertHandler_) {
            disconnectAlertHandler_();
        }
    });

    connect(dtrCheck_, &QCheckBox::toggled, this, &SerialPage::onDtrChanged);
    connect(rtsCheck_, &QCheckBox::toggled, this, &SerialPage::onRtsChanged);

    if (settingsService_) {
        const QSignalBlocker dtrBlocker(dtrCheck_);
        const QSignalBlocker rtsBlocker(rtsCheck_);
        dtrCheck_->setChecked(
            settingsService_->value(core::common::settings_keys::kSerialDtr).toBool());
        rtsCheck_->setChecked(
            settingsService_->value(core::common::settings_keys::kSerialRts).toBool());
    }

    retranslateUi();
}

void SerialPage::onConnectClicked(const io::SerialConfig& config) {
    if (monitor_) {
        monitor_->appendInfo(tr("Opening %1...").arg(config.portName));
    }
    sessionCtrl_.openSerial(config);
}

void SerialPage::onDisconnectClicked() {
    sessionCtrl_.closeChannel();
}

void SerialPage::onWorkerStateChanged(io::ChannelState state, quint64 gen) {
    sessionCtrl_.onWorkerStateChanged(state, gen);
}

void SerialPage::onSessionSnapshotChanged(const application::SessionUiSnapshot& snap) {
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

    if (snap.state == io::ChannelState::Open) {
        const bool dtr = dtrCheck_ && dtrCheck_->isChecked();
        const bool rts = rtsCheck_ && rtsCheck_->isChecked();
        sessionCtrl_.setDtr(dtr);
        sessionCtrl_.setRts(rts);
        emit serialControlLinesApplied(dtr, rts);
    }
}

void SerialPage::onDtrChanged(bool checked) {
    if (settingsService_) {
        settingsService_->setValue(core::common::settings_keys::kSerialDtr, checked);
    }
    sessionCtrl_.setDtr(checked);
}

void SerialPage::onRtsChanged(bool checked) {
    if (settingsService_) {
        settingsService_->setValue(core::common::settings_keys::kSerialRts, checked);
    }
    sessionCtrl_.setRts(checked);
}

void SerialPage::onWorkerError(const QString& /*deviceHint*/, io::ChannelErrorCode code, const QString& error) {
    if (monitor_) {
        const QString localized = application::ChannelSessionController::formatChannelError(code, error);
        monitor_->appendError(tr("Error: %1").arg(localized));
    }
}

void SerialPage::retranslateUi() {
    if (controlSection_) controlSection_->setTitle(tr("Control"));
    if (inputSection_) inputSection_->setTitle(tr("Send Data"));
    if (dtrCheck_) dtrCheck_->setText(QStringLiteral("DTR"));
    if (rtsCheck_) rtsCheck_->setText(QStringLiteral("RTS"));
}

void SerialPage::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::views::serial
