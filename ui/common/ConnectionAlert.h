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

inline QString tr(const char* sourceText, const char* disambiguation = nullptr, int n = -1)
{
    return QCoreApplication::translate("ui::common::connection_alert", sourceText, disambiguation, n);
}

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
