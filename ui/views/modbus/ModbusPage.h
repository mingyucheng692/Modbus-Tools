// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QWidget>
#include <QString>
#include <QMap>
#include "application/modbus/IModbusPageView.h"
#include "application/modbus/ModbusTypes.h"
#include "modbus/base/ModbusTypes.h"
#include "analyzer/AnalyzerCommon.h"
#include "infra/io/SerialConfig.h"

class QComboBox;
class QLabel;
class QStackedWidget;
class QSplitter;
class QVBoxLayout;

namespace infra::config {
class ISettingsService;
}

namespace core::common {
class SettingsController;
}

namespace ui::widgets {
class BaseConnectionWidget;
class TcpConnectionWidget;
class SerialConnectionWidget;
class CollapsibleSection;
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
    void setSettingsController(core::common::SettingsController* controller) noexcept { settingsController_ = controller; }

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
    void setMetadataMap(const QMap<uint16_t, ::modbus::analyzer::DataMetadata>& meta);
    [[nodiscard]] ::modbus::base::ModbusMode currentMode() const noexcept { return currentMode_; }

    [[nodiscard]] ui::widgets::modbus::ModbusWorkbenchWidget* workbenchWidget() const noexcept { return workbenchWidget_; }
    [[nodiscard]] ui::widgets::modbus::ModbusTelemetryWidget* telemetryWidget() const noexcept { return telemetryWidget_; }
    [[nodiscard]] QLabel* protocolLabel() const noexcept { return protocolLabel_; }
    [[nodiscard]] QLabel* statusBannerLabel() const noexcept { return statusBannerLabel_; }
    [[nodiscard]] QComboBox* protocolCombo() const noexcept { return protocolCombo_; }

signals:
    void linkageDataReceived(const ::modbus::base::Pdu& pdu, ::modbus::parser::ProtocolType protocol, uint16_t addr);
    void linkageToggled(bool active);
    void linkageSourceDisconnected();
    void inspectInAnalyzerRequested(const QString& hexString);
    void addressBaseChanged(::modbus::address::AddressBase base);

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
    void switchToProtocol(::modbus::base::ModbusMode mode);
    void applyProtocolSettingsGroup(::modbus::base::ModbusMode mode);
    void retranslateUi();

    infra::config::ISettingsService* settingsService_ = nullptr;
    core::common::SettingsController* settingsController_ = nullptr;
    ui::application::modbus::ModbusPagePresenter* pagePresenter_ = nullptr;
    ::modbus::base::ModbusMode currentMode_ = ::modbus::base::ModbusMode::TCP;

    QLabel* protocolLabel_ = nullptr;
    QComboBox* protocolCombo_ = nullptr;
    ui::widgets::CollapsibleSection* protoSection_ = nullptr;
    QStackedWidget* connectionStack_ = nullptr;
    ui::widgets::TcpConnectionWidget* tcpConnectionWidget_ = nullptr;
    ui::widgets::SerialConnectionWidget* serialConnectionWidget_ = nullptr;
    ui::widgets::BaseConnectionWidget* activeConnectionWidget_ = nullptr;

    ui::widgets::modbus::ModbusWorkbenchWidget* workbenchWidget_ = nullptr;
    ui::widgets::modbus::ModbusTelemetryWidget* telemetryWidget_ = nullptr;
    QLabel* statusBannerLabel_ = nullptr;
    QSplitter* splitter_ = nullptr;
};

} // namespace ui::views::modbus
