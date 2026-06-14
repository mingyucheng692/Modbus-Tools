/**
 * @file TcpServerConnectionWidget.h
 * @brief TCP Server connection widget — Listen IP/Port with Start/Stop button.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "NetworkConnectionWidget.h"

namespace ui::widgets {

class TcpServerConnectionWidget : public NetworkConnectionWidget {
    Q_OBJECT

public:
    using DisplayState = BaseConnectionWidget::DisplayState;

    explicit TcpServerConnectionWidget(ui::common::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~TcpServerConnectionWidget() override;

signals:
    void startListenClicked(const QString& ip, int port);
    void stopListenClicked();

public slots:
    void setConnected(bool connected) override;

protected:
    void applyDisplayState() override;
    void setupProtocolUi() override;
    void updateProtocolUi() override;

private:
    void setupUi();
};

} // namespace ui::widgets