// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "NetworkConnectionWidget.h"

namespace infra::config {
class ISettingsService;
}

namespace ui::widgets {

enum class TcpRole { Client, Server };

// Role: Unified TCP connection widget parameterised for Client or Server roles.
// Thread-safety: Main-thread only (QWidget derivative).
// Ownership: Owned by parent QWidget via Qt object hierarchy.
class TcpConnectionWidget : public NetworkConnectionWidget {
    Q_OBJECT

public:
    using DisplayState = BaseConnectionWidget::DisplayState;

    explicit TcpConnectionWidget(TcpRole role, infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~TcpConnectionWidget() override;

    [[nodiscard]] TcpRole role() const noexcept { return role_; }

signals:
    void connectClicked(const QString& ip, int port);
    void startListenClicked(const QString& ip, int port);
    void stopListenClicked();

protected:
    void setupProtocolUi() override;
    void updateProtocolUi() override;
    void setupButtonConnection() override;

    [[nodiscard]] StateDisplayInfo getStateDisplayInfo(DisplayState state) const override;
    [[nodiscard]] bool isActiveState(DisplayState state) const override;
    [[nodiscard]] DisplayState connectedState() const override;

private:
    TcpRole role_;
};

} // namespace ui::widgets