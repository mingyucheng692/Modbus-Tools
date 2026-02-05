#pragma once
#include <QWidget>
#include <vector>
#include <cstdint>

class QLineEdit;
class QTextBrowser;
class QComboBox;
class QPushButton;

class ChecksumWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChecksumWidget(QWidget* parent = nullptr);

private slots:
    void onCalculate();

private:
    std::vector<uint8_t> parseInput();
    
    // Algos
    uint16_t crc16Modbus(const std::vector<uint8_t>& data);
    uint16_t crc16Ccitt(const std::vector<uint8_t>& data);
    uint8_t checksumXor(const std::vector<uint8_t>& data);
    uint8_t checksumSum(const std::vector<uint8_t>& data);

    QLineEdit* inputEdit_;
    QComboBox* typeCombo_;
    QPushButton* calcBtn_;
    QTextBrowser* resultBrowser_;
};
