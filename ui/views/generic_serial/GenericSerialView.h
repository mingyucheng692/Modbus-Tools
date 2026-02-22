#pragma once

#include <QWidget>
#include <memory>
#include "../../../core/io/SerialChannel.h" // For SerialConfig

namespace io {
class GenericIoWorker;
}

namespace ui::widgets {
class SerialConnectionWidget;
class TrafficMonitorWidget;
class GenericInputWidget;
class QuickCommandWidget;
}

class QVBoxLayout;
class QHBoxLayout;
class QSplitter;
class QCheckBox;
class QThread;

namespace ui::views::generic_serial {

class GenericSerialView : public QWidget {
    Q_OBJECT

public:
    explicit GenericSerialView(QWidget *parent = nullptr);
    ~GenericSerialView() override;

private slots:
    void onConnectClicked(const io::SerialConfig& config);
    void onDisconnectClicked();
    void onSendRequested(const QByteArray& data);
    void onWorkerStateChanged(int state);
    void onWorkerError(const QString& error);
    void onWorkerDataReceived(const QByteArray& data);
    void onWorkerMonitor(bool isTx, const QByteArray& data);
    
    // Serial Control
    void onDtrChanged(bool checked);
    void onRtsChanged(bool checked);

private:
    void setupUi();
    void startWorker();
    void stopWorker();

    // UI Components
    widgets::SerialConnectionWidget* connectionWidget_ = nullptr;
    widgets::TrafficMonitorWidget* trafficMonitor_ = nullptr;
    widgets::GenericInputWidget* inputWidget_ = nullptr;
    widgets::QuickCommandWidget* quickCommandWidget_ = nullptr;
    
    QCheckBox* dtrCheck_ = nullptr;
    QCheckBox* rtsCheck_ = nullptr;

    // Backend
    std::unique_ptr<io::GenericIoWorker> worker_; 
    QThread* workerThread_ = nullptr;
};

} // namespace ui::views::generic_serial
