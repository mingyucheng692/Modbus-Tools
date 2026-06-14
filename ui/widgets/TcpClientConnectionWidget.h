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

namespace ui::widgets {

class TcpClientConnectionWidget : public NetworkConnectionWidget {
    Q_OBJECT

public:
    using DisplayState = BaseConnectionWidget::DisplayState;

    explicit TcpClientConnectionWidget(ui::common::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~TcpClientConnectionWidget() override;

signals:
    void connectClicked(const QString& ip, int port);

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