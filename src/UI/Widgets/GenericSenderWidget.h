#pragma once
#include <QWidget>
#include <vector>

class QTextEdit;
class QCheckBox;
class QSpinBox;
class QPushButton;
class QTimer;
class QComboBox;

class GenericSenderWidget : public QWidget {
    Q_OBJECT
public:
    explicit GenericSenderWidget(QWidget* parent = nullptr);

signals:
    void sendRaw(const std::vector<uint8_t>& data);

private slots:
    void onSendClicked();
    void onAutoSendToggled(bool checked);
    void onTimerTimeout();

private:
    std::vector<uint8_t> parseInput();

    QTextEdit* inputEdit_;
    QComboBox* formatCombo_; // Hex / ASCII
    QCheckBox* autoSendCheck_;
    QSpinBox* intervalSpin_;
    QPushButton* sendBtn_;
    
    QTimer* autoTimer_;
};
