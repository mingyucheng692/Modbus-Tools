// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ModbusPage.h"
#include "Config.h"
#include "common/SettingsKeys.h"
#include "infra/config/ISettingsService.h"
#include "../../application/modbus/ModbusPagePresenter.h"
#include "../../common/ConnectionAlert.h"
#include "../../widgets/BaseConnectionWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../widgets/modbus/ModbusWorkbenchWidget.h"
#include "../../widgets/modbus/ModbusTelemetryWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include <QSplitter>
#include <QEvent>
#include <QSignalBlocker>

namespace ui::views::modbus {

namespace {

void applyPresenterTiming(::modbus::base::ModbusConfig& config,
                          const ui::application::modbus::ModbusPagePresenter* presenter,
                          const ui::widgets::modbus::ModbusWorkbenchWidget* workbench) {
    const auto timing = presenter
        ? presenter->timingParams()
        : ui::application::modbus::ModbusTimingParams{};
    config.slaveId = workbench ? workbench->slaveId() : config::Modbus::kDefaultSlaveId;
    config.timeoutMs = static_cast<int>(timing.timeout.count());
    config.retries = timing.retryCount;
    config.retryIntervalMs = static_cast<int>(timing.retryInterval.count());
}

class CompactStackedWidget : public QStackedWidget {
public:
    using QStackedWidget::QStackedWidget;

    QSize sizeHint() const override {
        if (auto* curr = currentWidget()) {
            return curr->sizeHint();
        }
        return QStackedWidget::sizeHint();
    }

    QSize minimumSizeHint() const override {
        if (auto* curr = currentWidget()) {
            return curr->minimumSizeHint();
        }
        return QStackedWidget::minimumSizeHint();
    }
};

} // namespace

ModbusPage::ModbusPage(infra::config::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
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
                this, [this]() { showStatusMessage(QString(), false); });
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
        connect(telemetryWidget_, &widgets::modbus::ModbusTelemetryWidget::pollFramesToggled,
                pagePresenter_, &ui::application::modbus::ModbusPagePresenter::setPollFramesEnabled);
        connect(telemetryWidget_, &widgets::modbus::ModbusTelemetryWidget::resetMetricsRequested,
                pagePresenter_, &ui::application::modbus::ModbusPagePresenter::onResetMetricsRequested);
    }

    connect(pagePresenter_, &ui::application::modbus::ModbusPagePresenter::linkageDataReceived,
            this, &ModbusPage::linkageDataReceived);
    connect(pagePresenter_, &ui::application::modbus::ModbusPagePresenter::linkageToggled,
            this, &ModbusPage::linkageToggled);
    connect(pagePresenter_, &ui::application::modbus::ModbusPagePresenter::linkageSourceDisconnected,
            this, &ModbusPage::linkageSourceDisconnected);
    connect(pagePresenter_, &ui::application::modbus::ModbusPagePresenter::sessionConnectionLostAlert,
            this, [this]() { ui::common::connection_alert::showDisconnected(this); });

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
    setupStatusBanner(mainLayout);

    setupSplitterBody();
    mainLayout->addWidget(splitter_, 1);

    retranslateUi();
}

void ModbusPage::setupConnectionBar() {
    auto* connRow = new QWidget(this);
    auto* connLayout = new QHBoxLayout(connRow);
    connLayout->setContentsMargins(0, 0, 0, 6);
    connLayout->setSpacing(4);
    connLayout->setAlignment(Qt::AlignTop);

    protoSection_ = new widgets::CollapsibleSection(nullptr, connRow);
    protoSection_->setObjectName(QStringLiteral("modbusProtocolSection"));
    protoSection_->setTitle(tr("Protocol:"));
    protoSection_->setCollapsible(false);
    protoSection_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    protocolLabel_ = protoSection_->titleLabel();

    auto* protoContentLayout = new QHBoxLayout(protoSection_->contentWidget());
    protoContentLayout->setContentsMargins(4, 0, 4, 0);
    protoContentLayout->setSpacing(2);

    protocolCombo_ = new QComboBox(protoSection_->contentWidget());
    protocolCombo_->setObjectName(QStringLiteral("modbusProtocolCombo"));
    protocolCombo_->setMinimumWidth(80);
    protocolCombo_->addItem(QStringLiteral("TCP"), static_cast<int>(ui::application::modbus::SessionMode::Tcp));
    protocolCombo_->addItem(QStringLiteral("RTU"), static_cast<int>(ui::application::modbus::SessionMode::Rtu));
    protocolCombo_->addItem(QStringLiteral("ASCII"), static_cast<int>(ui::application::modbus::SessionMode::Ascii));
    protoContentLayout->addWidget(protocolCombo_);

    connLayout->addWidget(protoSection_, 0, Qt::AlignTop);

    connectionStack_ = new CompactStackedWidget(connRow);
    connectionStack_->setFrameShape(QFrame::NoFrame);
    connectionStack_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    tcpConnectionWidget_ = new widgets::TcpConnectionWidget(widgets::TcpRole::Client, settingsService_, connectionStack_);
    tcpConnectionWidget_->setSettingsGroup(QStringLiteral("modbus/tcp"));
    connectionStack_->addWidget(tcpConnectionWidget_);

    serialConnectionWidget_ = new widgets::SerialConnectionWidget(settingsService_, connectionStack_);
    serialConnectionWidget_->setSettingsGroup(QStringLiteral("modbus/rtu/serial"));
    connectionStack_->addWidget(serialConnectionWidget_);

    connectionStack_->setCurrentIndex(0);
    connectionWidget_ = tcpConnectionWidget_;
    connLayout->addWidget(connectionStack_, 1, Qt::AlignTop);

    layout()->addWidget(connRow);
    bindConnectionBarSignals();

    if (connectionWidget_) {
        protoSection_->setExpanded(connectionWidget_->isSectionExpanded());
    }
}

void ModbusPage::bindConnectionBarSignals() {
    connect(protocolCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusPage::onProtocolChanged);

    auto syncSectionExpanded = [this](bool expanded) {
        if (protoSection_) {
            protoSection_->setExpanded(expanded);
        }
        if (connectionStack_) {
            connectionStack_->updateGeometry();
        }
    };

    connect(tcpConnectionWidget_, &widgets::BaseConnectionWidget::sectionExpandedChanged,
            this, syncSectionExpanded);
    connect(serialConnectionWidget_, &widgets::BaseConnectionWidget::sectionExpandedChanged,
            this, syncSectionExpanded);

    connect(tcpConnectionWidget_, &widgets::TcpConnectionWidget::connectClicked,
            this, &ModbusPage::onTcpConnectClicked);
    connect(tcpConnectionWidget_, &widgets::TcpConnectionWidget::disconnectClicked,
            this, &ModbusPage::onDisconnectClicked);
    connect(serialConnectionWidget_, &widgets::SerialConnectionWidget::connectClicked,
            this, &ModbusPage::onSerialConnectClicked);
    connect(serialConnectionWidget_, &widgets::SerialConnectionWidget::disconnectClicked,
            this, &ModbusPage::onDisconnectClicked);
}

void ModbusPage::setupStatusBanner(QVBoxLayout* mainLayout) {
    statusBannerLabel_ = new QLabel(this);
    statusBannerLabel_->setObjectName(QStringLiteral("modbusStatusBannerLabel"));
    statusBannerLabel_->setWordWrap(true);
    statusBannerLabel_->hide();
    mainLayout->addWidget(statusBannerLabel_);
}

void ModbusPage::setupSplitterBody() {
    splitter_ = new QSplitter(Qt::Horizontal, this);
    splitter_->setChildrenCollapsible(false);
    workbenchWidget_ = new widgets::modbus::ModbusWorkbenchWidget(settingsService_, splitter_);
    workbenchWidget_->setMinimumWidth(280);
    splitter_->addWidget(workbenchWidget_);

    telemetryWidget_ = new widgets::modbus::ModbusTelemetryWidget(splitter_);
    splitter_->addWidget(telemetryWidget_);

    splitter_->setStretchFactor(0, 0);
    splitter_->setStretchFactor(1, 1);

    connect(workbenchWidget_, &widgets::modbus::ModbusWorkbenchWidget::addressBaseChanged,
            this, [this](::modbus::address::AddressBase base) {
                if (settingsService_) {
                    settingsService_->setValue(core::common::settings_keys::kModbusAddressBase, static_cast<int>(base));
                }
                updateAddressBase(base);
            });

    connect(telemetryWidget_, &widgets::modbus::ModbusTelemetryWidget::inspectInAnalyzerRequested,
            this, &ModbusPage::inspectInAnalyzerRequested);
}

void ModbusPage::renderConnectionState(ui::application::modbus::SessionConnectionState state) {
    if (!connectionWidget_) return;
    using namespace ui::application::modbus;
    if (state == SessionConnectionState::Connected) {
        connectionWidget_->setConnected(true);
        showStatusMessage(QString(), false);
    } else if (state == SessionConnectionState::Disconnected) {
        connectionWidget_->setConnected(false);
    } else if (state == SessionConnectionState::Disconnecting) {
        connectionWidget_->setDisplayState(ui::widgets::DisplayState::Disconnecting);
    } else {
        connectionWidget_->setDisplayState(ui::widgets::DisplayState::Connecting);
    }
}

void ModbusPage::updateAddressBase(::modbus::address::AddressBase base) {
    if (workbenchWidget_) {
        workbenchWidget_->setAddressBase(base);
    }
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
    if (protocolCombo_) {
        protocolCombo_->setEnabled(state.protocolSwitchEnabled);
    }
    if (workbenchWidget_) workbenchWidget_->setExecutionGuards(state);
}

void ModbusPage::setLinkageState(bool active) {
    if (telemetryWidget_) telemetryWidget_->setLinkageState(active);
}

void ModbusPage::showStatusMessage(const QString& message, bool isError) {
    setToolTip(message);
    if (!statusBannerLabel_) return;
    if (message.trimmed().isEmpty()) {
        statusBannerLabel_->clear();
        statusBannerLabel_->hide();
        return;
    }
    const QString style = isError
        ? QStringLiteral("background-color: #ffebee; color: #b71c1c; border: 1px solid #ef9a9a; border-radius: 3px; padding: 4px 8px;")
        : QStringLiteral("background-color: #fff8e1; color: #5d4037; border: 1px solid #ffe082; border-radius: 3px; padding: 4px 8px;");
    statusBannerLabel_->setStyleSheet(style);
    statusBannerLabel_->setText(message);
    statusBannerLabel_->show();
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

void ModbusPage::setMetadataMap(const QMap<uint16_t, ::modbus::analyzer::DataMetadata>& meta) {
    if (telemetryWidget_) {
        telemetryWidget_->setMetadataMap(meta);
    }
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

    if (protoSection_ && connectionWidget_) {
        protoSection_->setExpanded(connectionWidget_->isSectionExpanded());
    }
    connectionStack_->updateGeometry();

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
    showStatusMessage(QString(), false);
    ::modbus::base::ModbusConfig config;
    config.mode = ::modbus::base::ModbusMode::TCP;
    config.ipAddress = ip;
    config.port = port;
    applyPresenterTiming(config, pagePresenter_, workbenchWidget_);

    ui::application::modbus::ModbusConnectionSpec spec;
    spec.config = config;
    if (pagePresenter_) pagePresenter_->requestConnect(spec);
}

void ModbusPage::onSerialConnectClicked(const io::SerialConfig& config) {
    const auto descriptor = ui::application::modbus::modeDescriptor(currentMode_);
    showStatusMessage(QString(), false);

    ::modbus::base::ModbusConfig modbusConfig;
    modbusConfig.mode = descriptor.modbusMode;
    modbusConfig.portName = config.portName;
    modbusConfig.baudRate = config.baudRate;
    modbusConfig.dataBits = config.dataBits;
    modbusConfig.stopBits = static_cast<int>(config.stopBits);
    modbusConfig.parity = static_cast<int>(config.parity);
    applyPresenterTiming(modbusConfig, pagePresenter_, workbenchWidget_);

    ui::application::modbus::ModbusConnectionSpec spec;
    spec.config = modbusConfig;
    spec.serialConfig = config;
    if (pagePresenter_) pagePresenter_->requestConnect(spec);
}

void ModbusPage::onDisconnectClicked() {
    if (pagePresenter_) pagePresenter_->requestDisconnect();
}

void ModbusPage::retranslateUi() {
    if (protoSection_) {
        protoSection_->setTitle(tr("Protocol:"));
    }
}

void ModbusPage::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) retranslateUi();
    QWidget::changeEvent(event);
}

} // namespace ui::views::modbus
