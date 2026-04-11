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
    class TcpConnectionWidget;
    class FunctionWidget;
    class TrafficMonitorWidget;
    class ControlWidget;
    class CollapsibleSection;
}

namespace ui::views::modbus_tcp {

class ModbusTcpView : public QWidget {
    Q_OBJECT

public:
    explicit ModbusTcpView(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~ModbusTcpView() override;
    void updateModbusSettings(int timeoutMs, int retries, int retryIntervalMs);
    void setLinked(bool linked);

signals:
    void linkageDataReceived(const modbus::base::Pdu& pdu, modbus::core::parser::ProtocolType protocol, uint16_t addr);

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

    enum class RequestKind { Read, Write, Poll };

    QVBoxLayout* mainLayout_ = nullptr;
    ui::widgets::TcpConnectionWidget* connectionWidget_ = nullptr;
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
    bool tcpSessionConnected_ = false;
    bool suppressDisconnectAlert_ = false;
    int timeoutMs_ = app::constants::Values::Modbus::kDefaultTimeoutMs;
    int retries_ = 0;
    int retryIntervalMs_ = app::constants::Values::Modbus::kDefaultRetryIntervalMs;
    modbus::base::ModbusConfig currentConfig_;
    ui::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::views::modbus_tcp
