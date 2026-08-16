/**
 * @file ConnectionAlert.h
 * @brief Header file for ConnectionAlert.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <spdlog/spdlog.h>

class QWidget;

namespace ui::common {

namespace connection_alert {

inline QString tr(const char* sourceText, const char* disambiguation = nullptr, int n = -1)
{
    return QCoreApplication::translate("ui::common::connection_alert", sourceText, disambiguation, n);
}

/// True when a full GUI application instance exists. Unit tests run under a
/// bare QCoreApplication: QMessageBox requires QApplication and its modal
/// exec() would block automation, so callers degrade to a log there.
[[nodiscard]] inline bool guiAvailable() {
    return qobject_cast<QApplication*>(QCoreApplication::instance()) != nullptr;
}

inline void showNotConnected(QWidget* parent) {
    if (!guiAvailable()) {
        SPDLOG_WARN("ConnectionAlert: request rejected (not connected)");
        return;
    }
    QMessageBox::warning(parent,
                         QCoreApplication::translate("ConnectionAlert", "Not Connected"),
                         QCoreApplication::translate("ConnectionAlert", "Please connect first."));
}

inline void showDisconnected(QWidget* parent) {
    if (!guiAvailable()) {
        SPDLOG_WARN("ConnectionAlert: connection lost");
        return;
    }
    QMessageBox::warning(parent,
                         QCoreApplication::translate("ConnectionAlert", "Connection Lost"),
                         QCoreApplication::translate("ConnectionAlert", "Connection was closed."));
}

} // namespace connection_alert

} // namespace ui::common
