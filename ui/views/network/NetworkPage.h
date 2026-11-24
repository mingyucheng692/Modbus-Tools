// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QWidget>
#include <QList>
#include <functional>
#include "../../application/ChannelSessionController.h"
#include "../../../infra/io/IChannel.h"

class QComboBox;
class QEvent;

namespace infra::config {
class ISettingsService;
}

namespace ui::widgets {
class CompactStackedWidget;
class TcpConnectionWidget;
class UdpConnectionWidget;
class BaseConnectionWidget;
class ServerClientPanel;
class ByteMonitorWidget;
class GenericInputWidget;
class CollapsibleSection;
}

namespace ui::views::network {

// Role: Self-contained network debugging view supporting TCP Client, TCP Server, and UDP.
// Thread-safety: Single-threaded (bound to GUI main thread).
// Ownership: Owned by QObject parent hierarchy; delegates IO to ChannelSessionController.
class NetworkPage : public QWidget {
    Q_OBJECT

public:
    enum class Protocol {
        TcpClient = 0,
        TcpServer,
        Udp
    };
    Q_ENUM(Protocol)

    using DisconnectAlertHandler = std::function<void()>;

    explicit NetworkPage(infra::config::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~NetworkPage() noexcept override;

    void setDisconnectAlertHandler(DisconnectAlertHandler handler) noexcept;
    [[nodiscard]] bool isConnected() const noexcept;

signals:
    void unexpectedDisconnectDetected();

public slots:
    void onConnectClicked(const QString& ip, int port);
    void onDisconnectClicked();
    void onStartListenClicked(const QString& ip, int port);
    void onStopListenClicked();
    void onBindClicked(const QString& localIp, int localPort,
                       const QString& remoteIp, int remotePort);
    void onUnbindClicked();
    void onProtocolChanged(int index);
    void onSendRequested(const QByteArray& data);
    void onWorkerStateChanged(io::ChannelState state, quint64 gen = 0);

private slots:
    void onSessionSnapshotChanged(const application::SessionUiSnapshot& snap);
    void onWorkerError(const QString& deviceHint, io::ChannelErrorCode code, const QString& error);

private:
    void setupUi();
    void retranslateUi();
    void switchToProtocol(Protocol protocol);
    [[nodiscard]] widgets::BaseConnectionWidget* activeConnectionWidget() const noexcept;
    void changeEvent(QEvent* event) override;

    infra::config::ISettingsService* settingsService_ = nullptr;
    application::ChannelSessionController sessionCtrl_;

    Protocol currentProtocol_ = Protocol::TcpClient;
    DisconnectAlertHandler disconnectAlertHandler_;

    // UI Components
    widgets::CollapsibleSection* protocolSection_ = nullptr;
    QComboBox* protocolCombo_ = nullptr;
    widgets::CompactStackedWidget* connectionStack_ = nullptr;
    widgets::TcpConnectionWidget* tcpClientWidget_ = nullptr;
    widgets::TcpConnectionWidget* tcpServerWidget_ = nullptr;
    widgets::UdpConnectionWidget* udpWidget_ = nullptr;
    widgets::ServerClientPanel* serverClientPanel_ = nullptr;
    widgets::ByteMonitorWidget* monitor_ = nullptr;
    widgets::GenericInputWidget* inputWidget_ = nullptr;
    widgets::CollapsibleSection* inputSection_ = nullptr;
};

} // namespace ui::views::network
