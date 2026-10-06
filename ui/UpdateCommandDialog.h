// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QDialog>

class QLabel;
class QPlainTextEdit;
class QPushButton;

namespace ui {

// Role: Modal dialog providing copy-ready terminal command sequence for Linux application updates.
// Thread-safety: Main-thread only (QWidget derivative).
// Ownership: Owned by caller or parent QWidget.
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
