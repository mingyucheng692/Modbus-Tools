#include "ModbusAsciiPage.h"

#include "../../../core/common/ISettingsService.h"
#include "../../widgets/FunctionWidget.h"
#include "../../widgets/SerialConnectionWidget.h"
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

namespace ui::views::modbus_ascii {

ModbusAsciiPage::ModbusAsciiPage(core::common::ISettingsService* settingsService, QWidget* parent)
    : BaseModbusPage(ui::application::modbus::SessionMode::Ascii, settingsService, parent)
{
    initializeAsciiSerialDefaults(settingsService_);

    auto* connWidget = new widgets::SerialConnectionWidget(settingsService_, this);
    connWidget->setSettingsGroup(QStringLiteral("modbus/ascii/serial"));

    setupUi(connWidget);

    connect(connWidget, &widgets::SerialConnectionWidget::connectClicked,
            this, &ModbusAsciiPage::onConnectClicked);
    connect(connWidget, &widgets::SerialConnectionWidget::disconnectClicked,
            this, &ModbusAsciiPage::onDisconnectClicked);
}

ModbusAsciiPage::~ModbusAsciiPage() noexcept = default;

void ModbusAsciiPage::onConnectClicked(const io::SerialConfig& config)
{
    spdlog::info("ModbusAsciiPage: Connect requested to {}", config.portName.toStdString());

    modbus::base::ModbusConfig modbusConfig;
    modbusConfig.mode = modbus::base::ModbusMode::ASCII;
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
    sessionPresenter_->requestConnect(spec);
}

void ModbusAsciiPage::onDisconnectClicked()
{
    spdlog::info("ModbusAsciiPage: Disconnect requested");
    sessionPresenter_->requestDisconnect();
}

} // namespace ui::views::modbus_ascii
