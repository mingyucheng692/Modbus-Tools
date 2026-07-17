/**
 * @file ConnectionAlert.h
 * @brief Header file for ConnectionAlert.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QCoreApplication>
#include <QMessageBox>

class QWidget;

namespace ui::common {

namespace connection_alert {

Q_DECLARE_TR_FUNCTIONS(ui::common::connection_alert)

inline void showNotConnected(QWidget* parent) {
    QMessageBox::warning(parent,
                         QCoreApplication::translate("ConnectionAlert", "Not Connected"),
                         QCoreApplication::translate("ConnectionAlert", "Please connect first."));
}

inline void showDisconnected(QWidget* parent) {
    QMessageBox::warning(parent,
                         QCoreApplication::translate("ConnectionAlert", "Connection Lost"),
                         QCoreApplication::translate("ConnectionAlert", "Connection was closed."));
}

} // namespace connection_alert

} // namespace ui::common
