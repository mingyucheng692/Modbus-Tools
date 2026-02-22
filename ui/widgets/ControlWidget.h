#pragma once

#include <QWidget>

class QCheckBox;
class QSpinBox;
class QComboBox;
class QLabel;
class QTimer;
class QEvent;

namespace ui::widgets {

class ControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit ControlWidget(QWidget *parent = nullptr);
    ~ControlWidget() override;

    void updateStats(bool isTx, int rttMs, bool isError = false);
    void resetStats();

signals:
    // Poll Requested: Function Code, Address, Quantity
    void pollRequested(uint8_t functionCode, int address, int quantity);

private:
    void setupUi();
    void onTimer();
    void updateStatsLabel();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    QLabel* intervalLabel_ = nullptr;
    QLabel* fcLabel_ = nullptr;
    QLabel* addrLabel_ = nullptr;
    QLabel* qtyLabel_ = nullptr;
    QCheckBox* enablePollCheck_ = nullptr;
    QSpinBox* intervalSpin_ = nullptr;
    QComboBox* fcCombo_ = nullptr;
    QSpinBox* addrSpin_ = nullptr;
    QSpinBox* qtySpin_ = nullptr;
    
    QLabel* statsLabel_ = nullptr;
    
    QTimer* pollTimer_ = nullptr;

    // Stats
    int txCount_ = 0;
    int rxCount_ = 0;
    int errorCount_ = 0;
    long long totalRtt_ = 0;
};

} // namespace ui::widgets
