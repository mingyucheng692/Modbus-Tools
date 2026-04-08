#pragma once

#include <QWidget>
#include <memory>
#include <unordered_map>
#include <chrono>
#include "AppConstants.h"
#include "modbus/base/ModbusConfig.h"

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
    int requestId_ = 0;
    quint64 connectionGeneration_ = 0;
    bool rtuSessionConnected_ = false;
    int timeoutMs_ = app::constants::Values::Modbus::kDefaultTimeoutMs;
    int retries_ = 0;
    int retryIntervalMs_ = app::constants::Values::Modbus::kDefaultRetryIntervalMs;
    modbus::base::ModbusConfig currentConfig_;
    ui::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::views::modbus_rtu
