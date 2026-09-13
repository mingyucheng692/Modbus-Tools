/**
 * @file UpdateCommandDialog.h
 * @brief Copy-ready terminal command dialog for the Linux update flow (方案 C).
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QDialog>

class QLabel;
class QPlainTextEdit;
class QPushButton;

namespace ui {

/// Shows the three-segment terminal command sequence (download / checksum /
/// in-place replace) with one-click copy, an "open download page" fallback
/// and a quit-first hint. Modal; read-only command view.
class UpdateCommandDialog : public QDialog {
    Q_OBJECT

public:
    explicit UpdateCommandDialog(const QString& latestVersion,
                                 const QString& command,
                                 const QString& releaseUrl,
                                 QWidget* parent = nullptr);

private:
    void copyCommands();
    void openDownloadPage();

    QPlainTextEdit* commandEdit_ = nullptr;
    QPushButton* copyButton_ = nullptr;
    QString releaseUrl_;
};

} // namespace ui
