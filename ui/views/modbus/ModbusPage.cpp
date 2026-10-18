// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ModbusPage.h"
#include "Config.h"
#include "common/SettingsKeys.h"
#include "core/common/ISettingsService.h"
#include "../../application/modbus/ModbusPagePresenter.h"
#include "../../widgets/BaseConnectionWidget.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/modbus/ModbusWorkbenchWidget.h"
#include "../../widgets/modbus/ModbusTelemetryWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QStackedWidget>
#include <QSplitter>
#include <QEvent>
#include <QSignalBlocker>
#include <spdlog/spdlog.h>

namespace {

void initializeAsciiSerialDefaults(infra::config::ISettingsService* settingsService) {
    if (!settingsService) return;
    const auto ensureValue = [settingsService](const QString& key, const QVariant& val) {
        if (!settingsService->contains(key)) settingsService->setValue(key, val);
    };
    ensureValue(QStringLiteral("modbus/ascii/serial/baudRate"), QStringLiteral("9600"));
    ensureValue(QStringLiteral("modbus/ascii/serial/dataBits"), QStringLiteral("7"));
    ensureValue(QStringLiteral("modbus/ascii/serial/parity"), QStringLiteral("even"));
    ensureValue(QStringLiteral("modbus/ascii/serial/stopBits"), QStringLiteral("1"));
    ensureValue(QStringLiteral("modbus/ascii/serial/flowControl"), QStringLiteral("none"));
}

} // namespace

namespace ui::views::modbus {

ModbusPage::ModbusPage(infra::config::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
    initializeAsciiSerialDefaults(settingsService_);
    setupUi();
}

ModbusPage::~ModbusPage() noexcept {
    if (pagePresenter_) {
        pagePresenter_->detachView();
    }
}

void ModbusPage::bindPresenter(ui::application::modbus::ModbusPagePresenter* presenter) {
    pagePresenter_ = presenter;
    if (!pagePresenter_) return;

    if (workbenchWidget_) {
        connect(workbenchWidget_, &widgets::modbus::ModbusWorkbenchWidget::executeRequested,
                pagePresenter_, &ui::application::modbus::ModbusPagePresenter::executeRequest);
        connect(workbenchWidget_, &widgets::modbus::ModbusWorkbenchWidget::pollToggleRequested,
                pagePresenter_, &ui::application::modbus::ModbusPagePresenter::togglePolling);
        connect(workbenchWidget_, &widgets::modbus::ModbusWorkbenchWidget::pollIntervalChanged,
                pagePresenter_, &ui::application::modbus::ModbusPagePresenter::changePollInterval);
        connect(workbenchWidget_, &widgets::modbus::ModbusWorkbenchWidget::logMessageRequested,
                pagePresenter_, &ui::application::modbus::ModbusPagePresenter::handleLogMessage);
    }

    if (telemetryWidget_) {
        connect(telemetryWidget_, &widgets::modbus::ModbusTelemetryWidget::linkageToggled,
                pagePresenter_, &ui::application::modbus::ModbusPagePresenter::setLinked);
        connect(telemetryWidget_, &widgets::modbus::ModbusTelemetryWidget::rawFramesToggled,
                pagePresenter_, &ui::application::modbus::ModbusPagePresenter::setRawFramesEnabled);
    }

    connect(pagePresenter_, &ui::application::modbus::ModbusPagePresenter::linkageDataReceived,
            this, &ModbusPage::linkageDataReceived);
    connect(pagePresenter_, &ui::application::modbus::ModbusPagePresenter::linkageToggled,
            this, &ModbusPage::linkageToggled);
    connect(pagePresenter_, &ui::application::modbus::ModbusPagePresenter::linkageSourceDisconnected,
            this, &ModbusPage::linkageSourceDisconnected);

    int baseVal = 0;
    if (settingsService_) {
        const QVariant v = settingsService_->value(core::common::settings_keys::kModbusAddressBase);
        if (v.isValid()) baseVal = v.toInt();
    }
    const auto addrBase = (baseVal == 1) ? ::modbus::address::AddressBase::PlcAddress1Based
                                         : ::modbus::address::AddressBase::Offset0Based;
    updateAddressBase(addrBase);
    setProtocolType(ui::application::modbus::modeDescriptor(currentMode_).protocolType);
}

void ModbusPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    setupConnectionBar();
    mainLayout->addWidget(connectionStack_->parentWidget());

    setupSplitterBody();
    mainLayout->addWidget(splitter_, 1);

    retranslateUi();
}

void ModbusPage::setupConnectionBar() {
    auto* connRow = new QWidget(this);
    auto* connLayout = new QHBoxLayout(connRow);
    connLayout->setContentsMargins(0, 0, 0, 0);
    connLayout->setSpacing(4);

    protocolCombo_ = new QComboBox(connRow);
    protocolCombo_->setMinimumWidth(80);
    protocolCombo_->addItem(QStringLiteral("TCP"), static_cast<int>(ui::application::modbus::SessionMode::Tcp));
    protocolCombo_->addItem(QStringLiteral("RTU"), static_cast<int>(ui::application::modbus::SessionMode::Rtu));
    protocolCombo_->addItem(QStringLiteral("ASCII"), static_cast<int>(ui::application::modbus::SessionMode::Ascii));
    connLayout->addWidget(protocolCombo_);

    connectionStack_ = new QStackedWidget(connRow);
    connectionStack_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    tcpConnectionWidget_ = new widgets::TcpConnectionWidget(widgets::TcpRole::Client, settingsService_, connectionStack_);
    tcpConnectionWidget_->setSettingsGroup(QStringLiteral("modbus/tcp"));
    connectionStack_->addWidget(tcpConnectionWidget_);

    serialConnectionWidget_ = new widgets::SerialConnectionWidget(settingsService_, connectionStack_);
    serialConnectionWidget_->setSettingsGroup(QStringLiteral("modbus/rtu/serial"));
    connectionStack_->addWidget(serialConnectionWidget_);

    connectionStack_->setCurrentIndex(0);
    connectionWidget_ = tcpConnectionWidget_;
    connLayout->addWidget(connectionStack_, 1);

    addrBaseCombo_ = new QComboBox(connRow);
    addrBaseCombo_->setMinimumWidth(110);
    addrBaseCombo_->addItem(tr("0-Based"), static_cast<int>(::modbus::address::AddressBase::Offset0Based));
    addrBaseCombo_->addItem(tr("1-Based"), static_cast<int>(::modbus::address::AddressBase::PlcAddress1Based));
    connLayout->addWidget(addrBaseCombo_);

    connect(protocolCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusPage::onProtocolChanged);
    connect(addrBaseCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                const auto base = (idx == 1) ? ::modbus::address::AddressBase::PlcAddress1Based
                                             : ::modbus::address::AddressBase::Offset0Based;
                if (settingsService_) {
                    settingsService_->setValue(core::common::settings_keys::kModbusAddressBase, static_cast<int>(base));
                }
                updateAddressBase(base);
            });

    connect(tcpConnectionWidget_, &widgets::TcpConnectionWidget::connectClicked,
            this, &ModbusPage::onTcpConnectClicked);
    connect(tcpConnectionWidget_, &widgets::TcpConnectionWidget::disconnectClicked,
            this, &ModbusPage::onDisconnectClicked);
    connect(serialConnectionWidget_, &widgets::SerialConnectionWidget::connectClicked,
            this, &ModbusPage::onSerialConnectClicked);
    connect(serialConnectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked,
            this, &ModbusPage::onDisconnectClicked);
}

void ModbusPage::setupSplitterBody() {
    splitter_ = new QSplitter(Qt::Horizontal, this);
    workbenchWidget_ = new widgets::modbus::ModbusWorkbenchWidget(settingsService_, splitter_);
    workbenchWidget_->setMinimumWidth(360);
    splitter_->addWidget(workbenchWidget_);

    telemetryWidget_ = new widgets::modbus::ModbusTelemetryWidget(splitter_);
    splitter_->addWidget(telemetryWidget_);

    splitter_->setStretchFactor(0, 0);
    splitter_->setStretchFactor(1, 1);
}

void ModbusPage::renderConnectionState(ui::application::modbus::SessionConnectionState state) {
    if (!connectionWidget_) return;
    using namespace ui::application::modbus;
    if (state == SessionConnectionState::Connected) {
        connectionWidget_->setConnected(true);
    } else if (state == SessionConnectionState::Disconnected) {
        connectionWidget_->setConnected(false);
    } else if (state == SessionConnectionState::Disconnecting) {
        connectionWidget_->setDisplayState(ui::widgets::DisplayState::Disconnecting);
    } else {
        connectionWidget_->setDisplayState(ui::widgets::DisplayState::Connecting);
    }
}

void ModbusPage::updateAddressBase(::modbus::address::AddressBase base) {
    if (addrBaseCombo_) {
        const int idx = (base == ::modbus::address::AddressBase::PlcAddress1Based) ? 1 : 0;
        if (addrBaseCombo_->currentIndex() != idx) {
            QSignalBlocker blocker(addrBaseCombo_);
            addrBaseCombo_->setCurrentIndex(idx);
        }
    }
    if (workbenchWidget_) workbenchWidget_->setAddressBase(base);
}

void ModbusPage::setProtocolType(::modbus::parser::ProtocolType protocol) {
    if (workbenchWidget_) workbenchWidget_->setProtocolType(protocol);
    if (telemetryWidget_) telemetryWidget_->setProtocolType(protocol);
}

void ModbusPage::appendTrafficTelemetry(const ::ui::common::TrafficEvent& event) {
    if (telemetryWidget_) telemetryWidget_->appendTrafficEvent(event);
}

void ModbusPage::updateMetrics(int tx, int rx, int err, int rttMs) {
    if (telemetryWidget_) telemetryWidget_->updateMetrics(tx, rx, err, rttMs);
}

void ModbusPage::setExecutionGuards(const ui::application::modbus::ExecutionGuardState& state) {
    if (workbenchWidget_) workbenchWidget_->setExecutionGuards(state);
}

void ModbusPage::setLinkageState(bool active) {
    if (telemetryWidget_) telemetryWidget_->setLinkageState(active);
}

void ModbusPage::showStatusMessage(const QString& message, bool isError) {
    Q_UNUSED(isError);
    setToolTip(message);
}

void ModbusPage::updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs) {
    if (pagePresenter_) {
        pagePresenter_->updateSettings({std::chrono::milliseconds(timeoutMs), retries, std::chrono::milliseconds(retryIntervalMs)});
    }
}

void ModbusPage::setLinked(bool linked) {
    if (pagePresenter_) pagePresenter_->setLinked(linked);
    if (telemetryWidget_) telemetryWidget_->setLinkageState(linked);
}

bool ModbusPage::isLinked() const {
    return pagePresenter_ ? pagePresenter_->isLinked() : false;
}

void ModbusPage::onProtocolChanged(int index) {
    if (index < 0 || index >= protocolCombo_->count()) return;
    const auto mode = static_cast<ui::application::modbus::SessionMode>(protocolCombo_->itemData(index).toInt());
    switchToProtocol(mode);
}

void ModbusPage::switchToProtocol(ui::application::modbus::SessionMode mode) {
    if (mode == currentMode_) return;
    currentMode_ = mode;
    applyProtocolSettingsGroup(mode);

    const auto descriptor = ui::application::modbus::modeDescriptor(mode);
    if (descriptor.usesSerialConnection) {
        connectionStack_->setCurrentWidget(serialConnectionWidget_);
        connectionWidget_ = serialConnectionWidget_;
    } else {
        connectionStack_->setCurrentWidget(tcpConnectionWidget_);
        connectionWidget_ = tcpConnectionWidget_;
    }

    if (pagePresenter_) pagePresenter_->switchMode(mode);
    setProtocolType(descriptor.protocolType);
}

void ModbusPage::applyProtocolSettingsGroup(ui::application::modbus::SessionMode mode) {
    if (!serialConnectionWidget_) return;
    if (mode == ui::application::modbus::SessionMode::Rtu) {
        serialConnectionWidget_->setSettingsGroup(QStringLiteral("modbus/rtu/serial"));
    } else if (mode == ui::application::modbus::SessionMode::Ascii) {
        serialConnectionWidget_->setSettingsGroup(QStringLiteral("modbus/ascii/serial"));
    }
}

void ModbusPage::onTcpConnectClicked(const QString& ip, int port) {
    SPDLOG_INFO("ModbusPage: TCP connect requested to {}:{}", ip.toStdString(), port);
    ::modbus::base::ModbusConfig config;
    config.mode = ::modbus::base::ModbusMode::TCP;
    config.ipAddress = ip;
    config.port = port;
    config.slaveId = workbenchWidget_ ? workbenchWidget_->slaveId() : 1;
    config.timeoutMs = 1000;
    config.retries = 0;
    config.retryIntervalMs = 200;

    ui::application::modbus::ModbusConnectionSpec spec;
    spec.config = config;
    if (pagePresenter_) pagePresenter_->requestConnect(spec);
}

void ModbusPage::onSerialConnectClicked(const io::SerialConfig& config) {
    const auto descriptor = ui::application::modbus::modeDescriptor(currentMode_);
    SPDLOG_INFO("ModbusPage: {} connect requested to {}", descriptor.logName, config.portName.toStdString());

    ::modbus::base::ModbusConfig modbusConfig;
    modbusConfig.mode = descriptor.modbusMode;
    modbusConfig.portName = config.portName;
    modbusConfig.baudRate = config.baudRate;
    modbusConfig.dataBits = config.dataBits;
    modbusConfig.stopBits = static_cast<int>(config.stopBits);
    modbusConfig.parity = static_cast<int>(config.parity);
    modbusConfig.slaveId = workbenchWidget_ ? workbenchWidget_->slaveId() : 1;
    modbusConfig.timeoutMs = 1000;
    modbusConfig.retries = 0;
    modbusConfig.retryIntervalMs = 200;

    ui::application::modbus::ModbusConnectionSpec spec;
    spec.config = modbusConfig;
    spec.serialConfig = config;
    if (pagePresenter_) pagePresenter_->requestConnect(spec);
}

void ModbusPage::onDisconnectClicked() {
    SPDLOG_INFO("ModbusPage: disconnect requested");
    if (pagePresenter_) pagePresenter_->requestDisconnect();
}

void ModbusPage::retranslateUi() {
    if (addrBaseCombo_) {
        addrBaseCombo_->setItemText(0, tr("0-Based"));
        addrBaseCombo_->setItemText(1, tr("1-Based"));
    }
}

void ModbusPage::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}

} // namespace ui::views::modbus
