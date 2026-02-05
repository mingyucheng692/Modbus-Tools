#pragma once
#include <QMainWindow>
#include <QDockWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

signals:
    void requestConnectTcp(const QString& ip, int port);
    void requestDisconnect();

private:
    void initUI();
    void createDocks();
    void createActions();
    void applyDarkTheme();

    QDockWidget* connectionDock_;
    QDockWidget* logDock_;
};
