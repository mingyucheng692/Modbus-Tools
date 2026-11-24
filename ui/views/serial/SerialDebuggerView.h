// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QWidget>
#include <functional>
#include "application/ChannelSessionController.h"
#include "infra/io/IChannel.h"
#include "infra/io/SerialConfig.h"

class QCheckBox;

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

class SerialDebuggerView : public QWidget {
    Q_OBJECT

public:
    using DisconnectAlertHandler = std::function<void()>;

    explicit SerialDebuggerView(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~SerialDebuggerView() noexcept override;

    void setDisconnectAlertHandler(DisconnectAlertHandler handler) noexcept;
    [[nodiscard]] bool isConnected() const noexcept;
    [[nodiscard]] QCheckBox* dtrCheck() const noexcept { return dtrCheck_; }
    [[nodiscard]] QCheckBox* rtsCheck() const noexcept { return rtsCheck_; }

signals:
    void unexpectedDisconnectDetected();
    void serialControlLinesApplied(bool dtr, bool rts);

public slots:
    void onWorkerStateChanged(io::ChannelState state, quint64 generation = 0);
    void onWorkerError(const QString& deviceHint, io::ChannelErrorCode code, const QString& error);

private slots:
    void onConnectClicked(const io::SerialConfig& config);
    void onDisconnectClicked();
    void onSessionSnapshotChanged(const application::SessionUiSnapshot& snap);

    // Serial Control
    void onDtrChanged(bool checked);
    void onRtsChanged(bool checked);

protected:
    void retranslateUi();
    void changeEvent(QEvent* event) override;

private:
    void setupUi();
    void applySerialControlLines();

    // UI Components
    widgets::SerialConnectionWidget* connectionWidget_ = nullptr;
    widgets::CollapsibleSection* controlSection_ = nullptr;
    widgets::ByteMonitorWidget* monitor_ = nullptr;
    widgets::GenericInputWidget* inputWidget_ = nullptr;
    widgets::CollapsibleSection* inputSection_ = nullptr;

    QCheckBox* dtrCheck_ = nullptr;
    QCheckBox* rtsCheck_ = nullptr;

    // Session Controller
    application::ChannelSessionController sessionCtrl_;
    DisconnectAlertHandler disconnectAlertHandler_;

    infra::config::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::views::serial
