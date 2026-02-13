#pragma once

#include <QWidget>

class QVBoxLayout;
class QLabel;

namespace ui::widgets {
    class TcpConnectionWidget;
    class FunctionWidget;
    class TrafficMonitorWidget;
}

namespace ui::views::tcp {

class TcpView : public QWidget {
    Q_OBJECT

public:
    explicit TcpView(QWidget *parent = nullptr);
    ~TcpView() override;

private:
    void setupUi();

    QVBoxLayout* mainLayout_ = nullptr;
    ui::widgets::TcpConnectionWidget* connectionWidget_ = nullptr;
    ui::widgets::FunctionWidget* functionWidget_ = nullptr;
    ui::widgets::TrafficMonitorWidget* trafficMonitor_ = nullptr;
};

} // namespace ui::views::tcp
