#pragma once

#include <QWidget>
#include <memory>
#include <QThread>

namespace io {
class GenericIoWorker;
}

namespace ui::widgets {
class TcpConnectionWidget;
class TrafficMonitorWidget;
class GenericInputWidget;
}

class QVBoxLayout;
class QHBoxLayout;
class QSplitter;
class QGroupBox;
class QEvent;

namespace ui::views::generic_tcp {

class GenericTcpView : public QWidget {
    Q_OBJECT

public:
    explicit GenericTcpView(QWidget *parent = nullptr);
    ~GenericTcpView() override;

private slots:
    void onConnectClicked(const QString& ip, int port);
    void onDisconnectClicked();
    void onSendRequested(const QByteArray& data);
    void onWorkerStateChanged(int state);
    void onWorkerError(const QString& error);
    void onWorkerDataReceived(const QByteArray& data);
    void onWorkerMonitor(bool isTx, const QByteArray& data);
    void onWorkerBytesWritten(qint64 bytes);

private:
    void setupUi();
    void startWorker();
    void stopWorker();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    // UI Components
    widgets::TcpConnectionWidget* connectionWidget_ = nullptr;
    widgets::TrafficMonitorWidget* trafficMonitor_ = nullptr;
    widgets::GenericInputWidget* inputWidget_ = nullptr;
    QGroupBox* inputGroup_ = nullptr;

    // Backend
    std::unique_ptr<io::GenericIoWorker> worker_; // We might move this to a thread, so unique_ptr
    QThread* workerThread_ = nullptr;
};

} // namespace ui::views::generic_tcp
