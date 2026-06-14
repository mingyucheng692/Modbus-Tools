/**
 * @file UdpConnectionWidget.h
 * @brief UDP connection widget — Local/Remote IP/Port with Bind/Unbind button.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "NetworkConnectionWidget.h"

class QLineEdit;
class QSpinBox;
class QLabel;

namespace ui::widgets {

class UdpConnectionWidget : public NetworkConnectionWidget {
    Q_OBJECT

public:
    using DisplayState = BaseConnectionWidget::DisplayState;

    explicit UdpConnectionWidget(ui::common::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~UdpConnectionWidget() override;

signals:
    void bindClicked(const QString& localIp, int localPort,
                     const QString& remoteIp, int remotePort);
    void unbindClicked();

protected:
    void loadSettings() override;
    void saveSettings() override;
    void setupProtocolUi() override;
    void updateProtocolUi() override;
    void setupButtonConnection() override;

    [[nodiscard]] StateDisplayInfo getStateDisplayInfo(DisplayState state) const override;
    [[nodiscard]] bool isActiveState(DisplayState state) const override;
    [[nodiscard]] DisplayState connectedState() const override;

private:
    QLabel* remoteIpLabel_ = nullptr;
    QLineEdit* remoteIpEdit_ = nullptr;
    QLabel* remotePortLabel_ = nullptr;
    QSpinBox* remotePortEdit_ = nullptr;
};

} // namespace ui::widgets