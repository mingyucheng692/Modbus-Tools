#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>

class ConnectionDock : public QWidget {
    Q_OBJECT
public:
    explicit ConnectionDock(QWidget* parent = nullptr);

signals:
    void connectTcp(const QString& ip, int port);
    void disconnectRequested();

private slots:
    void onConnectClicked();

private:
    QComboBox* typeCombo_;
    QLineEdit* ipEdit_;
    QSpinBox* portSpin_;
    QPushButton* connectBtn_;
    bool isConnected_ = false;
};
