#pragma once
#include <QMainWindow>
#include <QDockWidget>
#include "Widgets/ConnectionDock.h"
#include "Widgets/LogWidget.h"
#include "Widgets/TrafficWidget.h"
#include "Widgets/FrameAnalyzer.h"
#include <QLabel>
#include "Widgets/WaveformWidget.h"
#include "Core/Modbus/ModbusDefs.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    void connectWorker(class CoreWorker* worker);

signals:
    void requestConnectTcp(const QString& ip, int port);
    void requestDisconnect();
    void requestSend(int slaveId, int funcCode, int startAddr, int count, const QString& dataHex);
    void pollToggled(bool enabled, int intervalMs);
    void requestSimulation(int dropRate, int minDelay, int maxDelay);

private slots:
    void onWorkerReady();
    void onResponseReceived(uint16_t transactionId, uint8_t unitId, Modbus::FunctionCode fc, const std::vector<uint8_t>& data, int responseTimeMs, uint16_t startAddr);

private:
    void setupUi();
    void createDocks();
    void createActions();
    void applyDarkTheme();
    
    QDockWidget* connectionDock_;
    QDockWidget* logDock_;
    QDockWidget* trafficDock_;
    QDockWidget* analyzerDock_;
    QDockWidget* waveformDock_;
    
    LogWidget* logWidget_;
    TrafficWidget* trafficWidget_;
    FrameAnalyzer* frameAnalyzer_;
    WaveformWidget* waveformWidget_;
    
    QLabel* statusLabel_ = nullptr;
    QLabel* rttLabel_ = nullptr;
};
