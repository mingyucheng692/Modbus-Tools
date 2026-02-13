#pragma once

#include <QWidget>

class QVBoxLayout;
class QLabel;

namespace ui::views::rtu {

class RtuView : public QWidget {
    Q_OBJECT

public:
    explicit RtuView(QWidget *parent = nullptr);
    ~RtuView() override;

private:
    void setupUi();

    QVBoxLayout* mainLayout_ = nullptr;
    QLabel* titleLabel_ = nullptr;
};

} // namespace ui::views::rtu
