/**
 * @file TcpConnectionWidget.h
 * @brief Unified TCP connection widget — Client or Server role selected at construction.
 *
 * Merges TcpClientConnectionWidget and TcpServerConnectionWidget (P2-v2-8).
 * Uses TcpRole{Client, Server} to parameterize behaviour.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "NetworkConnectionWidget.h"

namespace core::common {
class ISettingsService;
}

namespace ui::widgets {

enum class TcpRole { Client, Server };

class TcpConnectionWidget : public NetworkConnectionWidget {
    Q_OBJECT

public:
    using DisplayState = BaseConnectionWidget::DisplayState;

    explicit TcpConnectionWidget(TcpRole role, core::common::ISettingsService* settingsService, QWidget* parent = nullptr);
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