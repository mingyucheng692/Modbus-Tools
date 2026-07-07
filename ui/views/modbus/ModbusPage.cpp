/**
 * @file ModbusPage.cpp
 * @brief Implementation of ModbusPage.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusPage.h"
#include "AppConstants.h"
#include "../../../core/common/ISettingsService.h"
#include "../../application/modbus/ModbusPagePresenter.h"
#include "../../application/modbus/ModbusSessionPresenter.h"
#include "../../widgets/BaseConnectionWidget.h"
#include "../../widgets/TcpClientConnectionWidget.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/FunctionWidget.h"
#include "../../widgets/TrafficMonitorWidget.h"
#include "../../widgets/ControlWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../common/ConnectionAlert.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QStackedWidget>
#include <QEvent>
#include <QClipboard>
#include <QGuiApplication>
#include <QCoreApplication>
#include <spdlog/spdlog.h>

namespace {

void initializeAsciiSerialDefaults(core::common::ISettingsService* settingsService)
{
    if (!settingsService) {
        return;
    }

    const auto ensureValue = [settingsService](const QString& key, const QVariant& value) {
        if (!settingsService->contains(key)) {
            settingsService->setValue(key, value);
        }
    };

    ensureValue(QStringLiteral("modbus/ascii/serial/baudRate"), QStringLiteral("9600"));
    ensureValue(QStringLiteral("modbus/ascii/serial/dataBits"), QStringLiteral("7"));
    ensureValue(QStringLiteral("modbus/ascii/serial/parity"), QStringLiteral("even"));
    ensureValue(QStringLiteral("modbus/ascii/serial/stopBits"), QStringLiteral("1"));
    ensureValue(QStringLiteral("modbus/ascii/serial/flowControl"), QStringLiteral("none"));
}

} // namespace

namespace ui::views::modbus {

ModbusPage::ModbusPage(core::common::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    initializeAsciiSerialDefaults(settingsService_);
    setupUi();
}

ModbusPage::~ModbusPage() noexcept = default;

void ModbusPage::setupUi() {
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(2, 2, 2, 2);
    mainLayout_->setSpacing(2);

    // Protocol selector + stacked connection widgets.
    auto* connRow = new QWidget(this);
    auto* connLayout = new QHBoxLayout(connRow);
    connLayout->setContentsMargins(0, 0, 0, 0);
    connLayout->setSpacing(2);

    protocolCombo_ = new QComboBox(this);
    protocolCombo_->setFixedWidth(96);
    protocolCombo_->addItem(QCoreApplication::translate("ui::views::modbus::ModbusPage", "TCP"),
                            static_cast<int>(ui::application::modbus::SessionMode::Tcp));
    protocolCombo_->addItem(QCoreApplication::translate("ui::views::modbus::ModbusPage", "RTU"),
                            static_cast<int>(ui::application::modbus::SessionMode::Rtu));
    protocolCombo_->addItem(QCoreApplication::translate("ui::views::modbus::ModbusPage", "ASCII"),
                            static_cast<int>(ui::application::modbus::SessionMode::Ascii));
    connLayout->addWidget(protocolCombo_);

    connectionStack_ = new QStackedWidget(this);
    connectionStack_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    tcpConnectionWidget_ = new widgets::TcpClientConnectionWidget(settingsService_, connectionStack_);
    tcpConnectionWidget_->setSettingsGroup(QStringLiteral("modbus/tcp"));
    connectionStack_->addWidget(tcpConnectionWidget_);

    serialConnectionWidget_ = new widgets::SerialConnectionWidget(settingsService_, connectionStack_);
    serialConnectionWidget_->setSettingsGroup(QStringLiteral("modbus/rtu/serial"));
    connectionStack_->addWidget(serialConnectionWidget_);

    connectionStack_->setCurrentIndex(0);
    connectionWidget_ = tcpConnectionWidget_;
    connLayout->addWidget(connectionStack_);
    mainLayout_->addWidget(connRow);

    // Function widget.
    functionWidget_ = new widgets::FunctionWidget(settingsService_, this);
    functionWidget_->setSettingsGroup(QStringLiteral("modbus/tcp/standard"));
    functionWidget_->setTransportMode(ui::application::modbus::TransportUiMode::Tcp);
    mainLayout_->addWidget(functionWidget_);

    // Data monitor (collapsible).
    dataGroup_ = new widgets::CollapsibleSection(settingsService_, this);
    dataGroup_->setSettingsKey(QStringLiteral("modbus/tcp/ui/dataMonitorCollapsed"));
    auto dataLayout = new QHBoxLayout(dataGroup_->contentWidget());
    dataLayout->setContentsMargins(4, 0, 4, 4);
    dataLayout->setSpacing(4);

    receiveGroup_ = new QGroupBox(dataGroup_->contentWidget());
    auto receiveLayout = new QVBoxLayout(receiveGroup_);
    auto receiveToolbar = new QHBoxLayout();
    copyReceiveButton_ = new QPushButton(receiveGroup_);
    clearReceiveButton_ = new QPushButton(receiveGroup_);
    receiveToolbar->addStretch();
    receiveToolbar->addWidget(copyReceiveButton_);
    receiveToolbar->addWidget(clearReceiveButton_);
    receiveTextEdit_ = new QTextEdit(receiveGroup_);
    receiveTextEdit_->setReadOnly(true);
    receiveTextEdit_->document()->setMaximumBlockCount(app::constants::Values::Ui::kTrafficMonitorMaxBlockCount);
    receiveLayout->addLayout(receiveToolbar);
    receiveLayout->addWidget(receiveTextEdit_);

    sendGroup_ = new QGroupBox(dataGroup_->contentWidget());
    auto sendLayout = new QVBoxLayout(sendGroup_);
    auto sendToolbar = new QHBoxLayout();
    copySendButton_ = new QPushButton(sendGroup_);
    clearSendButton_ = new QPushButton(sendGroup_);
    sendToolbar->addStretch();
    sendToolbar->addWidget(copySendButton_);
    sendToolbar->addWidget(clearSendButton_);
    sendTextEdit_ = new QTextEdit(sendGroup_);
    sendTextEdit_->setReadOnly(true);
    sendTextEdit_->document()->setMaximumBlockCount(app::constants::Values::Ui::kTrafficMonitorMaxBlockCount);
    sendLayout->addLayout(sendToolbar);
    sendLayout->addWidget(sendTextEdit_);

    dataLayout->addWidget(receiveGroup_, 1);
    dataLayout->addWidget(sendGroup_, 1);

    trafficMonitor_ = new widgets::TrafficMonitorWidget(settingsService_, this);
    trafficMonitor_->setMinimumHeight(140);
    trafficMonitor_->setSettingsGroup(QStringLiteral("modbus/tcp/traffic"));

    mainLayout_->addWidget(dataGroup_);
    mainLayout_->addWidget(trafficMonitor_);

    controlWidget_ = new widgets::ControlWidget(settingsService_, this);
    controlWidget_->setSettingsGroup(QStringLiteral("modbus/tcp/control"));
    mainLayout_->addWidget(controlWidget_);

    // Create the Presenter (composition root) and wire backend services.
    pagePresenter_ = new ui::application::modbus::ModbusPagePresenter(
        this, currentMode_, this);
    pagePresenter_->setup(connectionWidget_, controlWidget_,
                          functionWidget_, trafficMonitor_);
    sessionPresenter_ = pagePresenter_->sessionPresenter();

    // Forward Presenter linkage signals.
    connect(pagePresenter_, &ui::application::modbus::ModbusPagePresenter::linkageDataReceived,
            this, &ModbusPage::linkageDataReceived);
    connect(pagePresenter_, &ui::application::modbus::ModbusPagePresenter::linkageToggled,
            this, &ModbusPage::linkageToggled);
    connect(pagePresenter_, &ui::application::modbus::ModbusPagePresenter::linkageSourceDisconnected,
            this, &ModbusPage::linkageSourceDisconnected);

    setupViewOnlyConnections();

    // Protocol switching.
    connect(protocolCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusPage::onProtocolChanged);

    // Connection widget signals.
    connect(tcpConnectionWidget_, &widgets::TcpClientConnectionWidget::connectClicked,
            this, &ModbusPage::onTcpConnectClicked);
    connect(tcpConnectionWidget_, &widgets::TcpClientConnectionWidget::disconnectClicked,
            this, &ModbusPage::onDisconnectClicked);
    connect(serialConnectionWidget_, &widgets::SerialConnectionWidget::connectClicked,
            this, &ModbusPage::onSerialConnectClicked);
    connect(serialConnectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked,
            this, &ModbusPage::onDisconnectClicked);

    mainLayout_->addStretch();

    retranslateUi();
}

void ModbusPage::setupViewOnlyConnections() {
    if (controlWidget_) {
        auto ensureConnected = [this]() {
            if (pagePresenter_ && pagePresenter_->isSessionConnected()) {
                return true;
            }
            ui::common::ConnectionAlert::showNotConnected(this);
            return false;
        };
        controlWidget_->setConnectionValidator(ensureConnected);
    }

    connect(clearReceiveButton_, &QPushButton::clicked, this, [this]() {
        lastReceiveFrame_.clear();
        if (receiveTextEdit_) receiveTextEdit_->clear();
    });
    connect(clearSendButton_, &QPushButton::clicked, this, [this]() {
        lastSendFrame_.clear();
        if (sendTextEdit_) sendTextEdit_->clear();
    });
    connect(copyReceiveButton_, &QPushButton::clicked, this, [this]() {
        if (!receiveTextEdit_) return;
        auto* clipboard = QGuiApplication::clipboard();
        if (!clipboard) return;
        clipboard->setText(receiveTextEdit_->toPlainText());
    });
    connect(copySendButton_, &QPushButton::clicked, this, [this]() {
        if (!sendTextEdit_) return;
        auto* clipboard = QGuiApplication::clipboard();
        if (!clipboard) return;
        clipboard->setText(sendTextEdit_->toPlainText());
    });
}

void ModbusPage::applyProtocolSettingsGroup(ui::application::modbus::SessionMode mode) {
    const auto descriptor = ui::application::modbus::modeDescriptor(mode);
    const QString prefix = QString::fromLatin1(descriptor.settingsPrefix);

    if (functionWidget_) {
        functionWidget_->setSettingsGroup(prefix + QStringLiteral("/standard"));
        functionWidget_->setTransportMode(descriptor.transportUiMode);
    }
    if (dataGroup_) {
        dataGroup_->setSettingsKey(prefix + QStringLiteral("/ui/dataMonitorCollapsed"));
    }
    if (trafficMonitor_) {
        trafficMonitor_->setSettingsGroup(prefix + QStringLiteral("/traffic"));
    }
    if (controlWidget_) {
        controlWidget_->setSettingsGroup(prefix + QStringLiteral("/control"));
    }

    // Connection widget group: TCP uses prefix directly; serial uses prefix + "/serial".
    if (descriptor.usesSerialConnection && serialConnectionWidget_) {
        serialConnectionWidget_->setSettingsGroup(prefix + QStringLiteral("/serial"));
    } else if (tcpConnectionWidget_) {
        tcpConnectionWidget_->setSettingsGroup(prefix);
    }
}

void ModbusPage::switchToProtocol(ui::application::modbus::SessionMode mode) {
    if (mode == currentMode_) {
        return;
    }

    const auto descriptor = ui::application::modbus::modeDescriptor(mode);

    // Swap the visible connection widget.
    if (descriptor.usesSerialConnection) {
        connectionStack_->setCurrentWidget(serialConnectionWidget_);
        connectionWidget_ = serialConnectionWidget_;
    } else {
        connectionStack_->setCurrentWidget(tcpConnectionWidget_);
        connectionWidget_ = tcpConnectionWidget_;
    }

    // Update settings groups for the new protocol.
    applyProtocolSettingsGroup(mode);

    // Rebuild backend services with the new mode and connection widget.
    // NOTE: currentMode_ is intentionally updated AFTER switchMode() completes.
    // During switchMode(), teardownServices() deletes the old SessionPresenter,
    // whose destructor emits linkageSourceDisconnected (if was linked). The
    // AnalyzerLinkCoordinator::handleSourceDisconnected() queries currentMode()
    // to compute the disconnecting source — it must see the OLD mode so that
    // state_.source matches and stopInternal() is called. Updating currentMode_
    // before teardown would cause a source mismatch and leave the coordinator
    // stuck in Live state.
    if (pagePresenter_) {
        pagePresenter_->switchMode(mode, connectionWidget_);
        sessionPresenter_ = pagePresenter_->sessionPresenter();
    }

    currentMode_ = mode;

    spdlog::info("ModbusPage: switched to protocol {}", descriptor.logName);
}

void ModbusPage::onProtocolChanged(int index) {
    if (index < 0 || index >= protocolCombo_->count()) {
        return;
    }
    const auto mode = static_cast<ui::application::modbus::SessionMode>(
        protocolCombo_->itemData(index).toInt());
    switchToProtocol(mode);
}

void ModbusPage::onTcpConnectClicked(const QString& ip, int port) {
    spdlog::info("ModbusPage: TCP connect requested to {}:{}", ip.toStdString(), port);

    ::modbus::base::ModbusConfig config;
    config.mode = ::modbus::base::ModbusMode::TCP;
    config.ipAddress = ip;
    config.port = port;
    config.slaveId = static_cast<uint8_t>(functionWidget_->getSlaveId());
    config.timeoutMs = 1000;
    config.retries = 0;
    config.retryIntervalMs = 200;

    ui::application::modbus::ModbusConnectionSpec spec;
    spec.config = config;
    if (sessionPresenter_) {
        sessionPresenter_->requestConnect(spec);
    }
}

void ModbusPage::onSerialConnectClicked(const io::SerialConfig& config) {
    const auto descriptor = ui::application::modbus::modeDescriptor(currentMode_);
    spdlog::info("ModbusPage: {} connect requested to {}",
                 descriptor.logName, config.portName.toStdString());

    ::modbus::base::ModbusConfig modbusConfig;
    modbusConfig.mode = descriptor.modbusMode;
    modbusConfig.portName = config.portName;
    modbusConfig.baudRate = config.baudRate;
    modbusConfig.dataBits = config.dataBits;
    modbusConfig.stopBits = static_cast<int>(config.stopBits);
    modbusConfig.parity = static_cast<int>(config.parity);
    modbusConfig.slaveId = static_cast<uint8_t>(functionWidget_->getSlaveId());
    modbusConfig.timeoutMs = 1000;
    modbusConfig.retries = 0;
    modbusConfig.retryIntervalMs = 200;

    ui::application::modbus::ModbusConnectionSpec spec;
    spec.config = modbusConfig;
    spec.serialConfig = config;
    if (sessionPresenter_) {
        sessionPresenter_->requestConnect(spec);
    }
}

void ModbusPage::onDisconnectClicked() {
    spdlog::info("ModbusPage: disconnect requested");
    if (sessionPresenter_) {
        sessionPresenter_->requestDisconnect();
    }
}

void ModbusPage::updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs) {
    if (pagePresenter_) {
        ui::application::modbus::ModbusTimingParams params;
        params.timeout = std::chrono::milliseconds(timeoutMs);
        params.retryCount = retries;
        params.retryInterval = std::chrono::milliseconds(retryIntervalMs);
        pagePresenter_->updateSettings(params);
    }
}

void ModbusPage::setLinked(bool linked) {
    if (pagePresenter_) {
        pagePresenter_->setLinked(linked);
    }
}

bool ModbusPage::isLinked() const {
    return pagePresenter_ && pagePresenter_->isLinked();
}

void ModbusPage::appendTrafficData(bool isTx, const QByteArray& data) {
    if (isTx) {
        appendSendData(data);
    } else {
        appendReceiveData(data);
    }
}

QString ModbusPage::formatData(const QByteArray& data, bool hex) const {
    if (hex) {
        return QString(data.toHex(' ').toUpper());
    }
    return QString::fromLatin1(data);
}

void ModbusPage::appendReceiveData(const QByteArray& data) {
    lastReceiveFrame_ = data;
    refreshReceiveDisplay();
}

void ModbusPage::appendSendData(const QByteArray& data) {
    lastSendFrame_ = data;
    refreshSendDisplay();
}

void ModbusPage::refreshReceiveDisplay() {
    if (!receiveTextEdit_) return;
    if (lastReceiveFrame_.isEmpty()) {
        receiveTextEdit_->clear();
        return;
    }
    const bool hex = true;
    receiveTextEdit_->setPlainText(tr("[%1] %2")
                                       .arg(tr("RX"))
                                       .arg(formatData(lastReceiveFrame_, hex)));
}

void ModbusPage::refreshSendDisplay() {
    if (!sendTextEdit_) return;
    if (lastSendFrame_.isEmpty()) {
        sendTextEdit_->clear();
        return;
    }
    const bool hex = true;
    sendTextEdit_->setPlainText(tr("[%1] %2")
                                    .arg(tr("TX"))
                                    .arg(formatData(lastSendFrame_, hex)));
}

void ModbusPage::retranslateUi() {
    if (dataGroup_) dataGroup_->setTitle(tr("Data Monitor"));
    if (receiveGroup_) receiveGroup_->setTitle(tr("Receive Data"));
    if (sendGroup_) sendGroup_->setTitle(tr("Send Data"));
    if (copyReceiveButton_) copyReceiveButton_->setText(tr("Copy"));
    if (copySendButton_) copySendButton_->setText(tr("Copy"));
    if (clearReceiveButton_) clearReceiveButton_->setText(tr("Clear"));
    if (clearSendButton_) clearSendButton_->setText(tr("Clear"));
    refreshReceiveDisplay();
    refreshSendDisplay();
}

void ModbusPage::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
}

} // namespace ui::views::modbus
