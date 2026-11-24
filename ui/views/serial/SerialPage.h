// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QWidget>
#include <functional>
#include "../../application/ChannelSessionController.h"
#include "../../../infra/io/IChannel.h"

class QCheckBox;
class QEvent;

namespace infra::config {
class ISettingsService;
}

namespace ui::widgets {
class SerialConnectionWidget;
class ByteMonitorWidget;
class GenericInputWidget;
class CollapsibleSection;
}

namespace ui::views::serial {

// Role: Self-contained serial port debugging page with hardware control lines and session lifecycle.
// Thread-safety: Single-threaded (bound to GUI main thread).
// Ownership: Owned by QObject parent hierarchy; delegates IO to ChannelSessionController.
class SerialPage : public QWidget {
    Q_OBJECT

public:
    using DisconnectAlertHandler = std::function<void()>;

    explicit SerialPage(infra::config::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~SerialPage() noexcept override;

    void setDisconnectAlertHandler(DisconnectAlertHandler handler) noexcept;
    [[nodiscard]] bool isConnected() const noexcept;

signals:
    void unexpectedDisconnectDetected();
    void serialControlLinesApplied(bool dtr, bool rts);

public slots:
    void onConnectClicked(const io::SerialConfig& config);
    void onDisconnectClicked();
    void onWorkerStateChanged(io::ChannelState state, quint64 gen = 0);
    void onSessionSnapshotChanged(const application::SessionUiSnapshot& snap);
    void onWorkerError(const QString& deviceHint, io::ChannelErrorCode code, const QString& error);

private slots:
    void onDtrChanged(bool checked);
    void onRtsChanged(bool checked);

private:
    void setupUi();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    infra::config::ISettingsService* settingsService_ = nullptr;
    application::ChannelSessionController sessionCtrl_;

    DisconnectAlertHandler disconnectAlertHandler_;

    // UI Components
    widgets::SerialConnectionWidget* connectionWidget_ = nullptr;
    widgets::CollapsibleSection* controlSection_ = nullptr;
    QCheckBox* dtrCheck_ = nullptr;
    QCheckBox* rtsCheck_ = nullptr;

    widgets::ByteMonitorWidget* monitor_ = nullptr;
    widgets::GenericInputWidget* inputWidget_ = nullptr;
    widgets::CollapsibleSection* inputSection_ = nullptr;
};

} // namespace ui::views::serial
