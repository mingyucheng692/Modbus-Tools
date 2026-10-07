// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "BaseConnectionWidget.h"
#include "Config.h"

class QLineEdit;
class QSpinBox;
class QLabel;


namespace infra::config {
class ISettingsService;
}

namespace ui::widgets {

// Role: Common base widget for TCP/UDP network connection panels (IP/port fields, layout).
// Thread-safety: Single-threaded (bound to GUI thread).
// Ownership: Owned by QObject parent hierarchy.
class NetworkConnectionWidget : public BaseConnectionWidget {
    Q_OBJECT

public:
    using DisplayState = BaseConnectionWidget::DisplayState;

    explicit NetworkConnectionWidget(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~NetworkConnectionWidget() override;

    void setDefaultPort(int port);
    [[nodiscard]] QString getIpAddress() const;
    [[nodiscard]] int getPort() const;

public slots:
    void setConnected(bool connected) override;

protected:
    // ---- Settings ----
    void loadSettings() override;
    void saveSettings() override;
    void retranslateUi();

    // ---- UI Setup ----
    void setupCommonUi();
    void setupNetworkUi();
    virtual void setupProtocolUi() = 0;
    virtual void setupButtonConnection() = 0;
    void updateProtocolUi() override = 0;

    // ---- Display State (data hooks) ----
    // applyDisplayState() skeleton and inputsLocked() live in BaseConnectionWidget.
    // NetworkConnectionWidget only contributes the per-protocol
    // data via the following pure-virtual hooks.
    [[nodiscard]] StateDisplayInfo getStateDisplayInfo(DisplayState state) const override = 0;
    void applyInputWidgetsState(bool enabled) override;
    [[nodiscard]] virtual bool isActiveState(DisplayState state) const = 0;
    [[nodiscard]] virtual DisplayState connectedState() const = 0;

    QLabel* hostLabel_ = nullptr;
    QLabel* portLabel_ = nullptr;
    QLineEdit* ipEdit_ = nullptr;
    QSpinBox* portEdit_ = nullptr;
    int defaultPort_ = config::Network::kDefaultModbusTcpPort;
};

} // namespace ui::widgets
