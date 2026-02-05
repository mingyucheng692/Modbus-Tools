#pragma once
#include <QMainWindow>
#include <QDockWidget>
#include "Widgets/FrameAnalyzer.h"

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
    QDockWidget* trafficDock_;
    
    FrameAnalyzer* analyzer_;
};
