#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>
#include "ControlWidget.h"

class ConnectionDock : public QWidget {
    Q_OBJECT
public:
    explicit ConnectionDock(QWidget* parent = nullptr);

signals:
    void connectTcp(const QString& ip, int port);
    void disconnectRequested();
    
    // Proxy signals from ControlWidget
    void sendRequest(int slaveId, int funcCode, int startAddr, int count, const QString& dataHex);
    void pollToggled(bool enabled, int intervalMs);

private slots:
    void onConnectClicked();

private:
    QComboBox* typeCombo_;
    QLineEdit* ipEdit_;
    QSpinBox* portSpin_;
    QPushButton* connectBtn_;
    ControlWidget* controlWidget_;
    bool isConnected_ = false;
};
