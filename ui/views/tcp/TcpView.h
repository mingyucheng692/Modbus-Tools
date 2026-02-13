#pragma once

#include <QWidget>

class QVBoxLayout;
class QLabel;

namespace ui::views::tcp {

class TcpView : public QWidget {
    Q_OBJECT

public:
    explicit TcpView(QWidget *parent = nullptr);
    ~TcpView() override;

private:
    void setupUi();

    QVBoxLayout* mainLayout_ = nullptr;
    QLabel* titleLabel_ = nullptr;
};

} // namespace ui::views::tcp
