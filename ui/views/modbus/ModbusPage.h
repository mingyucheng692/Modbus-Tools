// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QWidget>
#include <QString>
#include "application/modbus/IModbusPageView.h"
#include "application/modbus/ModbusTypes.h"
#include "infra/io/SerialConfig.h"

class QComboBox;
class QLabel;
class QStackedWidget;
class QSplitter;
class QVBoxLayout;

namespace infra::config {
class ISettingsService;
}

namespace ui::widgets {
class BaseConnectionWidget;
class TcpConnectionWidget;
class SerialConnectionWidget;
}

namespace ui::widgets::modbus {
class ModbusWorkbenchWidget;
class ModbusTelemetryWidget;
}

namespace ui::application::modbus {
class ModbusPagePresenter;
}

namespace ui::views::modbus {

/// Role: Clean-room composition view for Modbus debugging, assembling connection bar, workbench, and telemetry.
/// Thread-safety: Thread-affine (bound to GUI main thread).
/// Ownership: Owned by QObject parent hierarchy (MainWindow); owns child UI widgets and observes ModbusPagePresenter.
class ModbusPage : public QWidget, public ui::application::modbus::IModbusPageView {
    Q_OBJECT

public:
    explicit ModbusPage(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~ModbusPage() noexcept override;

    void bindPresenter(ui::application::modbus::ModbusPagePresenter* presenter);

    // --- IModbusPageView Contract Implementation ---
    void renderConnectionState(ui::application::modbus::SessionConnectionState state) override;
    void updateAddressBase(::modbus::address::AddressBase base) override;
    void setProtocolType(::modbus::parser::ProtocolType protocol) override;
    void appendTrafficTelemetry(const ::ui::common::TrafficEvent& event) override;
    void updateMetrics(int tx, int rx, int err, int rttMs) override;
    void setExecutionGuards(const ui::application::modbus::ExecutionGuardState& state) override;
    void setLinkageState(bool active) override;
    void showStatusMessage(const QString& message, bool isError) override;

    // --- External APIs for MainWindow & AnalyzerLinkCoordinator ---
    void updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs);
    void setLinked(bool linked);
    [[nodiscard]] bool isLinked() const;
    [[nodiscard]] ui::application::modbus::SessionMode currentMode() const noexcept { return currentMode_; }

    [[nodiscard]] ui::widgets::modbus::ModbusWorkbenchWidget* workbenchWidget() const noexcept { return workbenchWidget_; }
    [[nodiscard]] ui::widgets::modbus::ModbusTelemetryWidget* telemetryWidget() const noexcept { return telemetryWidget_; }
    [[nodiscard]] QLabel* protocolLabel() const noexcept { return protocolLabel_; }
    [[nodiscard]] QLabel* statusBannerLabel() const noexcept { return statusBannerLabel_; }
    [[nodiscard]] QComboBox* protocolCombo() const noexcept { return protocolCombo_; }

signals:
    void linkageDataReceived(const ::modbus::base::Pdu& pdu, ::modbus::parser::ProtocolType protocol, uint16_t addr);
    void linkageToggled(bool active);
    void linkageSourceDisconnected();

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onProtocolChanged(int index);
    void onTcpConnectClicked(const QString& ip, int port);
    void onSerialConnectClicked(const io::SerialConfig& config);
    void onDisconnectClicked();

private:
    void setupUi();
    void setupConnectionBar();
    void setupStatusBanner(QVBoxLayout* mainLayout);
    void bindConnectionBarSignals();
    void setupSplitterBody();
    void switchToProtocol(ui::application::modbus::SessionMode mode);
    void applyProtocolSettingsGroup(ui::application::modbus::SessionMode mode);
    void retranslateUi();

    infra::config::ISettingsService* settingsService_ = nullptr;
    ui::application::modbus::ModbusPagePresenter* pagePresenter_ = nullptr;
    ui::application::modbus::SessionMode currentMode_ = ui::application::modbus::SessionMode::Tcp;

    QLabel* protocolLabel_ = nullptr;
    QComboBox* protocolCombo_ = nullptr;
    QStackedWidget* connectionStack_ = nullptr;
    ui::widgets::TcpConnectionWidget* tcpConnectionWidget_ = nullptr;
    ui::widgets::SerialConnectionWidget* serialConnectionWidget_ = nullptr;
    ui::widgets::BaseConnectionWidget* connectionWidget_ = nullptr;

    ui::widgets::modbus::ModbusWorkbenchWidget* workbenchWidget_ = nullptr;
    ui::widgets::modbus::ModbusTelemetryWidget* telemetryWidget_ = nullptr;
    QLabel* statusBannerLabel_ = nullptr;
    QSplitter* splitter_ = nullptr;
};

} // namespace ui::views::modbus
