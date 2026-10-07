// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "NetworkConnectionWidget.h"

class QLineEdit;
class QSpinBox;
class QLabel;

namespace infra::config {
class ISettingsService;
}

namespace ui::widgets {

class UdpConnectionWidget : public NetworkConnectionWidget {
    Q_OBJECT

public:
    using DisplayState = BaseConnectionWidget::DisplayState;

    explicit UdpConnectionWidget(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
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
