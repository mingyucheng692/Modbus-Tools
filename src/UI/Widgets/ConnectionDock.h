#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QStackedWidget>
#include <vector>

class ControlWidget;
class GenericSenderWidget;

class ConnectionDock : public QWidget {
    Q_OBJECT
public:
    explicit ConnectionDock(QWidget* parent = nullptr);

signals:
    void connectTcp(const QString& ip, int port);
    void connectRtu(const QString& portName, int baudRate, int dataBits, int stopBits, int parity);
    void disconnectRequested();
    
    // Proxy signals
    void sendRequest(int slaveId, int funcCode, int startAddr, int count, const QString& dataHex);
    void sendRaw(const std::vector<uint8_t>& data);
    void pollToggled(bool enabled, int intervalMs);
    void simulationChanged(int dropRate, int minDelay, int maxDelay);
    void modeChanged(int mode); // 0: Modbus, 1: Generic

private slots:
    void onConnectClicked();
    void onSimApplyClicked();
    void onTypeChanged(int index);

private:
    QComboBox* typeCombo_;
    QPushButton* connectBtn_;
    
    // Senders
    QStackedWidget* senderStack_;
    ControlWidget* controlWidget_;
    GenericSenderWidget* genericSender_;
    
    // TCP UI
    QWidget* tcpWidget_;
    QLineEdit* ipEdit_;
    QSpinBox* portSpin_;
    
    // RTU UI
    QWidget* rtuWidget_;
    QComboBox* portCombo_;
    QComboBox* baudCombo_;
    QComboBox* dataCombo_;
    QComboBox* stopCombo_;
    QComboBox* parityCombo_;
    
    QStackedWidget* settingsStack_;
    
    // Simulation UI
    QSpinBox* simDropSpin_;
    QSpinBox* simMinDelaySpin_;
    QSpinBox* simMaxDelaySpin_;
    QPushButton* simApplyBtn_;
    
    bool isConnected_ = false;
};
