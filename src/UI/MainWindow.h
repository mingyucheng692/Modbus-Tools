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
    void requestSend(int slaveId, int funcCode, int startAddr, int count, const QString& dataHex);
    void pollToggled(bool enabled, int intervalMs);

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
