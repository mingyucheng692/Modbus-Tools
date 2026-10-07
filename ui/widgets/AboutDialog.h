// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QDialog>

class QLabel;
class QDialogButtonBox;

namespace ui::widgets {

// Role: Dialog displaying application metadata, build info, and repository links.
// Thread-safety: Single-threaded (bound to GUI thread).
// Ownership: Modal dialog owned by parent QWidget.
class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);

private:
    void setupUi();
};

} // namespace ui::widgets
