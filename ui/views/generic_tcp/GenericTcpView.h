#pragma once

#include <QWidget>
#include <memory>
#include <QThread>
#include "../../../core/io/IChannel.h"

namespace io {
class ChannelOperationWorker;
}

namespace ui::widgets {
class TcpConnectionWidget;
class TrafficMonitorWidget;
class GenericInputWidget;
class CollapsibleSection;
}

class QVBoxLayout;
class QHBoxLayout;
class QSplitter;
class QEvent;

namespace ui::common {
class ISettingsService;
}

namespace ui::views::generic_tcp {

class GenericTcpView : public QWidget {
    Q_OBJECT

public:
    explicit GenericTcpView(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~GenericTcpView() override;

private slots:
    void onConnectClicked(const QString& ip, int port);
    void onDisconnectClicked();
    void onSendRequested(const QByteArray& data);
    void onFileSendRequested(const QString& filePath);
    void onWorkerStateChanged(io::ChannelState state, quint64 generation);
    void onWorkerError(const QString& deviceHint, const QString& error);
    void onWorkerMonitor(bool isTx, const QByteArray& data);

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
    widgets::CollapsibleSection* inputSection_ = nullptr;

    // Backend
    io::ChannelOperationWorker* worker_ = nullptr;
    QThread* workerThread_ = nullptr;
    bool isConnected_ = false;
    bool suppressDisconnectAlert_ = false;
    quint64 connectionGeneration_ = 0;
    ui::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::views::generic_tcp
