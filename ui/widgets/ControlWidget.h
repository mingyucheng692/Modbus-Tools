#pragma once

#include <QWidget>

class QCheckBox;
class QSpinBox;
class QComboBox;
class QLabel;
class QTimer;
class QEvent;
class QString;

namespace ui::common {
class ISettingsService;
}

namespace ui::widgets {

class ControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit ControlWidget(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~ControlWidget() override;

    void recordTx();
    void recordRx(int rttMs);
    void recordError();
    void resetStats();
    void setSettingsGroup(const QString& group);

signals:
    // Poll Requested: Function Code, Address, Quantity
    void pollRequested(uint8_t functionCode, int address, int quantity);

private:
    void setupUi();
    void onTimer();
    void updateStatsLabel();
    void loadSettings();
    void saveSettings();
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
    int lastRtt_ = 0;

    QString settingsGroup_;
    ui::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::widgets
