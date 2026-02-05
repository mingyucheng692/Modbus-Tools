#pragma once
#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

class ControlWidget : public QWidget {
    Q_OBJECT
public:
    explicit ControlWidget(QWidget* parent = nullptr);

public slots:
    void setSlaveId(int id);

signals:
    void sendRequest(int slaveId, int funcCode, int startAddr, int count, const QString& dataHex);
    void pollToggled(bool enabled, int intervalMs);

private slots:
    void onSendClicked();
    void onPollToggled(bool checked);
    void updateUiState();

private:
    QSpinBox* slaveIdSpin_;
    QComboBox* funcCodeCombo_;
    QSpinBox* startAddrSpin_;
    QSpinBox* countSpin_;
    QLineEdit* dataInput_;
    QPushButton* sendBtn_;
    
    QCheckBox* pollCheck_;
    QSpinBox* pollIntervalSpin_;
};
