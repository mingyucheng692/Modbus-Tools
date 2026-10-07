// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QWidget>
#include <QByteArray>
#include <QString>
#include <cstdint>
#include "modbus/base/ModbusFrame.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "../../application/modbus/ModbusTypes.h"

#include "modbus/base/ModbusAddressMapping.h"

class QVBoxLayout;
class QGroupBox;
class QTextEdit;
class QPushButton;
class QComboBox;
class QStackedWidget;
class QEvent;
class QTimer;

namespace infra::config {
class ISettingsService;
}

namespace ui::application::modbus {
class ModbusPagePresenter;
}

namespace ui::widgets {
class BaseConnectionWidget;
class TcpConnectionWidget;
class SerialConnectionWidget;
class FunctionWidget;
class TrafficMonitorWidget;
class ControlWidget;
class CollapsibleSection;
}

namespace ui::views::modbus {

// Role: View for Modbus debugging 鈥?manages UI widgets and delegates orchestration to ModbusPagePresenter.
// Thread-safety: Single-threaded (bound to GUI thread).
// Ownership: Owned by QObject parent hierarchy (typically MainWindow); owns child UI widgets.
class ModbusPage : public QWidget {
    Q_OBJECT

public:
    explicit ModbusPage(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~ModbusPage() noexcept override;

    void updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs);
    void updateAddressBase(::modbus::address::AddressBase base);
    void setLinked(bool linked);
    [[nodiscard]] bool isLinked() const;
    void appendTrafficData(bool isTx, const QByteArray& data);

    [[nodiscard]] ui::application::modbus::SessionMode currentMode() const noexcept { return currentMode_; }

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
    void switchToProtocol(ui::application::modbus::SessionMode mode);
    void applyProtocolSettingsGroup(ui::application::modbus::SessionMode mode);
    void setupViewOnlyConnections();
    void retranslateUi();

    void appendReceiveData(const QByteArray& data);
    void appendSendData(const QByteArray& data);
    void flushPendingTrafficDisplay();
    void refreshReceiveDisplay();
    void refreshSendDisplay();
    [[nodiscard]] QString formatData(const QByteArray& data, bool hex) const;
    [[nodiscard]] QString buildDisplayText(const QString& directionLabel,
                                           const QByteArray& data,
                                           bool hex) const;

    // Layout
    QVBoxLayout* mainLayout_ = nullptr;
    QComboBox* protocolCombo_ = nullptr;
    QStackedWidget* connectionStack_ = nullptr;
    ui::widgets::TcpConnectionWidget* tcpConnectionWidget_ = nullptr;
    ui::widgets::SerialConnectionWidget* serialConnectionWidget_ = nullptr;
    ui::widgets::BaseConnectionWidget* connectionWidget_ = nullptr; // currently active

    ui::widgets::FunctionWidget* functionWidget_ = nullptr;
    ui::widgets::TrafficMonitorWidget* trafficMonitor_ = nullptr;
    ui::widgets::ControlWidget* controlWidget_ = nullptr;
    ui::widgets::CollapsibleSection* dataGroup_ = nullptr;
    QGroupBox* receiveGroup_ = nullptr;
    QGroupBox* sendGroup_ = nullptr;
    QTextEdit* receiveTextEdit_ = nullptr;
    QTextEdit* sendTextEdit_ = nullptr;
    QPushButton* copyReceiveButton_ = nullptr;
    QPushButton* copySendButton_ = nullptr;
    QPushButton* clearReceiveButton_ = nullptr;
    QPushButton* clearSendButton_ = nullptr;
    QTimer* dataFlushTimer_ = nullptr;
    QByteArray lastReceiveFrame_;
    QByteArray lastSendFrame_;
    QString lastRenderedReceiveText_;
    QString lastRenderedSendText_;
    bool receiveDirty_ = false;
    bool sendDirty_ = false;

    ui::application::modbus::ModbusPagePresenter* pagePresenter_ = nullptr;

    infra::config::ISettingsService* settingsService_ = nullptr;
    ui::application::modbus::SessionMode currentMode_ = ui::application::modbus::SessionMode::Tcp;
};

} // namespace ui::views::modbus
