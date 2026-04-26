/**
 * @file ModbusRtuView.h
 * @brief Header file for ModbusRtuView.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <cstdint>
#include "AppConstants.h"
#include "modbus/base/ModbusConfig.h"
#include "modbus/base/ModbusFrame.h"
#include "modbus/parser/ModbusFrameParser.h"

class QVBoxLayout;
class QLabel;
class QGroupBox;
class QCheckBox;
class QTextEdit;
class QPushButton;
class QEvent;

namespace ui::common {
class ISettingsService;
}

namespace modbus::dispatch { class ModbusWorker; }
namespace modbus::session { class IModbusClient; }
namespace io { class IChannel; }
class QThread;

namespace ui::widgets {
    class SerialConnectionWidget;
    class FunctionWidget;
    class TrafficMonitorWidget;
    class ControlWidget;
    class CollapsibleSection;
}

namespace ui::views::modbus_rtu {

class ModbusRtuView : public QWidget {
    Q_OBJECT

public:
    explicit ModbusRtuView(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~ModbusRtuView() override;
    void updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs);
    void setLinked(bool linked);
    bool isLinked() const;

signals:
    void pollRequested(uint8_t fc, int addr, int qty);
    void linkageDataReceived(const modbus::base::Pdu& pdu, modbus::core::parser::ProtocolType protocol, uint16_t addr);
    void linkageToggled(bool active);

private:
    void setupUi();
    void appendReceiveData(const QByteArray& data);
    void appendSendData(const QByteArray& data);
    void refreshReceiveDisplay();
    void refreshSendDisplay();
    QString formatData(const QByteArray& data, bool hex) const;
    void retranslateUi();
    void changeEvent(QEvent* event) override;
    void releaseStack();
    int nextRequestId();
    void appendConnectionInfo(const QString& message);
    void flushPollSummary(bool force);
    void handlePollCompletion(bool success, int rttMs, const QString& error);

    enum class RequestKind { Read, Write, Poll };

    QVBoxLayout* mainLayout_ = nullptr;
    ui::widgets::SerialConnectionWidget* connectionWidget_ = nullptr;
    ui::widgets::FunctionWidget* functionWidget_ = nullptr;
    ui::widgets::TrafficMonitorWidget* trafficMonitor_ = nullptr;
    ui::widgets::ControlWidget* controlWidget_ = nullptr;
    ui::widgets::CollapsibleSection* dataGroup_ = nullptr;
    QGroupBox* receiveGroup_ = nullptr;
    QGroupBox* sendGroup_ = nullptr;
    QTextEdit* receiveTextEdit_ = nullptr;
    QTextEdit* sendTextEdit_ = nullptr;
    QCheckBox* receiveHexCheck_ = nullptr;
    QCheckBox* sendHexCheck_ = nullptr;
    QPushButton* copyReceiveButton_ = nullptr;
    QPushButton* copySendButton_ = nullptr;
    QPushButton* clearReceiveButton_ = nullptr;
    QPushButton* clearSendButton_ = nullptr;
    QByteArray lastReceiveFrame_;
    QByteArray lastSendFrame_;

    std::shared_ptr<io::IChannel> channel_;
    std::shared_ptr<modbus::session::IModbusClient> client_;
    std::shared_ptr<modbus::dispatch::ModbusWorker> worker_;
    std::shared_ptr<QThread> workerThread_;
    std::unordered_map<int, std::chrono::steady_clock::time_point> requestStart_;
    std::unordered_map<int, RequestKind> requestKinds_;
    std::unordered_map<int, uint16_t> requestAddrs_;
    int requestId_ = 0;
    quint64 connectionGeneration_ = 0;
    bool rtuSessionConnected_ = false;
    bool pollInFlight_ = false;
    bool suppressPollTrafficLog_ = false;
    std::chrono::steady_clock::time_point pollSummaryWindowStart_{};
    int pollSummarySuccessCount_ = 0;
    int pollSummaryErrorCount_ = 0;
    qint64 pollSummaryTotalRttMs_ = 0;
    uint8_t pollFunctionCode_ = 0;
    int pollAddress_ = 0;
    int pollQuantity_ = 0;
    int pollSlaveId_ = 0;
    bool linked_ = false;
    int timeoutMs_ = app::constants::Values::Modbus::kDefaultTimeoutMs;
    int retries_ = 0;
    int retryIntervalMs_ = app::constants::Values::Modbus::kDefaultRetryIntervalMs;
    modbus::base::ModbusConfig currentConfig_;
    ui::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::views::modbus_rtu
