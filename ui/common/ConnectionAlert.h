// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <spdlog/spdlog.h>

class QWidget;

namespace ui::common {

namespace connection_alert {

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
                         QCoreApplication::translate("ui::common::connection_alert", "Not Connected"),
                         QCoreApplication::translate("ui::common::connection_alert", "Please connect first."));
}

inline void showDisconnected(QWidget* parent) {
    if (!guiAvailable()) {
        SPDLOG_WARN("ConnectionAlert: connection lost");
        return;
    }
    QMessageBox::warning(parent,
                         QCoreApplication::translate("ui::common::connection_alert", "Connection Lost"),
                         QCoreApplication::translate("ui::common::connection_alert", "Connection was closed."));
}

} // namespace connection_alert

} // namespace ui::common
