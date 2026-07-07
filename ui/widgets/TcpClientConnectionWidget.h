/**
 * @file TcpClientConnectionWidget.h
 * @brief TCP Client connection widget — IP/Port input with Connect/Disconnect button.
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

class TcpClientConnectionWidget : public NetworkConnectionWidget {
    Q_OBJECT

public:
    using DisplayState = BaseConnectionWidget::DisplayState;

    explicit TcpClientConnectionWidget(core::common::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~TcpClientConnectionWidget() override;

signals:
    void connectClicked(const QString& ip, int port);

protected:
    void setupProtocolUi() override;
    void updateProtocolUi() override;
    void setupButtonConnection() override;

    [[nodiscard]] StateDisplayInfo getStateDisplayInfo(DisplayState state) const override;
    [[nodiscard]] bool isActiveState(DisplayState state) const override;
    [[nodiscard]] DisplayState connectedState() const override;
};

} // namespace ui::widgets