#pragma once

#include <QMainWindow>

class QStackedWidget;
class QListWidget;

namespace ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void setupUi();
    void createNavigation();

    QListWidget* navigationList_ = nullptr;
    QStackedWidget* stackedWidget_ = nullptr;
};

} // namespace ui
