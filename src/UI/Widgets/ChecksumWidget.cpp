#include "ChecksumWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextBrowser>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QRegularExpression>

ChecksumWidget::ChecksumWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Input
    layout->addWidget(new QLabel("Hex Input:"));
    inputEdit_ = new QLineEdit(this);
    inputEdit_->setPlaceholderText("e.g. 01 03 00 00 00 01");
    layout->addWidget(inputEdit_);
    
    // Controls
    QHBoxLayout* btnLayout = new QHBoxLayout();
    calcBtn_ = new QPushButton("Calculate All", this);
    btnLayout->addWidget(calcBtn_);
    layout->addLayout(btnLayout);
    
    // Results
    layout->addWidget(new QLabel("Results:"));
    resultBrowser_ = new QTextBrowser(this);
    resultBrowser_->setFont(QFont("Consolas"));
    layout->addWidget(resultBrowser_);
    
    connect(calcBtn_, &QPushButton::clicked, this, &ChecksumWidget::onCalculate);
}

void ChecksumWidget::onCalculate() {
    auto data = parseInput();
    if (data.empty()) {
        resultBrowser_->setText("Invalid Input");
        return;
    }
    
    QString html;
    
    // Modbus CRC16
    uint16_t crcModbus = crc16Modbus(data);
    html += QString("<b>CRC16 (Modbus):</b> 0x%1 (Low: %2, High: %3)<br>")
            .arg(crcModbus, 4, 16, QChar('0')).toUpper()
            .arg(crcModbus & 0xFF, 2, 16, QChar('0')).toUpper()
            .arg(crcModbus >> 8, 2, 16, QChar('0')).toUpper();
            
    // CRC16 CCITT
    uint16_t crcCcitt = crc16Ccitt(data);
    html += QString("<b>CRC16 (CCITT):</b> 0x%1<br>").arg(crcCcitt, 4, 16, QChar('0')).toUpper();
    
    // XOR
    uint8_t xorVal = checksumXor(data);
    html += QString("<b>BCC (XOR):</b> 0x%1<br>").arg(xorVal, 2, 16, QChar('0')).toUpper();
    
    // Sum
    uint8_t sumVal = checksumSum(data);
    html += QString("<b>Checksum (Sum):</b> 0x%1<br>").arg(sumVal, 2, 16, QChar('0')).toUpper();
    
    resultBrowser_->setHtml(html);
}

std::vector<uint8_t> ChecksumWidget::parseInput() {
    std::vector<uint8_t> data;
    QString text = inputEdit_->text();
    QStringList parts = text.split(QRegularExpression("[\\s,]+"), Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        bool ok;
        uint8_t b = static_cast<uint8_t>(part.toUInt(&ok, 16));
        if (ok) data.push_back(b);
    }
    return data;
}

uint16_t ChecksumWidget::crc16Modbus(const std::vector<uint8_t>& data) {
    uint16_t crc = 0xFFFF;
    for (uint8_t b : data) {
        crc ^= b;
        for (int i = 0; i < 8; ++i) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = (crc >> 1);
            }
        }
    }
    return crc;
}

uint16_t ChecksumWidget::crc16Ccitt(const std::vector<uint8_t>& data) {
    uint16_t crc = 0xFFFF;
    for (uint8_t b : data) {
        crc  = (uint8_t)(crc >> 8) | (crc << 8);
        crc ^= b;
        crc ^= (uint8_t)(crc & 0xff) >> 4;
        crc ^= (crc << 8) << 4;
        crc ^= ((crc & 0xff) << 4) << 1;
    }
    return crc;
}

uint8_t ChecksumWidget::checksumXor(const std::vector<uint8_t>& data) {
    uint8_t xorVal = 0;
    for (uint8_t b : data) xorVal ^= b;
    return xorVal;
}

uint8_t ChecksumWidget::checksumSum(const std::vector<uint8_t>& data) {
    uint8_t sum = 0;
    for (uint8_t b : data) sum += b;
    return sum;
}
