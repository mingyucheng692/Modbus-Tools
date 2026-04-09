#pragma once

#include <QDialog>

class QLabel;
class QDialogButtonBox;

namespace ui::widgets {

/**
 * @brief Dialog showing application information and links.
 */
class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);

private:
    void setupUi();
};

} // namespace ui::widgets
