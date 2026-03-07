#pragma once

#include <QWidget>
#include <memory>
#include <unordered_map>
#include <chrono>

class QVBoxLayout;
class QLabel;
class QGroupBox;
class QCheckBox;
class QTextEdit;
class QPushButton;
class QEvent;

namespace modbus::dispatch { class ModbusWorker; }
namespace modbus::session { class IModbusClient; }
namespace io { class IChannel; }
class QThread;

namespace ui::widgets {
    class SerialConnectionWidget;
    class FunctionWidget;
    class TrafficMonitorWidget;
    class ControlWidget;
}

namespace ui::views::rtu {

class RtuView : public QWidget {
    Q_OBJECT

public:
    explicit RtuView(QWidget *parent = nullptr);
    ~RtuView() override;

private:
    void setupUi();
    void appendReceiveData(const QByteArray& data);
    void appendSendData(const QByteArray& data);
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
    QGroupBox* dataGroup_ = nullptr;
    QGroupBox* receiveGroup_ = nullptr;
    QGroupBox* sendGroup_ = nullptr;
    QTextEdit* receiveTextEdit_ = nullptr;
    QTextEdit* sendTextEdit_ = nullptr;
    QCheckBox* receiveHexCheck_ = nullptr;
    QCheckBox* sendHexCheck_ = nullptr;
    QPushButton* clearReceiveButton_ = nullptr;
    QPushButton* clearSendButton_ = nullptr;

    std::shared_ptr<io::IChannel> channel_;
    std::shared_ptr<modbus::session::IModbusClient> client_;
    std::shared_ptr<modbus::dispatch::ModbusWorker> worker_;
    std::shared_ptr<QThread> workerThread_;
    std::unordered_map<int, std::chrono::steady_clock::time_point> requestStart_;
    std::unordered_map<int, RequestKind> requestKinds_;
    int requestId_ = 0;
};

} // namespace ui::views::rtu
