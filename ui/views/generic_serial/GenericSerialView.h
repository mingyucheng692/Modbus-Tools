#pragma once

#include <QWidget>
#include <memory>
#include "../../../core/io/SerialChannel.h"
#include "../../../core/io/IChannel.h"

namespace io {
class GenericIoWorker;
}

namespace ui::widgets {
class SerialConnectionWidget;
class TrafficMonitorWidget;
class GenericInputWidget;
class CollapsibleSection;
}

class QVBoxLayout;
class QHBoxLayout;
class QSplitter;
class QCheckBox;
class QThread;
class QGroupBox;
class QEvent;

namespace ui::common {
class ISettingsService;
}

namespace ui::views::generic_serial {

class GenericSerialView : public QWidget {
    Q_OBJECT

public:
    explicit GenericSerialView(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~GenericSerialView() override;

private slots:
    void onConnectClicked(const io::SerialConfig& config);
    void onDisconnectClicked();
    void onSendRequested(const QByteArray& data);
    void onWorkerStateChanged(io::ChannelState state);
    void onWorkerError(const QString& error);
    void onWorkerMonitor(bool isTx, const QByteArray& data);
    
    // Serial Control
    void onDtrChanged(bool checked);
    void onRtsChanged(bool checked);

private:
    void setupUi();
    void startWorker();
    void stopWorker();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    // UI Components
    widgets::SerialConnectionWidget* connectionWidget_ = nullptr;
    widgets::TrafficMonitorWidget* trafficMonitor_ = nullptr;
    widgets::GenericInputWidget* inputWidget_ = nullptr;
    
    QCheckBox* dtrCheck_ = nullptr;
    QCheckBox* rtsCheck_ = nullptr;
    QGroupBox* controlGroup_ = nullptr;
    widgets::CollapsibleSection* inputSection_ = nullptr;

    // Backend
    io::GenericIoWorker* worker_ = nullptr;
    QThread* workerThread_ = nullptr;
    bool isConnected_ = false;
    ui::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::views::generic_serial
